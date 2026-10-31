#!/usr/bin/env python3
"""Compares the arms of a goe-ab run (four-way against eight-way movement).

Usage: analyze.py RESULTS.csv [MORE.csv ...] [--boot 10000] [--out report.md]

Reads the rows goe-ab writes (one per game), groups them by start (normal or demo drop) and arm,
and prints a Markdown report: means and medians per arm, the paired differences 8x4 - 4x4 (what
diagonal movement gives the enemies) and 8x8 - 8x4 (what it gives the player) with bootstrap 95%
confidence intervals paired by seed, and the energy lost and avatars lost by cause.
Only numpy is needed.
"""
import argparse
import csv
import sys

import numpy as np

METRICS = [
    ("score", "score"),
    ("ticks", "ticks survived"),
    ("avatars", "avatars used"),
    ("apples", "golden apples"),
    ("doors", "doors opened"),
    ("keys", "keys picked up"),
    ("kills", "monsters and drones killed"),
    ("damage_total", "energy lost"),
    ("cells", "cells visited"),
    ("chunks", "chunks reached"),
]
COMPARISONS = [("8x4", "4x4", "what diagonal movement gives the enemies"),
               ("8x8", "8x4", "what it gives the player"),
               ("8x8", "4x4", "both together")]


def load(paths):
    rows = []
    for p in paths:
        with open(p, newline="") as f:
            rows.extend(csv.DictReader(f))
    causes = [k[len("damage_"):] for k in rows[0] if k.startswith("damage_")] if rows else []
    for r in rows:
        r["damage_total"] = sum(int(r["damage_" + c]) for c in causes)
    return rows, causes


def by_seed(rows, start, arm, key):
    return {int(r["seed"]): float(r[key]) for r in rows if r["drop"] == start and r["arm"] == arm}


def paired_ci(a, b, stat, boots, rng):
    """the statistic of a - b over the seeds both have, and its bootstrap 95% interval"""
    seeds = sorted(set(a) & set(b))
    d = np.array([a[s] - b[s] for s in seeds])
    if len(d) == 0:
        return float("nan"), float("nan"), float("nan"), 0
    idx = rng.integers(0, len(d), size=(boots, len(d)))
    samples = stat(d[idx], axis=1)
    lo, hi = np.percentile(samples, [2.5, 97.5])
    return float(stat(d)), float(lo), float(hi), len(d)


def fmt(x):
    return f"{x:.1f}" if abs(x) < 1000 else f"{x:.0f}"


def report(rows, causes, boots, out):
    rng = np.random.default_rng(23)
    starts = sorted({r["drop"] for r in rows})
    arms = [a for a in ("4x4", "8x4", "8x8") if any(r["arm"] == a for r in rows)]
    w = out.write
    for start in starts:
        n = len({r["seed"] for r in rows if r["drop"] == start})
        w(f"## Start: {'demo drop (far out)' if start == '1' else 'normal start'} ({n} seeds)\n\n")
        w("Means (medians in brackets) per arm. An arm is movement x autopilot ways.\n\n")
        w("| metric | " + " | ".join(arms) + " |\n|---|" + "---|" * len(arms) + "\n")
        for key, name in METRICS:
            cells = []
            for arm in arms:
                v = np.array(list(by_seed(rows, start, arm, key).values()))
                cells.append(f"{fmt(v.mean())} ({fmt(float(np.median(v)))})" if len(v) else "-")
            w(f"| {name} | " + " | ".join(cells) + " |\n")
        capped = [sum(int(r["capped"]) for r in rows if r["drop"] == start and r["arm"] == a) for a in arms]
        w("| games that reached the tick cap | " + " | ".join(str(c) for c in capped) + " |\n\n")

        for a, b, what in COMPARISONS:
            if a not in arms or b not in arms:
                continue
            w(f"**{a} - {b}** ({what}): paired by seed, bootstrap 95% CI ({boots} resamples)\n\n")
            w("| metric | mean difference [95% CI] | median difference [95% CI] |\n|---|---|---|\n")
            for key, name in METRICS:
                x, y = by_seed(rows, start, a, key), by_seed(rows, start, b, key)
                m, mlo, mhi, _ = paired_ci(x, y, np.mean, boots, rng)
                md, dlo, dhi, _ = paired_ci(x, y, np.median, boots, rng)
                mark = " *" if mlo > 0 or mhi < 0 else ""
                w(f"| {name} | {fmt(m)} [{fmt(mlo)}, {fmt(mhi)}]{mark} | {fmt(md)} [{fmt(dlo)}, {fmt(dhi)}] |\n")
            w("\n\\* the mean difference's interval does not include 0.\n\n")

        w("Energy lost per game (mean) and avatars lost (total), by cause:\n\n")
        w("| cause | " + " | ".join(f"energy {a}" for a in arms) + " | "
          + " | ".join(f"lost {a}" for a in arms) + " |\n|---|" + "---|" * (2 * len(arms)) + "\n")
        for c in causes:
            dm = [np.mean(list(by_seed(rows, start, a, "damage_" + c).values())) for a in arms]
            dd = [int(sum(by_seed(rows, start, a, "deaths_" + c).values())) for a in arms]
            if max(dm) == 0 and max(dd) == 0:
                continue
            w(f"| {c} | " + " | ".join(fmt(x) for x in dm) + " | " + " | ".join(str(x) for x in dd) + " |\n")
        w("\n")


def main():
    ap = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    ap.add_argument("csv", nargs="+")
    ap.add_argument("--boot", type=int, default=10000)
    ap.add_argument("--out")
    args = ap.parse_args()
    rows, causes = load(args.csv)
    if not rows:
        sys.exit("no rows")
    out = open(args.out, "w") if args.out else sys.stdout
    report(rows, causes, args.boot, out)


if __name__ == "__main__":
    main()
