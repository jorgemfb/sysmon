# Local Mode

Ejecuta la monitorización local y renderiza la UI de terminal.

## Comando

```bash
./bin/sysmon --mode local
```

También funciona como modo por defecto:

```bash
./bin/sysmon
```

## Qué hace

- Inicializa app, registry, scheduler y snapshot.
- Registra módulos por defecto.
- Ejecuta el ciclo de colección mínimo del runtime.
- Evalúa findings/alerts y genera reporte Markdown.
- Renderiza UI local con:
  - panel de findings
  - detalle del finding seleccionado
  - resumen de reporte
  - paneles de snapshot

## Notas

- Si hay TTY interactivo y alerts, la vista de findings permite navegación básica (`j/k`, flechas arriba/abajo, `q`).
- Si no hay TTY (por ejemplo `| sed` o redirección a archivo), la salida se mantiene no interactiva y estable.
