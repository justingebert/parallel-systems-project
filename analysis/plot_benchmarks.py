import json
import statistics
import sys
from collections import defaultdict
from pathlib import Path

import matplotlib.pyplot as plt

HERE = Path(__file__).resolve().parent
DATA_DIR = HERE.parent / "data"
OUT_DIR = HERE / "plots"


def latest_report() -> Path:
    files = sorted(DATA_DIR.glob("*.json"))
    if not files:
        sys.exit(f"No JSON reports found in {DATA_DIR}")
    return files[-1]


def summarize(result: dict) -> dict:
    """Stats from the raw per-run data; C only collects."""
    times = [run["time"] for run in result["runs"]]
    return {
        "technology": result["technology"],
        "algorithm": result["algorithm"],
        "cores": result["cores"],
        "mean": statistics.mean(times),
        "median": statistics.median(times),
        "stddev": statistics.stdev(times) if len(times) > 1 else 0.0,
        "min": min(times),
        "max": max(times),
    }


def group_by_algo(stats: list) -> dict:
    """Group points by (technology, algorithm), each list sorted by cores."""
    groups = defaultdict(list)
    for s in stats:
        groups[(s["technology"], s["algorithm"])].append(s)
    for points in groups.values():
        points.sort(key=lambda s: s["cores"])
    return groups


def plot_bars(stats: list, target_cores: int, subtitle: str):
    """One bar per approach at a fixed core count."""
    def pick(points):
        at_target = [p for p in points if p["cores"] == target_cores]
        return at_target[0] if at_target else max(points, key=lambda p: p["cores"])

    rows = [pick(points) for points in group_by_algo(stats).values()]
    labels = [f'{s["technology"]}/{s["algorithm"]}\n{s["cores"]} cores' for s in rows]
    means = [s["mean"] for s in rows]
    median = [s["median"] for s in rows]
    lo = [s["mean"] - s["min"] for s in rows]
    hi = [s["max"] - s["mean"] for s in rows]

    fig, ax = plt.subplots(figsize=(10, 6))
    bars = ax.bar(labels, means, yerr=[lo, hi], capsize=5, label="mean")
    ax.scatter(labels, median, color="black", marker="D", zorder=3, label="median")
    ax.bar_label(bars, fmt="%.3f", padding=3)
    ax.set_ylabel("solve time (s)")
    ax.legend()
    ax.set_title(f"Solve time by approach: error bars: min/max\n{subtitle}")
    fig.tight_layout()
    return fig


def plot_scaling(stats: list, subtitle: str):
    """Mean solve time vs cores, one line per approach."""
    fig, ax = plt.subplots(figsize=(10, 6))
    for (tech, algo), points in group_by_algo(stats).items():
        cores = [s["cores"] for s in points]
        means = [s["mean"] for s in points]
        ax.plot(cores, means, marker="o", label=f"{tech}/{algo}")

    ax.set_xlabel("cores")
    ax.set_ylabel("solve time (s)")
    #ax.set_yscale("log")
    ax.set_xticks(sorted({s["cores"] for s in stats}))
    ax.legend()
    ax.set_title(f"Scaling: time vs cores\n{subtitle}")
    fig.tight_layout()
    return fig


def main():
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else latest_report()
    report = json.loads(path.read_text())
    stats = [summarize(r) for r in report["results"]]

    seeds = report.get("seeds", [report.get("seed")])
    subtitle = (f'scramble={report["scrambleMoves"]} moves · '
                f'{len(seeds)} seeds × {report["repeats"]} repeats')
    max_cores = max(s["cores"] for s in stats)

    OUT_DIR.mkdir(exist_ok=True)
    figures = {
        f'{report["timestamp"]}-bars.png': plot_bars(stats, max_cores, subtitle),
        f'{report["timestamp"]}-scaling.png': plot_scaling(stats, subtitle),
    }
    for name, fig in figures.items():
        out = OUT_DIR / name
        fig.savefig(out, dpi=120)
        print(f"Wrote {out}")


if __name__ == "__main__":
    main()
