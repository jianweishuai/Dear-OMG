#ifndef FASTQ_H
#define FASTQ_H

#include <string>
#include <fstream>
#include <iostream>

#include "namespace.h"

void DearOMG::LoadFastqFile(std::string inputFile)
{
	struct __stat64 fileStat;

	_stat64(inputFile.c_str(), &fileStat);
	int64_t fileVolume = fileStat.st_size; // Bytes

	FILE* fastqFile = fopen(inputFile.c_str(), "r");

	if (fastqFile == NULL)
	{
		std::cout << "Cannot open " << inputFile << "\n"
			<< "Please check your file or directory!" << std::endl;
		exit(0);
	}

	char* buffer = (char*)malloc((fastqBufferSize + 1) * sizeof(char));
	buffer[fastqBufferSize] = 0;

	int lineCount = 0;
	std::string contents = "";
	int nLines = fastqBatchSize * 4;

	uint32_t index = 0;
	uint32_t readStartId = 0;
	uint32_t estReadCount = 0;
	std::vector<std::string> readsData;

	uint32_t checkPoint = 0;

	while (true)
	{
		if (feof(fastqFile)) break;

		size_t len = fread(buffer, 1, fastqBufferSize, fastqFile);

		for (int i = 0; i < len; ++i)
		{
			if (buffer[i] == '\r') continue;

			if (buffer[i] != '\n')
			{
				contents += buffer[i];
			}
			else
			{
				++lineCount;

				readsData.push_back(contents);

				if ((lineCount & 3) == 0)
				{
					++index;
				}

				if (lineCount == nLines)
				{
					if (readStartId == 0)
					{
						genoHeader.readLength = std::to_string(readsData[1].length());
						estReadCount = fileVolume / (3 * readsData[1].length());

						checkPoint = (uint32_t)(0.01 * estReadCount);
					}

					GenoData genoData;
					
					genoData.readsData = readsData;

					genoData.readEndId = index - 1;
					genoData.readStartId = readStartId;

					genoQueue.push(genoData);

					readStartId = index;

					lineCount = 0;
					readsData.clear();

					if (readStartId == 0)
					{
						std::cout << "Process: ";
					}
					if (index % checkPoint == 0)
					{
						if (index < estReadCount)
						{
							std::cout << index / checkPoint << "..." << std::flush;
						}
					}
				}

				contents = "";
			}
		}
	}

	if (readsData.size() > 0)
	{
		GenoData genoData;

		genoData.readsData = readsData;

		genoData.readEndId = index - 1;
		genoData.readStartId = readStartId;

		genoQueue.push(genoData);

		readsData.clear();
	}

	genoHeader.readCount = std::to_string(index);

	GenoData genoData;
	genoData.stop = true;
	genoQueue.push(genoData);

	free(buffer);
	fclose(fastqFile);

	std::cout << "100...Done!\n" << std::flush;

}

#endif // !FASTQ_H