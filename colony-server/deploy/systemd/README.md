# Debian service deployment for colony-server

This folder contains a minimal systemd deployment kit for running colony-server continuously.

## Files

- colony-server.service: unit template with secure defaults
- install-colony-service.sh: installer script that builds release binary and installs service

## Quick install on Debian

1. Clone or copy colony-server to a stable path, for example `/opt/oo/colony-server`.
2. Ensure Rust toolchain is installed (`rustup` + `cargo`).
3. Create runtime user once:

```bash
sudo useradd --system --create-home --shell /usr/sbin/nologin oo || true
```

4. Run installer:

```bash
cd /opt/oo/colony-server/deploy/systemd
sudo chmod +x install-colony-service.sh
sudo ./install-colony-service.sh \
  --project-dir /opt/oo/colony-server \
  --bind 0.0.0.0:8080 \
  --user oo
```

## Operations

```bash
sudo systemctl status colony-server.service
sudo journalctl -u colony-server.service -f
sudo systemctl restart colony-server.service
```

## Security and network notes

- Open port `8080/tcp` only for trusted OO hosts.
- Consider binding to a private subnet IP instead of `0.0.0.0`.
- Keep fossil data backed up regularly (`fossil/` directory).
