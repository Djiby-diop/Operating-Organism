mod models;
mod storage;

use axum::{
    extract::State,
    http::StatusCode,
    routing::{get, post},
    Json, Router,
};
use chrono::Utc;
use hmac::{Hmac, Mac};
use models::{
    Directive,
    GossipAck,
    GossipEnvelope,
    Heartbeat,
    HeartbeatResponse,
    MeshState,
    PeerRegistration,
};
use reqwest::Client;
use sha2::Sha256;
use std::collections::HashMap;
use std::sync::atomic::{AtomicU64, Ordering};
use std::sync::{Arc, Mutex};
use storage::Storage;
use tokio::time::{self, Duration};
use tracing_subscriber;

type HmacSha256 = Hmac<Sha256>;

struct AppState {
    storage: Arc<Mutex<Storage>>,
    node_profile: Arc<models::NodeProfile>,
    seen_gossip: Arc<Mutex<HashMap<String, i64>>>,
    metrics: Arc<MeshMetrics>,
    runtime: Arc<MeshRuntimeConfig>,
    peer_failures: Arc<Mutex<HashMap<String, u32>>>,
}

struct MeshRuntimeConfig {
    dedup_window_s: i64,
    gossip_hmac_signing_key: Option<String>,
    gossip_hmac_verify_keys: Vec<String>,
}

struct MeshMetrics {
    inbound_total: AtomicU64,
    inbound_duplicate: AtomicU64,
    inbound_auth_failed: AtomicU64,
    inbound_ttl_dropped: AtomicU64,
    inbound_relayed: AtomicU64,
    outbound_attempted: AtomicU64,
    outbound_success: AtomicU64,
    outbound_failure: AtomicU64,
}

impl MeshMetrics {
    fn new() -> Self {
        Self {
            inbound_total: AtomicU64::new(0),
            inbound_duplicate: AtomicU64::new(0),
            inbound_auth_failed: AtomicU64::new(0),
            inbound_ttl_dropped: AtomicU64::new(0),
            inbound_relayed: AtomicU64::new(0),
            outbound_attempted: AtomicU64::new(0),
            outbound_success: AtomicU64::new(0),
            outbound_failure: AtomicU64::new(0),
        }
    }

    fn snapshot(&self) -> serde_json::Value {
        serde_json::json!({
            "inbound_total": self.inbound_total.load(Ordering::Relaxed),
            "inbound_duplicate": self.inbound_duplicate.load(Ordering::Relaxed),
            "inbound_auth_failed": self.inbound_auth_failed.load(Ordering::Relaxed),
            "inbound_ttl_dropped": self.inbound_ttl_dropped.load(Ordering::Relaxed),
            "inbound_relayed": self.inbound_relayed.load(Ordering::Relaxed),
            "outbound_attempted": self.outbound_attempted.load(Ordering::Relaxed),
            "outbound_success": self.outbound_success.load(Ordering::Relaxed),
            "outbound_failure": self.outbound_failure.load(Ordering::Relaxed),
        })
    }
}

