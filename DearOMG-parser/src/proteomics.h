#ifndef PROTEOMICS_H
#define PROTEOMICS_H

#include "namespace.h"

OMGParser::EntryData OMGParser::ProteomicsEntry(int entryId)
{
	Char2Float char2float;
	Char2UInt32 char2uint32;
	Char2UInt64 char2uint64;

	uint64_t startPos = entryTable[entryId][0];
	uint64_t byteCount = entryTable[entryId][1];

	char* entryBuff = (char*)malloc(byteCount * sizeof(char));
	if (readMode == "disk")
	{
		filePtrPos = startPos;
		fsetpos(omgFilePtr, &filePtrPos);

		size_t readSize = fread(entryBuff, sizeof(char), byteCount, omgFilePtr);
	}
	else if (readMode == "memory")
	{
		memcpy(entryBuff, fullFileBuffer + startPos, byteCount);
	}

	std::string entryString(entryBuff, entryBuff + byteCount);
	free(entryBuff);

	EntryData entry;
	std::vector<char> mzCompData;
	std::vector<char> intCompData;;
	std::vector<char> mobiCompData;

	if (writeMode == "json" || writeMode == "yaml")
	{
		std::vector<std::string> keyList;
		std::vector<std::string> valueList;

		std::string title;
		PaserTextData(entryString, title, keyList, valueList);

		int pos = title.find('_');
		int loadEntry = std::stoi(title.substr(pos + 1));

		if (loadEntry != entryId)
		{
			std::cout << "[ERROR] Wrong in comparing load entryId and input entryId!" << std::endl;
			exit(0);
		}

		for (int n = 0; n < keyList.size(); ++n)
		{
			if (keyList[n] != "mz_arr" && 
				keyList[n] != "int_arr" &&
				keyList[n] != "mobilityIndex")
			{
				entry.numTypeKey.push_back(keyList[n]);

				std::vector<float> value = { std::stof(valueList[n]) };
				entry.numTypeValue.push_back(value);
			}
			if (keyList[n] == "mz_arr")
			{
				Base64Decode(valueList[n], mzCompData);
			}
			if (keyList[n] == "int_arr")
			{
				Base64Decode(valueList[n], intCompData);
			}
			if (keyList[n] == "mobilityIndex")
			{
				Base64Decode(valueList[n], mobiCompData);
			}
		}
	}
	else if (writeMode == "binary")
	{
		entry.numTypeKey = { "RT", "precursorMz", "collisionEnergy", 
			"scanIndex", "msLevel", "chargeState" };

		int mzBytes = -1;
		int intBytes = -1;
		int mobiBytes = -1;

		for (int n = 0; n < 9; ++n)
		{
			if (n < 3)
			{
				for (int i = 0; i < 4; ++i)
				{
					char2float.Char[i] = entryString[n * 4 + i];
				}
				std::vector<float> value = { char2float.Float };
				entry.numTypeValue.push_back(value);
			}
			else
			{
				for (int i = 0; i < 4; ++i)
				{
					char2uint32.Char[i] = entryString[n * 4 + i];
				}
				if (n < 6)
				{
					std::vector<float> value = { (float)char2uint32.UInt32 };
					entry.numTypeValue.push_back(value);
				}
				if (n == 6) mzBytes = char2uint32.UInt32;
				if (n == 7) intBytes = char2uint32.UInt32;
				if (n == 8) mobiBytes = char2uint32.UInt32;
			}
		}

		int offset = 9 * 4;
		mzCompData.assign(entryString.begin() + offset, entryString.begin() + offset + mzBytes);

		offset += mzBytes;
		intCompData.assign(entryString.begin() + offset, entryString.begin() + offset + intBytes);

		if (mobiBytes > 0)
		{
			offset += intBytes;
			mobiCompData.assign(entryString.begin() + offset, entryString.end());
		}
	}

	std::vector<char> mzDecompData;
	ZSTDDecode(mzCompData, mzDecompData);

	std::vector<uint32_t> mzEFCode;
	for (int j = 0; j < mzDecompData.size() / 4; ++j)
	{
		for (int k = 0; k < 4; ++k)
		{
			char2uint32.Char[k] = mzDecompData[j * 4 + k];
		}
		mzEFCode.push_back(char2uint32.UInt32);
	}

	std::vector<uint32_t> mzArrInt;
	EliasFanoDecode32(mzEFCode, mzArrInt);

	std::vector<float> mzArrFloat(mzArrInt.size());
	for (int j = 0; j < mzArrInt.size(); ++j)
	{
		mzArrFloat[j] = mzArrInt[j] / mzPrecision;
	}

	entry.numTypeKey.push_back("mz_arr");
	entry.numTypeValue.push_back(mzArrFloat);

	std::vector<char> intDecompData;
	ZSTDDecode(intCompData, intDecompData);

	std::vector<uint64_t> intEFCode;
	for (int j = 0; j < intDecompData.size() / 8; ++j)
	{
		for (int k = 0; k < 8; ++k)
		{
			char2uint64.Char[k] = intDecompData[j * 8 + k];
		}
		intEFCode.push_back(char2uint64.UInt64);
	}

	std::vector<uint64_t> intArrInt;
	EliasFanoDecode64(intEFCode, intArrInt);

	std::vector<float> intArrFloat(intArrInt.size());
	intArrFloat[0] = powf((float)intArrInt[0], 2.0f);

	for (int j = 1; j < intArrInt.size(); ++j)
	{
		intArrFloat[j] = powf((float)intArrInt[j] - (float)intArrInt[j - 1], 2.0f);
	}

	if (mzArrFloat.size() != intArrFloat.size())
	{
		std::cout << "[Error] The size of mzArr was not equal to intensityArr!" << std::endl;
		exit(0);
	}

	entry.numTypeKey.push_back("int_arr");
	entry.numTypeValue.push_back(intArrFloat);

	if (mobiCompData.size() > 0)
	{
		std::vector<char> mobiDecompData;
		ZSTDDecode(mobiCompData, mobiDecompData);

		std::vector<uint32_t> mobiEFCode;
		for (int j = 0; j < mobiDecompData.size() / 4; ++j)
		{
			for (int k = 0; k < 4; ++k)
			{
				char2uint32.Char[k] = mobiDecompData[j * 4 + k];
			}
			mobiEFCode.push_back(char2uint32.UInt32);
		}

		std::vector<uint32_t> mobiArrInt;
		EliasFanoDecode32(mobiEFCode, mobiArrInt);

		std::vector<uint32_t> mobiIndex(mobiArrInt.size());
		mobiIndex[0] = mobiArrInt[0];

		for (int j = 1; j < mobiArrInt.size(); ++j)
		{
			mobiIndex[j] = mobiArrInt[j] - mobiArrInt[j - 1];
		}

		int mIdx = -1;
		for (int j = 0; j < basicEntry.numTypeKey.size(); ++j)
		{
			if (basicEntry.numTypeKey[j] == "mobilityValue") mIdx = j;
		}
		if (mIdx == -1)
		{
			std::cout << "[ERROR] wrong at mobilityValue of basic info!\n";
			exit(0);
		}

		std::vector<float> mobiArrFloat(mobiIndex.size());
		for (int j = 0; j < mobiIndex.size(); ++j)
		{
			mobiArrFloat[j] = basicEntry.numTypeValue[mIdx][mobiIndex[j]];
		}

		if (mzArrFloat.size() != mobiArrFloat.size())
		{
			std::cout << "[Error] The size of mzArr was not equal to mobilityArr!" << std::endl;
			exit(0);
		}

		entry.numTypeKey.push_back("mobi_arr");
		entry.numTypeValue.push_back(mobiArrFloat);
	}

	return entry;
}

#endif // !PROTEOMICS_H
