#ifndef OMGPARSER_H
#define OMGPARSER_H

#include "namespace.h"

OMGParser::OMGParser(std::string omgFileName, std::string readMode)
{
	if (readMode != "disk" && readMode != "memory")
	{
		std::cout << "[ERROR] Please select the read mode as \"disk\" or \"memory\"!\n";
		exit(0);
	}

	this->readMode = readMode;
	this->omgFileName = omgFileName;

	omgFilePtr = fopen(omgFileName.c_str(), "rb");
	if (!omgFilePtr)
	{
		std::cout << "[ERROR] Cannot open " << omgFileName << ".\n"
			<< "[ERROR] Please check your file or directory!\n";
		exit(0);
	}

	struct __stat64 fileStat;

	_stat64(omgFileName.c_str(), &fileStat);
	int64_t omgFileVolume = fileStat.st_size; // Bytes

	if (readMode == "memory")
	{
		fullFileBuffer = (char*)malloc(omgFileVolume * sizeof(char));
		int64_t fullSize = fread(fullFileBuffer, sizeof(char), omgFileVolume, omgFilePtr);

		if (fullSize != omgFileVolume)
		{
			std::cout << "[ERROR] Something was wrong in fread omgfile!\n";
			exit(0);
		}
	}

	char* offsetLenArr = (char*)malloc(64 * sizeof(char));

	if (readMode == "disk")
	{
#ifdef _WIN32
		filePtrPos = omgFileVolume - 64;
#endif // !_WIN32

#ifdef __linux__
		filePtrPos.__pos = omgFileVolume - 64;
#endif // __linux__

		fsetpos(omgFilePtr, &filePtrPos);
		size_t size = fread(offsetLenArr, sizeof(char), 64, omgFilePtr);
	}
	else if (readMode == "memory")
	{
		memcpy(offsetLenArr, fullFileBuffer + omgFileVolume - 64, 64);
	}

	bool flag = false;
	std::string lenStr = "";
	for (int i = 0; i < 64; ++i)
	{
		if (flag && offsetLenArr[i] == ' ')
		{
			break;
		}

		if (flag)
		{
			if (offsetLenArr[i] == '\"') continue;
			lenStr += offsetLenArr[i];
		}

		if (offsetLenArr[i] == ':')
		{
			flag = true;
			continue;
		}
	}

	free(offsetLenArr);

	int offsetLen = std::stoi(lenStr);

	char* offsetVecChar = (char*)malloc(offsetLen * sizeof(char));

	if (readMode == "disk")
	{
#ifdef _WIN32
		filePtrPos = omgFileVolume - offsetLen - 64;
#endif // !_WIN32

#ifdef __linux__
		filePtrPos.__pos = omgFileVolume - offsetLen - 64;
#endif // __linux__

		fsetpos(omgFilePtr, &filePtrPos);
		size_t size = fread(offsetVecChar, sizeof(char), offsetLen, omgFilePtr);
	}
	else if (readMode == "memory")
	{
		memcpy(offsetVecChar, fullFileBuffer + omgFileVolume - offsetLen - 64, offsetLen);
	}

	flag = false;
	std::string offsetBase64Code = "";
	for (int i = 0; i < offsetLen; ++i)
	{
		if (offsetVecChar[i] == '\n') break;

		if (flag)
		{
			if (offsetVecChar[i] == '\"' || offsetVecChar[i] == ' ') continue;
			offsetBase64Code += offsetVecChar[i];
		}

		if (offsetVecChar[i] == ':')
		{
			flag = true;
			continue;
		}
	}
	free(offsetVecChar);

	size_t size = Base64::textToBinarySize(offsetBase64Code.length());
	unsigned char* result = (unsigned char*)malloc(size * sizeof(unsigned char));
	Base64::textToBinary(offsetBase64Code.c_str(), offsetBase64Code.length(), result);

	Char2UInt32 char2uint32;

	for (int i = 0; i < size / 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			char2uint32.Char[j] = result[i * 4 + j];
		}
		offsetVector.push_back(char2uint32.UInt32);
	}
	free(result);

	uint64_t startPos = offsetVector[0];

	for (int i = 1; i < offsetVector.size(); ++i)
	{
		std::vector<uint64_t> tmp = { startPos, offsetVector[i] };
		entryTable.push_back(tmp);
		startPos += offsetVector[i];
	}

	uint32_t basicInfoLen = offsetVector[0];
	char* basicInfoBuf = (char*)malloc(basicInfoLen * sizeof(char));

	if (readMode == "disk")
	{
		filePtrPos = 0;
		fsetpos(omgFilePtr, &filePtrPos);

		size_t readSize = fread(basicInfoBuf, sizeof(char), basicInfoLen, omgFilePtr);
	}
	else if (readMode == "memory")
	{
		memcpy(basicInfoBuf, fullFileBuffer, basicInfoLen);
	}

	for (int i = 0; i < basicInfoLen; ++i)
	{
		if (basicInfoBuf[i] == '{' || basicInfoBuf[i] == '}' ||
			basicInfoBuf[i] == '\"' || basicInfoBuf[i] == ',' || basicInfoBuf[i] == ' ')
		{
			continue;
		}
		basicInfo += basicInfoBuf[i];
	}
	free(basicInfoBuf);

	basicEntry = GetOMGFileBasicInfo();

	for (int i = 0; i < basicEntry.strTypeKey.size(); ++i)
	{
		if (basicEntry.strTypeKey[i] == "omics")
		{
			this->omics = basicEntry.strTypeValue[i];
		}
		if (basicEntry.strTypeKey[i] == "writeFormat")
		{
			this->writeMode = basicEntry.strTypeValue[i];
		}
		if (basicEntry.strTypeKey[i] == "parentFormat")
		{
			this->parentFormat = basicEntry.strTypeValue[i];
		}
	}

	if (this->omics == "Genomics")
	{
		for (int i = 0; i < basicEntry.strTypeKey.size(); ++i)
		{
			if (basicEntry.strTypeKey[i] == "readCount")
			{
				this->entryCount = std::stoi(basicEntry.strTypeValue[i]);
			}
			if (basicEntry.strTypeKey[i] == "readLength")
			{
				this->genoReadLen = std::stoi(basicEntry.strTypeValue[i]);
			}
		}
	}
	else
	{
		for (int i = 0; i < basicEntry.strTypeKey.size(); ++i)
		{
			if (basicEntry.strTypeKey[i] == "scanCount")
			{
				this->entryCount = std::stoi(basicEntry.strTypeValue[i]);
				break;
			}
		}
	}
}


#endif // !OMGPARSER_H
