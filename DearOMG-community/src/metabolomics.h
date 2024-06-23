#ifndef METABOLOMICS_H
#define METABOLOMICS_H

#include "namespace.h"
#include "eliasfano.h"
#include "utility.h"

void DearOMG::EncodeMetabolomics(int nThreads, std::string inputFile)
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
		tbb::make_filter<void, MetaData>(tbb::filter::serial_out_of_order,
			[&](tbb::flow_control& fc)
			{
				MetaData metaData;
				metaQueue.pop(metaData);

				if (metaData.stop)
				{
					fc.stop();
				}

				return metaData;
			}) &
		tbb::make_filter<MetaData, MetaData>(tbb::filter::parallel,
			[&](MetaData metaData)
			{
				if (metaData.arrayLength == -1)
				{
					return metaData;
				}

				Char2UInt32 char2uint32;
				Char2UInt64 char2uint64;

				if (metaHeader.imzMLModel == "continuous")
				{
					std::vector<double> intensityRawData;

					DecodeImzMLData(metaData.encodeSpectrum[0],
						intensityRawData, metaData.arrayLength,
						metaHeader.intenCompressType,
						metaHeader.intenEncodeType);

					double minIntensity = *std::min_element(intensityRawData.begin(), intensityRawData.end());
					metaData.minIntensity = minIntensity;

					uint32_t accumulateIntensity = 0;
					std::vector<uint32_t> intensityArr;

					for (int i = 0; i < metaData.arrayLength; ++i)
					{
						accumulateIntensity += (uint32_t)(round(sqrt((intensityRawData[i] - minIntensity) * 1e4)));
						//accumulateIntensity += (uint32_t)(round((intensityRawData[i] - minIntensity) * 1e3));

						intensityArr.push_back(accumulateIntensity);
					}

					std::vector<uint32_t> intensityEFcode;
					EliasFanoEncode32(intensityArr, intensityEFcode);

					std::vector<char> intenCharCode;
					for (int i = 0; i < intensityEFcode.size(); ++i)
					{
						char2uint32.UInt32 = intensityEFcode[i];
						for (int j = 0; j < 4; ++j)
						{
							intenCharCode.push_back(char2uint32.Char[j]);
						}
					}
					std::vector<char> intenCompData;
					ZSTDEncode(intenCharCode, intenCompData);

					if (writeMode == "json" || writeMode == "yaml")
					{
						std::vector<char> intensityCode;
						Base64Encode(intenCompData, intensityCode);

						if (writeMode == "json")
						{
							metaData.intensityEncode = "  \"int_arr\": \"";
							metaData.intensityEncode.append(intensityCode.begin(), intensityCode.end());
							metaData.intensityEncode = "\"\n },\n";
						}
						if (writeMode == "yaml")
						{
							metaData.intensityEncode = " int_arr: ";
							metaData.intensityEncode.append(intensityCode.begin(), intensityCode.end());
							metaData.intensityEncode = "\n\n";
						}
					}
					if (writeMode == "binary")
					{
						metaData.intensityEncode.assign(intenCompData.begin(), intenCompData.end());
					}
				}
				if (metaHeader.imzMLModel == "processed")
				{
					std::vector<double> mzRowData;
					std::vector<double> intensityRawData;

					DecodeImzMLData(metaData.encodeSpectrum[0],
						mzRowData, metaData.arrayLength,
						metaHeader.mzCompressType,
						metaHeader.mzEncodeType);

					DecodeImzMLData(metaData.encodeSpectrum[1],
						intensityRawData, metaData.arrayLength,
						metaHeader.intenCompressType,
						metaHeader.intenEncodeType);

					double minIntensity = *std::min_element(intensityRawData.begin(), intensityRawData.end());
					metaData.minIntensity = minIntensity;

					uint32_t accumulateIntensity = 0;

					std::vector<uint32_t> mzArr(metaData.arrayLength);
					std::vector<uint32_t> intensityArr(metaData.arrayLength);

					for (int i = 0; i < metaData.arrayLength; ++i)
					{
						accumulateIntensity += (uint32_t)(round((intensityRawData[i] - minIntensity) * 1e5));

						intensityArr[i] = accumulateIntensity;
						mzArr[i] = (uint32_t)(std::round(mzRowData[i] * mzPrecision));
					}

					std::vector<uint32_t> mzEFcode;
					std::vector<uint32_t> intensityEFcode;

					EliasFanoEncode32(mzArr, mzEFcode);
					EliasFanoEncode32(intensityArr, intensityEFcode);

					std::vector<char> mzCharCode;
					std::vector<char> intenCharCode;

					for (int i = 0; i < mzEFcode.size(); ++i)
					{
						char2uint32.UInt32 = mzEFcode[i];
						for (int j = 0; j < 4; ++j)
						{
							mzCharCode.push_back(char2uint32.Char[j]);
						}
					}

					for (int i = 0; i < intensityEFcode.size(); ++i)
					{
						char2uint64.UInt64 = intensityEFcode[i];
						for (int j = 0; j < 8; ++j)
						{
							intenCharCode.push_back(char2uint64.Char[j]);
						}
					}

					std::vector<char> mzCompData;
					ZSTDEncode(mzCharCode, mzCompData);

					std::vector<char> intenCompData;
					ZSTDEncode(intenCharCode, intenCompData);

					if (writeMode == "json" || writeMode == "yaml")
					{
						std::vector<char> mzCode;
						std::vector<char> intensityCode;

						Base64Encode(mzCompData, mzCode);
						Base64Encode(intenCompData, intensityCode);

						if (writeMode == "json")
						{
							metaData.mzEncode = "  \"mz_arr\": \"";
							metaData.mzEncode.append(mzCode.begin(), mzCode.end());
							metaData.mzEncode += "\",\n";
						}
						if (writeMode == "yaml")
						{
							metaData.mzEncode = " mz_arr: ";
							metaData.mzEncode.append(mzCode.begin(), mzCode.end());
							metaData.mzEncode += "\n";
						}

						if (writeMode == "json")
						{
							metaData.intensityEncode = "  \"int_arr\": \"";
							metaData.intensityEncode.append(intensityCode.begin(), intensityCode.end());
							metaData.intensityEncode = "\"\n },\n";
						}
						if (writeMode == "yaml")
						{
							metaData.intensityEncode = " int_arr: ";
							metaData.intensityEncode.append(intensityCode.begin(), intensityCode.end());
							metaData.intensityEncode = "\n\n";
						}
					}
					if (writeMode == "binary")
					{
						metaData.mzEncode.assign(mzCompData.begin(), mzCompData.end());
						metaData.intensityEncode.assign(intenCompData.begin(), intenCompData.end());
					}
				}

				metaData.info = "";
				if (writeMode == "json")
				{
					metaData.info += " \"spectrum_" + metaData.spectrumId + "\": {\n"
						+ "  \"posX\": " + metaData.position_x + ",\n"
						+ "  \"posY\": " + metaData.position_y + ",\n"
						+ "  \"3DPosX\": " + metaData.position3D_x + ",\n"
						+ "  \"3DPosY\": " + metaData.position3D_y + ",\n"
						+ "  \"3DPosZ\": " + metaData.position3D_z + ",\n"
						+ "  \"minIntensity\": " + std::to_string(metaData.minIntensity) + ",\n";
				}
				if (writeMode == "yaml")
				{
					metaData.info += "spectrum_" + metaData.spectrumId + ":\n"
						+ " posX: " + metaData.position_x + "\n"
						+ " posY: " + metaData.position_y + "\n"
						+ " 3DPosX: " + metaData.position3D_x + "\n"
						+ " 3DPosY: " + metaData.position3D_y + "\n"
						+ " 3DPosZ: " + metaData.position3D_z + "\n"
						+ " minIntensity: " + std::to_string(metaData.minIntensity) + "\n";
				}
				if (writeMode == "binary")
				{
					Char2Float char2float;
					std::vector<float> paramsFlaot =
					{	
						std::stof(metaData.position_x),
						std::stof(metaData.position_y),
						std::stof(metaData.position3D_x),
						std::stof(metaData.position3D_y),
						std::stof(metaData.position3D_z),
						(float)metaData.minIntensity
					};

					for (int i = 0; i < paramsFlaot.size(); ++i)
					{
						char2float.Float = paramsFlaot[i];
						for (int j = 0; j < 4; ++j)
						{
							metaData.info += char2float.Char[j];
						}
					}

					std::vector<uint32_t> paramsUInt =
					{
						(uint32_t)std::stoi(metaData.spectrumId),
						(uint32_t)metaData.intensityEncode.length(),
						(uint32_t)metaData.mzEncode.length()
					};

					for (int i = 0; i < paramsUInt.size(); ++i)
					{
						char2uint32.UInt32 = paramsUInt[i];
						for (int j = 0; j < 4; ++j)
						{
							metaData.info += char2uint32.Char[j];
						}
					}

				}

				return metaData;

			}) &
		tbb::make_filter<MetaData, void>(tbb::filter::serial_out_of_order,
			[&](MetaData metaData)
			{
				if (metaData.arrayLength == -1)
				{
					return NULL;
				}

				fwrite(metaData.info.c_str(), 1, metaData.info.length(), tmpFile);
				fwrite(metaData.intensityEncode.c_str(), 1, metaData.intensityEncode.length(), tmpFile);
				
				if (metaHeader.imzMLModel == "processed")
				{
					fwrite(metaData.mzEncode.c_str(), 1, metaData.mzEncode.length(), tmpFile);
				}
				size_t writeBytes = metaData.info.length() + metaData.intensityEncode.length();

				if (metaHeader.imzMLModel == "processed")
				{
					writeBytes += metaData.mzEncode.length();
				}

				std::vector<uint64_t> tmp = { metaData.scanIndx, startPos, writeBytes };
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
			"  \"omics\": \"Metabolomics\",\n"
			"  \"writeFormat\": \"" + writeMode + "\",\n"
			"  \"date\": \"" + date + "\",\n"
			"  \"parentFile\": \"" + inputFile + "\",\n"
			"  \"parentFormat\": \"" + nameSuffix[1] + "\",\n"
			"  \"scanCount\": \"" + metaHeader.spectrumCount + "\",\n"
			"  \"pixelSize_X\": \"" + metaHeader.pixelSize_X + "\",\n"
			"  \"pixelSize_Y\": \"" + metaHeader.pixelSize_Y + "\",\n"
			"  \"maxDimension_X\": \"" + metaHeader.maxDimension_X + "\",\n"
			"  \"maxDimension_Y\": \"" + metaHeader.maxDimension_Y + "\",\n"
			"  \"maxCountofPixel_X\": \"" + metaHeader.maxCountofPixel_X + "\",\n"
			"  \"maxCountofPixel_Y\": \"" + metaHeader.maxCountofPixel_Y + "\",\n"
			"  \"msModel\": \"" + metaHeader.msModel + "\",\n"
			"  \"msDetector\": \"" + metaHeader.msDetector + "\",\n"
			"  \"msIonisation\": \"" + metaHeader.msIonisation + "\",\n"
			"  \"msMassAnalyzer\": \"" + metaHeader.msMassAnalyzer + "\",\n"
			"  \"continuousMzCode\": \"" + metaHeader.continuousMzCode + "\"\n"
			" },\n";
	}
	if (writeMode == "yaml" || writeMode == "binary")
	{
		baseInfo = "BasicInfo: \n"
			" omics: Metabolomics\n"
			" writeFormat: " + writeMode + "\n"
			" date: " + date + "\n"
			" parentFile: " + inputFile + "\n"
			" parentFormat: " + nameSuffix[1] + "\n"
			" scanCount: " + metaHeader.spectrumCount + "\n"
			" pixelSize_X: " + metaHeader.pixelSize_X + "\n"
			" pixelSize_Y: " + metaHeader.pixelSize_Y + "\n"
			" maxDimension_X: " + metaHeader.maxDimension_X + "\n"
			" maxDimension_Y: " + metaHeader.maxDimension_Y + "\n"
			" maxCountofPixel_X: " + metaHeader.maxCountofPixel_X + "\n"
			" maxCountofPixel_Y: " + metaHeader.maxCountofPixel_Y + "\n"
			" msModel: " + metaHeader.msModel + "\n"
			" msDetector: " + metaHeader.msDetector + "\n"
			" msIonisation: " + metaHeader.msIonisation + "\n"
			" msMassAnalyzer: " + metaHeader.msMassAnalyzer + "\n"
			" continuousMzCode: " + metaHeader.continuousMzCode + "\n"
			"\n";
	}

	ReWriteOMGFile(inputFile, nameSuffix[0], baseInfo, offsetVectorTmp);
}

#endif // !METABOLOMICS_H
