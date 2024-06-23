#ifndef IMZML_H
#define IMZML_H

#include <string>
#include <vector>
#include <iostream>

#include "namespace.h"
#include "cv.h"

std::string DearOMG::FindImzMLKeyValue(const std::string& in_str, const std::string& key)
{
	int pos = in_str.find(key);
	if (pos != in_str.npos)
	{
		pos += key.length();
		for (int i = pos; i < in_str.length(); ++i)
		{
			if (in_str[i] == '\"')
			{
				return in_str.substr(pos, i - pos);
			}
		}
	}
	return "";
}

void DearOMG::DecodeImzMLData(std::vector<unsigned char>& encodeSpectrum,
	std::vector<double>& decodeSpectrum,
	int arrayLength, std::string compressType, std::string EncodeType)
{
	decodeSpectrum.resize(arrayLength);

	/*
	"32-bit float": float
	"64-bit float": double
	"32-bit integer": int32_t
	"64-bit integer": int64_t
	*/

	int precision;

	if (EncodeType.find("32") != EncodeType.npos) precision = 4;
	if (EncodeType.find("64") != EncodeType.npos) precision = 8;

	unsigned char* uncomp_result;

	if (compressType.find("zlib") != compressType.npos)
	{
		unsigned long retLen = arrayLength * precision;
		uncomp_result = (unsigned char*)malloc(retLen * sizeof(unsigned char));

		int OK = uncompress(uncomp_result, &retLen, encodeSpectrum.data(), encodeSpectrum.size());

		if (OK != 0)
		{
			std::cout << "Something was wrong in decode peak data.\n\n";

			free(uncomp_result);
			exit(3);
		}
	}
	else
	{
		uncomp_result = (unsigned char*)malloc(encodeSpectrum.size());
		
		for (int i = 0; i < encodeSpectrum.size(); ++i)
		{
			uncomp_result[i] = encodeSpectrum[i];
		}
	}

	if (EncodeType == "32-bit float")
	{
		float* arr = (float*)uncomp_result;
		for (int i = 0; i < arrayLength; ++i)
		{
			decodeSpectrum[i] = (double)(arr[i]);
		}
	}
	else if (EncodeType == "64-bit float")
	{
		double* arr = (double*)uncomp_result;
		for (int i = 0; i < arrayLength; ++i)
		{
			decodeSpectrum[i] = (double)(arr[i]);
		}
	}
	else if (EncodeType == "32-bit integer")
	{

		int32_t* arr = (int32_t*)uncomp_result;
		for (int i = 0; i < arrayLength; ++i)
		{
			decodeSpectrum[i] = (double)(arr[i]);
		}
	}
	else if (EncodeType == "64-bit integer")
	{
		int64_t* arr = (int64_t*)uncomp_result;
		for (int i = 0; i < arrayLength; ++i)
		{
			decodeSpectrum[i] = (double)(arr[i]);
		}
	}
	free(uncomp_result);
}


