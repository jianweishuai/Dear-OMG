#pragma warning(disable:4996)
#define NOMINMAX
#define _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_DEPRECATE

#include "namespace.h"
#include "utility.h"
#include "interface.h"

#include "omgParser.h"
#include "genomics.h"
#include "proteomics.h"
#include "metabolomics.h"

int main(int argc, char* argv[])
{
	int nThreads = 0;
	std::string readMode = "";
	std::string omgFileName = "";

	for (int i = 1; i < argc; ++i)
	{
		std::string line = argv[i];
		
		if (line.find("--mt=") != line.npos)
		{
			int pos = line.find("--mt=");
			nThreads = std::stoi(line.substr(pos + 5));
		}

		if (line.find("--read=") != line.npos)
		{
			int pos = line.find("--read=");
			readMode = line.substr(pos + 7);
		}
		if (line.find("--omg=") != line.npos)
		{
			int pos = line.find("--omg=");
			omgFileName = line.substr(pos + 6);
			for (int j = 0; j < omgFileName.length(); ++j)
			{
				if (omgFileName[j] == '\\') omgFileName[j] = '/';
			}
		}

	}

	clock_t startTime = clock();

	OMGParser parser(omgFileName, readMode);

	int nEntrys = parser.GetEntryNumber();

	int nReaders = nEntrys / nThreads;

	std::vector<std::thread> readers(nThreads);
	for (int n = 0; n < nThreads; ++n)
	{
		int st = n * nReaders;
		int ed = (n + 1) * nReaders;

		if (n == nThreads - 1 && ed < nEntrys) ed = nEntrys;
		if (ed > nEntrys) ed = nEntrys;

		std::cout << st << " " << ed << "\n";

		readers[n] = std::thread([&](int begin, int stop)
			{
				for (int i = begin; i < stop; ++i)
				{
					OMGParser::EntryData entry = parser.GetOMGFileEntry(i);
				}
			}, st, ed);

	}

	for (int n = 0; n < nThreads; ++n)
	{
		readers[n].join();
	}
	
	std::cout << "nThread: " << nThreads <<
		" elapse time: " << (double)(clock() - startTime) / CLOCKS_PER_SEC << "\n";

	/*
	for (int i = 0; i < entry.strTypeKey.size(); ++i)
	{
		std::cout << entry.strTypeKey[i] << " " << entry.strTypeValue[i] << "\n";
	}
	for (int i = 0; i < entry.numTypeKey.size(); ++i)
	{
		std::cout << entry.numTypeKey[i] << " ";
		for (int j = 0; j < entry.numTypeValue[i].size(); ++j)
		{
			std::cout << entry.numTypeValue[i][j] << " ";
		}
		std::cout << "\n\n";
	}
	*/

	return 0;
}