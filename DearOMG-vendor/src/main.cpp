#pragma warning(disable:4996)
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

//#define DEBUG

#include "namespace.h"
#include "configure.h"
#include "proteomics.h"

#include "Sciex.h"
#include "ThermoFisher.h"

#if defined _WIN32
#pragma unmanaged
#endif

void DearOMG::RunConverter()
{
	int nThreads = 2 * std::thread::hardware_concurrency();
	ProteomeQueue.set_capacity(nThreads);

	std::cout << "===========================[Dear-OMG]=================================\n";

	for (int fid = 0; fid < inputFileList.size(); ++fid)
	{
		clock_t runStartTime = clock();

		std::string inputFile = inputFileList[fid];
		std::vector<std::string> nameSuffix = GetInputFileNameAndSuffix(inputFile);

		std::cout << "[INFO] Start convert file " << fid << ": " << inputFile << "\n";

		std::thread dataReader;
		if (nameSuffix[1] == "wiff")
		{
			dataReader = std::thread(&DearOMG::LoadWiffFile, this, inputFile);
		}
		else if (nameSuffix[1] == "raw")
		{
			dataReader = std::thread(&DearOMG::LoadThermoRawFile, this, inputFile);
		}
		else
		{
			std::cout << "[ERROR] Error format!\n"
				<< "[INFO] DearOMG-vendor.exe supports *.wiff and *.raw formats!\n";
			exit(0);
		}

		EncodeProteomics(nThreads, inputFile);

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