void DearOMG::GetImzMLHeaderInfo(std::string inputFile)
{
	FILE* xml_file;
	xml_file = fopen(inputFile.c_str(), "r");

	if (xml_file == NULL)
	{
		std::cout << "[ERROR] Cannot open " << inputFile << "\n"
			<< "[ERROR] Please check your file or directory!" << std::endl;

		exit(0);
	}

	char* buffer_char = (char*)malloc((xmlBufferSize + 1) * sizeof(char));
	buffer_char[xmlBufferSize] = 0;

	int p = inputFile.find(".imzML");
	std::string ibdFileName = inputFile.substr(0, p) + ".ibd";

	FILE* ibd_file;
	ibd_file = fopen(ibdFileName.c_str(), "rb");

	if (!ibd_file)
	{
		std::cerr << "[ERROR] Cannot open " << ibdFileName << "\n"
			<< "[ERROR] Please cheak your file or directory." << std::endl;
	}

	fpos_t ibdFilePos;

#ifdef _WIN32
	ibdFilePos = 0;
#endif // !_WIN32
#ifdef __linux__
	ibdFilePos.__pos = 0;
#endif // __linux__

	int pos;
	bool stopMsg = false;

	bool flag = false;
	bool cvFlag = true;
	bool instrumentNameFlag = true;

	bool sourceFlag = false;
	bool analyzerFlag = false;
	bool detectorFlag = false;
	bool instrumentFlag = false;

	bool scanSettingFlag = false;
	bool fileContentFlag = false;
	bool referenceableParamFlag = false;

	std::string id = "";
	std::string unitName = "";
	std::string accession = "";

	std::string name = "";
	std::string contents = "";

	while (1)
	{
		if (stopMsg) break;

		if (feof(xml_file)) break;
		size_t len = fread(buffer_char, 1, xmlBufferSize, xml_file);
		
		for (int i = 0; i < xmlBufferSize; ++i)
		{
			if (stopMsg) break;

			if (buffer_char[i] < ' ') buffer_char[i] = ' ';

			if (!flag && buffer_char[i] == '<')
			{
				flag = true;
			}
			else if (flag && buffer_char[i] != '<' && buffer_char[i] != '>')
			{
				contents += buffer_char[i];
			}
			else if (flag && buffer_char[i] == '>')
			{
				flag = false;
				contents += buffer_char[i];

				pos = contents.find(' ');
				name = contents.substr(0, pos);

				if (name == "fileContent>") fileContentFlag = true;
				if (name == "/fileContent>") fileContentFlag = false;

				if (fileContentFlag)
				{
					if (name == "cvParam")
					{
						if (contents.find("IMS:1000031") != contents.npos)
						{
							metaHeader.imzMLModel = "processed";
						}
						if (contents.find("IMS:1000030") != contents.npos)
						{
							metaHeader.imzMLModel = "continuous";
						}
					}
				}

				if (name == "referenceableParamGroup")
				{
					id = FindImzMLKeyValue(contents, "id=\"");
					referenceableParamFlag = true;
				}
				if (name == "/referenceableParamGroup>") referenceableParamFlag = false;

				if (referenceableParamFlag)
				{
					if (id.find("mz") != id.npos)
					{
						if (name == "cvParam")
						{
							if (contents.find("-bit") != contents.npos)
							{
								accession = FindImzMLKeyValue(contents, "accession=\"");
								metaHeader.mzEncodeType = CV::CVMap[accession];
							}
							if (contents.find("compression") != contents.npos)
							{
								accession = FindImzMLKeyValue(contents, "accession=\"");
								metaHeader.mzCompressType = CV::CVMap[accession];
							}
						}
					}
					if (id.find("inten") != id.npos)
					{
						if (name == "cvParam")
						{
							if (contents.find("-bit") != contents.npos)
							{
								accession = FindImzMLKeyValue(contents, "accession=\"");
								metaHeader.intenEncodeType = CV::CVMap[accession];
							}
							if (contents.find("compression") != contents.npos)
							{
								accession = FindImzMLKeyValue(contents, "accession=\"");
								metaHeader.intenCompressType = CV::CVMap[accession];
							}
						}
					}
				}

				if (name == "scanSettings") scanSettingFlag = true;
				if (name == "/scanSettings>") scanSettingFlag = false;

				if (scanSettingFlag)
				{
					if (name == "cvParam")
					{
						if (contents.find("IMS:1000042") != contents.npos)
						{
							metaHeader.maxCountofPixel_X = FindImzMLKeyValue(contents, "value=\"");
						}
						if (contents.find("IMS:1000043") != contents.npos)
						{
							metaHeader.maxCountofPixel_Y = FindImzMLKeyValue(contents, "value=\"");
						}
						if (contents.find("IMS:1000044") != contents.npos)
						{
							unitName = FindImzMLKeyValue(contents, "unitName=\"");
							metaHeader.maxDimension_X = FindImzMLKeyValue(contents, "value=\"") + " " + unitName;
						}
						if (contents.find("IMS:1000045") != contents.npos)
						{
							unitName = FindImzMLKeyValue(contents, "unitName=\"");
							metaHeader.maxDimension_Y = FindImzMLKeyValue(contents, "value=\"") + " " + unitName;
						}
						if (contents.find("IMS:1000046") != contents.npos)
						{
							unitName = FindImzMLKeyValue(contents, "unitName=\"");
							metaHeader.pixelSize_X = FindImzMLKeyValue(contents, "value=\"") + " " + unitName;
						}
						if (contents.find("IMS:1000047") != contents.npos)
						{
							unitName = FindImzMLKeyValue(contents, "unitName=\"");
							metaHeader.pixelSize_Y = FindImzMLKeyValue(contents, "value=\"") + " " + unitName;
						}
					}
				}

				if (name == "instrumentConfiguration") instrumentFlag = true;
				if (name == "/instrumentConfiguration>") instrumentFlag = false;

				if (instrumentFlag)
				{
					if (name == "cvParam" && instrumentNameFlag)
					{
						accession = FindImzMLKeyValue(contents, "accession=\"");
						metaHeader.msModel = CV::CVMap[accession];
						instrumentNameFlag = false;
					}
					
					if (name == "source")
					{
						cvFlag = true;
						sourceFlag = true;
					}
					if (name == "cvParam" && sourceFlag && cvFlag)
					{
						accession = FindImzMLKeyValue(contents, "accession=\"");
						metaHeader.msIonisation = CV::CVMap[accession];
						cvFlag = false;
					}
					if (name == "/source>") sourceFlag = false;
					
					if (name == "analyzer")
					{
						cvFlag = true;
						analyzerFlag = true;
					}
					if (name == "cvParam" && analyzerFlag && cvFlag)
					{
						accession = FindImzMLKeyValue(contents, "accession=\"");
						metaHeader.msMassAnalyzer = CV::CVMap[accession];
						cvFlag = false;
					}
					if (name == "/analyzer>") analyzerFlag = false;

					if (name == "detector")
					{
						cvFlag = true;
						detectorFlag = true;
					}
					if (name == "cvParam" && detectorFlag && cvFlag)
					{
						accession = FindImzMLKeyValue(contents, "accession=\"");
						metaHeader.msDetector = CV::CVMap[accession];
						cvFlag = false;
					}
					if (name == "/detector>") detectorFlag = false;
				}

				if (name == "spectrumList")
				{
					metaHeader.spectrumCount = FindImzMLKeyValue(contents, "count=\"");
				}

				if (name == "spectrum")
				{
					stopMsg = true;
					contents.clear();
					break;
				}

				contents.clear();
			}
		}
	}
	free(buffer_char);
	fclose(xml_file);
	fclose(ibd_file);
}