#[tokio::main]
async fn main() {
    // Initialize tracing
    tracing_subscriber::fmt::init();

    // Initialize storage
    let data_dir = std::env::var("COLONY_DATA_DIR").unwrap_or_else(|_| ".".to_string());
    let storage = Storage::new(&data_dir)
        .expect("Failed to initialize storage");
    let node_profile = storage
        .get_or_create_node_profile(&std::env::var("COLONY_BIND").unwrap_or_else(|_| "127.0.0.1:8080".to_string()))
        .expect("Failed to initialize node profile");
    let bootstrap_peers = parse_bootstrap_peers();
    let bootstrap_applied = storage
        .register_bootstrap_peers(&bootstrap_peers, &node_profile.node_id)
        .expect("Failed to register bootstrap peers");
    let gossip_interval_s = parse_gossip_interval_s();
    let gossip_max_hops = parse_gossip_max_hops();
    let gossip_dedup_window_s = parse_gossip_dedup_window_s();
    let (gossip_hmac_signing_key, gossip_hmac_verify_keys) = parse_gossip_hmac_keys();
    let hmac_enabled = !gossip_hmac_verify_keys.is_empty();
    let seen_gossip = storage.load_seen_gossip(gossip_dedup_window_s).unwrap_or_default();
    let seen_count = seen_gossip.len();
    
    let state = Arc::new(AppState {
        storage: Arc::new(Mutex::new(storage)),
        node_profile: Arc::new(node_profile),
        seen_gossip: Arc::new(Mutex::new(seen_gossip)),
        metrics: Arc::new(MeshMetrics::new()),
        runtime: Arc::new(MeshRuntimeConfig {
            dedup_window_s: gossip_dedup_window_s,
            gossip_hmac_signing_key,
            gossip_hmac_verify_keys,
        }),
        peer_failures: Arc::new(Mutex::new(HashMap::new())),
    });
    let node_profile_snapshot = state.node_profile.clone();
    let gossip_state = state.clone();

    tokio::spawn(async move {
        run_gossip_loop(gossip_state, gossip_interval_s, gossip_max_hops, gossip_dedup_window_s).await;
    });

    // Build router
    let app = Router::new()
        .route("/heartbeat", post(handle_heartbeat))
        .route("/status", get(handle_status))
        .route("/mesh/status", get(handle_mesh_status))
        .route("/mesh/metrics", get(handle_mesh_metrics))
        .route("/mesh/peer", post(handle_mesh_peer))
        .route("/mesh/gossip", post(handle_mesh_gossip))
        .route("/threats/latest", get(handle_threats_latest))
        .route("/mutations/fitness", get(handle_mutations_fitness))
        .route("/admin/dump", get(handle_admin_dump))
        .with_state(state);

    // Start server
    let bind_addr = std::env::var("COLONY_BIND")
        .unwrap_or_else(|_| "127.0.0.1:8080".to_string());
    let listener = tokio::net::TcpListener::bind(&bind_addr)
        .await
        .expect("Failed to bind colony server socket");

    println!("🧬 Colony Server running on http://{}", bind_addr);
    println!("   Node identity     - {} [{}]", node_profile_snapshot.node_id, node_profile_snapshot.role);
    println!("   Bootstrap peers   - {} loaded from COLONY_BOOTSTRAP_PEERS", bootstrap_applied);
    println!("   Gossip interval   - {}s (COLONY_GOSSIP_INTERVAL_S)", gossip_interval_s);
    println!("   Gossip max hops   - {} (COLONY_GOSSIP_MAX_HOPS)", gossip_max_hops);
    println!("   Dedup window      - {}s (COLONY_GOSSIP_DEDUP_WINDOW_S)", gossip_dedup_window_s);
    println!("   Dedup cache       - loaded {} entries from disk", seen_count);
    println!(
        "   Gossip auth       - {} (COLONY_GOSSIP_HMAC_KEY + COLONY_GOSSIP_HMAC_PREV_KEYS)",
        if hmac_enabled { "enabled" } else { "disabled" }
    );
    println!("   POST   /heartbeat       - OO organisms send heartbeats");
    println!("   GET    /status          - Colony aggregate status");
    println!("   GET    /mesh/status     - Mesh topology snapshot");
    println!("   GET    /mesh/metrics    - Mesh gossip metrics");
    println!("   POST   /mesh/peer       - Register a peer node");
    println!("   POST   /mesh/gossip     - Mesh node gossip envelope");
    println!("   GET    /threats/latest  - Threat signatures");
    println!("   GET    /mutations/fitness - Viable mutations");
    println!("   GET    /admin/dump      - Full fossil record");

    axum::serve(listener, app)
        .await
        .expect("Server error");
}

/// POST /heartbeat - Receive heartbeat from OO organism
async fn handle_heartbeat(
    State(state): State<Arc<AppState>>,
    Json(heartbeat): Json<Heartbeat>,
) -> Result<Json<HeartbeatResponse>, StatusCode> {
    let storage = state.storage.lock().unwrap();

    match storage.store_heartbeat(&heartbeat) {
        Ok(()) => {
            // Build directives to send back
            let directives = vec![
                Directive {
                    kind: "status".to_string(),
                    content: serde_json::json!({
                        "received_at": chrono::Utc::now(),
                        "organism_id": heartbeat.organism_id,
                    }),
                },
            ];

            Ok(Json(HeartbeatResponse {
                status: "ok".to_string(),
                message: format!(
                    "Heartbeat from {} received and stored",
                    heartbeat.organism_id
                ),
                directives,
            }))
        }
        Err(e) => {
            eprintln!("Failed to store heartbeat: {}", e);
            Err(StatusCode::INTERNAL_SERVER_ERROR)
        }
    }
}

