# sysmon

Herramienta de monitorización modular en C para Linux.

## Estado actual v1

Este repositorio contiene una implementación funcional con modos `local`, `server` y `client`, flujo modular de collectors/módulos y salida terminal legible.

### Incluye actualmente
- runtime base (`main -> app -> registry -> scheduler -> module -> collector -> snapshot`)
- collectors/módulos de sistema (identity, memory, cpu, disk, filesystem, network, process, service, temperature, smart, logs, package, trend, report)
- evaluación de alertas y reporte Markdown
- salida UI local y salida UI básica de cliente remoto

## Regla principal

La estructura del repositorio está congelada y no debe reorganizarse.

## Build y test

```bash
make
make test              # unit tests
make test-integration  # integration tests
```

## Uso rápido

```bash
# Ayuda CLI
./bin/sysmon --help

# Modo local (por defecto)
./bin/sysmon
./bin/sysmon --mode local

# Modo server
./bin/sysmon --mode server --bind 127.0.0.1 --port 19090

# Modo client
./bin/sysmon --mode client --host 127.0.0.1 --port 19090
```

## Documentación operativa

- `docs/user/local-mode.md`
- `docs/user/server-mode.md`
- `docs/user/client-mode.md`
- `docs/development/build.md`

## Objetivo inmediato (continuidad v1)

Endurecer robustez operativa y documentación sin rediseñar la arquitectura base.