void DearOMG::LoadImzMLFile(std::string inputFile)
{
	FILE* xml_file;
	xml_file = fopen(inputFile.c_str(), "r");

	if (xml_file == NULL)
	{
		std::cout << "[ERROR] Cannot open " << inputFile << "\n"
			<< "[ERROR] Please check your file or directory!" << std::endl;

		exit(0);
	}

	char* buffer_char = (char*)malloc((xmlBufferSize + 1) * sizeof(char));
	buffer_char[xmlBufferSize] = 0;

	int p = inputFile.find(".imzML");
	std::string ibdFileName = inputFile.substr(0, p) + ".ibd";

	FILE* ibd_file;
	ibd_file = fopen(ibdFileName.c_str(), "rb");

	if (!ibd_file)
	{
		std::cerr << "Cannot open " << ibdFileName << "\n"
			<< "Please cheak your file or directory." << std::endl;
	}

	fpos_t ibdFilePos;

#ifdef _WIN32
	ibdFilePos = 0;
#endif // !_WIN32
#ifdef __linux__
	ibdFilePos.__pos = 0;
#endif // __linux__

	int pos = 0;
	bool flag = false;
	std::string name = "";
	std::string contents = "";

	std::string position_x = "";
	std::string position_y = "";

	std::string position3D_x = "";
	std::string position3D_y = "";
	std::string position3D_z = "";

	uint64_t offset = 0;
	uint64_t encodeLength = 0;

	std::string arrayRef = "";

	// Encode mode: 64-bit float; 32-bit float; 64-bit integer; 32-bit integer
	// Compressed mode: zlib; non zlib

	int arrayLength = -1;
	std::string spectrumId = "";

	bool scanFlag = false;
	bool spectrumFlag = false;
	bool readContineMz = false;
	bool binaryArrayFlag = false;

	std::vector< std::vector<unsigned char> > encodeSpectrum;

	uint32_t spectrumIndex = 0;
	uint32_t checkPoint = (int)(0.01 * std::stoi(metaHeader.spectrumCount));

	while (1)
	{
		if (feof(xml_file)) break;
		size_t len = fread(buffer_char, 1, xmlBufferSize, xml_file);

		for (int i = 0; i < xmlBufferSize; ++i)
		{
			if (buffer_char[i] < ' ') buffer_char[i] = ' ';

			if (!flag && buffer_char[i] == '<')
			{
				flag = true;
			}
			else if (flag && buffer_char[i] != '<' && buffer_char[i] != '>')
			{
				contents += buffer_char[i];
			}
			else if (flag && buffer_char[i] == '>')
			{
				flag = false;
				contents += buffer_char[i];

				pos = contents.find(' ');
				name = contents.substr(0, pos);

				if (name == "scan")
				{
					scanFlag = true;
				}
				if (name == "/scan>")
				{
					scanFlag = false;
				}

				if (scanFlag)
				{
					if (name == "cvParam")
					{
						if (contents.find("IMS:1000050") != contents.npos)
						{
							position_x = FindImzMLKeyValue(contents, "value=\"");
						}
						if (contents.find("IMS:1000051") != contents.npos)
						{
							position_y = FindImzMLKeyValue(contents, "value=\"");
							//std::cout << position_y << std::endl;
						}
					}

					if (name == "userParam")
					{
						if (contents.find("3DPositionX") != contents.npos)
						{
							position3D_x = FindImzMLKeyValue(contents, "value=\"");
							//std::cout << position3D_x << std::endl;
						}
						if (contents.find("3DPositionY") != contents.npos)
						{
							position3D_y = FindImzMLKeyValue(contents, "value=\"");
							//std::cout << position3D_y << std::endl;
						}
						if (contents.find("3DPositionZ") != contents.npos)
						{
							position3D_z = FindImzMLKeyValue(contents, "value=\"");
							//std::cout << position3D_z << std::endl;
						}
					}
				}

				if (name == "binaryDataArrayList")
				{
					binaryArrayFlag = true;
				}
				if (name == "/binaryDataArrayList>")
				{
					binaryArrayFlag = false;
				}

				if (binaryArrayFlag)
				{
					if (name == "referenceableParamGroupRef")
					{
						arrayRef = FindImzMLKeyValue(contents, "ref=\"");
					}

					if (name == "cvParam")
					{
						if (contents.find("IMS:1000103") != contents.npos)
						{
							arrayLength = std::stoi(FindImzMLKeyValue(contents, "value=\"").c_str());
						}
						if (contents.find("IMS:1000104") != contents.npos)
						{
							encodeLength = std::stoull(FindImzMLKeyValue(contents, "value=\"").c_str());
						}
						if (contents.find("IMS:1000102") != contents.npos)
						{
							offset = std::stoull(FindImzMLKeyValue(contents, "value=\"").c_str());
						}
					}

					if (name == "binary/>")
					{
						if (arrayRef.find("mz") != arrayRef.npos &&
							metaHeader.imzMLModel == "continuous" &&
							metaHeader.continuousMzCode.length() == 0)
						{
#ifdef _WIN32
							ibdFilePos = offset;
#endif // !_WIN32
#ifdef __linux__
							ibdFilePos.__pos = offset;
#endif // __linux__
							fsetpos(ibd_file, &ibdFilePos);

							std::vector<char> ibd_buffer(encodeLength);
							fread(ibd_buffer.data(), 1, encodeLength, ibd_file);

							std::vector<char> mzCode;
							Base64Encode(ibd_buffer, mzCode);

							metaHeader.continuousMzCode.assign(mzCode.begin(), mzCode.end());

							offset = 0;
							arrayRef = "";
							encodeLength = 0;
						}

						if ((metaHeader.imzMLModel == "continuous" && arrayRef.find("inten") != arrayRef.npos) ||
							metaHeader.imzMLModel == "processed")
						{
#ifdef _WIN32
							ibdFilePos = offset;
#endif // !_WIN32
#ifdef __linux__
							ibdFilePos.__pos = offset;
#endif // __linux__
							fsetpos(ibd_file, &ibdFilePos);

							std::vector<unsigned char> ibd_buffer(encodeLength);
							fread(ibd_buffer.data(), 1, encodeLength, ibd_file);

							encodeSpectrum.push_back(ibd_buffer);

							offset = 0;
							arrayRef = "";
							encodeLength = 0;
						}
					}
				}

				if (name == "spectrum")
				{
					spectrumFlag = true;
					spectrumId = FindImzMLKeyValue(contents, "index=\"");
				}

				if (spectrumFlag)
				{
					if (name == "/spectrum>")
					{
						if (spectrumIndex == 0)
						{
							std::cout << "Process: ";
						}

						if (spectrumIndex % checkPoint == 0)
						{
							std::cout << spectrumIndex / checkPoint << "..." << std::flush;
						}

						MetaData metaData;

						metaData.spectrumId = spectrumId;
						metaData.scanIndx = spectrumIndex;

						metaData.position_x = position_x;
						metaData.position_y = position_y;

						metaData.position3D_x = position3D_x;
						metaData.position3D_y = position3D_y;
						metaData.position3D_z = position3D_z;

						metaData.arrayLength = arrayLength;
						metaData.encodeSpectrum = encodeSpectrum;

						metaQueue.push(metaData);

						++spectrumIndex;
						spectrumFlag = false;
						encodeSpectrum.clear();
					}
				}
				
				contents.clear();
			}
		}
	}

	(free)(buffer_char);

	std::cout << "Done!\n" << std::flush;

	fclose(xml_file);
	fclose(ibd_file);

	MetaData metaData;
	metaData.stop = true;

	metaQueue.push(metaData);
}

#endif // !IMZML_H