/// GET /status - Colony aggregate status
async fn handle_status(
    State(state): State<Arc<AppState>>,
) -> Result<Json<serde_json::Value>, StatusCode> {
    let storage = state.storage.lock().unwrap();

    match storage.get_colony_status() {
        Ok(status) => Ok(Json(serde_json::to_value(status)
            .unwrap_or_else(|_| serde_json::json!({"error": "serialization failed"})))),
        Err(e) => {
            eprintln!("Failed to get colony status: {}", e);
            Err(StatusCode::INTERNAL_SERVER_ERROR)
        }
    }
}

/// GET /mesh/status - Mesh topology snapshot
async fn handle_mesh_status(
    State(state): State<Arc<AppState>>,
) -> Result<Json<serde_json::Value>, StatusCode> {
    let storage = state.storage.lock().unwrap();

    match storage.get_mesh_state(state.node_profile.as_ref()) {
        Ok(mesh_state) => Ok(Json(serde_json::to_value(mesh_state)
            .unwrap_or_else(|_| serde_json::json!({"error": "serialization failed"})))),
        Err(e) => {
            eprintln!("Failed to get mesh status: {}", e);
            Err(StatusCode::INTERNAL_SERVER_ERROR)
        }
    }
}

/// GET /mesh/metrics - Mesh gossip counters and dedup cache size
async fn handle_mesh_metrics(
    State(state): State<Arc<AppState>>,
) -> Result<Json<serde_json::Value>, StatusCode> {
    let seen_cache_size = {
        let seen = state.seen_gossip.lock().unwrap();
        seen.len()
    };

    Ok(Json(serde_json::json!({
        "local_node_id": state.node_profile.node_id.clone(),
        "seen_cache_size": seen_cache_size,
        "counters": state.metrics.snapshot(),
        "generated_at": Utc::now(),
    })))
}

/// POST /mesh/peer - Register a peer node
async fn handle_mesh_peer(
    State(state): State<Arc<AppState>>,
    Json(peer): Json<PeerRegistration>,
) -> Result<Json<serde_json::Value>, StatusCode> {
    let storage = state.storage.lock().unwrap();

    match storage.register_peer(&peer) {
        Ok(record) => Ok(Json(serde_json::json!({
            "status": "ok",
            "local_node": state.node_profile.as_ref(),
            "peer": record,
            "mesh": storage.get_mesh_state(state.node_profile.as_ref()).unwrap_or_else(|_| MeshState {
                local_node: state.node_profile.as_ref().clone(),
                peer_count: 0,
                peers: vec![],
                colony_status: storage.get_colony_status().unwrap(),
            }),
        }))),
        Err(e) => {
            eprintln!("Failed to register peer: {}", e);
            Err(StatusCode::INTERNAL_SERVER_ERROR)
        }
    }
}

