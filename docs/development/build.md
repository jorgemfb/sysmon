# Build

## Build

```bash
make clean && make
```

Genera el binario:

`bin/sysmon`

## Tests

```bash
# Unit tests
make test

# Integration tests
make test-integration

# Full suite
make test-all
```

Nota: `make test-integration` usa loopback TCP (`127.0.0.1`). En entornos sandbox puede requerir permisos de red/local socket.

## CLI Help

```bash
./bin/sysmon --help
```

## Smoke manual

```bash
# Local
./bin/sysmon --mode local

# Server (terminal 1)
./bin/sysmon --mode server --bind 127.0.0.1 --port 19090

# Client (terminal 2)
./bin/sysmon --mode client --host 127.0.0.1 --port 19090
```
