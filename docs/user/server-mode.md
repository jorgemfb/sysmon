# Server Mode

Ejecuta monitorización local y expone un envío mínimo de reporte por transporte TCP.

## Comando

```bash
./bin/sysmon --mode server --bind 127.0.0.1 --port 19090
```

## Qué hace

- Ejecuta el runtime local (colección + findings + reporte).
- Inicializa servidor de transporte.
- Acepta una conexión cliente.
- Envía un único mensaje de reporte serializado.
- Cierra sockets de forma limpia.

## Parámetros

- `--bind`: dirección de bind (por defecto `127.0.0.1`).
- `--port`: puerto TCP (1-65535, por defecto `19090`).

## Notas

- Esta implementación v1 usa flujo simple de una conexión / un mensaje.
- No implementa reconexión, TLS ni orquestación avanzada.
