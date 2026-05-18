# OO Debian bootstrap (full stack)

This folder provides a one-shot installer for the two runtime services:

- colony-server.service
- oo-host-heartbeat-watch.service

## Script

- install-oo-stack.sh

## Prerequisites

1. Debian/Ubuntu host with sudo access
2. Rust toolchain installed (`cargo` available)
3. Both repositories present:
   - `/opt/oo/colony-server`
   - `/opt/oo/oo-host`
4. Service installers executable:

```bash
sudo chmod +x /opt/oo/colony-server/deploy/systemd/install-colony-service.sh
sudo chmod +x /opt/oo/oo-host/deploy/systemd/install-heartbeat-service.sh
sudo chmod +x /opt/oo/deploy/systemd/install-oo-stack.sh
```

## One-shot install

```bash
cd /opt/oo/deploy/systemd
sudo ./install-oo-stack.sh \
  --colony-project-dir /opt/oo/colony-server \
  --host-project-dir /opt/oo/oo-host \
  --bind 0.0.0.0:8080 \
  --user oo \
  --create-user
```

Optional tuning:

- `--host-colony-url http://127.0.0.1:8080`
- `--interval-s 15`
- `--max-retries 5`
- `--backoff-ms 500`
- `--rust-log info`

## Verify

```bash
sudo systemctl status colony-server.service
sudo systemctl status oo-host-heartbeat-watch.service
sudo journalctl -u colony-server.service -f
sudo journalctl -u oo-host-heartbeat-watch.service -f
```
