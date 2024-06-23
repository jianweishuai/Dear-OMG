#ifndef PROTEOMICS_H
#define PROTEOMICS_H

#include "namespace.h"
#include "eliasfano.h"
#include "utility.h"

void DearOMG::EncodeProteomics(int nThreads, std::string inputFile)
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
		tbb::make_filter<void, ProtData>(tbb::filter::serial_out_of_order,
			[&](tbb::flow_control& fc)
			{
				ProtData protData;
				protQueue.pop(protData);

				if (protData.stop)
				{
					fc.stop();
				}

				return protData;
			})&
		tbb::make_filter<ProtData, ProtData>(tbb::filter::parallel,
			[&](ProtData protData)
			{
				if (protData.msLevel == -1 || protData.spectrum[0].size() < 10)
				{
					return protData;
				}

				uint32_t sumMobility = 0;
				uint64_t sumIntensity = 0;

				std::vector<uint32_t> mzArr;
				std::vector<uint32_t> mobilityArr;
				std::vector<uint64_t> intensityArr;

				int arrSize = protData.spectrum[0].size();
				std::vector< std::vector<double> > sortMobiTriplet(arrSize, std::vector<double>(3));
				for (int i = 0; i < arrSize; ++i)
				{
					sortMobiTriplet[i][0] = protData.spectrum[0][i];
					sortMobiTriplet[i][1] = protData.spectrum[1][i];
					sortMobiTriplet[i][2] = (double)protData.mobilityIndex[i];
				}

				tbb::parallel_sort(sortMobiTriplet.begin(), sortMobiTriplet.end(),
					[](std::vector<double>& x, std::vector<double>& y)
					{
						return x[0] < y[0];
					});

				for (int i = 0; i < arrSize; ++i)
				{
					if (skipZeroIntensity && sortMobiTriplet[i][1] < 0.01) continue;
					
					mzArr.push_back((uint32_t)(std::round(sortMobiTriplet[i][0] * mzPrecision)));

					sumIntensity += (uint64_t)(std::round(std::sqrt(sortMobiTriplet[i][1])));

					intensityArr.push_back(sumIntensity);

					sumMobility += (uint32_t)sortMobiTriplet[i][2];

					mobilityArr.push_back(sumMobility);
				}
				
				if (mzArr.size() == 0)
				{
					return protData;
				}

				std::vector<uint32_t> mzEFcode;
				EliasFanoEncode32(mzArr, mzEFcode);

				Char2UInt32 char2uint32;
				std::vector<char> mzCharCode(mzEFcode.size() * 4);
				for (int i = 0; i < mzEFcode.size(); ++i)
				{
					char2uint32.UInt32 = mzEFcode[i];
					for (int j = 0; j < 4; ++j)
					{
						mzCharCode[4 * i + j] = char2uint32.Char[j];
					}
				}

				std::vector<uint64_t> intensityEFcode;
				EliasFanoEncode64(intensityArr, intensityEFcode);

				Char2UInt64 char2uint64;
				std::vector<char> intenCharCode(intensityEFcode.size() * 8);

				for (int i = 0; i < intensityEFcode.size(); ++i)
				{
					char2uint64.UInt64 = intensityEFcode[i];
					for (int j = 0; j < 8; ++j)
					{
						intenCharCode[8 * i + j] = char2uint64.Char[j];
					}
				}

				std::vector<uint32_t> mobiEFcode;
				EliasFanoEncode32(mobilityArr, mobiEFcode);

				std::vector<char> mobiCharCode;
				for (int i = 0; i < mobiEFcode.size(); ++i)
				{
					char2uint32.UInt32 = mobiEFcode[i];
					for (int j = 0; j < 4; ++j)
					{
						mobiCharCode.push_back(char2uint32.Char[j]);
					}
				}

				std::vector<char> mzCompData;
				std::vector<char> mobiCompData;
				std::vector<char> intenCompData;

				ZSTDEncode(mzCharCode, mzCompData);
				ZSTDEncode(mobiCharCode, mobiCompData);
				ZSTDEncode(intenCharCode, intenCompData);

				if (writeMode == "json" || writeMode == "yaml")
				{
					std::vector<char> mzCode;
					std::vector<char> mobiCode;
					std::vector<char> intensityCode;

					Base64Encode(mzCompData, mzCode);
					Base64Encode(mobiCompData, mobiCode);
					Base64Encode(intenCompData, intensityCode);

					if (writeMode == "json")
					{
						protData.info = " \"scan_" + std::to_string(protData.scanIndex) + "\": {\n"
							+ "  \"RT\": " + std::to_string(protData.rt) + ",\n"
							+ "  \"msLevel\": " + std::to_string(protData.msLevel) + ",\n"
							+ "  \"precursorMz\": " + std::to_string(protData.precursorMz) + ",\n"
							+ "  \"collisionEnergy\": " + std::to_string(protData.collisionEnergy) + ",\n"
							+ "  \"chargeState\": " + std::to_string(protData.charge) + ",\n";

						protData.mzEncode = "  \"mz_arr\": \"";
						protData.mzEncode.append(mzCode.begin(), mzCode.end());
						protData.mzEncode.append("\",\n");

						protData.intensityEncode = "  \"int_arr\": \"";
						protData.intensityEncode.append(intensityCode.begin(), intensityCode.end());
						protData.intensityEncode.append("\",\n");

						if (writeMobility)
						{
							protData.mobilityEncode = "  \"mobilityIndex\": \"";
							protData.mobilityEncode.append(mobiCode.begin(), mobiCode.end());
							protData.mobilityEncode.append("\"\n },\n");
						}
						else
						{
							protData.mobilityEncode = "";
						}
						
					}

					if (writeMode == "yaml")
					{
						protData.info = "scan_" + std::to_string(protData.scanIndex) + ":\n"
							+ " RT: " + std::to_string(protData.rt) + "\n"
							+ " msLevel: " + std::to_string(protData.msLevel) + "\n"
							+ " precursorMz: " + std::to_string(protData.precursorMz) + "\n"
							+ " collisionEnergy: " + std::to_string(protData.collisionEnergy) + "\n"
							+ " chargeState: " + std::to_string(protData.charge) + "\n";

						protData.mzEncode = " mz_arr: ";
						protData.mzEncode.append(mzCode.begin(), mzCode.end());
						protData.mzEncode.append("\n");

						protData.intensityEncode = " int_arr: ";
						protData.intensityEncode.append(intensityCode.begin(), intensityCode.end());
						protData.intensityEncode.append("\n");

						if (writeMobility)
						{
							protData.mobilityEncode = " mobilityIndex: ";
							protData.mobilityEncode.append(mobiCode.begin(), mobiCode.end());
							protData.mobilityEncode.append("\n\n");
						}
						else
						{
							protData.mobilityEncode = "";
						}
						
					}
				}
				if (writeMode == "binary")
				{
					protData.mzEncode.assign(mzCompData.begin(), mzCompData.end());
					protData.intensityEncode.assign(intenCompData.begin(), intenCompData.end());
					
					if (writeMobility)
					{
						protData.mobilityEncode.assign(mobiCompData.begin(), mobiCompData.end());
					}
					else
					{
						protData.mobilityEncode = "";
					}

					protData.info = "";
					Char2Float char2float;
					Char2UInt32 char2uint32;

					std::vector<float> paramsFloat = { protData.rt , protData.precursorMz, protData.collisionEnergy };
					
					for (int i = 0; i < paramsFloat.size(); ++i)
					{
						char2float.Float = paramsFloat[i];
						for (int j = 0; j < 4; ++j)
						{
							protData.info += char2float.Char[j];
						}
					}

					std::vector<uint32_t> paramsUInt =
					{
						protData.scanIndex,
						(uint32_t)protData.msLevel,
						(uint32_t)protData.charge,
						(uint32_t)protData.mzEncode.length(),
						(uint32_t)protData.intensityEncode.length(),
						(uint32_t)protData.mobilityEncode.length()
					};

					for (int i = 0; i < paramsUInt.size(); ++i)
					{
						char2uint32.UInt32 = paramsUInt[i];
						for (int j = 0; j < 4; ++j)
						{
							protData.info += char2uint32.Char[j];
						}
					}
				}

				return protData;

			}) &

		tbb::make_filter<ProtData, void>(tbb::filter::serial_out_of_order,
			[&](ProtData protData)
			{
				if (protData.mzEncode.length() == 0)
				{
					return NULL;
				}
				
				fwrite(protData.info.c_str(), 1, protData.info.length(), tmpFile);
				fwrite(protData.mzEncode.c_str(), 1, protData.mzEncode.length(), tmpFile);
				fwrite(protData.intensityEncode.c_str(), 1, protData.intensityEncode.length(), tmpFile);
				fwrite(protData.mobilityEncode.c_str(), 1, protData.mobilityEncode.length(), tmpFile);

				size_t writeBytes = protData.info.length() +
					protData.mzEncode.length() +
					protData.intensityEncode.length() +
					protData.mobilityEncode.length();

				std::vector<uint64_t> tmp = { protData.scanIndex, startPos, writeBytes };
				offsetVectorTmp.push_back(tmp);

				startPos += writeBytes;

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
			"  \"omics\": \"Proteomics\",\n"
			"  \"writeFormat\": \"" + writeMode + "\",\n"
			"  \"date\": \"" + date + "\",\n"
			"  \"parentFile\": \"" + inputFile + "\",\n"
			"  \"parentFormat\": \"" + nameSuffix[1] + "\",\n"
			"  \"scanCount\": \"" + protHeader.scanCount + "\",\n"
			"  \"startTime\": \"" + protHeader.startTime + "\",\n"
			"  \"endTime\": \"" + protHeader.endTime + "\",\n"
			"  \"msModel\": \"" + protHeader.msModel + "\",\n"
			"  \"msDetector\": \"" + protHeader.msDetector + "\",\n"
			"  \"msIonisation\": \"" + protHeader.msIonisation + "\",\n"
			"  \"msMassAnalyzer\": \"" + protHeader.msMassAnalyzer + "\",\n"
			"  \"msManufacturer\": \"" + protHeader.msManufacturer + "\"\n"
			"  \"mobilityValue\": \"" + protHeader.mobolityValues + "\"\n"
			" },\n";
	}

	if (writeMode == "yaml" || writeMode == "binary")
	{
		baseInfo = "BasicInfo: \n"
			" omics: Proteomics\n"
			" writeFormat: " + writeMode + "\n"
			" date: " + date + "\n"
			" parentFile: " + inputFile + "\n"
			" parentFormat: " + nameSuffix[1] + "\n"
			" scanCount: " + protHeader.scanCount + "\n"
			" startTime: " + protHeader.startTime + "\n"
			" endTime: " + protHeader.endTime + "\n"
			" msModel: " + protHeader.msModel + "\n"
			" msDetector: " + protHeader.msDetector + "\n"
			" msIonisation: " + protHeader.msIonisation + "\n"
			" msMassAnalyzer: " + protHeader.msMassAnalyzer + "\n"
			" msManufacturer: " + protHeader.msManufacturer + "\n"
			" mobilityValue: " + protHeader.mobolityValues + "\n"
			"\n";
	}

	ReWriteOMGFile(inputFile, nameSuffix[0], baseInfo, offsetVectorTmp);
}

#endif // !PROTEOMICS_H
