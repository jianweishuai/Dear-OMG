#ifndef UTILITY_H
#define UTILITY_H

#include "namespace.h"

std::string DearOMG::GetTime()
{
	time_t now = time(0);
	tm* ltm = localtime(&now);

	std::string date = std::to_string(1900 + ltm->tm_year) + "-" +
		std::to_string(1 + ltm->tm_mon) + "-" +
		std::to_string(ltm->tm_mday) + "_" +
		std::to_string(ltm->tm_hour) + "-" + 
		std::to_string(ltm->tm_min) + "-" +
		std::to_string(ltm->tm_sec);

	return date;
}

void DearOMG::ZSTDEncode(std::vector<char>& input, std::vector<char>& output)
{
	int bound = ZSTD_compressBound(input.size());
	output.resize(bound);

	int compSize = ZSTD_compress(output.data(), bound, input.data(), input.size(), 1);
	output.resize(compSize);
}

void DearOMG::Base64Encode(std::vector<char>& input, std::vector<char>& output)
{
	size_t b2TSize = Base64::binaryToTextSize(input.size());
	output.resize(b2TSize);

	size_t count1 = Base64::binaryToText(&input[0], input.size(), &output[0]);
}

std::vector<std::string> DearOMG::GetInputFileNameAndSuffix(std::string inputFileName)
{
	std::string name = "";
	std::string rev_name = "";

	std::string suffix = "";
	std::string rev_suffix = "";

	bool flag = false;
	for (int i = inputFileName.length() - 1; i >= 0; --i)
	{
		if (inputFileName[i] == '/' ||
			inputFileName[i] == '\\')
		{
			break;
		}
		if (inputFileName[i] == '.')
		{
			flag = true;
			continue;
		}
		if (!flag)
		{
			rev_suffix += inputFileName[i];
		}

		if (flag)
		{
			rev_name += inputFileName[i];
		}
	}
	for (int i = rev_suffix.length() - 1; i >= 0; --i)
	{
		suffix += rev_suffix[i];
	}
	for (int i = rev_name.length() - 1; i >= 0; --i)
	{
		name += rev_name[i];
	}

	std::vector<std::string> nameSuffix(2);
	nameSuffix[0] = name;
	nameSuffix[1] = suffix;

	return nameSuffix;
}

void DearOMG::ReWriteOMGFile(std::string& inputFile, 
	std::string& fileName, std::string& baseInfo,
	std::vector< std::vector<uint64_t> >& offsetVectorTmp)
{
	std::string tmpFileName = outputDir + fileName + ".omg.tmp";

	FILE* tmpFile = fopen(tmpFileName.c_str(), "rb");

	if (!tmpFile)
	{
		std::cout << "[ERROR] Cannot create " << tmpFileName << "\n"
			<< "[ERROR] Please check your file or directory!" << std::endl;
		exit(0);
	}

	std::string omgFileName = outputDir + fileName + ".omg";

	FILE* omgFile = fopen(omgFileName.c_str(), "wb");

	if (!omgFile)
	{
		std::cout << "[ERROR] Cannot create " << omgFileName << "\n"
			<< "[ERROR] Please check your file or directory!" << std::endl;
		exit(0);
	}

	fwrite(baseInfo.c_str(), 1, baseInfo.length(), omgFile);

	fpos_t ptrPos;

	std::vector<uint32_t> offsetVector;
	offsetVector.push_back((uint32_t)baseInfo.length());

	for (int i = 0; i < offsetVectorTmp.size(); ++i)
	{
		offsetVector.push_back((uint32_t)offsetVectorTmp[i][2]);

#ifdef _WIN32
		ptrPos = offsetVectorTmp[i][1];
#endif // !_WIN32

#ifdef __linux__
		ptrPos.__pos = offsetVectorTmp[i][1];
#endif // __linux__

		fsetpos(tmpFile, &ptrPos);

		char* buff = (char*)malloc(offsetVectorTmp[i][2]);
		size_t size = fread(buff, sizeof(char), offsetVectorTmp[i][2], tmpFile);

		size_t writeLen = fwrite(buff, sizeof(char), offsetVectorTmp[i][2], omgFile);

		free(buff);
	}

	Char2UInt32 char2uint32;
	std::vector<char> offsetCharVec(offsetVector.size() * sizeof(uint32_t));

	for (int i = 0; i < offsetVector.size(); ++i)
	{
		char2uint32.UInt32 = offsetVector[i];
		for (int j = 0; j < 4; ++j)
		{
			offsetCharVec[i * 4 + j] = char2uint32.Char[j];
		}
	}

	char* offsetCode = (char*)malloc(2 * offsetCharVec.size());
	size_t writeLen = Base64::binaryToText(offsetCharVec.data(), offsetCharVec.size(), offsetCode);

	std::string offsetString(offsetCode, offsetCode + writeLen);
	free(offsetCode);

	std::string writeOffset = "";

	if (writeMode == "json")
	{
		writeOffset = " \"offsetArr\": \"" + offsetString + "\"\n";

		std::string writeOffsetLen = " \"offsetLen\":";
		std::string len = std::to_string(writeOffset.length());

		writeOffsetLen += len;

		int res = 64 - writeOffsetLen.length() - 2;
		for (int i = 0; i < res; ++i)
		{
			writeOffsetLen += ' ';
		}
		writeOffsetLen += "\n}";

		writeOffset += writeOffsetLen;
	}
	if (writeMode == "yaml" || writeMode == "binary")
	{
		writeOffset = "offsetArr: " + offsetString + "\n";

		std::string writeOffsetLen = "offsetLen:";
		std::string len = std::to_string(writeOffset.length());

		writeOffsetLen += len;

		int res = 64 - writeOffsetLen.length() - 1;
		for (int i = 0; i < res; ++i)
		{
			writeOffsetLen += ' ';
		}
		writeOffsetLen += "\n";

		writeOffset += writeOffsetLen;
	}

	fwrite(writeOffset.data(), sizeof(char), writeOffset.length(), omgFile);

	fclose(tmpFile);
	fclose(omgFile);

	if (remove(tmpFileName.c_str()) == 0)
	{
		std::cout << "[INFO] Delete .tmp file success!" << std::endl;
	}
	else
	{
		std::cout << "[WARNING] Cannot delete .tmp file!" << std::endl;
	}
}


#endif // !UTILITY_H

