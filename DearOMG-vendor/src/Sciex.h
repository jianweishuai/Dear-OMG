#ifndef SCIEX_H
#define SCIEX_H

#ifdef _WIN32
#pragma managed

#include <vector>
#include <string>
#include <fstream>
#include <vcclr.h>
#include <gcroot.h>
#include <iostream>
#include <windows.h>
#include <algorithm>
#include <msclr/auto_gcroot.h>

//=========================================================================
// AB SCIEX venndor dlls
//=========================================================================
#using "./lib/SciexToolKit.dll"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace Clearcore2::DataReader;

namespace ABSCIEX
{
	ref class WiffReader
	{
	public:
		void OpenWiffFile(std::string file_name);
		int GetNumSamples();
		int GetNumExperiments(int sampleIdx);
		int GetNumCycles(int sampleIdx, int experimentIdx);

		std::string GetSampleName();
		std::string GetInstrumentName();

		void GetSpectrumIndex(int sampleIdx, int experimentIdx, int cycle_Idx,
			int& msLevel, float& precursorMz, float& RT,
			std::vector< std::vector<double> >& spectrmArr);

		IDataReader^ _wiffReader;
		IList< Clearcore2::DataReader::ISampleInformation^ >^ ISampleInfo;

	};

	void WiffReader::OpenWiffFile(std::string file_name)
	{
		System::String^ _wiffFile = gcnew System::String(file_name.c_str());

		this->_wiffReader = DataReaderFactory::CreateReader();
		this->ISampleInfo = this->_wiffReader->ExtractSampleInformation(_wiffFile);
	}

	int WiffReader::GetNumSamples()
	{
		return this->ISampleInfo->Count;
	}

	std::string WiffReader::GetInstrumentName()
	{
		const char* chars =
			(const char*)(Marshal::StringToHGlobalAnsi(this->ISampleInfo[0]->InstrumentName)).ToPointer();

		std::string tmp = chars;
		std::string instrument = "";

		for (int i = 0; i < tmp.length(); ++i)
		{
			if (tmp[i] != ' ')
			{
				instrument += tmp[i];
			}
		}
		Marshal::FreeHGlobal(IntPtr((void*)chars));

		return instrument;
	}

	std::string WiffReader::GetSampleName()
	{
		const char* chars =
			(const char*)(Marshal::StringToHGlobalAnsi(this->ISampleInfo[0]->SampleName)).ToPointer();

		std::string tmp = chars;
		std::string sampleName = "";

		for (int i = 0; i < tmp.length(); ++i)
		{
			if (tmp[i] != ' ')
			{
				sampleName += tmp[i];
			}
		}
		Marshal::FreeHGlobal(IntPtr((void*)chars));

		return sampleName;
	}

	int WiffReader::GetNumExperiments(int sampleIdx)
	{
		return this->ISampleInfo[sampleIdx]->NumberOfExperiments;
	}

	int WiffReader::GetNumCycles(int sampleIdx, int experimentIdx)
	{
		return this->ISampleInfo[sampleIdx]->GetExperiment(experimentIdx)->NumberOfScans;
	}

	void WiffReader::GetSpectrumIndex(int sampleIdx, int experimentIdx, int cycle_Idx,
		int& msLevel, float& precursorMz, float& RT,
		std::vector< std::vector<double> >& spectrmArr)
	{
		gcroot<IExperiment^> _experiment = this->ISampleInfo[sampleIdx]->GetExperiment(experimentIdx);
		gcroot< Clearcore2::DataReader::IXyData<double>^ > _spectrum = _experiment->GetSpectrum(cycle_Idx);

		msLevel = _experiment->GetMsLevel(cycle_Idx);

		if (msLevel == 2)
		{
			precursorMz = (float)_experiment->GetPrecursorMz(cycle_Idx);
		}
		else
		{
			precursorMz = -1.0f;
		}

		RT = (float)_experiment->GetRetentionTimeFromScanIndex(cycle_Idx) * 60.0f;

		//float isowidth = _experiment->IsolationWidth;

		//std::cout << (_experiment->GetTotalIonChromatogram()->GetYValue)(0) << std::endl; // totIonCurrent

		//std::cout << _experiment->IsPolarityPositive << std::endl;

		//std::cout << _experiment->MassRange->Start << '\t' << _experiment->MassRange->End << std::endl;

		spectrmArr.resize(2, std::vector<double>(_spectrum->Count));
		for (int i = 0; i < _spectrum->Count; ++i)
		{
			spectrmArr[0][i] = (double)(_spectrum->GetXValue)(i);
			spectrmArr[1][i] = (double)(_spectrum->GetYValue)(i);
		}
	}
}

void DearOMG::LoadWiffFile(std::string inputFile)
{
	ABSCIEX::WiffReader reader;

	reader.OpenWiffFile(inputFile);

	protHeader.msModel = reader.GetInstrumentName();
	protHeader.sampleName = reader.GetSampleName();

	int n_samples = reader.GetNumSamples();

	int n_experiments = reader.GetNumExperiments(0); // total windows = ms1 windows + ms2 windows
	int n_cycles = reader.GetNumCycles(0, 0); // the number of cycles
	int scanCount = n_experiments * n_cycles;
	int checkPoint = (int)(0.01 * scanCount);

	protHeader.scanCount = std::to_string(scanCount);

	if (n_samples > 1)
	{
		std::cout << "Just support n_samples = 1" << std::endl;
		exit(0);
	}

	float rt = 0.0f;
	int msLevel = 0;
	int cycle_id = -1;
	float precursorMz = 0.0f;
	float retentionTime = 0.0f;

	uint32_t scanIndex = 0;
	std::vector<float> rtList;

	for (int n_c = 0; n_c < n_cycles; ++n_c)
	{
		for (int n_e = 0; n_e < n_experiments; ++n_e)
		{
			ProtData protData;
			reader.GetSpectrumIndex(0, n_e, n_c, msLevel, precursorMz, rt, protData.spectrum);

			if (msLevel == 1)
			{
				++cycle_id;
			}
			if (cycle_id == -1) continue;

			protData.rt = rt;
			rtList.push_back(rt);

			if (scanIndex == 0)
			{
				std::cout << "Process: ";
			}

			if (scanIndex % checkPoint == 0)
			{
				std::cout << scanIndex / checkPoint << "..." << std::flush;
			}

			if (msLevel == 1)
			{
				protData.scanIndex = scanIndex;

				protData.msLevel = 1;
				protData.charge = 0;
				protData.precursorMz = -1;

				ProteomeQueue.push(protData);

				++scanIndex;
			}

			if (msLevel == 2)
			{
				protData.scanIndex = scanIndex;

				protData.msLevel = 2;
				protData.charge = 0;
				protData.precursorMz = precursorMz;

				ProteomeQueue.push(protData);

				++scanIndex;
			}
		}
	}

	std::cout << "Done!\n" << std::flush;

	ProtData protData;
	protData.stop = true;

	ProteomeQueue.push(protData);

	std::ostringstream startTime;
	startTime << std::setprecision(2) << std::fixed;

	startTime << "PT" << rtList[0] << "S";
	protHeader.startTime = startTime.str();

	std::ostringstream endTime;
	endTime << std::setprecision(2) << std::fixed;

	endTime << "PT" << rtList.back() << "S";
	protHeader.endTime = endTime.str();

}

#endif

#endif

