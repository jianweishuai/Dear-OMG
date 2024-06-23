#ifndef CONFIGURE_H
#define CONFIGURE_H

#include "namespace.h"

void DearOMG::ReadConfigure(int argc, char* argv[])
{
#ifndef DEBUG
	if (argc < 2)
	{
		std::cout << HELP_INFO << "\n";
		exit(0);
	}
	else if (argc == 2)
	{
		std::string inParam = argv[1];

		if (inParam.find("help") != inParam.npos ||
			inParam == "--h" || inParam == "-h")
		{
			std::cout << HELP_INFO << std::endl;
			exit(0);
		}
	}
	else
	{
		for (int i = 1; i < argc; ++i)
		{
			std::string line = argv[i];

			if (line.find("--write_mode") != line.npos)
			{
				int pos = line.find("=");
				writeMode = line.substr(pos + 1);
			}

			if (line.find("--precision") != line.npos)
			{
				int pos = line.find("=");
				mzPrecision = 1.0 / std::stod(line.substr(pos + 1));
			}

			if (line.find("--skip_zero") != line.npos)
			{
				int pos = line.find("=");
				std::string sub = line.substr(pos + 1);
				
				if (sub == "0")
				{
					skipZeroIntensity = false;
				}
				else
				{
					skipZeroIntensity = true;
				}
			}

			if (line.find("--out_dir") != line.npos)
			{
				int pos = line.find("=");
				outputDir = line.substr(pos + 1);

				for (int j = 0; j < outputDir.length(); ++j)
				{
					if (outputDir[j] == '\\') outputDir[j] = '/';
				}
				if (outputDir.back() != '/') outputDir += '/';
			}

			if (line.find("--input=") != line.npos)
			{
				int pos = line.find("=");

				std::string fileName = "";
				for (int j = pos + 1; j < line.length(); ++j)
				{
					if (line[j] == ';')
					{
						inputFileList.push_back(fileName);
						fileName = "";
						continue;
					}
					fileName += line[j];
				}
				if (fileName.length() > 0)
				{
					inputFileList.push_back(fileName);
				}
			}
		}

		if (outputDir.length() == 0)
		{
			printf("\nThere is no parameter of output directory: --out_dir\n");
			printf(HELP_INFO.c_str());
			exit(0);
		}
		if (inputFileList.size() == 0)
		{
			printf("\nThere is no parameter of input file: --input\n");
			printf(HELP_INFO.c_str());
			exit(0);
		}
		if (writeMode.length() == 0)
		{
			printf("\nThere is no parameter of write mode: --write_mode\n");
			printf(HELP_INFO.c_str());
			exit(0);
		}
		if (mzPrecision < 0.0f)
		{
			printf("\nThere is no parameter of write mode: --precision\n");
			printf(HELP_INFO.c_str());
			exit(0);
		}
	}

#endif // !DEBUG

#ifdef DEBUG

	outputDir = "F:/Dear-OMG/TestData/dearomg/";

	int mode = 3;

	switch (mode)
	{
	case 1:
		writeMode = "json";
		break;
	case 2:
		writeMode = "yaml";
		break;
	case 3:
		writeMode = "binary";
	default:
		break;
	}

	mzPrecision = 1e3;
	skipZeroIntensity = true;

	int omics = 1;
	switch (omics)
	{
	case 1:
		inputFileList = { "F:/Dear-OMG/BenchmarkData/Proteomics/LFQ_Orbitrap_AIF_Yeast_01.raw" };
		break;
	case 2:
		inputFileList = { "F:/Dear-OMG/BenchmarkData/Proteomics/LFQ_TTOF6600_ScanningSWATH_Human_01.wiff" };
		break;
	case 3:
		inputFileList = { "F:/Dear-OMG/BenchmarkData/Proteomics/LFQ_TTOF5600_SWATH_Human_01.wiff" };
		break;
	default:
		break;
	}

#endif // DEBUG
}


#endif // !CONFIGURE_H