/// POST /mesh/gossip - Receive gossip from a peer node
async fn handle_mesh_gossip(
    State(state): State<Arc<AppState>>,
    Json(gossip): Json<GossipEnvelope>,
) -> Result<Json<serde_json::Value>, StatusCode> {
    state.metrics.inbound_total.fetch_add(1, Ordering::Relaxed);

    // Replay protection: check age of sent_at
    let now = Utc::now();
    let max_age_s = 300; // 5 minutes
    if (now - gossip.sent_at).num_seconds() > max_age_s {
        let known_peer_count = {
            let storage = state.storage.lock().unwrap();
            storage.get_mesh_registry().map(|r| r.peers.len()).unwrap_or(0)
        };

        let ack = GossipAck {
            status: "stale_replay".to_string(),
            local_node_id: state.node_profile.node_id.clone(),
            gossip_id: gossip.gossip_id,
            duplicate: false,
            relayed: false,
            ttl_hops_remaining: gossip.ttl_hops,
            known_peer_count,
            received_at: now,
        };

        return Ok(Json(serde_json::json!({ "ack": ack })));
    }

    if !state.runtime.gossip_hmac_verify_keys.is_empty() {
        if !verify_gossip_signature_with_any_key(&gossip, &state.runtime.gossip_hmac_verify_keys) {
            state
                .metrics
                .inbound_auth_failed
                .fetch_add(1, Ordering::Relaxed);

            let known_peer_count = {
                let storage = state.storage.lock().unwrap();
                storage.get_mesh_registry().map(|r| r.peers.len()).unwrap_or(0)
            };

            let ack = GossipAck {
                status: "auth_failed".to_string(),
                local_node_id: state.node_profile.node_id.clone(),
                gossip_id: gossip.gossip_id,
                duplicate: false,
                relayed: false,
                ttl_hops_remaining: gossip.ttl_hops,
                known_peer_count,
                received_at: Utc::now(),
            };

            return Ok(Json(serde_json::json!({ "ack": ack })));
        }
    }

    if gossip.ttl_hops == 0 {
        state
            .metrics
            .inbound_ttl_dropped
            .fetch_add(1, Ordering::Relaxed);

        let known_peer_count = {
            let storage = state.storage.lock().unwrap();
            storage.get_mesh_registry().map(|r| r.peers.len()).unwrap_or(0)
        };

        let ack = GossipAck {
            status: "dropped_ttl".to_string(),
            local_node_id: state.node_profile.node_id.clone(),
            gossip_id: gossip.gossip_id,
            duplicate: false,
            relayed: false,
            ttl_hops_remaining: 0,
            known_peer_count,
            received_at: Utc::now(),
        };

        return Ok(Json(serde_json::json!({ "ack": ack })));
    }

    let seen_now = mark_gossip_seen(
        &state.seen_gossip,
        &gossip.gossip_id,
        state.runtime.dedup_window_s,
    );
    if !seen_now {
        state
            .metrics
            .inbound_duplicate
            .fetch_add(1, Ordering::Relaxed);

        let known_peer_count = {
            let storage = state.storage.lock().unwrap();
            storage.get_mesh_registry().map(|r| r.peers.len()).unwrap_or(0)
        };

        let ack = GossipAck {
            status: "duplicate".to_string(),
            local_node_id: state.node_profile.node_id.clone(),
            gossip_id: gossip.gossip_id,
            duplicate: true,
            relayed: false,
            ttl_hops_remaining: gossip.ttl_hops,
            known_peer_count,
            received_at: Utc::now(),
        };

        return Ok(Json(serde_json::json!({ "ack": ack })));
    }

    let sender_peer_id = gossip.from.peer_id.clone();
    let known_peer_count = {
        let storage = state.storage.lock().unwrap();

        if let Err(e) = storage.store_gossip(&gossip) {
            eprintln!("Failed to process mesh gossip: {}", e);
            return Err(StatusCode::INTERNAL_SERVER_ERROR);
        }

        storage.get_mesh_registry().map(|r| r.peers.len()).unwrap_or(0)
    };

    let mut relayed = false;
    let mut ttl_hops_remaining = gossip.ttl_hops;

    if gossip.ttl_hops > 1 {
        relayed = true;
        ttl_hops_remaining = gossip.ttl_hops - 1;
        state
            .metrics
            .inbound_relayed
            .fetch_add(1, Ordering::Relaxed);

        let relay_state = state.clone();
        let mut relayed_gossip = gossip.clone();
        relayed_gossip.ttl_hops = ttl_hops_remaining;

        tokio::spawn(async move {
            relay_gossip(relay_state, relayed_gossip, Some(sender_peer_id)).await;
        });
    }

    let ack = GossipAck {
        status: "ok".to_string(),
        local_node_id: state.node_profile.node_id.clone(),
        gossip_id: gossip.gossip_id,
        duplicate: false,
        relayed,
        ttl_hops_remaining,
        known_peer_count,
        received_at: Utc::now(),
    };

    Ok(Json(serde_json::json!({ "ack": ack })))
}

