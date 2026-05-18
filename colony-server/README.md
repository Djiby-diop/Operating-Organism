# colony-server

Minimal distributed coordinator for OO organisms.

## Run locally

```bash
cargo run
```

By default, the server binds to `127.0.0.1:8080`.

You can override bind address with:

```bash
COLONY_BIND=0.0.0.0:8080 cargo run
```

## Endpoints

- `POST /heartbeat`
- `GET /status`
- `GET /threats/latest`
- `GET /mutations/fitness`
- `GET /admin/dump`

## Debian systemd deployment

See [deploy/systemd/README.md](deploy/systemd/README.md).

Main files:

- [deploy/systemd/colony-server.service](deploy/systemd/colony-server.service)
- [deploy/systemd/install-colony-service.sh](deploy/systemd/install-colony-service.sh)
