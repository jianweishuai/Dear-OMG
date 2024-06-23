#ifndef NAMESPACE_H
#define NAMESPACE_H

#if defined _WIN32
#pragma comment(lib, "./lib/tbb.lib")
#pragma comment(lib, "./lib/libzstd.lib")
#endif

#if defined _WIN32
#pragma unmanaged
#endif

#include <ctime>
#include <cmath>
#include <bitset>
#include <vector>
#include <string>
#include <thread>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "zstd.h"
#include "Base64.h"
#include "tbb/tbb.h"

class DearOMG
{
public:
	DearOMG(int argc, char* argv[])
	{
		std::cout << SOFTWARE_INFO << "\n";
		ReadConfigure(argc, argv);
	}
	
	void RunConverter();

	//=========================================================================
	// Custom parameters
	//=========================================================================
	std::string writeMode;
	std::string outputDir;
	std::vector<std::string> inputFileList;

	double mzPrecision = -1.0;
	bool skipZeroIntensity = true;

private:
	//=========================================================================
	// Information of software
	//=========================================================================
	std::string LOGO = "";
	
	std::string SOFTWARE_VERSION = "v1.0.0";
	std::string SOFTWARE_NAME = "DearOMG";

	std::string SOFTWARE_INFO = LOGO +
		"\n Development team from Xiamen University.\n" +
		"\n Welcome to use " + SOFTWARE_NAME + SOFTWARE_VERSION + " software.\n" +
		"\n The vendor version only run on Windows system!\n" +
		"\n Usage: " + SOFTWARE_NAME + ".exe --write_mode=binary --precision=0.001 --skip_zero=1 --out_dir=/path/to/output_dir/ --input=xx.wiff;yy.raw;zz.d\n";

	std::string HELP_INFO = "\nIntroduction of args:\n"
		"\n--write_mode:\tdata storage type. " + SOFTWARE_NAME + " support binary, json, yaml. defalut=binary.\n"
		"\n--precision:\tprecision of m/z array. defalut=0.001.\n"
		"\n--skip_zero:\tskip ions with zero intensity or not. true for 1 and false for 0. defalut=1.\n"
		"\n--out_dir:\toutput directory or path.\n" +
		"\n--input:\tinput file list. " + SOFTWARE_NAME + " supports *.wiff, *.raw, *.d, *.imzML, *.fastq formats.\n" +
		"\nNote: the paths of input files and output directory should not include 'Space'.\n\n";

	//=====================================================================//
	// Data structures
	//=====================================================================//
	struct ProtHeader
	{
		std::string sampleName = "";
		std::string endTime = "UnKnown";
		std::string startTime = "UnKnown";
		std::string scanCount = "UnKnown";

		std::string schemaType = "None";
		std::string mobolityValues = "None";

		std::string msModel = "UnKnown";
		std::string msDetector = "UnKnown";
		std::string msIonisation = "UnKnown";
		std::string msManufacturer = "UnKnown";
		std::string msMassAnalyzer = "UnKnown";
	};
	
	struct ProtData
	{
		bool stop = false;
		uint32_t scanIndex;

		float rt;
		int charge;
		int cycleId;
		int msLevel = -1;
		float precursorMz;

		std::string info;
		std::string mzEncode;
		std::string intensityEncode;

		std::vector< std::vector<double> > spectrum;
	};

	union Char2Float
	{
		char Char[4];
		float Float;
	};

	union Char2UInt32
	{
		char Char[4];
		uint32_t UInt32;
	};

	union Char2UInt64
	{
		char Char[8];
		uint64_t UInt64;
	};

	ProtHeader protHeader;
	tbb::concurrent_bounded_queue<ProtData> ProteomeQueue;

	//=====================================================================//
	// Core functions
	//=====================================================================//
	void ReadConfigure(int argc, char* argv[]);

	void EliasFanoEncode32(std::vector<uint32_t>& x, std::vector<uint32_t>& EFCode);
	void EliasFanoDecode32(std::vector<uint32_t>& EFCode, std::vector<uint32_t>& x);

	void EliasFanoEncode64(std::vector<uint64_t>& x, std::vector<uint64_t>& EFCode);
	void EliasFanoDecode64(std::vector<uint64_t>& EFCode, std::vector<uint64_t>& x);

	void LoadWiffFile(std::string inputFile);
	void LoadThermoRawFile(std::string inputFile);

	void EncodeProteomics(int nThreads, std::string inputFile);

	//=====================================================================//
	// Utilities
	//=====================================================================//
	std::string GetTime();
	void ZSTDEncode(std::vector<char>& input, std::vector<char>& output);
	void Base64Encode(std::vector<char>& input, std::vector<char>& output);

	void ReWriteOMGFile(std::string& inputFile,
		std::string& fileName, std::string& baseInfo,
		std::vector< std::vector<uint64_t> >& offsetVectorTmp);

	std::vector<std::string> GetInputFileNameAndSuffix(std::string inputFileName);

};


#endif // !NAMESPACE_H
