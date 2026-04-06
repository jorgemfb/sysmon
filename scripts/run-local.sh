#!/usr/bin/env bash
set -euo pipefail
make
./bin/sysmon
