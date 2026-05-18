# colony-server

Minimal distributed coordinator for OO organisms.

This service is being extended toward a mesh-style colony node: local identity, peer registration, and topology snapshots instead of a single sovereign backend.

## Run locally

```bash
cargo run
```

By default, the server binds to `127.0.0.1:8080`.

You can override bind address with:

```bash
COLONY_BIND=0.0.0.0:8080 cargo run
```

Bootstrap known mesh peers at startup:

```bash
COLONY_BOOTSTRAP_PEERS="peer-a|10.0.0.2:8080|relay,peer-b|10.0.0.3:8080|sentinel" cargo run
```

Format is `peer_id|address|role` separated by commas.

Control automatic outgoing gossip frequency (default: 15 seconds):

```bash
COLONY_GOSSIP_INTERVAL_S=5 cargo run
```

Control gossip propagation depth in mesh hops (default: 3):

```bash
COLONY_GOSSIP_MAX_HOPS=2 cargo run
```

Control deduplication retention window for gossip IDs in seconds (default: 300):

```bash
COLONY_GOSSIP_DEDUP_WINDOW_S=120 cargo run
```

Enable authenticated gossip with a shared HMAC secret:

```bash
COLONY_GOSSIP_HMAC_KEY="replace-with-shared-secret" cargo run
```

Rotate keys without downtime by accepting previous secrets for verification:

```bash
COLONY_GOSSIP_HMAC_KEY="new-shared-secret" COLONY_GOSSIP_HMAC_PREV_KEYS="old-secret-1,old-secret-2" cargo run
```

## Endpoints

- `POST /heartbeat`
- `GET /status`
- `GET /mesh/status`
- `GET /mesh/metrics`
- `POST /mesh/peer`
- `POST /mesh/gossip`
- `GET /threats/latest`
- `GET /mutations/fitness`
- `GET /admin/dump`

## Mesh Behavior

- On startup, the node loads peers from `COLONY_BOOTSTRAP_PEERS`.
- In background, it sends `POST /mesh/gossip` to every known peer every `COLONY_GOSSIP_INTERVAL_S` seconds.
- Incoming gossip updates the peer registry and is appended to `fossil/mesh/gossip.ndjson`.
- Every gossip envelope carries `gossip_id` and `ttl_hops`.
- Nodes deduplicate by `gossip_id`, drop `ttl_hops=0`, and relay only while TTL remains positive.
- Dedup retention is controlled by `COLONY_GOSSIP_DEDUP_WINDOW_S`.
- Runtime counters are available on `GET /mesh/metrics` (inbound/outbound, duplicate, TTL drops, relay count).
- When `COLONY_GOSSIP_HMAC_KEY` is set, outgoing gossip is signed and incoming gossip must validate against `COLONY_GOSSIP_HMAC_KEY` or `COLONY_GOSSIP_HMAC_PREV_KEYS`.

## Debian systemd deployment

See [deploy/systemd/README.md](deploy/systemd/README.md).

Main files:

- [deploy/systemd/colony-server.service](deploy/systemd/colony-server.service)
- [deploy/systemd/install-colony-service.sh](deploy/systemd/install-colony-service.sh)
