#!/usr/bin/env bash
set -euo pipefail

# Install colony-server as a systemd service on Debian/Ubuntu.
# Usage:
#   sudo ./install-colony-service.sh \
#     --project-dir /opt/oo/colony-server \
#     --bind 0.0.0.0:8080 \
#     --user oo

PROJECT_DIR="/opt/oo/colony-server"
BIND_ADDR="0.0.0.0:8080"
RUN_USER="oo"
RUN_GROUP="oo"
RUST_LOG_LEVEL="info"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --project-dir)
      PROJECT_DIR="$2"; shift 2 ;;
    --bind)
      BIND_ADDR="$2"; shift 2 ;;
    --user)
      RUN_USER="$2"; shift 2 ;;
    --group)
      RUN_GROUP="$2"; shift 2 ;;
    --rust-log)
      RUST_LOG_LEVEL="$2"; shift 2 ;;
    *)
      echo "Unknown argument: $1" >&2
      exit 1 ;;
  esac
done

if [[ $EUID -ne 0 ]]; then
  echo "Please run as root (sudo)." >&2
  exit 1
fi

if [[ ! -f "$PROJECT_DIR/Cargo.toml" ]]; then
  echo "Missing Cargo.toml in $PROJECT_DIR" >&2
  exit 1
fi

if ! id "$RUN_USER" >/dev/null 2>&1; then
  echo "User '$RUN_USER' does not exist. Create it first." >&2
  exit 1
fi

mkdir -p "$PROJECT_DIR/fossil"
chown -R "$RUN_USER:$RUN_GROUP" "$PROJECT_DIR/fossil"

echo "Building colony-server release binary..."
cd "$PROJECT_DIR"
cargo build --release

SERVICE_PATH="/etc/systemd/system/colony-server.service"
cat > "$SERVICE_PATH" <<EOF
[Unit]
Description=OO Colony Server (Distributed Organism Coordinator)
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
User=$RUN_USER
Group=$RUN_GROUP
WorkingDirectory=$PROJECT_DIR
Environment=RUST_LOG=$RUST_LOG_LEVEL
Environment=COLONY_BIND=$BIND_ADDR
ExecStart=$PROJECT_DIR/target/release/colony-server
Restart=always
RestartSec=3
NoNewPrivileges=true
PrivateTmp=true
ProtectSystem=full
ProtectHome=true
ReadWritePaths=$PROJECT_DIR/fossil

[Install]
WantedBy=multi-user.target
EOF

systemctl daemon-reload
systemctl enable colony-server.service
systemctl restart colony-server.service

echo "Service installed and restarted: colony-server.service"
echo "Check status: systemctl status colony-server.service"
echo "Logs: journalctl -u colony-server.service -f"
