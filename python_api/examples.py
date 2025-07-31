#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Dear-OMG Python API Examples

This script demonstrates how to use the Dear-OMG Python API for
converting and parsing omics data files.

Author: Xiamen University Development Team
Version: 1.0.0
"""

import os
from pathlib import Path
from dear_omg import DearOMGConverter, DearOMGParser, convert_to_omg, parse_omg, DearOMGError


def example_basic_conversion():
    """
    Example 1: Basic file conversion using the convenience function.
    """
    print("=== Example 1: Basic File Conversion ===")
    
    try:
        # Example input files (replace with your actual file paths)
        input_files = [
            "example_data/sample.raw",  # Thermo Fisher file
            "example_data/sample.wiff", # Sciex file
            "example_data/sample.d",    # Bruker file
            "example_data/sample.fastq" # Genomics file
        ]
        
        output_dir = "output/converted"
        
        # Convert files to OMG format
        results = convert_to_omg(
            input_files=input_files,
            output_dir=output_dir,
            write_mode="binary",
            precision=0.001,
            skip_zero=True
        )
        
        print("Conversion results:")
        for input_file, output_file in results.items():
            print(f"  {input_file} -> {output_file}")
            
    except DearOMGError as e:
        print(f"Conversion error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")


def example_advanced_conversion():
    """
    Example 2: Advanced conversion with custom settings using the class interface.
    """
    print("\n=== Example 2: Advanced Conversion ===")
    
    try:
        # Initialize converter with custom path
        converter = DearOMGConverter(dear_omg_path=".")
        
        # Check supported formats
        formats = converter.get_supported_formats()
        print("Supported formats:")
        for tool, exts in formats.items():
            print(f"  {tool}: {', '.join(exts)}")
        
        # Convert proteomics files with high precision
        proteomics_files = ["example_data/protein_sample.raw"]
        
        results = converter.convert_files(
            input_files=proteomics_files,
            output_dir="output/proteomics",
            write_mode="json",  # Use JSON format for readability
            precision=0.0001,   # Higher precision
            skip_zero=False     # Keep zero intensity values
        )
        
        print("\nProteomics conversion results:")
        for input_file, output_file in results.items():
            print(f"  {input_file} -> {output_file}")
            
    except DearOMGError as e:
        print(f"Conversion error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")


def example_parsing():
    """
    Example 3: Parsing OMG files.
    """
    print("\n=== Example 3: Parsing OMG Files ===")
    
    try:
        # Initialize parser
        parser = DearOMGParser()
        
        # Example OMG file (replace with your actual file)
        omg_file = "output/converted/sample.omg"
        
        # Get basic file information
        file_info = parser.get_file_info(omg_file)
        print("File information:")
        print(f"  Path: {file_info['file_path']}")
        print(f"  Size: {file_info['file_size_mb']} MB")
        
        # Parse the file with multiple threads
        parsing_result = parser.parse_omg_file(
            omg_file=omg_file,
            num_threads=8,
            read_mode="memory"  # Load entire file into memory for faster access
        )
        
        print("\nParsing results:")
        print(f"  Threads used: {parsing_result.get('threads_used', 'N/A')}")
        print(f"  Elapsed time: {parsing_result.get('elapsed_time', 'N/A')} seconds")
        print(f"  Success: {parsing_result['success']}")
        
    except DearOMGError as e:
        print(f"Parsing error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")


def example_batch_processing():
    """
    Example 4: Batch processing multiple files.
    """
    print("\n=== Example 4: Batch Processing ===")
    
    try:
        # Process all files in a directory
        input_dir = Path("example_data")
        output_dir = "output/batch"
        
        if not input_dir.exists():
            print(f"Input directory {input_dir} does not exist. Creating example...")
            input_dir.mkdir(parents=True, exist_ok=True)
            print("Please place your data files in the 'example_data' directory.")
            return
        
        # Find all supported files
        supported_extensions = [".raw", ".wiff", ".d", ".imzml", ".fastq"]
        input_files = []
        
        for ext in supported_extensions:
            input_files.extend(input_dir.glob(f"*{ext}"))
        
        if not input_files:
            print("No supported files found in the input directory.")
            return
        
        print(f"Found {len(input_files)} files to process:")
        for file in input_files:
            print(f"  {file.name}")
        
        # Convert all files
        converter = DearOMGConverter()
        results = converter.convert_files(
            input_files=[str(f) for f in input_files],
            output_dir=output_dir,
            write_mode="binary",
            precision=0.001,
            skip_zero=True
        )
        
        print("\nBatch conversion completed:")
        successful = 0
        for input_file, output_file in results.items():
            if "failed" not in output_file.lower():
                successful += 1
            print(f"  {Path(input_file).name} -> {Path(output_file).name}")
        
        print(f"\nSuccessfully converted {successful}/{len(input_files)} files.")
        
    except DearOMGError as e:
        print(f"Batch processing error: {e}")
    except Exception as e:
        print(f"Unexpected error: {e}")


def example_multi_omics_workflow():
    """
    Example 5: Multi-omics data processing workflow.
    """
    print("\n=== Example 5: Multi-omics Workflow ===")
    
    try:
        converter = DearOMGConverter()
        parser = DearOMGParser()
        
        # Define multi-omics dataset
        multi_omics_data = {
            "genomics": ["data/genome_sample.fastq"],
            "proteomics": ["data/protein_sample.raw", "data/protein_sample.wiff"],
            "metabolomics": ["data/metabolite_sample.imzML"]
        }
        
        results_summary = {}
        
        for omics_type, files in multi_omics_data.items():
            print(f"\nProcessing {omics_type} data...")
            
            # Create output directory for this omics type
            output_dir = f"output/multi_omics/{omics_type}"
            
            try:
                # Convert files
                conversion_results = converter.convert_files(
                    input_files=files,
                    output_dir=output_dir,
                    write_mode="binary",
                    precision=0.001 if omics_type != "genomics" else 1.0,
                    skip_zero=True
                )
                
                # Parse converted files
                parsing_results = []
                for input_file, omg_file in conversion_results.items():
                    if "failed" not in omg_file.lower() and Path(omg_file).exists():
                        parse_result = parser.parse_omg_file(
                            omg_file=omg_file,
                            num_threads=4,
                            read_mode="memory"
                        )
                        parsing_results.append(parse_result)
                
                results_summary[omics_type] = {
                    "files_processed": len(files),
                    "conversions_successful": len([r for r in conversion_results.values() 
                                                  if "failed" not in r.lower()]),
                    "parsing_results": parsing_results
                }
                
                print(f"  Processed {len(files)} {omics_type} files")
                
            except DearOMGError as e:
                print(f"  Error processing {omics_type}: {e}")
                results_summary[omics_type] = {"error": str(e)}
        
        # Print summary
        print("\n=== Multi-omics Processing Summary ===")
        for omics_type, results in results_summary.items():
            if "error" in results:
                print(f"{omics_type}: Error - {results['error']}")
            else:
                print(f"{omics_type}: {results['conversions_successful']}/{results['files_processed']} files converted successfully")
        
    except Exception as e:
        print(f"Multi-omics workflow error: {e}")


def main():
    """
    Run all examples.
    """
    print("Dear-OMG Python API Examples")
    print("=============================")
    print("Note: These examples assume you have Dear-OMG executables available.")
    print("Please ensure the following files exist in your Dear-OMG installation:")
    print("  - DearOMG-community/bin/DearOMG-community.exe")
    print("  - DearOMG-vendor/bin/DearOMG-vendor.exe")
    print("  - DearOMG-parser/bin/DearOMG-parser.exe")
    print()
    
    # Create output directories
    Path("output").mkdir(exist_ok=True)
    Path("example_data").mkdir(exist_ok=True)
    
    # Run examples
    example_basic_conversion()
    example_advanced_conversion()
    example_parsing()
    example_batch_processing()
    example_multi_omics_workflow()
    
    print("\n=== Examples completed ===")
    print("Note: Some examples may show errors if example data files are not available.")
    print("Replace the example file paths with your actual data files to test the API.")


if __name__ == "__main__":
    main()