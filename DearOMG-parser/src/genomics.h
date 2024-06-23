#ifndef GENOMICS_H
#define GENOMICS_H

#include "namespace.h"

OMGParser::EntryData OMGParser::GenomicsEntry(int entryId)
{
	Char2UInt32 char2uint32;
	Char2UInt64 char2uint64;

	int readId = entryId;

	uint64_t startPos = entryTable[readId][0];
	uint64_t byteCount = entryTable[readId][1];

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

	int batchSize = -1;
	std::vector<std::string> keyList;
	std::vector<std::string> valueList;

	if (writeMode == "json" || writeMode == "yaml")
	{
		std::string title;
		PaserTextData(entryString, title, keyList, valueList);

		int pos1 = title.find("_") + 1;
		int pos2 = title.find("-");

		int st = std::stoi(title.substr(pos1, pos2 - pos1));
		int ed = std::stoi(title.substr(pos2 + 1));

		batchSize = ed - st + 1;
	}
	if (writeMode == "binary")
	{
		keyList = { "idKey", "idValue", "seq", "npos", "qKey", "qValue" };

		std::vector<uint32_t> params;
		for (int i = 0; i < 8; ++i)
		{
			for (int j = 0; j < 4; ++j)
			{
				char2uint32.Char[j] = entryString[i * 4 + j];
			}
			params.push_back(char2uint32.UInt32);
		}

		if (readId < params[0] || readId > params[1])
		{
			std::cout << "Error in decompress genomics data on binary format!" << std::endl;
		}

		batchSize = params[1] - params[0] + 1;

		uint32_t stPos = 32;
		for (int i = 2; i < params.size(); ++i)
		{
			valueList.push_back(entryString.substr(stPos, params[i]));
			stPos += params[i];
		}
	}
	if (batchSize == -1)
	{
		std::cout << "Error in decode batch size!" << std::endl;
		exit(0);
	}

	std::vector<uint32_t> identifierValue;
	std::vector< std::vector<std::string> > idKeyList;

	std::string baseSequence = "";
	std::vector<uint32_t> NBasePos;

	std::vector<char> qKey;
	std::vector<uint32_t> qualityArr;

	for (int n = 0; n < keyList.size(); ++n)
	{
		if (keyList[n] == "idKey")
		{
			std::string token = "";
			for (int i = 0; i < valueList[n].length(); ++i)
			{
				if (valueList[n][i] == ';' || i == valueList[n].length() - 1)
				{
					if (i == valueList[n].length() - 1) token += valueList[n][i];

					std::string k = "";
					std::vector<std::string> columnKey;
					for (int j = 0; j < token.length(); ++j)
					{
						if (token[j] == ',')
						{
							columnKey.push_back(k);
							k = "";
						}
						k += token[j];
					}
					if (k.length() > 0) columnKey.push_back(k);

					idKeyList.push_back(columnKey);
					token = "";

					continue;
				}
				token += valueList[n][i];
			}
		}
		if (keyList[n] == "idValue")
		{
			std::vector<char> src;
			if (writeMode == "json" || writeMode == "yaml")
			{
				Base64Decode(valueList[n], src);
			}
			if (writeMode == "binary")
			{
				src.assign(valueList[n].begin(), valueList[n].end());
			}

			std::vector<uint32_t> idEFCode;
			for (int j = 0; j < src.size() / 4; ++j)
			{
				for (int k = 0; k < 4; ++k)
				{
					char2uint32.Char[k] = src[j * 4 + k];
				}
				idEFCode.push_back(char2uint32.UInt32);
			}

			std::vector<uint32_t> idDecodeValue;
			EliasFanoDecode32(idEFCode, idDecodeValue);

			identifierValue.resize(idDecodeValue.size());
			identifierValue[0] = idDecodeValue[0];

			for (int j = 1; j < idDecodeValue.size(); ++j)
			{
				identifierValue[j] = idDecodeValue[j] - idDecodeValue[j - 1];
			}
		}
		if (keyList[n] == "seq")
		{
			std::vector<char> src;
			if (writeMode == "json" || writeMode == "yaml")
			{
				Base64Decode(valueList[n], src);
			}
			if (writeMode == "binary")
			{
				src.assign(valueList[n].begin(), valueList[n].end());
			}

			std::vector<char> seqDecode;
			ZSTDDecode(src, seqDecode);

			for (int i = 0; i < seqDecode.size(); ++i)
			{
				std::bitset<8> bitChar(seqDecode[i]);

				for (int j = 0; j < 4; ++j)
				{
					if (bitChar.test(7 - 2 * j) == false && bitChar.test(6 - 2 * j) == false) baseSequence += "A";
					if (bitChar.test(7 - 2 * j) == false && bitChar.test(6 - 2 * j) == true) baseSequence += "T";
					if (bitChar.test(7 - 2 * j) == true && bitChar.test(6 - 2 * j) == false) baseSequence += "G";
					if (bitChar.test(7 - 2 * j) == true && bitChar.test(6 - 2 * j) == true) baseSequence += "C";
				}
			}
		}
		if (keyList[n] == "npos")
		{
			if (valueList[n].length() > 0)
			{
				std::vector<char> src;
				if (writeMode == "json" || writeMode == "yaml")
				{
					Base64Decode(valueList[n], src);
				}
				if (writeMode == "binary")
				{
					src.assign(valueList[n].begin(), valueList[n].end());
				}

				for (int j = 0; j < src.size() / 4; ++j)
				{
					for (int k = 0; k < 4; ++k)
					{
						char2uint32.Char[k] = src[j * 4 + k];
					}
					NBasePos.push_back(char2uint32.UInt32);
				}
			}
		}
		if (keyList[n] == "qKey")
		{
			qKey.assign(valueList[n].begin(), valueList[n].end());
		}
		if (keyList[n] == "qValue")
		{
			if (qKey.size() > 1)
			{
				std::vector<char> qualityCompData;
				if (writeMode == "json" || writeMode == "yaml")
				{
					Base64Decode(valueList[n], qualityCompData);
				}
				if (writeMode == "binary")
				{
					qualityCompData.assign(valueList[n].begin(), valueList[n].end());
				}

				std::vector<char> qualityCharData;
				ZSTDDecode(qualityCompData, qualityCharData);

				std::vector<uint32_t> qualityEFCode;
				for (int i = 0; i < qualityCharData.size() / 4; ++i)
				{
					for (int j = 0; j < 4; ++j)
					{
						char2uint32.Char[j] = qualityCharData[i * 4 + j];
					}
					qualityEFCode.push_back(char2uint32.UInt32);
				}

				std::vector<uint32_t> qualityAccumulateArr;
				EliasFanoDecode32(qualityEFCode, qualityAccumulateArr);

				qualityArr.resize(qualityAccumulateArr.size());
				qualityArr[0] = qualityAccumulateArr[0];

				for (int i = 1; i < qualityAccumulateArr.size(); ++i)
				{
					qualityArr[i] = qualityAccumulateArr[i] - qualityAccumulateArr[i - 1];
				}

			}
		}

	}

	EntryData entry;

	std::string line = "";
	for (int i = 0; i < batchSize; ++i)
	{
		line = "";
		int c = 0;
		for (int j = 0; j < idKeyList.size(); ++j)
		{
			if (idKeyList[j].size() == 1)
			{
				if (idKeyList[j][0] == "\t")
				{
					if (line.back() == ':') line.pop_back();
					line += " ";
				}
				else if (idKeyList[j][0] == "?")
				{
					line += std::to_string(identifierValue[c * batchSize + i]) + ":";
					++c;
				}
				else
				{
					line += idKeyList[j][0] + ":";
				}
			}
			else
			{
				int k = identifierValue[c * batchSize + i];
				line += idKeyList[j][k] + ":";
				++c;
			}
		}
		if (line.back() == ':') line.pop_back();

		entry.strTypeKey.push_back("identifier");
		entry.strTypeValue.push_back(line);
	}

	for (int i = 0; i < NBasePos.size(); ++i)
	{
		baseSequence[NBasePos[i]] = 'N';
	}

	for (int i = 0; i < batchSize; ++i)
	{
		std::string seq(baseSequence, i * genoReadLen, genoReadLen);
		
		entry.strTypeKey.push_back("sequence");
		entry.strTypeValue.push_back(seq);
	}

	if (qKey.size() > 1)
	{
		std::string qualityStr = "";
		for (int i = 0; i < qualityArr.size() / 2; ++i)
		{
			for (int j = 0; j < qualityArr[2 * i + 1]; ++j)
			{
				qualityStr += qKey[qualityArr[2 * i]];
			}
		}

		for (int i = 0; i < batchSize; ++i)
		{
			std::string quality(qualityStr, i * genoReadLen, genoReadLen);
			
			entry.strTypeKey.push_back("quality");
			entry.strTypeValue.push_back(quality);
		}
	}
	else
	{
		for (int i = 0; i < batchSize; ++i)
		{
			std::string quality(genoReadLen, qKey[0]);

			entry.strTypeKey.push_back("quality");
			entry.strTypeValue.push_back(quality);
		}
	}

	return entry;
}

#endif // !GENOMICS_H