fn parse_bootstrap_peers() -> Vec<PeerRegistration> {
    // Format: "peer-a|10.0.0.2:8080|relay,peer-b|10.0.0.3:8080|sentinel"
    let raw = std::env::var("COLONY_BOOTSTRAP_PEERS").unwrap_or_default();
    if raw.trim().is_empty() {
        return vec![];
    }

    raw.split(',')
        .filter_map(|entry| {
            let parts: Vec<_> = entry.split('|').map(str::trim).collect();
            if parts.len() != 3 {
                return None;
            }

            if parts.iter().any(|p| p.is_empty()) {
                return None;
            }

            Some(PeerRegistration {
                peer_id: parts[0].to_string(),
                address: parts[1].to_string(),
                role: parts[2].to_string(),
            })
        })
        .collect()
}

fn parse_gossip_interval_s() -> u64 {
    std::env::var("COLONY_GOSSIP_INTERVAL_S")
        .ok()
        .and_then(|v| v.parse::<u64>().ok())
        .filter(|v| *v > 0)
        .unwrap_or(15)
}

fn parse_gossip_max_hops() -> u8 {
    std::env::var("COLONY_GOSSIP_MAX_HOPS")
        .ok()
        .and_then(|v| v.parse::<u8>().ok())
        .filter(|v| *v > 0)
        .unwrap_or(3)
}

fn parse_gossip_dedup_window_s() -> i64 {
    std::env::var("COLONY_GOSSIP_DEDUP_WINDOW_S")
        .ok()
        .and_then(|v| v.parse::<i64>().ok())
        .filter(|v| *v > 0)
        .unwrap_or(300)
}

fn parse_gossip_hmac_keys() -> (Option<String>, Vec<String>) {
    let signing_key = std::env::var("COLONY_GOSSIP_HMAC_KEY")
        .ok()
        .map(|v| v.trim().to_string())
        .filter(|v| !v.is_empty());

    let mut verify_keys = Vec::new();

    if let Some(active) = signing_key.as_ref() {
        verify_keys.push(active.clone());
    }

    if let Ok(raw_prev_keys) = std::env::var("COLONY_GOSSIP_HMAC_PREV_KEYS") {
        for key in raw_prev_keys.split(',').map(str::trim).filter(|k| !k.is_empty()) {
            if !verify_keys.iter().any(|existing| existing == key) {
                verify_keys.push(key.to_string());
            }
        }
    }

    (signing_key, verify_keys)
}

fn gossip_signing_payload(envelope: &GossipEnvelope) -> String {
    let threats = envelope.observed_threats.join(",");
    format!(
        "{}|{}|{}|{}|{}|{}|{}|{}",
        envelope.gossip_id,
        envelope.ttl_hops,
        envelope.from.peer_id,
        envelope.from.address,
        envelope.from.role,
        envelope.observed_organisms,
        envelope.sent_at.timestamp_millis(),
        threats,
    )
}

fn compute_gossip_hmac(envelope: &GossipEnvelope, key: &str) -> Option<String> {
    let payload = gossip_signing_payload(envelope);
    let mut mac = HmacSha256::new_from_slice(key.as_bytes()).ok()?;
    mac.update(payload.as_bytes());
    Some(hex::encode(mac.finalize().into_bytes()))
}

fn sign_gossip_envelope(envelope: &mut GossipEnvelope, key: &str) {
    envelope.hmac_sha256 = compute_gossip_hmac(envelope, key);
}

fn constant_time_eq(a: &[u8], b: &[u8]) -> bool {
    if a.len() != b.len() {
        return false;
    }
    let mut res = 0;
    for (x, y) in a.iter().zip(b.iter()) {
        res |= x ^ y;
    }
    res == 0
}

fn verify_gossip_signature(envelope: &GossipEnvelope, key: &str) -> bool {
    let provided = match envelope.hmac_sha256.as_deref() {
        Some(v) if !v.is_empty() => v,
        _ => return false,
    };

    let expected = match compute_gossip_hmac(envelope, key) {
        Some(v) => v,
        None => return false,
    };

    constant_time_eq(expected.as_bytes(), provided.as_bytes())
}

