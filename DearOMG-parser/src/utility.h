#ifndef UTILITY_H
#define UTILITY_H

#include "namespace.h"

void OMGParser::PaserTextData(std::string& text, std::string& title, 
	std::vector<std::string>& keyList, std::vector<std::string>& valueList)
{
	bool flag = false;
	std::string key = "";
	std::string value = "";

	int lineCount = 0;
	for (int i = 0; i < text.length(); ++i)
	{
		if (text[i] == '{' || text[i] == '}' ||
			text[i] == '\"' || text[i] == ',' || text[i] == ' ') continue;

		if (text[i] == '\n')
		{
			++lineCount;
			if (lineCount == 1)
			{
				title = key;
			}
			else
			{
				if (key.length() > 0)
				{
					keyList.push_back(key);
					valueList.push_back(value);
				}
			}

			key = "";
			value = "";
			flag = false;
		}
		else
		{
			if (text[i] == ':' && !flag)
			{
				flag = true;
				continue;
			}
			
			if (!flag)
			{
				key += text[i];
			}
			else
			{
				value += text[i];
			}
		}
	}
}

void OMGParser::ZSTDDecode(std::vector<char>& input, std::vector<char>& output)
{
	unsigned long long BufSize = ZSTD_getDecompressedSize(input.data(), input.size());

	output.resize(BufSize);
	size_t dstSize = ZSTD_decompress(output.data(), BufSize, input.data(), input.size());

	if (ZSTD_isError(dstSize))
	{
		std::cout << "Error in ZSTD decompress!" << std::endl;
		exit(0);
	}
}

void OMGParser::Base64Decode(std::string& input, std::vector<char>& output)
{
	size_t srcSize = Base64::textToBinarySize(input.length());

	output.resize(srcSize);
	size_t binaryCount = Base64::textToBinary(input.c_str(), input.length(), &output[0]);
	output.resize(binaryCount);
}

void OMGParser::EliasFanoDecode32(std::vector<uint32_t>& EFCode, std::vector<uint32_t>& x)
{
	//std::cout << "===========Decoding==============" << std::endl;

	uint32_t n = EFCode[EFCode.size() - 2];
	uint32_t totalBits = 32 * (EFCode.size() - 2);

	uint32_t Length = EFCode.back() % 100;
	uint32_t upperBits = (uint32_t)(EFCode.back() / 100);
	uint32_t lowerBits = totalBits - upperBits;

	uint32_t beginBit = totalBits;

	uint32_t bitMod = 0;

	uint32_t lowerIdx = 0;
	uint32_t upperIdx = 0;

	uint32_t count0 = 0;
	uint32_t count1 = 0;

	uint32_t count = 0;
	uint32_t inferior = 0;

	x.resize(n, 0);

	for (uint32_t i = 0; i < n; ++i)
	{
		for (uint32_t j = beginBit; j > lowerBits; --j)
		{
			if ((bitMod = (j & 31)) == 0) bitMod = 32; // (j % 32) == (j & 31)

			if ((EFCode[upperIdx >> 5] >> (bitMod - 1)) & 1)
			{
				++count1;
			}
			else
			{
				++count0;
			}

			++upperIdx;

			if (count1 >= (i + 1))
			{
				beginBit = j - 1;
				break;
			}
		}

		lowerIdx = upperBits + i * Length;

		count = Length - 1;
		inferior = 0;

		for (int k = lowerBits - i * Length;
			k > lowerBits - (i + 1) * Length; --k)
		{
			if ((bitMod = (k & 31)) == 0) bitMod = 32;

			if ((EFCode[lowerIdx >> 5] >> (bitMod - 1)) & 1)
			{
				inferior += 1 << count;
			}
			--count;
			++lowerIdx;
		}

		x[i] = (count0 << Length) + inferior;
	}
}

void OMGParser::EliasFanoDecode64(std::vector<uint64_t>& EFCode, std::vector<uint64_t>& x)
{
	//std::cout << "===========Decoding==============" << std::endl;

	uint64_t n = EFCode[EFCode.size() - 2];
	uint64_t totalBits = 64ULL * (EFCode.size() - 2);

	uint64_t Length = EFCode.back() % 100ULL;
	uint64_t upperBits = (uint64_t)(EFCode.back() / 100ULL);
	uint64_t lowerBits = totalBits - upperBits;

	uint64_t beginBit = totalBits;

	uint64_t bitMod = 0;

	uint64_t lowerIdx = 0;
	uint64_t upperIdx = 0;

	uint64_t count0 = 0;
	uint64_t count1 = 0;

	uint64_t count = 0;
	uint64_t inferior = 0;

	x.resize(n, 0);

	for (uint64_t i = 0; i < n; ++i)
	{
		for (uint64_t j = beginBit; j > lowerBits; --j)
		{
			if ((bitMod = (j & 63ULL)) == 0) bitMod = 64ULL; // (j % 64) == (j & 63)

			if ((EFCode[upperIdx >> 6ULL] >> (bitMod - 1ULL)) & 1ULL)
			{
				++count1;
			}
			else
			{
				++count0;
			}

			++upperIdx;

			if (count1 >= (i + 1))
			{
				beginBit = j - 1;
				break;
			}
		}

		lowerIdx = upperBits + i * Length;

		count = Length - 1;
		inferior = 0;

		for (int k = lowerBits - i * Length;
			k > lowerBits - (i + 1) * Length; --k)
		{
			if ((bitMod = (k & 63ULL)) == 0) bitMod = 64ULL;

			if ((EFCode[lowerIdx >> 6ULL] >> (bitMod - 1ULL)) & 1ULL)
			{
				inferior += 1ULL << count;
			}
			--count;
			++lowerIdx;
		}

		x[i] = (count0 << Length) + inferior;
	}
}

#endif // !UTILITY_H
