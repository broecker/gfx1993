#!/usr/bin/env python3
"""
Gfx:1993 Benchmark Comparison & Memory Profiling Utility

This script automates the process of building and running benchmarks across different 
Git checkpoints to verify performance gains and monitor memory usage.

Required Libraries/Tools:
    - Python 3.6+ (with typing support)
    - Git, CMake, and a C++ compiler (gcc/g++)
    - Google Performance Tools (for --pprof support):
        Ubuntu/Debian: sudo apt install google-perftools libgoogle-perftools-dev pprof

Usage:
    - Basic comparison (Current vs HEAD~1):
        python3 scripts/compare_benchmarks.py
    - Memory profiling using pprof:
        python3 scripts/compare_benchmarks.py --pprof
    - Compare against a specific branch or commit:
        python3 scripts/compare_benchmarks.py --baseline master
"""
import subprocess
import os
import sys
import re
import argparse
from typing import Dict, Optional, Tuple

# Ensure we are running from the project root
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, ".."))
os.chdir(PROJECT_ROOT)

def run_cmd(cmd: str, env: Optional[Dict[str, str]] = None) -> Optional[str]:
    result = subprocess.run(cmd, shell=True, capture_output=True, text=True, env={**os.environ, **(env or {})})
    if result.returncode != 0:
        print(f"Error running: {cmd}\n{result.stderr}")
        return None
    return result.stdout

def get_current_commit() -> str:
    return run_cmd("git rev-parse HEAD").strip()

def parse_benchmark_output(output: str) -> Dict[str, float]:
    """Parses the output from RenderProfile::print()"""
    stats = {}
    # Example line: rasterize.tris.shade.fill  :  0.22ms	[    0-    1]; 100 samples
    pattern = re.compile(r"([\w\.]+)\s+:\s+([\d\.]+)ms")
    for line in output.splitlines():
        match = pattern.search(line)
        if match:
            stats[match.group(1)] = float(match.group(2))
    return stats

def build_and_run(pprof_enabled: bool = False) -> Optional[Tuple[Dict[str, float], str]]:
    print("Building...")
    if not run_cmd("cmake -B build -S . -DCMAKE_BUILD_TYPE=Release && cmake --build build --target headless_benchmark -j$(nproc)"):
        return None
    
    print("Running benchmark...")
    env = {}
    if pprof_enabled:
        # Requires libgoogle-perftools-dev. 
        # We use HEAPPROFILE to trigger tcmalloc's heap profiler
        env["HEAPPROFILE"] = "/tmp/gfx_bench.heap"
    
    output = run_cmd("./build/examples/headless_benchmark", env=env)
    
    mem_info = ""
    if pprof_enabled and output:
        # Look for the first heap file generated
        if os.path.exists("/tmp/gfx_bench.heap.0001.heap"):
            mem_info = run_cmd("pprof --text ./build/examples/headless_benchmark /tmp/gfx_bench.heap.0001.heap | head -n 10")
            run_cmd("rm /tmp/gfx_bench.heap.*")

    return parse_benchmark_output(output) if output else None, mem_info

def main() -> None:
    parser = argparse.ArgumentParser(description='Compare Gfx:1993 benchmarks between git checkpoints.')
    parser.add_argument('--baseline', type=str, default='HEAD~1', help='Git commit to compare against (default: HEAD~1)')
    parser.add_argument('--pprof', action='store_true', help='Enable pprof memory profiling (requires tcmalloc)')
    args = parser.parse_args()

    # 0. Identify and read benchmark files to persist them across checkouts
    bench_src_path = "examples/headless_benchmark.cpp"
    bench_cmake_path = "examples/CMakeLists.txt"
    
    with open(bench_src_path, 'r') as f: bench_src_content = f.read()
    with open(bench_cmake_path, 'r') as f: bench_cmake_content = f.read()

    original_branch = run_cmd("git rev-parse --abbrev-ref HEAD").strip()
    has_stashed = False

    try:
        # 1. Capture Current State
        print(f"--- Collecting stats for CURRENT WORKSPACE ---")
        current_stats, current_mem = build_and_run(args.pprof)
        if not current_stats: return

        # 2. Stash and Checkout Baseline
        print(f"\n--- Switching to baseline: {args.baseline} ---")
        status = run_cmd("git status --porcelain")
        if status:
            print("Stashing local changes...")
            run_cmd("git stash push -u -m 'temp_bench_stash'")
            has_stashed = True
        
        run_cmd(f"git checkout {args.baseline}")

        # Restore benchmark files to the baseline checkout
        with open(bench_src_path, 'w') as f: f.write(bench_src_content)
        with open(bench_cmake_path, 'w') as f: f.write(bench_cmake_content)

        baseline_stats, baseline_mem = build_and_run(args.pprof)

    finally:
        # 3. Restore Workspace
        print(f"\n--- Restoring workspace ---")
        run_cmd(f"git checkout {original_branch}")
        if has_stashed:
            run_cmd("git stash pop")

    # 4. Comparison Report
    if not baseline_stats:
        print("Failed to collect baseline stats.")
        return

    print("\n" + "="*60)
    print(f"{'Stage':<30} | {'Baseline':<10} | {'Current':<10} | {'Diff'}")
    print("-"*60)
    
    all_keys = sorted(set(current_stats.keys()) | set(baseline_stats.keys()))
    for key in all_keys:
        b = baseline_stats.get(key, 0)
        c = current_stats.get(key, 0)
        diff = c - b
        percent = (diff / b * 100) if b != 0 else 0
        color = "\033[91m" if diff > 0.01 else "\033[92m" # Red if slower, Green if faster
        if abs(diff) < 0.01: color = ""
        
        print(f"{key:<30} | {b:>8.2f}ms | {c:>8.2f}ms | {color}{diff:>+7.2f}ms ({percent:>+6.1f}%)\033[0m")
    
    if args.pprof:
        print("\n--- Memory Profile (Current) ---")
        print(current_mem)

if __name__ == "__main__":
    main()