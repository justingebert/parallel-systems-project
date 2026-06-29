import json
import statistics
import sys
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
    times = [run["time"] for run in result["runs"]]
    solved = sum(1 for run in result["runs"] if run["solved"])
    return {
        "label": f'{result["technology"]}\n{result["algorithm"]}',
        "n": len(times),
        "solved": solved,
        "mean": statistics.mean(times),
        "median": statistics.median(times),
        "stddev": statistics.stdev(times) if len(times) > 1 else 0.0,
        "min": min(times),
        "max": max(times),
    }


def main():
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else latest_report()
    report = json.loads(path.read_text())
    stats = [summarize(r) for r in report["results"]]

    labels = [s["label"] for s in stats]
    mean = [s["mean"] for s in stats]
    median = [s["median"] for s in stats]
    stddev = [s["stddev"] for s in stats]

    seeds = report.get("seeds", [report.get("seed")])
    n_runs = stats[0]["n"] if stats else 0

    fig, ax = plt.subplots(figsize=(10, 6))
    bars = ax.bar(labels, mean, yerr=stddev, capsize=5, label="mean ± std dev")
    ax.scatter(labels, median, color="black", marker="D", zorder=3, label="median")
    ax.bar_label(bars, fmt="%.2f", padding=3)
    ax.set_ylabel("Time (s)")
    ax.legend()
    ax.set_title(
        f'Solve time by algorithm\n'
        f'scramble={report["scrambleMoves"]} moves · cores={report["cores"]} · '
        f'{len(seeds)} seeds × {report["repeats"]} repeats = {n_runs} runs'
    )

    OUT_DIR.mkdir(exist_ok=True)
    out = OUT_DIR / f'{report["timestamp"]}.png'
    fig.savefig(out, dpi=120)
    print(f"Wrote {out}  (from {path.name})")


if __name__ == "__main__":
    main()
