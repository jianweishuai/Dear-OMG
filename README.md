## <p align="center">Dear-OMG: an omics-general compression method for genomics, proteomics and metabolomics data</p>

[![Platform](https://img.shields.io/badge/platform-Windows-blue.svg)](https://github.com/jianweishuai/Dear-OMG)
[![Version](https://img.shields.io/badge/version-v1.0.0-orange.svg)](https://github.com/jianweishuai/Dear-OMG)

## Introduction

Dear-OMG is a unified, compact, flexible, and high-performance metadata storage solution for multi-omics data. This software introduces a novel file storage structure and utilizes the Elias-Fano encoding algorithm to compress and store proteomics, genomics, and metabolomics metadata into the unified OMG format.

### Key Features

- **Multi-omics Support**: Handles genomics (FASTQ), proteomics (RAW, WIFF, D), and metabolomics (imzML) data formats
- **High Performance**: Achieves 80% reduction in storage space and 90% decrease in conversion time compared to mzXML/mzML
- **Parallel Processing**: Enables parallel random access to any data block with ~10-fold speed improvement
- **Cross-format Compatibility**: Unified OMG format for all omics data types
- **Flexible Output**: Supports binary, JSON, and YAML output formats
- **Python API**: Convenient Python wrapper for programmatic access

### Performance Benchmarks

Compared to commonly used proteomics formats (mzXML and mzML), Dear-OMG achieves:

- **80% reduction** in storage space
- **90% decrease** in conversion time  
- **~10-fold speed improvement** with parallel random access support

## System Requirements

### Platform Support

- **Operating System**: Windows (Windows 10 or later recommended)
- **Architecture**: x64
- **Memory**: Minimum 4GB RAM (8GB+ recommended for large datasets)
- **Storage**: Sufficient space for input data and compressed output files

**Note**: Dear-OMG currently supports Windows platform only. Linux and macOS support are planned for future releases.

### Dependencies

- Microsoft Visual C++ Redistributable (included with Windows)
- No additional dependencies required for the executables

## Installation and Build Instructions

### Pre-built Executables

The repository includes pre-built executables in the `bin/` directories:

- `DearOMG-community/bin/DearOMG-community.exe`
- `DearOMG-vendor/bin/DearOMG-vendor.exe`  
- `DearOMG-parser/bin/DearOMG-parser.exe`

### Building from Source

If you need to build from source:

1. **Prerequisites**:
   
   - Visual Studio 2019 or later with C++ development tools
   - Windows SDK

2. **Build Steps**:
   
   ```bash
   # Open the solution files in Visual Studio:
   # - DearOMG-community/DearOMG-community.sln
   # - DearOMG-vendor/DearOMG-vendor.sln
   # - DearOMG-parser/DearOMG-parser.sln
   
   # Build in Release mode for optimal performance
   # The executables will be generated in the respective bin/ directories
   ```

## Usage Guide

### Supported File Formats

Dear-OMG supports multiple omics data formats:

| Omics Type       | Supported Formats                       | Tool Required         |
| ---------------- | --------------------------------------- | --------------------- |
| **Proteomics**   | `.raw` (Thermo Fisher), `.wiff` (Sciex) | DearOMG-vendor.exe    |
| **Proteomics**   | `.d` (Bruker)                           | DearOMG-community.exe |
| **Metabolomics** | `.imzML`                                | DearOMG-community.exe |
| **Genomics**     | `.fastq`                                | DearOMG-community.exe |

### Basic Usage

#### Converting Data Files

**For vendor-specific formats (.raw, .wiff)**:

```bash
DearOMG-vendor.exe --write_mode=binary --precision=0.001 --skip_zero=1 --out_dir=./output/ --input=sample.raw;sample.wiff
```

**For community formats (.d, .imzML, .fastq)**:

```bash
DearOMG-community.exe --write_mode=binary --precision=0.001 --skip_zero=1 --out_dir=./output/ --input=sample.d;sample.fastq
```

#### Command Line Parameters

| Parameter      | Description              | Options                        | Default  |
| -------------- | ------------------------ | ------------------------------ | -------- |
| `--write_mode` | Output format            | `binary`, `json`, `yaml`       | `binary` |
| `--precision`  | m/z array precision      | Floating point number          | `0.001`  |
| `--skip_zero`  | Skip zero intensity ions | `1` (true), `0` (false)        | `1`      |
| `--out_dir`    | Output directory path    | Valid directory path           | Required |
| `--input`      | Input file list          | Semicolon-separated file paths | Required |

**Important Notes**:

- File paths should not contain spaces
- Use forward slashes (/) or escaped backslashes (\\) in paths
- Multiple files can be specified using semicolon (;) separation

### Parsing OMG Files

After converting data to OMG format, use the parser to extract and analyze the data:

```bash
DearOMG-parser.exe --mt=4 --read=memory --omg=sample.omg
```

#### Parser Parameters

| Parameter | Description                               | Options          | Default  |
| --------- | ----------------------------------------- | ---------------- | -------- |
| `--mt`    | Number of threads for parallel processing | Integer (1-32)   | 4        |
| `--read`  | Data reading mode                         | `memory`, `disk` | `memory` |
| `--omg`   | Path to OMG file                          | Valid file path  | Required |

**Reading Modes**:

- `memory`: Load entire file into memory (faster for repeated access)
- `disk`: Read data from disk as needed (lower memory usage)

### Example Workflows

#### Single File Conversion

```bash
# Convert a single proteomics file
DearOMG-vendor.exe --write_mode=binary --precision=0.001 --skip_zero=1 --out_dir=./results/ --input=experiment1.raw

# Parse the converted file
DearOMG-parser.exe --mt=8 --read=memory --omg=./results/experiment1.omg
```

#### Batch Processing

```bash
# Convert multiple files at once
DearOMG-community.exe --write_mode=binary --precision=0.001 --skip_zero=1 --out_dir=./batch_results/ --input=sample1.d;sample2.fastq;sample3.imzML

# Parse each converted file
DearOMG-parser.exe --mt=4 --read=memory --omg=./batch_results/sample1.omg
DearOMG-parser.exe --mt=4 --read=memory --omg=./batch_results/sample2.omg
DearOMG-parser.exe --mt=4 --read=memory --omg=./batch_results/sample3.omg
```

#### Multi-omics Data Processing

```bash
# Process genomics data
DearOMG-community.exe --write_mode=binary --precision=1.0 --skip_zero=1 --out_dir=./genomics/ --input=reads.fastq

# Process proteomics data with high precision
DearOMG-vendor.exe --write_mode=binary --precision=0.0001 --skip_zero=1 --out_dir=./proteomics/ --input=proteins.raw

# Process metabolomics data
DearOMG-community.exe --write_mode=json --precision=0.001 --skip_zero=0 --out_dir=./metabolomics/ --input=metabolites.imzML
```

## Python API

Dear-OMG provides a comprehensive Python API for programmatic access to conversion and parsing functionality.

### Installation

1. **Ensure Dear-OMG executables are available**:
   
   - Download or build the Dear-OMG executables
   - Place them in your working directory or specify the path

2. **Install Python API**:
   
   ```bash
   # Navigate to the python_api directory
   cd python_api
   
   # Install dependencies (minimal - uses mostly standard library)
   pip install -r requirements.txt
   
   # The API is ready to use
   python examples.py
   ```

### Quick Start

```python
from python_api.dear_omg import DearOMGConverter, DearOMGParser

# Initialize converter
converter = DearOMGConverter(dear_omg_path="./")

# Convert files
results = converter.convert_files(
    input_files=["data/sample.raw", "data/sample.fastq"],
    output_dir="./output/",
    write_mode="binary",
    precision=0.001,
    skip_zero=True
)

# Initialize parser
parser = DearOMGParser(dear_omg_path="./")

# Parse converted files
for input_file, omg_file in results.items():
    if omg_file.endswith('.omg'):
        parse_result = parser.parse_omg_file(
            omg_file=omg_file,
            num_threads=4,
            read_mode="memory"
        )
        print(f"Parsed {omg_file} in {parse_result.get('elapsed_time', 'N/A')} seconds")
```

### API Features

- **Simple Interface**: Easy-to-use classes and functions
- **Error Handling**: Comprehensive error checking and reporting
- **Batch Processing**: Support for processing multiple files
- **Flexible Configuration**: All command-line options available programmatically
- **Multi-omics Support**: Unified interface for all omics data types

### API Documentation

#### DearOMGConverter Class

```python
class DearOMGConverter:
    def __init__(self, dear_omg_path: Optional[str] = None)
    def convert_files(self, input_files, output_dir, write_mode="binary", 
                     precision=0.001, skip_zero=True) -> Dict[str, str]
    def get_supported_formats(self) -> Dict[str, List[str]]
```

#### DearOMGParser Class

```python
class DearOMGParser:
    def __init__(self, dear_omg_path: Optional[str] = None)
    def parse_omg_file(self, omg_file, num_threads=4, 
                      read_mode="memory") -> Dict[str, any]
    def get_file_info(self, omg_file) -> Dict[str, any]
```

#### Convenience Functions

```python
def convert_to_omg(input_files, output_dir, dear_omg_path=None, **kwargs)
def parse_omg(omg_file, dear_omg_path=None, **kwargs)
```

### Example Use Cases

#### 1. High-throughput Proteomics Analysis

```python
from python_api.dear_omg import convert_to_omg, parse_omg
import glob

# Convert all RAW files in a directory
raw_files = glob.glob("proteomics_data/*.raw")
results = convert_to_omg(
    input_files=raw_files,
    output_dir="compressed_data/",
    precision=0.0001,  # High precision for proteomics
    skip_zero=True
)

# Analyze compression efficiency
for original, compressed in results.items():
    original_size = os.path.getsize(original)
    compressed_size = os.path.getsize(compressed)
    compression_ratio = (1 - compressed_size/original_size) * 100
    print(f"{original}: {compression_ratio:.1f}% compression")
```

#### 2. Multi-omics Data Integration

```python
from python_api.dear_omg import DearOMGConverter

converter = DearOMGConverter()

# Process different omics data types
omics_data = {
    "genomics": ["data/genome.fastq"],
    "proteomics": ["data/proteins.raw", "data/proteins.wiff"],
    "metabolomics": ["data/metabolites.imzML"]
}

for omics_type, files in omics_data.items():
    print(f"Processing {omics_type} data...")
    results = converter.convert_files(
        input_files=files,
        output_dir=f"results/{omics_type}/",
        write_mode="binary",
        precision=0.001 if omics_type != "genomics" else 1.0
    )
    print(f"Converted {len(results)} {omics_type} files")
```

#### 3. Automated Pipeline Integration

```python
import os
from pathlib import Path
from python_api.dear_omg import DearOMGConverter, DearOMGParser

def process_omics_pipeline(input_dir, output_dir):
    """Automated pipeline for omics data processing."""

    converter = DearOMGConverter()
    parser = DearOMGParser()

    # Find all supported files
    supported_exts = [".raw", ".wiff", ".d", ".imzml", ".fastq"]
    input_files = []

    for ext in supported_exts:
        input_files.extend(Path(input_dir).glob(f"*{ext}"))

    if not input_files:
        print("No supported files found")
        return

    # Convert files
    print(f"Converting {len(input_files)} files...")
    results = converter.convert_files(
        input_files=[str(f) for f in input_files],
        output_dir=output_dir,
        write_mode="binary",
        precision=0.001,
        skip_zero=True
    )

    # Parse and analyze
    print("Parsing converted files...")
    for original, omg_file in results.items():
        if omg_file.endswith('.omg'):
            parse_result = parser.parse_omg_file(omg_file, num_threads=8)
            print(f"Processed {Path(original).name} -> {Path(omg_file).name}")

    print("Pipeline completed successfully")

# Usage
 process_omics_pipeline("raw_data/", "processed_data/")
```

## Multi-omics Support Benefits

Dear-OMG's unified OMG format provides significant advantages for multi-omics research:

### 1. **Unified Data Format**

- Single file format for genomics, proteomics, and metabolomics data
- Consistent metadata structure across all omics types
- Simplified data management and storage

### 2. **Cross-platform Compatibility**

- Standardized format eliminates vendor-specific dependencies
- Easy data sharing between research groups
- Future-proof data storage solution

### 3. **Enhanced Performance**

- Parallel random access enables efficient data retrieval
- Optimized compression reduces storage requirements
- Fast conversion and parsing speeds

### 4. **Research Applications**

#### Integrated Multi-omics Analysis

```python
# Example: Correlating genomics and proteomics data
from python_api.dear_omg import DearOMGParser

parser = DearOMGParser()

# Parse genomics data
genome_data = parser.parse_omg_file("results/genome.omg", num_threads=8)

# Parse proteomics data  
protein_data = parser.parse_omg_file("results/proteins.omg", num_threads=8)

# Perform integrated analysis
# (Your analysis code here)
```

#### Longitudinal Studies

```python
# Process time-series multi-omics data
time_points = ["T0", "T1", "T2", "T3"]
omics_types = ["genomics", "proteomics", "metabolomics"]

for time_point in time_points:
    for omics_type in omics_types:
        input_files = glob.glob(f"data/{time_point}_{omics_type}/*")
        convert_to_omg(
            input_files=input_files,
            output_dir=f"results/{time_point}/{omics_type}/",
            precision=0.001
        )
```

#### Comparative Studies

```python
# Compare data across different conditions or treatments
conditions = ["control", "treatment_A", "treatment_B"]

for condition in conditions:
    # Process all omics data for each condition
    omics_files = {
        "genomics": glob.glob(f"data/{condition}/genomics/*.fastq"),
        "proteomics": glob.glob(f"data/{condition}/proteomics/*.raw"),
        "metabolomics": glob.glob(f"data/{condition}/metabolomics/*.imzML")
    }

    for omics_type, files in omics_files.items():
        if files:
            convert_to_omg(
                input_files=files,
                output_dir=f"results/{condition}/{omics_type}/",
                write_mode="binary"
            )
```

## Common Usage Patterns

### 1. **Data Import Workflow**

```bash
# Step 1: Organize your data
mkdir -p data/genomics data/proteomics data/metabolomics

# Step 2: Convert data by type
DearOMG-community.exe --write_mode=binary --precision=1.0 --skip_zero=1 --out_dir=./results/genomics/ --input=data/genomics/sample.fastq
DearOMG-vendor.exe --write_mode=binary --precision=0.001 --skip_zero=1 --out_dir=./results/proteomics/ --input=data/proteomics/sample.raw
DearOMG-community.exe --write_mode=binary --precision=0.001 --skip_zero=1 --out_dir=./results/metabolomics/ --input=data/metabolomics/sample.imzML

# Step 3: Verify conversion
DearOMG-parser.exe --mt=4 --read=memory --omg=./results/genomics/sample.omg
DearOMG-parser.exe --mt=4 --read=memory --omg=./results/proteomics/sample.omg
DearOMG-parser.exe --mt=4 --read=memory --omg=./results/metabolomics/sample.omg
```

### 2. **API Integration Workflow**

```python
from python_api.dear_omg import DearOMGConverter, DearOMGParser
import os

def complete_omics_workflow(data_dir, output_dir):
    """Complete workflow for multi-omics data processing."""

    converter = DearOMGConverter()
    parser = DearOMGParser()

    # Define data organization
    omics_config = {
        "genomics": {"extensions": [".fastq"], "precision": 1.0},
        "proteomics": {"extensions": [".raw", ".wiff", ".d"], "precision": 0.001},
        "metabolomics": {"extensions": [".imzml"], "precision": 0.001}
    }

    results = {}

    for omics_type, config in omics_config.items():
        print(f"Processing {omics_type} data...")

        # Find files
        files = []
        for ext in config["extensions"]:
            files.extend(glob.glob(f"{data_dir}/**/*{ext}", recursive=True))

        if not files:
            print(f"No {omics_type} files found")
            continue

        # Convert files
        omics_output_dir = f"{output_dir}/{omics_type}"
        conversion_results = converter.convert_files(
            input_files=files,
            output_dir=omics_output_dir,
            precision=config["precision"],
            write_mode="binary",
            skip_zero=True
        )

        # Parse and validate
        parsing_results = []
        for original, omg_file in conversion_results.items():
            if omg_file.endswith('.omg'):
                parse_result = parser.parse_omg_file(omg_file, num_threads=4)
                parsing_results.append({
                    "original": original,
                    "omg_file": omg_file,
                    "parse_time": parse_result.get("elapsed_time", 0)
                })

        results[omics_type] = {
            "files_converted": len(conversion_results),
            "parsing_results": parsing_results
        }

        print(f"Completed {omics_type}: {len(conversion_results)} files converted")

    return results

# Usage
results = complete_omics_workflow("raw_data/", "processed_data/")
print("Workflow completed:", results)
```

## Troubleshooting

### Common Issues and Solutions

#### 1. **File Path Issues**

- **Problem**: "File not found" errors
- **Solution**: Ensure file paths do not contain spaces; use forward slashes or escaped backslashes

#### 2. **Memory Issues**

- **Problem**: Out of memory errors during processing
- **Solution**: Use `--read=disk` mode for large files; increase system RAM if possible

#### 3. **Permission Issues**

- **Problem**: Cannot write to output directory
- **Solution**: Ensure write permissions for output directory; run as administrator if necessary

#### 4. **Vendor File Issues**

- **Problem**: Cannot read .raw or .wiff files
- **Solution**: Ensure vendor libraries are properly installed; use DearOMG-vendor.exe for these formats

#### 5. **Python API Issues**

- **Problem**: Cannot find executables
- **Solution**: Specify correct path in `dear_omg_path` parameter or place executables in working directory

### Performance Optimization

#### For Large Datasets

```bash
# Use more threads for faster processing
DearOMG-parser.exe --mt=16 --read=memory --omg=large_file.omg

# For very large files, use disk mode to reduce memory usage
DearOMG-parser.exe --mt=8 --read=disk --omg=very_large_file.omg
```

#### For Batch Processing

```python
# Process files in parallel using Python multiprocessing
from multiprocessing import Pool
from python_api.dear_omg import convert_to_omg

def convert_file(file_info):
    input_file, output_dir = file_info
    return convert_to_omg([input_file], output_dir)

# Parallel conversion
file_list = [(f, "output/") for f in input_files]
with Pool(processes=4) as pool:
    results = pool.map(convert_file, file_list)
```

## Frequently Asked Questions

**Q: What platforms are supported?**
A: Currently, Dear-OMG supports Windows only. Linux and macOS support are planned for future releases.

**Q: Can I convert files larger than available RAM?**
A: Yes, use the `--read=disk` mode for parsing large OMG files. The conversion process is optimized for memory efficiency.

**Q: How do I integrate Dear-OMG into my existing pipeline?**
A: Use the Python API for seamless integration. The API provides programmatic access to all functionality.

**Q: What is the maximum file size supported?**
A: There is no hard limit, but performance depends on available system resources. Files up to several GB have been tested successfully.

**Q: Can I customize the compression settings?**
A: Yes, adjust the `--precision` parameter to control compression vs. accuracy trade-offs. Lower precision values provide better compression.

**Q: How do I cite Dear-OMG in my research?**
A: Please cite the Dear-OMG paper (citation information will be provided upon publication).

## Contributing

We welcome contributions to Dear-OMG! Please:

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request

For bug reports and feature requests, please use the GitHub issue tracker.



## Contact

**Development Team**: Xiamen University  
**Project Repository**: https://github.com/jianweishuai/Dear-OMG  
**Version**: v1.0.0

For technical support and questions:

- Create an issue on GitHub
- Contact the development team through the repository

---

**Note**: Dear-OMG is actively developed and maintained. Please check the repository for the latest updates and releases.
