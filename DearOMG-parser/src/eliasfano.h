#ifndef ELIASFANO_H
#define ELIASFANO_H

#include "namespace.h"

void DearOMG::EliasFanoEncode32(std::vector<uint32_t>& x, std::vector<uint32_t>& EFCode)
{
	uint32_t n = x.size();
	uint32_t U = x.back();

	uint32_t Length;

	if (U <= n)
	{
		Length = 0;
	}
	else
	{
		Length = (uint32_t)(std::ceilf(std::log2f((float)U / n))); // Length = int( log2[U/n] )
	}

	uint32_t space = n * (Length + 2);

	uint32_t m = space >> 5;
	if ((space & 31) != 0) ++m;

	EFCode.resize(m, 0);

	uint32_t bitIdx = 0;

	for (uint32_t i = 0; i < n; ++i)
	{
		if (i == 0)
		{
			bitIdx += x[i] >> Length;
		}
		else
		{
			bitIdx += (x[i] >> Length) - (x[i - 1] >> Length);
		}

		EFCode[bitIdx >> 5] |= (1 << (31 - (bitIdx & 31)));

		++bitIdx;
	}

	uint32_t upperBits = bitIdx;

	if (Length > 0)
	{
		uint32_t mod;

		for (uint32_t i = 0; i < n; ++i)
		{
			mod = x[i] & ((1 << Length) - 1);

			for (uint32_t j = Length; j >= 1; --j)
			{
				if ((mod >> (j - 1)) & 1)
				{
					EFCode[bitIdx >> 5] |= (1 << (31 - (bitIdx & 31)));
				}
				++bitIdx;
			}
		}
	}

	EFCode.emplace_back(n);
	EFCode.emplace_back(upperBits * 100 + Length);

#if 0
	std::cout << "===========Encoding==============" << std::endl;
	for (int i = 0; i < m; ++i)
	{
		std::bitset<32> a(EFCode[i]);
		std::cout << a << '\n';
	}
	std::cout << std::endl;
#endif

}


void DearOMG::EliasFanoDecode32(std::vector<uint32_t>& EFCode, std::vector<uint32_t>& x)
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

void DearOMG::EliasFanoEncode64(std::vector<uint64_t>& x, std::vector<uint64_t>& EFCode)
{
	uint64_t n = x.size();
	uint64_t U = x.back();

	uint64_t Length;

	if (U <= n)
	{
		Length = 0;
	}
	else
	{
		Length = (uint64_t)(std::ceilf(std::log2f((float)U / n))); // Length = int( log2[U/n] )
	}

	uint64_t space = n * (Length + (uint64_t)(2));

	uint64_t m = space >> (uint64_t)(6);
	if ((space & (uint64_t)(63)) != 0) ++m;

	EFCode.resize(m, 0);

	uint64_t bitIdx = 0;

	for (uint64_t i = 0; i < n; ++i)
	{
		if (i == 0)
		{
			bitIdx += x[i] >> Length;
		}
		else
		{
			bitIdx += (x[i] >> Length) - (x[i - 1] >> Length);
		}

		EFCode[bitIdx >> (uint64_t)(6)] |= ((uint64_t)(1) << ((uint64_t)(63) - (bitIdx & (uint64_t)(63))));

		++bitIdx;
	}

	uint64_t upperBits = bitIdx;

	if (Length > 0)
	{
		uint64_t mod;

		for (uint64_t i = 0; i < n; ++i)
		{
			mod = x[i] & (((uint64_t)(1) << Length) - 1);

			for (uint64_t j = Length; j >= 1; --j)
			{
				if ((mod >> (j - (uint64_t)(1))) & (uint64_t)(1))
				{
					EFCode[bitIdx >> (uint64_t)(6)] |= ((uint64_t)(1) << ((uint64_t)(63) - (bitIdx & (uint64_t)(63))));
				}
				++bitIdx;
			}
		}
	}

	EFCode.emplace_back(n);
	EFCode.emplace_back(upperBits * 100 + Length);

#if 0
	std::cout << "===========Encoding==============" << std::endl;
	for (int i = 0; i < m; ++i)
	{
		std::bitset<64> a(EFCode[i]);
		std::cout << a << '\n';
	}
	std::cout << std::endl;
#endif

}


void DearOMG::EliasFanoDecode64(std::vector<uint64_t>& EFCode, std::vector<uint64_t>& x)
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

#endif
