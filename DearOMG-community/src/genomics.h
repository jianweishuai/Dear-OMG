#ifndef GENOMICS_H
#define GENOMICS_H

#include "namespace.h"
#include "eliasfano.h"
#include "utility.h"

void DearOMG::EncodeGenomics(int nThreads, std::string inputFile)
{
	std::vector<std::string> nameSuffix = GetInputFileNameAndSuffix(inputFile);

	std::string tmpFileName = outputDir + nameSuffix[0] + ".omg.tmp";

	FILE* tmpFile = fopen(tmpFileName.c_str(), "wb");

	if (!tmpFile)
	{
		std::cout << "[ERROR] Cannot create " << tmpFileName << "\n"
			<< "[ERROR] Please check your file or directory!" << std::endl;
		exit(0);
	}

	uint64_t startPos = 0;
	std::vector< std::vector<uint64_t> > offsetVectorTmp;

	tbb::parallel_pipeline(nThreads,
		tbb::make_filter<void, GenoData>(tbb::filter::serial_out_of_order,
			[&](tbb::flow_control& fc)
			{
				GenoData genoData;
				genoQueue.pop(genoData);

				if (genoData.stop)
				{
					fc.stop();
				}

				return genoData;
			}) &
		tbb::make_filter<GenoData, std::string>(tbb::filter::parallel,
			[&](GenoData genoData)
			{
				if (genoData.readsData.size() == 0)
				{
					std::string empty = "";
					return empty;
				}

				std::string qualityStr = "";
				std::string baseSequence = "";
				std::vector<std::string> identifider;

				for (int i = 0; i < genoData.readsData.size(); ++i)
				{
					if ((i & 3) == 0) // identifier (read ID)
					{
						identifider.push_back(genoData.readsData[i]);
					}
					if ((i & 3) == 1) // sequence: ATGC
					{
						baseSequence += genoData.readsData[i];
					}
					if ((i & 3) == 3) // quality score
					{
						qualityStr += genoData.readsData[i];
					}
				}

				if (identifider.size() == 0 ||
					baseSequence.length() == 0 ||
					qualityStr.length() == 0)
				{
					std::string empty = "";
					return empty;
				}

				Char2UInt32 char2uint32;

				//========================================================
				//	process identifier
				//========================================================
				std::vector< std::vector<std::string> > splitTokens;

				for (int i = 0; i < identifider.size(); ++i)
				{
					std::string token = "";
					std::vector<std::string> tokenList;

					for (int j = 0; j < identifider[i].length(); ++j)
					{
						if (identifider[i][j] == ':')
						{
							tokenList.push_back(token);
							token = "";
							continue;
						}

						if (identifider[i][j] == ' ')
						{
							tokenList.push_back(token);
							tokenList.push_back("\t");
							token = "";
							continue;
						}

						token += identifider[i][j];
					}
					if (token.length() > 0) tokenList.push_back(token);

					splitTokens.push_back(tokenList);
				}

				int columns = splitTokens[0].size();

				std::vector<int> isDigit(columns, 1);
				for (int i = 0; i < columns; ++i)
				{
					for (int j = 0; j < splitTokens[0][i].length(); ++j)
					{
						if (splitTokens[0][i][j] < '0' || splitTokens[0][i][j] > '9')
						{
							isDigit[i] = 0;
							break;
						}
					}
				}

				std::string identifierKeys = "";
				std::vector<uint32_t> identifierValue;

				for (int j = 0; j < columns; ++j)
				{
					int value = 0;
					tsl::robin_map<std::string, int> columMap;
					for (int i = 0; i < splitTokens.size(); ++i)
					{
						std::string key = splitTokens[i][j];
						if (columMap.find(key) == columMap.end())
						{
							columMap[key] = value;
							++value;
						}
					}

					if (columMap.size() == 1)
					{
						identifierKeys += splitTokens[0][j] + ";";
					}
					else
					{
						if (isDigit[j] == 1)
						{
							identifierKeys += "?;";

							for (int i = 0; i < splitTokens.size(); ++i)
							{
								identifierValue.push_back((uint32_t)stoi(splitTokens[i][j]));
							}
						}
						else
						{
							for (auto it = columMap.begin(); it != columMap.end(); ++it)
							{
								identifierKeys += it.key() + ",";
								identifierValue.push_back((uint32_t)it.value());
							}
							identifierKeys += ";";
						}
					}
				}
				identifierKeys.pop_back();

				for (int i = 1; i < identifierValue.size(); ++i)
				{
					identifierValue[i] += identifierValue[i - 1];
				}

				std::vector<uint32_t> idEFCode;
				EliasFanoEncode32(identifierValue, idEFCode);

				std::vector<char> idValueCahrData;
				for (int i = 0; i < idEFCode.size(); ++i)
				{
					char2uint32.UInt32 = idEFCode[i];
					for (int j = 0; j < 4; ++j)
					{
						idValueCahrData.push_back(char2uint32.Char[j]);
					}
				}

				std::string idKey = "";
				std::string idValue = "";

				if (writeMode == "json")
				{
					std::vector<char> idValueCode;
					Base64Encode(idValueCahrData, idValueCode);

					idKey = "  \"idKey\": \"" + identifierKeys + "\",\n";

					idValue = "  \"idValue\": \"";
					idValue.append(idValueCode.begin(), idValueCode.end());
					idValue.append("\",\n");
				}
				if (writeMode == "yaml")
				{
					std::vector<char> idValueCode;
					Base64Encode(idValueCahrData, idValueCode);

					idKey = " idKey: " + identifierKeys + "\n";

					idValue = " idValue: ";
					idValue.append(idValueCode.begin(), idValueCode.end());
					idValue.append("\n");
				}

				if (writeMode == "binary")
				{
					idKey = identifierKeys;
					idValue.append(idValueCahrData.begin(), idValueCahrData.end());
				}

				//========================================================
				//	process gene sequence
				//========================================================
				int k = 0;
				std::bitset<8> bitBase;
				std::vector<uint32_t> NBasePos;
				std::vector<char> seqBinaryEncode;

				std::string checkBase = "";

				for (int i = 0; i < baseSequence.size(); ++i)
				{
					if (k >= 8)
					{
						k = 0;
						seqBinaryEncode.push_back((char)(bitBase.to_ulong()));
						bitBase.reset();
					}
					if (baseSequence[i] == 'A') // A: 00
					{
						k += 2;
					}
					if (baseSequence[i] == 'T') // T: 01
					{
						bitBase.set(6 - k);
						k += 2;
					}
					if (baseSequence[i] == 'G') // G: 10
					{
						bitBase.set(7 - k);
						k += 2;
					}
					if (baseSequence[i] == 'C') // C: 11
					{
						bitBase.set(7 - k);
						bitBase.set(6 - k);
						k += 2;
					}
					if (baseSequence[i] == 'N') // use "A" base to replace "N" base
					{
						k += 2;
						NBasePos.push_back(i);
					}
					checkBase += baseSequence[i];
				}

				std::vector<char> seqCompData;
				ZSTDEncode(seqBinaryEncode, seqCompData);

				std::vector<char> NBaseCharData;
				for (int i = 0; i < NBasePos.size(); ++i)
				{
					char2uint32.UInt32 = NBasePos[i];
					for (int j = 0; j < 4; ++j)
					{
						NBaseCharData.push_back(char2uint32.Char[j]);
					}
				}

				std::string seqCode = "";
				std::string NBasePosCode = "";

				if (writeMode == "json" || writeMode == "yaml")
				{
					std::vector<char> seqB64Code;
					std::vector<char> NBasePosB64Code;

					Base64Encode(seqCompData, seqB64Code);
					Base64Encode(NBaseCharData, NBasePosB64Code);

					if (writeMode == "yaml") seqCode = " seq: ";
					if (writeMode == "json") seqCode = "  \"seq\": \"";

					seqCode.append(seqB64Code.begin(), seqB64Code.end());
					NBasePosCode.append(NBasePosB64Code.begin(), NBasePosB64Code.end());

					if (writeMode == "yaml") seqCode += "\n";
					if (writeMode == "json") seqCode += "\",\n";
				}
				if (writeMode == "binary")
				{
					seqCode.append(seqCompData.begin(), seqCompData.end());
					NBasePosCode.append(NBaseCharData.begin(), NBaseCharData.end());
				}

				//========================================================
				//	process quality score
				//========================================================

				std::vector<int> consecNumber;
				std::vector<char> qualityScore;

				int prevNum = 0;
				char prevChar = qualityStr[0];

				tsl::robin_map<char, int> counter;

				for (int i = 0; i < qualityStr.length(); ++i)
				{
					if (counter.find(qualityStr[i]) == counter.end())
					{
						counter[qualityStr[i]] = 1;
					}
					else
					{
						counter[qualityStr[i]] += 1;
					}

					if (qualityStr[i] == prevChar)
					{
						++prevNum;
					}
					else
					{
						qualityScore.push_back(prevChar);
						consecNumber.push_back(prevNum);

						prevNum = 1;
						prevChar = qualityStr[i];
					}
				}

				std::string qKey = "";
				if (writeMode == "yaml") qKey = " qKey: ";
				if (writeMode == "json") qKey = "  \"qKey\": \"";

				std::vector<char> qualityCompData;
				std::vector<uint32_t> qualityEFCode;

				if (counter.size() > 1)
				{
					std::vector< std::pair<char, int> > sortCounter;

					for (auto it = counter.begin(); it != counter.end(); ++it)
					{
						sortCounter.push_back(std::make_pair(it.key(), it.value()));
					}

					std::sort(sortCounter.begin(), sortCounter.end(),
						[](std::pair<char, int>& x, std::pair<char, int>& y)
						{
							return x.second > y.second;
						});

					for (int j = 0; j < sortCounter.size(); ++j)
					{
						counter[sortCounter[j].first] = j;
						qKey += sortCounter[j].first;
					}

					uint32_t accumulated = 0;
					std::vector<uint32_t> qualityArr;

					for (int j = 0; j < qualityScore.size(); ++j)
					{
						accumulated += counter[qualityScore[j]];
						qualityArr.push_back(accumulated);

						accumulated += consecNumber[j];
						qualityArr.push_back(accumulated);
					}
					EliasFanoEncode32(qualityArr, qualityEFCode);

					std::vector<char> qualityCharData;
					for (int i = 0; i < qualityEFCode.size(); ++i)
					{
						char2uint32.UInt32 = qualityEFCode[i];
						for (int j = 0; j < 4; ++j)
						{
							qualityCharData.push_back(char2uint32.Char[j]);
						}
					}
					ZSTDEncode(qualityCharData, qualityCompData);
				}
				else
				{
					for (auto it = counter.begin(); it != counter.end(); ++it)
					{
						qKey += it.key();
					}
				}

				if (writeMode == "yaml") qKey += "\n";
				if (writeMode == "json") qKey += "\",\n";

				std::string qValue = "";

				if (writeMode == "json" || writeMode == "yaml")
				{
					std::vector<char> qValueCode;
					Base64Encode(qualityCompData, qValueCode);

					if (writeMode == "yaml") qValue = " qValue: ";
					if (writeMode == "json") qValue = "  \"qValue\": \"";

					qValue.append(qValueCode.begin(), qValueCode.end());

					if (writeMode == "yaml") qValue += "\n\n";
					if (writeMode == "json") qValue += "\"\n },\n";
				}
				if (writeMode == "binary")
				{
					qValue.append(qualityCompData.begin(), qualityCompData.end());
				}

				std::string readInfo = "";
				char2uint32.UInt32 = genoData.readStartId;
				for (int i = 0; i < 4; ++i)
				{
					readInfo += char2uint32.Char[i];
				}

				if (writeMode == "yaml")
				{
					readInfo = "read_"
						+ std::to_string(genoData.readStartId) + "-"
						+ std::to_string(genoData.readEndId) + ":\n"
						+ idKey + idValue 
						+ seqCode + NBasePosCode
						+ qKey + qValue;
				}
				if (writeMode == "json")
				{
					readInfo = " \"read_" + std::to_string(genoData.readStartId) + "-"
						+ std::to_string(genoData.readEndId) + "\": {\n"
						+ idKey + idValue
						+ seqCode + NBasePosCode
						+ qKey + qValue;
				}
				if (writeMode == "binary")
				{
					std::vector<uint32_t> paramsUInt =
					{
						genoData.readStartId,
						genoData.readEndId,
						(uint32_t)idKey.length(),
						(uint32_t)idValue.length(),
						(uint32_t)seqCode.length(),
						(uint32_t)NBasePosCode.length(),
						(uint32_t)qKey.length(),
						(uint32_t)qValue.length()
					};

					Char2UInt32 char2uint32;
					for (int i = 0; i < paramsUInt.size(); ++i)
					{
						char2uint32.UInt32 = paramsUInt[i];
						for (int j = 0; j < 4; ++j)
						{
							readInfo += char2uint32.Char[j];
						}
					}

					readInfo += idKey + idValue
						+ seqCode + NBasePosCode
						+ qKey + qValue;
				}

				return readInfo;

			}) &

		tbb::make_filter<std::string, void>(tbb::filter::serial_out_of_order,
			[&](std::string readInfo)
			{
				if (readInfo.length() == 0)
				{
					return NULL;
				}

				Char2UInt32 char2uint32;
				for (int i = 0; i < 4; ++i)
				{
					char2uint32.Char[i] = readInfo[i];
				}

				fwrite(readInfo.c_str() + 4, 1, readInfo.length() - 4, tmpFile);
				
				std::vector<uint64_t> tmp = { char2uint32.UInt32, startPos, readInfo.length() };
				offsetVectorTmp.push_back(tmp);

				startPos += readInfo.length();

				return NULL;
			})
		);

	fclose(tmpFile);

	tbb::parallel_sort(offsetVectorTmp.begin(), offsetVectorTmp.end(),
		[](std::vector<uint64_t>& x, std::vector<uint64_t>& y)
		{
			return x[0] < y[0];
		});

	time_t tTime;
	char szDate[48];

	time(&tTime);
	strftime(szDate, 46, "%Y-%m-%dT%H:%M:%S", localtime(&tTime));
	std::string date = szDate;

	std::string baseInfo = "";
	if (writeMode == "json")
	{
		baseInfo = "{\n \"BasicInfo\": {\n"
			"  \"omics\": \"Genomics\",\n"
			"  \"writeFormat\": \"" + writeMode + "\",\n"
			"  \"date\": \"" + date + "\",\n"
			"  \"parentFile\": \"" + inputFile + "\",\n"
			"  \"parentFormat\": \"" + nameSuffix[1] + "\"\n"
			"  \"readLength\": \"" + genoHeader.readLength + "\"\n"
			"  \"readCount\": \"" + genoHeader.readCount + "\"\n"
			" },\n";
	}
	if (writeMode == "yaml" || writeMode == "binary")
	{
		baseInfo = "BasicInfo: \n"
			" omics: Genomics\n"
			" writeFormat: " + writeMode + "\n"
			" date: " + date + "\n"
			" parentFile: " + inputFile + "\n"
			" parentFormat: " + nameSuffix[1] + "\n"
			" readLength: " + genoHeader.readLength + "\n"
			" readCount: " + genoHeader.readCount + "\n"
			"\n";
	}

	ReWriteOMGFile(inputFile, nameSuffix[0], baseInfo, offsetVectorTmp);
}


#endif // !GENOMICS_H

