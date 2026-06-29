import json
import sys
from pathlib import Path

import matplotlib
import matplotlib.pyplot as plt

HERE = Path(__file__).resolve().parent
DATA_DIR = HERE.parent / "data"
OUT_DIR = HERE / "plots"


def latest_report() -> Path:
    files = sorted(DATA_DIR.glob("*.json"))
    if not files:
        sys.exit(f"No JSON reports found in {DATA_DIR}")
    return files[-1]


def main():
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else latest_report()
    report = json.loads(path.read_text())
    results = report["results"]

    labels = [f'{r["technology"]}\n{r["algorithm"]}' for r in results]
    avg = [r["avg"] for r in results]
    err_lo = [r["avg"] - r["min"] for r in results]
    err_hi = [r["max"] - r["avg"] for r in results]

    fig, ax = plt.subplots(figsize=(10, 6))
    bars = ax.bar(labels, avg, yerr=[err_lo, err_hi], capsize=5)
    ax.bar_label(bars, fmt="%.2f", padding=3)
    ax.set_ylabel("Time (s)")
    ax.set_title(
        f'Solve time by algorithm\n'
        f'scramble={report["scrambleMoves"]} moves · cores={report["cores"]} · '
        f'repeats={report["repeats"]} · seed={report["seed"]}'
    )

    OUT_DIR.mkdir(exist_ok=True)
    out = OUT_DIR / f'{report["timestamp"]}.png'
    fig.savefig(out, dpi=120)
    print(f"Wrote {out}  (from {path.name})")


if __name__ == "__main__":
    main()
