#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Dear-OMG Python API

A Python wrapper for the Dear-OMG omics data compression and parsing tools.
This module provides a convenient Python interface to convert and parse
genomic, proteomic, and metabolomic data using the Dear-OMG format.

Note: This API currently only supports Windows platform.

Author: Xiamen University Development Team
Version: 1.0.0
"""

import os
import sys
import subprocess
import json
from typing import List, Dict, Optional, Union, Tuple
from pathlib import Path


class DearOMGError(Exception):
    """Custom exception for Dear-OMG related errors."""
    pass


class DearOMGConverter:
    """
    Python wrapper for Dear-OMG data conversion tools.
    
    This class provides methods to convert various omics data formats
    (genomics, proteomics, metabolomics) to the unified OMG format.
    """
    
    def __init__(self, dear_omg_path: Optional[str] = None):
        """
        Initialize the DearOMG converter.
        
        Args:
            dear_omg_path: Path to the Dear-OMG installation directory.
                          If None, assumes executables are in the current directory.
        """
        if dear_omg_path is None:
            dear_omg_path = os.getcwd()
        
        self.dear_omg_path = Path(dear_omg_path)
        self.community_exe = self.dear_omg_path / "DearOMG-community" / "bin" / "DearOMG-community.exe"
        self.vendor_exe = self.dear_omg_path / "DearOMG-vendor" / "bin" / "DearOMG-vendor.exe"
        self.parser_exe = self.dear_omg_path / "DearOMG-parser" / "bin" / "DearOMG-parser.exe"
        
        # Check if executables exist
        if not self.community_exe.exists():
            raise DearOMGError(f"DearOMG-community.exe not found at {self.community_exe}")
        if not self.vendor_exe.exists():
            raise DearOMGError(f"DearOMG-vendor.exe not found at {self.vendor_exe}")
        if not self.parser_exe.exists():
            raise DearOMGError(f"DearOMG-parser.exe not found at {self.parser_exe}")
    
    def convert_files(self, 
                     input_files: Union[str, List[str]], 
                     output_dir: str,
                     write_mode: str = "binary",
                     precision: float = 0.001,
                     skip_zero: bool = True) -> Dict[str, str]:
        """
        Convert input files to OMG format.
        
        Args:
            input_files: Single file path or list of file paths to convert.
                        Supported formats: .wiff, .raw, .d, .imzML, .fastq
            output_dir: Directory to save the converted OMG files.
            write_mode: Data storage type. Options: "binary", "json", "yaml". Default: "binary"
            precision: Precision of m/z array. Default: 0.001
            skip_zero: Skip ions with zero intensity. Default: True
            
        Returns:
            Dictionary with conversion results and output file paths.
            
        Raises:
            DearOMGError: If conversion fails or invalid parameters provided.
        """
        # Validate inputs
        if isinstance(input_files, str):
            input_files = [input_files]
        
        if not input_files:
            raise DearOMGError("No input files provided")
        
        # Check if all input files exist
        for file_path in input_files:
            if not Path(file_path).exists():
                raise DearOMGError(f"Input file not found: {file_path}")
        
        # Create output directory if it doesn't exist
        Path(output_dir).mkdir(parents=True, exist_ok=True)
        
        # Determine which executable to use based on file extensions
        community_files = []
        vendor_files = []
        
        for file_path in input_files:
            ext = Path(file_path).suffix.lower()
            if ext in [".d", ".imzml", ".fastq"]:
                community_files.append(file_path)
            elif ext in [".wiff", ".raw"]:
                vendor_files.append(file_path)
            else:
                raise DearOMGError(f"Unsupported file format: {ext}")
        
        results = {}
        
        # Convert community files
        if community_files:
            result = self._run_conversion(
                self.community_exe, community_files, output_dir, 
                write_mode, precision, skip_zero
            )
            results.update(result)
        
        # Convert vendor files
        if vendor_files:
            result = self._run_conversion(
                self.vendor_exe, vendor_files, output_dir,
                write_mode, precision, skip_zero
            )
            results.update(result)
        
        return results
    
    def _run_conversion(self, exe_path: Path, input_files: List[str], 
                       output_dir: str, write_mode: str, precision: float, 
                       skip_zero: bool) -> Dict[str, str]:
        """
        Run the conversion process using the specified executable.
        
        Args:
            exe_path: Path to the executable
            input_files: List of input files
            output_dir: Output directory
            write_mode: Write mode
            precision: Precision value
            skip_zero: Skip zero flag
            
        Returns:
            Dictionary with conversion results
        """
        # Build command arguments
        input_str = ";".join(input_files)
        skip_zero_val = "1" if skip_zero else "0"
        
        cmd = [
            str(exe_path),
            f"--write_mode={write_mode}",
            f"--precision={precision}",
            f"--skip_zero={skip_zero_val}",
            f"--out_dir={output_dir}",
            f"--input={input_str}"
        ]
        
        try:
            # Run the conversion
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            
            # Parse output to get converted file information
            output_files = {}
            for input_file in input_files:
                input_name = Path(input_file).stem
                output_file = Path(output_dir) / f"{input_name}.omg"
                if output_file.exists():
                    output_files[input_file] = str(output_file)
                else:
                    output_files[input_file] = "Conversion failed"
            
            return output_files
            
        except subprocess.CalledProcessError as e:
            raise DearOMGError(f"Conversion failed: {e.stderr}")
    
    def get_supported_formats(self) -> Dict[str, List[str]]:
        """
        Get the list of supported input formats.
        
        Returns:
            Dictionary mapping tool types to supported formats.
        """
        return {
            "community": [".d", ".imzML", ".fastq"],
            "vendor": [".wiff", ".raw"]
        }


class DearOMGParser:
    """
    Python wrapper for Dear-OMG data parsing.
    
    This class provides methods to parse and extract data from OMG files.
    """
    
    def __init__(self, dear_omg_path: Optional[str] = None):
        """
        Initialize the DearOMG parser.
        
        Args:
            dear_omg_path: Path to the Dear-OMG installation directory.
        """
        if dear_omg_path is None:
            dear_omg_path = os.getcwd()
        
        self.dear_omg_path = Path(dear_omg_path)
        self.parser_exe = self.dear_omg_path / "DearOMG-parser" / "bin" / "DearOMG-parser.exe"
        
        if not self.parser_exe.exists():
            raise DearOMGError(f"DearOMG-parser.exe not found at {self.parser_exe}")
    
    def parse_omg_file(self, omg_file: str, num_threads: int = 4, 
                      read_mode: str = "memory") -> Dict[str, any]:
        """
        Parse an OMG file and extract its contents.
        
        Args:
            omg_file: Path to the OMG file to parse.
            num_threads: Number of threads to use for parsing. Default: 4
            read_mode: Read mode, either "disk" or "memory". Default: "memory"
            
        Returns:
            Dictionary containing parsed data and metadata.
            
        Raises:
            DearOMGError: If parsing fails.
        """
        if not Path(omg_file).exists():
            raise DearOMGError(f"OMG file not found: {omg_file}")
        
        if read_mode not in ["disk", "memory"]:
            raise DearOMGError("Read mode must be either 'disk' or 'memory'")
        
        cmd = [
            str(self.parser_exe),
            f"--mt={num_threads}",
            f"--read={read_mode}",
            f"--omg={omg_file}"
        ]
        
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, check=True)
            
            # Parse the output to extract timing and entry information
            lines = result.stdout.strip().split('\n')
            
            parsing_info = {
                "omg_file": omg_file,
                "num_threads": num_threads,
                "read_mode": read_mode,
                "output": result.stdout,
                "success": True
            }
            
            # Extract timing information if available
            for line in lines:
                if "elapse time:" in line:
                    parts = line.split()
                    if len(parts) >= 4:
                        parsing_info["elapsed_time"] = float(parts[3])
                        parsing_info["threads_used"] = int(parts[1])
            
            return parsing_info
            
        except subprocess.CalledProcessError as e:
            raise DearOMGError(f"Parsing failed: {e.stderr}")
    
    def get_file_info(self, omg_file: str) -> Dict[str, any]:
        """
        Get basic information about an OMG file without full parsing.
        
        Args:
            omg_file: Path to the OMG file.
            
        Returns:
            Dictionary with basic file information.
        """
        if not Path(omg_file).exists():
            raise DearOMGError(f"OMG file not found: {omg_file}")
        
        file_stat = Path(omg_file).stat()
        
        return {
            "file_path": omg_file,
            "file_size_bytes": file_stat.st_size,
            "file_size_mb": round(file_stat.st_size / (1024 * 1024), 2),
            "last_modified": file_stat.st_mtime
        }


def convert_to_omg(input_files: Union[str, List[str]], 
                   output_dir: str,
                   dear_omg_path: Optional[str] = None,
                   **kwargs) -> Dict[str, str]:
    """
    Convenience function to convert files to OMG format.
    
    Args:
        input_files: Single file path or list of file paths to convert.
        output_dir: Directory to save the converted OMG files.
        dear_omg_path: Path to Dear-OMG installation directory.
        **kwargs: Additional arguments for conversion (write_mode, precision, skip_zero).
        
    Returns:
        Dictionary with conversion results.
    """
    converter = DearOMGConverter(dear_omg_path)
    return converter.convert_files(input_files, output_dir, **kwargs)


def parse_omg(omg_file: str, 
              dear_omg_path: Optional[str] = None,
              **kwargs) -> Dict[str, any]:
    """
    Convenience function to parse an OMG file.
    
    Args:
        omg_file: Path to the OMG file to parse.
        dear_omg_path: Path to Dear-OMG installation directory.
        **kwargs: Additional arguments for parsing (num_threads, read_mode).
        
    Returns:
        Dictionary with parsing results.
    """
    parser = DearOMGParser(dear_omg_path)
    return parser.parse_omg_file(omg_file, **kwargs)


# Example usage and testing
if __name__ == "__main__":
    # Example usage
    print("Dear-OMG Python API v1.0.0")
    print("Platform: Windows only")
    print("\nSupported formats:")
    
    try:
        converter = DearOMGConverter()
        formats = converter.get_supported_formats()
        for tool, exts in formats.items():
            print(f"  {tool}: {', '.join(exts)}")
    except DearOMGError as e:
        print(f"Error: {e}")
        print("Please ensure Dear-OMG executables are available in the current directory.")