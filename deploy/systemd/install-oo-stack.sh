#!/usr/bin/env bash
set -euo pipefail

# Install OO colony stack on Debian/Ubuntu in one command:
#   1) colony-server.service
#   2) oo-host-heartbeat-watch.service
#
# Usage:
#   sudo ./install-oo-stack.sh \
#     --colony-project-dir /opt/oo/colony-server \
#     --host-project-dir /opt/oo/oo-host \
#     --bind 0.0.0.0:8080 \
#     --user oo \
#     --create-user

COLONY_PROJECT_DIR="/opt/oo/colony-server"
HOST_PROJECT_DIR="/opt/oo/oo-host"
BIND_ADDR="0.0.0.0:8080"
HOST_COLONY_URL=""
RUN_USER="oo"
RUN_GROUP="oo"
RUST_LOG_LEVEL="info"
INTERVAL_S="15"
MAX_RETRIES="5"
BACKOFF_MS="500"
CREATE_USER="false"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --colony-project-dir)
      COLONY_PROJECT_DIR="$2"; shift 2 ;;
    --host-project-dir)
      HOST_PROJECT_DIR="$2"; shift 2 ;;
    --bind)
      BIND_ADDR="$2"; shift 2 ;;
    --host-colony-url)
      HOST_COLONY_URL="$2"; shift 2 ;;
    --user)
      RUN_USER="$2"; shift 2 ;;
    --group)
      RUN_GROUP="$2"; shift 2 ;;
    --rust-log)
      RUST_LOG_LEVEL="$2"; shift 2 ;;
    --interval-s)
      INTERVAL_S="$2"; shift 2 ;;
    --max-retries)
      MAX_RETRIES="$2"; shift 2 ;;
    --backoff-ms)
      BACKOFF_MS="$2"; shift 2 ;;
    --create-user)
      CREATE_USER="true"; shift 1 ;;
    *)
      echo "Unknown argument: $1" >&2
      exit 1 ;;
  esac
done

if [[ $EUID -ne 0 ]]; then
  echo "Please run as root (sudo)." >&2
  exit 1
fi

if [[ "$CREATE_USER" == "true" ]] && ! id "$RUN_USER" >/dev/null 2>&1; then
  echo "Creating system user: $RUN_USER"
  useradd --system --create-home --shell /usr/sbin/nologin "$RUN_USER"
fi

if ! id "$RUN_USER" >/dev/null 2>&1; then
  echo "User '$RUN_USER' does not exist. Re-run with --create-user or create it manually." >&2
  exit 1
fi

COLONY_INSTALLER="$COLONY_PROJECT_DIR/deploy/systemd/install-colony-service.sh"
HOST_INSTALLER="$HOST_PROJECT_DIR/deploy/systemd/install-heartbeat-service.sh"

if [[ ! -x "$COLONY_INSTALLER" ]]; then
  echo "Missing or not executable: $COLONY_INSTALLER" >&2
  echo "Run: chmod +x $COLONY_INSTALLER" >&2
  exit 1
fi

if [[ ! -x "$HOST_INSTALLER" ]]; then
  echo "Missing or not executable: $HOST_INSTALLER" >&2
  echo "Run: chmod +x $HOST_INSTALLER" >&2
  exit 1
fi

if [[ -z "$HOST_COLONY_URL" ]]; then
  # Derive host URL from bind port. For host-local deployment, loopback is the safest default.
  if [[ "$BIND_ADDR" == *:* ]]; then
    PORT="${BIND_ADDR##*:}"
  else
    PORT="8080"
  fi
  HOST_COLONY_URL="http://127.0.0.1:${PORT}"
fi

echo "Installing colony-server service..."
"$COLONY_INSTALLER" \
  --project-dir "$COLONY_PROJECT_DIR" \
  --bind "$BIND_ADDR" \
  --user "$RUN_USER" \
  --group "$RUN_GROUP" \
  --rust-log "$RUST_LOG_LEVEL"

echo "Installing oo-host heartbeat service..."
"$HOST_INSTALLER" \
  --project-dir "$HOST_PROJECT_DIR" \
  --colony "$HOST_COLONY_URL" \
  --user "$RUN_USER" \
  --group "$RUN_GROUP" \
  --interval-s "$INTERVAL_S" \
  --max-retries "$MAX_RETRIES" \
  --backoff-ms "$BACKOFF_MS"

echo
echo "OO stack installation complete."
echo "  colony-server bind : $BIND_ADDR"
echo "  host colony URL    : $HOST_COLONY_URL"
echo
echo "Status commands:"
echo "  systemctl status colony-server.service"
echo "  systemctl status oo-host-heartbeat-watch.service"
echo
echo "Live logs:"
echo "  journalctl -u colony-server.service -f"
echo "  journalctl -u oo-host-heartbeat-watch.service -f"
