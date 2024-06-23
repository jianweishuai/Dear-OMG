## <p align="center">Dear-OMG: an omics-general compression method for genomics, proteomics and metabolomics data</p> 

## Introduction
- Dear-OMG, a unified, compact, flexible, and high-performance metadata storage solution. 
- Dear-OMG introduces a novel file storage structure and utilizes the Elias-Fano encoding algorithm to compress and store proteomics, genomics, and metabolomics metadata into the unified OMG format. 
- The OMG format not only demonstrates remarkably **high compression and decompression speeds**, but also **enables parallel random access** to any data block. 
- Test results reveal that, compared to the commonly used proteomics formats of mzXML and mzML, the OMG format achieves an **80% reduction in storage space**, a **90% decrease in conversion time**, and approximately a **10-fold speed improvement** with support for parallel random access.

## DearOMG Usage

- DearOMG-community.exe use for *.d, *.imzML and *.fastq files.
- DearOMG-vendor.exe use for *.wiff and *.raw files.

```
 Development team from Xiamen University.

 Welcome to use DearOMGv1.0.0 software.

 The vendor version only run on Windows system!

 Usage: DearOMG.exe --write_mode=binary --precision=0.001 --skip_zero=1 --out_dir=/path/to/output_dir/ --input=xx.wiff;yy.raw;zz.d


Introduction of args:

--write_mode:   data storage type. DearOMG support binary, json, yaml. defalut=binary.

--precision:    precision of m/z array. defalut=0.001.

--skip_zero:    skip ions with zero intensity or not. true for 1 and false for 0. defalut=1.

--out_dir:      output directory or path.

--input:        input file list. DearOMG supports *.wiff, *.raw, *.d, *.imzML, *.fastq formats.

Note: the paths of input files and output directory should not include 'Space'.
```

## DearOMG-parser Usage
```
DearOMG-parser.exe --mt=4 --read=memory --omg=xxx.omg

--mt: the number of threads
--read: read mode (disk or memory)
--omg: omg file name
```
