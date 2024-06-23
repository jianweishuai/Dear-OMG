#pragma warning(disable:4996)
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

//#define DEBUG

#include "namespace.h"
#include "configure.h"

#include "genomics.h"
#include "proteomics.h"
#include "metabolomics.h"

#include "Fastq.h"
#include "ImzML.h"
#include "Bruker.h"

void DearOMG::RunConverter()
{
	int nThreads = 2 * std::thread::hardware_concurrency();
	protQueue.set_capacity(nThreads);

	std::cout << "===========================[Dear-OMG]=================================\n";

	for (int fid = 0; fid < inputFileList.size(); ++fid)
	{
		clock_t runStartTime = clock();

		std::string inputFile = inputFileList[fid];
		std::vector<std::string> nameSuffix = GetInputFileNameAndSuffix(inputFile);

		std::cout << "[INFO] Start convert file " << fid << ": " << inputFile << "\n";

		std::thread dataReader;
		if (nameSuffix[1] == "d")
		{
			GetBrukerTDFBaseInfo(inputFile);
			dataReader = std::thread(&DearOMG::LoadBrukerTDFFile, this, inputFile);
			EncodeProteomics(nThreads, inputFile);
		}
		else if (nameSuffix[1] == "fastq")
		{
			dataReader = std::thread(&DearOMG::LoadFastqFile, this, inputFile);
			EncodeGenomics(nThreads, inputFile);
		}
		else if (nameSuffix[1] == "imzML")
		{
			dataReader = std::thread(&DearOMG::LoadImzMLFile, this, inputFile);
			EncodeMetabolomics(nThreads, inputFile);
		}
		else
		{
			std::cout << "[ERROR] Error format!\n"
				<< "[INFO] DearOMG-community.exe supports *.d, *.imzML and *.faseq formats!\n";
			exit(0);
		}
		
		dataReader.join();

		std::cout << "[INFO] Done! Conversion time: " << (double)(clock() - runStartTime) / CLOCKS_PER_SEC << " seconds." << std::endl;

		if (inputFileList.size() > 1)
		{
			std::cout << "============================================================\n";
		}
	}

	std::cout << "===========================[Xiamen University]=================================\n";
}

int main(int argc, char* argv[])
{
	DearOMG dearomg(argc, argv);
	dearomg.RunConverter();

	return 0;
}