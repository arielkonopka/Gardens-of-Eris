#!/bin/sh
# Runs the three arms of the four-way / eight-way A/B test, from the normal start and from the
# demo's far drop, then compares them. Usage, from the repository root after building goe-ab:
#   tools/ab/run.sh [build folder=build] [seeds=200] [cap=20000] [output folder=ab-results]
# Arms run in parallel, one process each (the world is a static, one per process). Games that
# crash are listed in errors.txt and left out of the comparison for their seed.
set -e
build=$(cd "${1:-build}" && pwd)
seeds=${2:-200}
cap=${3:-20000}
out=$(mkdir -p "${4:-ab-results}" && cd "${4:-ab-results}" && pwd)
here=$(cd "$(dirname "$0")" && pwd)
cd "$here/../../GoEoOL"
# one arm: if the process dies (a game bug that cannot be caught), the dead seed gets a crashed
# row and the arm goes on from the next seed
arm() {
    csv="$out/ab-$1${2:+-drop}.csv"
    first=1
    rm -f "$csv"
    while [ "$first" -le "$seeds" ]; do
        part="$csv.part"
        "$build/goe-ab" --arm "$1" --first "$first" --seeds $((seeds - first + 1)) --cap "$cap" $2 --out "$part" 2>>"$out/errors.txt" || true
        if [ -f "$csv" ]; then tail -n +2 "$part" >>"$csv"; else cp "$part" "$csv"; fi
        last=$(tail -n +2 "$csv" | tail -1 | cut -d, -f5)
        [ -n "$last" ] || last=$((first - 1))
        [ "$last" -ge "$seeds" ] && break
        dead=$((last + 1))
        echo "seed $dead arm $1${2:+ (drop)}: the process died" >>"$out/errors.txt"
        cols=$(head -1 "$csv" | tr ',' '\n' | wc -l)
        movement=${1%x*}; pilot=${1#*x}
        d=0; [ -n "$2" ] && d=1
        row="$1,$movement,$pilot,$d,$dead,1"
        i=7; while [ $i -le "$cols" ]; do row="$row,0"; i=$((i + 1)); done
        echo "$row" >>"$csv"
        first=$((dead + 1))
    done
    rm -f "$csv.part"
}
rm -f "$out/errors.txt"
for drop in "" "--drop"; do
    for a in 4x4 8x4 8x8; do
        arm $a $drop &
    done
done
wait
python3 "$here/analyze.py" "$out"/ab-*.csv --out "$out/report.md"
echo "report: $out/report.md"
