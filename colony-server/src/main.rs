mod models;
mod storage;

use axum::{
    extract::State,
    http::StatusCode,
    routing::{get, post},
    Json, Router,
};
use models::{Heartbeat, HeartbeatResponse, Directive};
use std::sync::{Arc, Mutex};
use storage::Storage;
use tracing_subscriber;

struct AppState {
    storage: Arc<Mutex<Storage>>,
}

#[tokio::main]
async fn main() {
    // Initialize tracing
    tracing_subscriber::fmt::init();

    // Initialize storage
    let storage = Storage::new(".")
        .expect("Failed to initialize storage");
    let state = Arc::new(AppState {
        storage: Arc::new(Mutex::new(storage)),
    });

    // Build router
    let app = Router::new()
        .route("/heartbeat", post(handle_heartbeat))
        .route("/status", get(handle_status))
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
    println!("   POST   /heartbeat       - OO organisms send heartbeats");
    println!("   GET    /status          - Colony aggregate status");
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