fn verify_gossip_signature_with_any_key(envelope: &GossipEnvelope, keys: &[String]) -> bool {
    keys.iter()
        .any(|key| verify_gossip_signature(envelope, key))
}

fn mark_gossip_seen(seen: &Arc<Mutex<HashMap<String, i64>>>, gossip_id: &str, dedup_window_s: i64) -> bool {
    let mut guard = seen.lock().unwrap();
    let now = Utc::now().timestamp();

    guard.retain(|_, seen_at| now - *seen_at <= dedup_window_s);

    if guard.contains_key(gossip_id) {
        return false;
    }

    // Keep memory bounded in long-running mesh nodes.
    if guard.len() >= 10_000 {
        guard.retain(|_, seen_at| now - *seen_at <= dedup_window_s / 2);

        if guard.len() >= 10_000 {
            guard.clear();
        }
    }

    guard.insert(gossip_id.to_string(), now);
    true
}

fn mesh_gossip_url(address: &str) -> String {
    let base = address.trim().trim_end_matches('/');

    if base.starts_with("http://") || base.starts_with("https://") {
        format!("{}/mesh/gossip", base)
    } else {
        format!("http://{}/mesh/gossip", base)
    }
}

async fn run_gossip_loop(state: Arc<AppState>, interval_s: u64, max_hops: u8, dedup_window_s: i64) {
    let client = Client::new();
    let mut ticker = time::interval(Duration::from_secs(interval_s.max(1)));

    loop {
        ticker.tick().await;

        let snapshot = {
            let storage = state.storage.lock().unwrap();

            let registry = match storage.get_mesh_registry() {
                Ok(r) => r,
                Err(e) => {
                    eprintln!("Gossip loop: failed to read mesh registry: {}", e);
                    continue;
                }
            };

            let status = match storage.get_colony_status() {
                Ok(s) => s,
                Err(e) => {
                    eprintln!("Gossip loop: failed to read colony status: {}", e);
                    continue;
                }
            };

            (
                registry.peers.values().cloned().collect::<Vec<_>>(),
                status.critical_threats,
                status.alive_organisms,
            )
        };

        let (peers, observed_threats, observed_organisms) = snapshot;
        if peers.is_empty() {
            continue;
        }

        let mut envelope = GossipEnvelope {
            gossip_id: uuid::Uuid::new_v4().to_string(),
            ttl_hops: max_hops,
            from: PeerRegistration {
                peer_id: state.node_profile.node_id.clone(),
                address: state.node_profile.bind_addr.clone(),
                role: state.node_profile.role.clone(),
            },
            observed_threats,
            observed_organisms,
            sent_at: Utc::now(),
            hmac_sha256: None,
        };

        if let Some(key) = state.runtime.gossip_hmac_signing_key.as_deref() {
            sign_gossip_envelope(&mut envelope, key);
        }

        mark_gossip_seen(&state.seen_gossip, &envelope.gossip_id, dedup_window_s);
        send_gossip_to_peers(&client, state.clone(), envelope, None).await;
    }
}

async fn relay_gossip(state: Arc<AppState>, envelope: GossipEnvelope, exclude_peer_id: Option<String>) {
    let client = Client::new();
    send_gossip_to_peers(&client, state, envelope, exclude_peer_id).await;
}

