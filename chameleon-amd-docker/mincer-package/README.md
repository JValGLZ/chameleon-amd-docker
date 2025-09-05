# MINCER Package

A comprehensive toolkit for computational experiment reproducibility and performance measurement. MINCER helps researchers document, validate, and reproduce computational experiments across different hardware platforms.

## Overview

The MINCER package provides tools to:
- Generate hardware and software environment manifests
- Validate experiment reproducibility through automated testing
- Measure performance using PAPI, perf, and GPU monitoring tools
- Create timestamped logs for experiment tracking

## Features

### Manifest Generation
- Automatic hardware detection (CPU, RAM, GPU)
- Performance monitoring tool discovery (PAPI, perf)
- Script and file inventory
- JSON-formatted output for easy integration

### Reproducibility Testing
- Automated experiment execution
- Output comparison against reference results
- System information logging
- Timestamped result archiving

### Performance Measurement
- PAPI integration for hardware counter measurement
- GPU performance monitoring support
- Automated compilation and execution workflows

## Installation

1. Clone or download the MINCER package
2. Run the bootstrap script to install dependencies:
   ```bash
   ./bootstrap.sh
   ```
   This will include:
   - Linux perf tools
   - PAPI v7.0.0 performance monitoring library

## Usage

### Generate Experiment Manifest

Create a manifest file documenting your experiment environment:

```bash
python3 mincer_package.py --path /path/to/experiment --output manifest.json
```

The manifest includes:
- Experiment name and location
- Hardware specifications (CPU, RAM, GPU)
- Operating system details
- Available performance monitoring tools
- Inventory of experiment scripts and files

### Run Reproducibility Tests

Test experiment reproducibility on current hardware:

```bash
python3 self_test.py --path /path/to/experiment --reference reference_output.txt
```

Advanced usage with custom script:
```bash
python3 self_test.py --path . --reference my_reference.txt --script my_experiment.sh
```

Options:
- `--path`: Directory containing experiment files
- `--reference`: Expected output file for comparison
- `--script`: Script to execute (defaults to `run.sh`)
- `--output`: Name for generated output file (defaults to `output.txt`)

### Performance Measurement

The package includes example measurement scripts:

```bash
# Run PAPI-based performance measurement
./measure_papi.sh
```

## File Structure

```
mincer-package/
├── mincer_package.py    # Main manifest generation tool
├── self_test.py         # Reproducibility testing framework
├── bootstrap.sh         # Dependency installation script
└── example/
    └── test_experiment/
        ├── run.sh           # Main experiment script
        ├── measure_papi.sh  # PAPI measurement script
        ├── matrix_mul.c     # Example C program
        ├── papi_test.c      # PAPI test program
        └── reference.txt    # Reference output
```

## Example Workflow

1. **Set up experiment directory** with source code and run script
2. **Generate manifest** to document environment:
   ```bash
   python3 mincer_package.py --path experiment_dir
   ```
3. **Create reference output** by running experiment on reference system
4. **Test reproducibility** on target systems:
   ```bash
   python3 self_test.py --path experiment_dir --reference reference.txt
   ```

## Output Formats

### Manifest JSON Structure
```json
{
  "experiment_name": "test_experiment",
  "hardware": {
    "cpu": "x86_64",
    "ram_gb": 16.0
  },
  "os": "Linux 5.4.0",
  "metrics": ["PAPI", "perf", "GPU::RTX3080"],
  "scripts": ["run.sh", "measure_papi.sh"],
  "reproducibility_notes": "Auto-generated manifest..."
}
```

### Test Results
Results are saved in timestamped directories:
```
self_test_log_2025-06-23T15-34-00/
├── output.txt                              # Experiment output
└── self_test_log_2025-06-23T15-34-00.txt  # Test summary
```

### Python Dependencies
- `psutil` - System information
- `argparse` - Command line parsing
- Standard library modules: `os`, `json`, `platform`, `subprocess`, `datetime`

## Supported Performance Tools

- **PAPI**: Hardware performance counters
- **perf**: Linux performance monitoring


