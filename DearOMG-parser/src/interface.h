#ifndef INTERFACE_H
#define INTERFACE_H

#include "namespace.h"

int OMGParser::GetEntryNumber()
{
	return entryTable.size();
}

OMGParser::EntryData OMGParser::GetOMGFileEntry(int entryId)
{
	if (this->omics == "Genomics")
	{
		return GenomicsEntry(entryId);
	}
	if (this->omics == "Proteomics")
	{
		return ProteomicsEntry(entryId);
	}
	if (this->omics == "Metabolomics")
	{
		return MetabolomicsEntry(entryId);
	}
}

OMGParser::EntryData OMGParser::GetOMGFileBasicInfo()
{
	EntryData entry;

	std::string title;
	std::vector<std::string> keyList;
	std::vector<std::string> valueList;

	PaserTextData(basicInfo, title, keyList, valueList);

	for (int i = 0; i < keyList.size(); ++i)
	{
		if ((keyList[i] == "mobilityValue" && valueList[i] != "None") ||
			(keyList[i] == "continuousMzCode" && valueList[i].length() > 0))
		{
			std::vector<char> src;
			Base64Decode(valueList[i], src);

			Char2Float char2float;
			std::vector<float> floatArr;

			for (int i = 0; i < src.size() / 4; ++i)
			{
				for (int j = 0; j < 4; ++j)
				{
					char2float.Char[j] = src[i * 4 + j];
				}
				floatArr.push_back(char2float.Float);
			}

			entry.numTypeKey.push_back(keyList[i]);
			entry.numTypeValue.push_back(floatArr);
		}
		else
		{
			entry.strTypeKey.push_back(keyList[i]);
			entry.strTypeValue.push_back(valueList[i]);
		}
	}

	return entry;
}


#endif // !INTERFACE_H

