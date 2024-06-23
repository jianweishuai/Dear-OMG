#ifndef NAMESPACE_H
#define NAMESPACE_H

#if defined _WIN32
#pragma comment(lib, "./lib/tbb.lib")
#pragma comment(lib, "./lib/libzstd.lib")
#pragma comment(lib, "./lib/zlibstat.lib")
//#pragma comment(lib, "./lib/sqlite3_static.lib")
#pragma comment(lib, "./lib/sqlite3.lib")
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
#include "zlib.h"
#include "zconf.h"
#include "Base64.h"
#include "sqlite3.h"
#include "tbb/tbb.h"
#include "tsl/robin_map.h"
#include "tsl/robin_set.h"

#define	MB 1048576

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
	bool writeMobility = false;
	bool skipZeroIntensity = true;

	int fastqBatchSize = 512;
	size_t fastqBufferSize = 10 * MB;
	size_t xmlBufferSize = 10 * MB;

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

	struct GenoHeader
	{
		std::string readCount = "";
		std::string readLength = "";
	};

	struct MetaHeader
	{
		std::string imzMLModel = "";
		std::string continuousMzCode = "";

		std::string mzEncodeType = "";
		std::string mzCompressType = "";

		std::string intenEncodeType = "";
		std::string intenCompressType = "";

		std::string spectrumCount = "";

		// scan settings
		std::string maxCountofPixel_X = "";
		std::string maxCountofPixel_Y = "";
		std::string maxDimension_X = "";
		std::string maxDimension_Y = "";
		std::string pixelSize_X = "";
		std::string pixelSize_Y = "";

		// instrument configuration
		std::string msModel = "UnKnown";
		std::string msDetector = "UnKnown";
		std::string msIonisation = "UnKnown";
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
		float collisionEnergy;

		std::string info;
		std::string mzEncode;
		std::string mobilityEncode;
		std::string intensityEncode;

		std::vector<uint32_t> mobilityIndex;
		std::vector< std::vector<double> > spectrum;
	};

	struct GenoData // save to disk
	{
		bool stop = false;

		uint32_t readEndId;
		uint32_t readStartId;
		std::vector<std::string> readsData;
	};

	struct MetaData
	{
		bool stop = false;

		uint32_t scanIndx;
		int arrayLength = -1;
		double minIntensity = 0.0;

		std::string spectrumId = "";

		std::string position_x = "NA";
		std::string position_y = "NA";

		std::string position3D_x = "NA";
		std::string position3D_y = "NA";
		std::string position3D_z = "NA";

		std::string info;
		std::string mzEncode;
		std::string intensityEncode;

		std::vector< std::vector<unsigned char> > encodeSpectrum;
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
	GenoHeader genoHeader;
	MetaHeader metaHeader;

	tbb::concurrent_bounded_queue<GenoData> genoQueue;
	tbb::concurrent_bounded_queue<MetaData> metaQueue;
	tbb::concurrent_bounded_queue<ProtData> protQueue;

	//=====================================================================//
	// Core functions
	//=====================================================================//
	void ReadConfigure(int argc, char* argv[]);

	void EliasFanoEncode32(std::vector<uint32_t>& x, std::vector<uint32_t>& EFCode);
	void EliasFanoDecode32(std::vector<uint32_t>& EFCode, std::vector<uint32_t>& x);

	void EliasFanoEncode64(std::vector<uint64_t>& x, std::vector<uint64_t>& EFCode);
	void EliasFanoDecode64(std::vector<uint64_t>& EFCode, std::vector<uint64_t>& x);

	void LoadFastqFile(std::string inputFile);

	void LoadBrukerTDFFile(std::string inputFolder);
	void GetBrukerTDFBaseInfo(std::string inputFolder);

	void LoadImzMLFile(std::string inputFile);
	void GetImzMLHeaderInfo(std::string inputFile);
	std::string FindImzMLKeyValue(const std::string& in_str, const std::string& key);

	void DecodeImzMLData(std::vector<unsigned char>& encodeSpectrum,
		std::vector<double>& decodeSpectrum,
		int arrayLength, std::string compressType, std::string EncodeType);

	void EncodeGenomics(int nThreads, std::string inputFile);
	void EncodeProteomics(int nThreads, std::string inputFile);
	void EncodeMetabolomics(int nThreads, std::string inputFile);

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
