#ifndef NAMESPACE_H
#define NAMESPACE_H

#pragma comment(lib, "./lib/libzstd.lib")

#include <bitset>
#include <string>
#include <vector>
#include <thread>
#include <string.h>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "zstd.h"
#include "Base64.h"

class OMGParser
{
public:

	struct EntryData
	{
		std::vector<std::string> strTypeKey;
		std::vector<std::string> strTypeValue;

		std::vector<std::string> numTypeKey;
		std::vector< std::vector<float> > numTypeValue;
	};

	~OMGParser() 
	{
		fclose(omgFilePtr);
		if (writeMode == "memory")
		{
			free(fullFileBuffer); 
			fullFileBuffer = NULL;
		}
	}
	OMGParser(std::string omgFileName, std::string readMode);

	EntryData GetOMGFileBasicInfo();
	EntryData GetOMGFileEntry(int entryId);

	int GetEntryNumber();

private:

	FILE* omgFilePtr;
	std::string omgFileName = "";

	fpos_t filePtrPos;
	char* fullFileBuffer;

	std::string omics = "";
	std::string parentFormat = "";

	std::string readMode = "";
	std::string writeMode = "";
	std::string basicInfo = "";

	double mzPrecision = 1e3;
	int fastqBatchSize = 512;

	uint32_t entryCount;
	EntryData basicEntry;
	uint32_t genoReadLen;
	std::vector<uint32_t> offsetVector;
	std::vector< std::vector<uint64_t> > entryTable;

	union Char2Float
	{
		char Char[4];
		float Float;
	};

	union Char2Double
	{
		char Char[8];
		double Double;
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

	EntryData GenomicsEntry(int entryId);
	EntryData ProteomicsEntry(int entryId);
	EntryData MetabolomicsEntry(int entryId);

	void PaserTextData(std::string& text, std::string& title,
		std::vector<std::string>& keyList, std::vector<std::string>& valueList);

	void Base64Decode(std::string& input, std::vector<char>& output);
	void ZSTDDecode(std::vector<char>& input, std::vector<char>& output);
	void EliasFanoDecode32(std::vector<uint32_t>& EFCode, std::vector<uint32_t>& x);
	void EliasFanoDecode64(std::vector<uint64_t>& EFCode, std::vector<uint64_t>& x);
};

#endif // !NAMESPACE_H

