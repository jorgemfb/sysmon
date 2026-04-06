# sysmon

Herramienta de monitorización modular en C para Linux.

## Estado actual

Este repositorio contiene una base coherente para empezar a trabajar con Codex sin tocar la estructura congelada del proyecto.

### Núcleo incluido
- core mínimo
- config mínima
- utilidades base
- scheduler básico
- registry básico
- snapshot básico
- app básica
- módulo vertical de ejemplo: identity

## Regla principal

La estructura del repositorio está congelada y no debe reorganizarse.

## Build rápido

```bash
make
./bin/sysmon
```

## Objetivo inmediato

Consolidar el núcleo y, a partir de ahí, implementar collectors y módulos reales sin rediseñar el proyecto.
