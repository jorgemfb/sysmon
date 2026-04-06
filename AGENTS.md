# AGENTS.md

## Reglas obligatorias

1. No cambiar la estructura de carpetas.
2. Headers públicos solo en `include/sysmon/...`.
3. Implementaciones solo en `src/...`.
4. No renombrar rutas ni archivos salvo error crítico real.
5. No meter dependencias externas innecesarias.
6. No implementar UI, transport o reporting completo mientras el core no esté consolidado.
7. Prioridad actual: `core`, `config`, `util`, `collectors/identity`, `modules/identity`.
8. Mantener C simple, portable y legible.
9. Entregar cambios pequeños, coherentes y compilables.
10. No introducir “nuevas arquitecturas” creativas.

## Estado actual esperado

- build funcional
- binario principal compilable
- módulo `identity` funcional como ejemplo vertical
- tests unitarios mínimos del core
- resto del árbol presente como base congelada o placeholder

## Prioridades

1. Core
2. Config
3. Util
4. Identity
5. Tests
6. Collectors reales adicionales
7. Alerting
8. Reporting
9. Transport
10. UI