async fn send_gossip_to_peers(
    client: &Client,
    state: Arc<AppState>,
    envelope: GossipEnvelope,
    exclude_peer_id: Option<String>,
) {
    let peers = {
        let storage = state.storage.lock().unwrap();
        match storage.get_mesh_registry() {
            Ok(registry) => registry.peers.values().cloned().collect::<Vec<_>>(),
            Err(e) => {
                eprintln!("Gossip send: failed to read mesh registry: {}", e);
                return;
            }
        }
    };

    for peer in peers {
        if peer.peer_id == state.node_profile.node_id {
            continue;
        }

        if let Some(excluded) = &exclude_peer_id {
            if &peer.peer_id == excluded {
                continue;
            }
        }        let target = mesh_gossip_url(&peer.address);
        let mut outbound = envelope.clone();
        if let Some(key) = state.runtime.gossip_hmac_signing_key.as_deref() {
            sign_gossip_envelope(&mut outbound, key);
        }

        // Circuit breaker check
        {
            let failures = state.peer_failures.lock().unwrap();
            if let Some(&count) = failures.get(&peer.peer_id) {
                if count >= 5 {
                    eprintln!("Circuit breaker OPEN for peer {} (skipping)", peer.peer_id);
                    continue;
                }
            }
        }

        let mut attempts = 0;
        let max_attempts = 3;
        let mut delay = std::time::Duration::from_secs(1);
        let mut success = false;

        while attempts < max_attempts {
            state.metrics.outbound_attempted.fetch_add(1, Ordering::Relaxed);
            let send_result = client.post(&target)
                .json(&outbound)
                .timeout(std::time::Duration::from_secs(5)) // Explicit timeout
                .send()
                .await;

            match send_result {
                Ok(response) if response.status().is_success() => {
                    state.metrics.outbound_success.fetch_add(1, Ordering::Relaxed);
                    success = true;
                    break;
                }
                Ok(response) => {
                    eprintln!("Gossip send: peer {} responded with status {} (attempt {})", peer.peer_id, response.status(), attempts + 1);
                }
                Err(e) => {
                    eprintln!("Gossip send: failed to send to peer {} (attempt {}): {}", peer.peer_id, attempts + 1, e);
                }
            }

            attempts += 1;
            if attempts < max_attempts {
                tokio::time::sleep(delay).await;
                delay *= 2; // Exponential backoff
            }
        }

        if !success {
            state.metrics.outbound_failure.fetch_add(1, Ordering::Relaxed);
            eprintln!("Gossip send: failed to send to peer {} after {} attempts", peer.peer_id, max_attempts);
            
            // Increment failure count for circuit breaker
            let mut failures = state.peer_failures.lock().unwrap();
            let count = failures.entry(peer.peer_id.clone()).or_insert(0);
            *count += 1;
        } else {
            // Reset failure count on success
            let mut failures = state.peer_failures.lock().unwrap();
            failures.remove(&peer.peer_id);
        }
    }
}

/// GET /threats/latest - Latest threat signatures
async fn handle_threats_latest(
    State(state): State<Arc<AppState>>,
) -> Result<Json<serde_json::Value>, StatusCode> {
    let storage = state.storage.lock().unwrap();

    match storage.get_threat_registry() {
        Ok(registry) => {
            let critical: Vec<_> = registry
                .threats
                .values()
                .filter(|t| t.severity == "critical")
                .collect();

            Ok(Json(serde_json::json!({
                "total_threats": registry.threats.len(),
                "critical_count": critical.len(),
                "critical_threats": critical,
            })))
        }
        Err(e) => {
            eprintln!("Failed to get threats: {}", e);
            Err(StatusCode::INTERNAL_SERVER_ERROR)
        }
    }
}

/// GET /mutations/fitness - Top viable mutations
async fn handle_mutations_fitness(
    State(_state): State<Arc<AppState>>,
) -> Result<Json<serde_json::Value>, StatusCode> {
    Ok(Json(serde_json::json!({
        "status": "coming_soon",
        "message": "Mutation fitness ranking not yet implemented",
    })))
}

/// GET /admin/dump - Full fossil record
async fn handle_admin_dump(
    State(state): State<Arc<AppState>>,
) -> Result<Json<serde_json::Value>, StatusCode> {
    let storage = state.storage.lock().unwrap();

    match (
        storage.get_organisms(),
        storage.get_threat_registry(),
        storage.get_colony_status(),
    ) {
        (Ok(organisms), Ok(threats), Ok(status)) => {
            Ok(Json(serde_json::json!({
                "organisms": organisms,
                "threats": threats,
                "status": status,
            })))
        }
        (Err(e), _, _) => {
            eprintln!("Failed to get dump: {}", e);
            Err(StatusCode::INTERNAL_SERVER_ERROR)
        }
        (_, Err(e), _) => {
            eprintln!("Failed to get dump: {}", e);
            Err(StatusCode::INTERNAL_SERVER_ERROR)
        }
        (_, _, Err(e)) => {
            eprintln!("Failed to get dump: {}", e);
            Err(StatusCode::INTERNAL_SERVER_ERROR)
        }
    }
}
