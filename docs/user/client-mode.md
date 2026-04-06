# Client Mode

Conecta a un server sysmon y muestra un resumen de reporte remoto en terminal.

## Comando

```bash
./bin/sysmon --mode client --host 127.0.0.1 --port 19090
```

## Qué hace

- Inicializa cliente de transporte.
- Conecta al server remoto.
- Recibe mensaje de reporte serializado.
- Deserializa y valida tipo de mensaje.
- Renderiza salida UI de cliente:
  - contexto remoto (`host:port`)
  - resumen de reporte
  - conteo de severidades de alertas (si se detectan en payload)
  - extracto del reporte

## Parámetros

- `--host`: dirección del servidor objetivo (por defecto `127.0.0.1`).
- `--port`: puerto TCP (1-65535, por defecto `19090`).

## Notas

- El cliente v1 prioriza salida legible y acotada, no dashboard remoto interactivo completo.
