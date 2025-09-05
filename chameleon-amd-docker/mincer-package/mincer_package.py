#!/usr/bin/env python3

import os
import json
import platform
import argparse
import psutil


import subprocess

def detect_metrics():
    detected = []

    def is_tool_available(cmd):
        return subprocess.call(
            f"type {cmd}", shell=True, stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL
        ) == 0

    if is_tool_available("papi_avail"):
        detected.append("PAPI")

    if is_tool_available("nvidia-smi"):
        try:
            out = subprocess.check_output("nvidia-smi --query-gpu=name --format=csv,noheader", shell=True)
            gpu_names = out.decode().strip().split("\n")
            detected.extend([f"GPU::{gpu}" for gpu in gpu_names])
        except Exception:
            detected.append("GPU::Unknown")

    if is_tool_available("perf"):
        detected.append("perf")

    return detected


def get_system_info():
    return {
        "os": f"{platform.system()} {platform.release()}",
        "cpu": platform.processor() or platform.machine(),
        "ram_gb": round(psutil.virtual_memory().total / (1024 ** 3), 2)
    }

def find_scripts(directory):
    script_list = []
    for filename in os.listdir(directory):
        if filename.endswith(('.sh', '.py', '.ipynb', '.yml', '.txt')):
            script_list.append(filename)
    return script_list

def generate_manifest(path, output_file):
    system_info = get_system_info()
    scripts_found = find_scripts(path)

    manifest = {
        "experiment_name": os.path.basename(os.path.abspath(path)),
        "hardware": {
            "cpu": system_info["cpu"],
            "ram_gb": system_info["ram_gb"]
        },
        "os": system_info["os"],
        "metrics": detect_metrics(),
        "scripts": scripts_found,
        "reproducibility_notes": "Auto-generated manifest. Please edit to add measurement tools and details."
    }

    with open(output_file, 'w') as f:
        json.dump(manifest, f, indent=2)

    print(f"Manifest written to {output_file}")

def main():
    parser = argparse.ArgumentParser(description="Generate reproducibility manifest for a MINCER experiment.")
    parser.add_argument("--path", type=str, required=True, help="Path to the experiment folder")
    parser.add_argument("--output", type=str, default="manifest.json", help="Output manifest file name")
    args = parser.parse_args()

    if not os.path.isdir(args.path):
        print("Error: Provided path is not a directory.")
        return

    generate_manifest(args.path, args.output)

if __name__ == "__main__":
    main()
