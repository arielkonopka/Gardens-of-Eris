#!/bin/sh
# Runs the three arms of the four-way / eight-way A/B test, from the normal start and from the
# demo's far drop, then compares them. Usage, from the repository root after building goe-ab:
#   tools/ab/run.sh [build folder=build] [seeds=200] [cap=20000] [output folder=ab-results]
# Arms run in parallel, one process each (the world is a static, one per process).
set -e
build=$(cd "${1:-build}" && pwd)
seeds=${2:-200}
cap=${3:-20000}
out=$(mkdir -p "${4:-ab-results}" && cd "${4:-ab-results}" && pwd)
here=$(cd "$(dirname "$0")" && pwd)
cd "$here/../../GoEoOL"
for drop in "" "--drop"; do
    for arm in 4x4 8x4 8x8; do
        "$build/goe-ab" --arm $arm --seeds "$seeds" --cap "$cap" $drop --out "$out/ab-$arm${drop:+-drop}.csv" 2>/dev/null &
    done
done
wait
python3 "$here/analyze.py" "$out"/ab-*.csv --out "$out/report.md"
echo "report: $out/report.md"
