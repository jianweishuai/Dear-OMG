#ifndef THERMOFISHER_H
#define THERMOFISHER_H

#ifdef _WIN32
#pragma managed

#include <vcclr.h>
#include <iostream>
#include <gcroot.h>
#include <windows.h>
#include <msclr/auto_gcroot.h>

#include "namespace.h"

//=========================================================================
// ThermoFisher dlls
//=========================================================================
#using "./lib/ThermoFisher.CommonCore.Data.dll"
#using "./lib/ThermoFisher.CommonCore.RawFileReader.dll"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;

using namespace ThermoFisher::CommonCore::Data;
using namespace ThermoFisher::CommonCore::Data::Business;
using namespace ThermoFisher::CommonCore::Data::Interfaces;
using namespace ThermoFisher::CommonCore::Data::FilterEnums;
using namespace ThermoFisher::CommonCore::RawFileReader;

namespace ThermoFisher
{
	inline std::string ToStdString(System::String^ source)
	{
		const char* chars =
			(const char*)(Marshal::StringToHGlobalAnsi(source)).ToPointer();

		std::string target = chars;
		Marshal::FreeHGlobal(IntPtr((void*)chars));

		return target;
	}

	class ThermoRawReader
	{
	public:
		msclr::auto_gcroot<IRawDataPlus^> _rawFile;

		void OpenRawFile(std::string rawFile);
		std::string GetInstrumentFriendlyName();
	};

	void ThermoRawReader::OpenRawFile(std::string rawFile)
	{
		System::String^ _rawFile = gcnew System::String(rawFile.c_str());
		this->_rawFile = RawFileReaderAdapter::FileFactory(_rawFile);

		//if (this->_rawFile->InstrumentCount > 1)
		//{
		//	std::cout << "The number of instruments largeer than 1." << std::endl;
		//}
		this->_rawFile->SelectInstrument(Device::MS, 1);

	}

	std::string ThermoRawReader::GetInstrumentFriendlyName()
	{
		array<System::String^>^ InstrumentNames = _rawFile->GetAllInstrumentFriendlyNamesFromInstrumentMethod();
		std::string instrumentName = ToStdString(InstrumentNames[InstrumentNames->Length - 1]);

		return instrumentName;
	}
}

//=========================================================================
// Parser Thermo file (*.raw)
//=========================================================================

void DearOMG::LoadThermoRawFile(std::string inputFile)
{
	ThermoFisher::ThermoRawReader reader;
	reader.OpenRawFile(inputFile);

	int scan_st = reader._rawFile->RunHeaderEx->FirstSpectrum;
	int scan_ed = reader._rawFile->RunHeaderEx->LastSpectrum;

	protHeader.scanCount = std::to_string(scan_ed - scan_st + 1);
	protHeader.sampleName = ThermoFisher::ToStdString(reader._rawFile->SampleInformation->SampleName);

	protHeader.msModel = reader.GetInstrumentFriendlyName();

	gcroot<LogEntry^> trailer = reader._rawFile->GetTrailerExtraInformation(scan_st);
	array<System::String^>^ labels = trailer->Labels;

	int chargeIdx = 0;
	for (int k = 0; k < labels->Length; ++k)
	{
		std::string strLabel = ThermoFisher::ToStdString(labels[k]);
		if (strLabel == "Charge State:")
		{
			chargeIdx = k;
			break;
		}
	}

	std::ostringstream startTime;
	startTime << std::setprecision(2) << std::fixed;

	startTime << "PT" << reader._rawFile->RetentionTimeFromScanNumber(scan_st) * 60.0f << "S";
	protHeader.startTime = startTime.str();

	std::ostringstream endTime;
	endTime << std::setprecision(2) << std::fixed;
	
	endTime << "PT" << reader._rawFile->RetentionTimeFromScanNumber(scan_ed) * 60.0f << "S";
	protHeader.endTime = endTime.str();

	float rt = 0.0f;
	int msLevel = 0;
	int cycle_id = -1;
	float precursorMz = 0.0f;
	float retentionTime = 0.0f;
	
	uint32_t scanIndex = 0;
	int checkPoint = (int)(0.01 * (scan_ed - scan_st + 1));

	for (int i = scan_st; i <= scan_ed; ++i)
	{
		double rt = reader._rawFile->RetentionTimeFromScanNumber(i);

		gcroot<IScanFilter^> info = reader._rawFile->GetFilterForScanNumber(i);

		std::string msLevel = ThermoFisher::ToStdString(info->MSOrder.ToString());

		gcroot<SegmentedScan^> scanFromScanNumber = reader._rawFile->GetSegmentedScanFromScanNumber(i, (ScanStatistics^)nullptr);

		int length = scanFromScanNumber->Positions->Length;

		ProtData protData;
		protData.spectrum.resize(2, std::vector<double>(length));

		for (int i = 0; i < length; ++i)
		{
			protData.spectrum[0][i] = (scanFromScanNumber->Positions[i]);
			protData.spectrum[1][i] = (scanFromScanNumber->Intensities[i]);
		}

		if (msLevel == "Ms")
		{
			++cycle_id;
		}
		if (cycle_id == -1) continue;

		if (scanIndex == 0)
		{
			std::cout << "[INFO] Process: ";
		}

		if (scanIndex % checkPoint == 0)
		{
			std::cout << scanIndex / checkPoint << "..." << std::flush;
		}

		protData.rt = rt;

		if (msLevel == "Ms")
		{
			protData.msLevel = 1;
			protData.charge = 0;
			protData.precursorMz = -1;
		}

		if (msLevel != "Ms")
		{
			if (msLevel == "Ms2") protData.msLevel = 2;
			if (msLevel == "Ms3") protData.msLevel = 3;

			protData.precursorMz = info->GetMass(0);

			//std::string chargeState = ThermoFisher::ToStdString(reader._rawFile->GetTrailerExtraInformation(i)->Values[chargeIdx]);
			
			std::string chargeState = "";

			if (chargeState.length() > 0)
			{
				protData.charge = std::stoi(chargeState);
			}
			else
			{
				protData.charge = 0;
			}
		}

		protData.scanIndex = scanIndex;
		ProteomeQueue.push(protData);

		++scanIndex;
	}

	std::cout << "Done!\n" << std::flush;

	ProtData protData;
	protData.stop = true;

	ProteomeQueue.push(protData);
}
#endif

#endif