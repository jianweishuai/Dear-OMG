#ifndef BRUKER_H
#define BRUKER_H

#include <windows.h>
#include "namespace.h"

namespace Bruker
{
	typedef uint64_t(*TIMS_OPEN)(const char* directory, bool use_recalibration);
	typedef void (*TIMS_CLOSE)(uint64_t handle);
	typedef uint32_t(*TIMS_GET_LAST_ERROR_STRING)(char* buf, uint32_t len);
	typedef uint32_t(*TIMS_HAS_RECALIBRATED_STATE)(uint64_t);
	typedef uint32_t(*TIMS_READ_SCANS_V2)(uint64_t handle, __int64 frame_id, uint32_t scan_begin, uint32_t scan_end, uint32_t* buf, uint32_t len);
	typedef uint32_t(*TIMS_INDEX_TO_MZ)(uint64_t handle, __int64 frame_id, double* in, double* out, uint32_t in_size);
	typedef uint32_t(*TIMS_MZ_TO_INDEX)(uint64_t handle, __int64 frame_id, double* in, double* out, uint32_t in_size);
	typedef uint32_t(*TIMS_SCANNUM_TO_ONEOVERK0)(uint64_t handle, __int64 frame_id, double* in, double* out, uint32_t in_size);
	typedef uint32_t(*TIMS_ONEOVERK0_TO_SCANNUM)(uint64_t handle, __int64 frame_id, double* in, double* out, uint32_t in_size);
	typedef uint32_t(*TIMS_SCANNUM_TO_VOLTAGE)(uint64_t handle, __int64 frame_id, double* in, double* out, uint32_t in_size);
	typedef uint32_t(*TIMS_VOLTAGE_TO_SCANNUM)(uint64_t handle, __int64 frame_id, double* in, double* out, uint32_t in_size);

	TIMS_OPEN tims_open;
	TIMS_CLOSE tims_close;
	TIMS_GET_LAST_ERROR_STRING tims_get_last_error_string;
	TIMS_HAS_RECALIBRATED_STATE tims_has_recalibrated_state;
	TIMS_READ_SCANS_V2 tims_read_scans_v2;
	TIMS_INDEX_TO_MZ tims_index_to_mz;
	TIMS_MZ_TO_INDEX tims_mz_to_index;
	TIMS_SCANNUM_TO_ONEOVERK0 tims_scannum_to_oneoverk0;
	TIMS_ONEOVERK0_TO_SCANNUM tims_oneoverk0_to_scannum;
	TIMS_SCANNUM_TO_VOLTAGE tims_scannum_to_voltage;
	TIMS_VOLTAGE_TO_SCANNUM tims_voltage_to_scannum;

	void InitializeBrukertion(std::string tims_dll_path);
	int callms2ids(void* data, int cNum, char* column_values[], char* column_names[]);

	HMODULE timsDll;

	union Float2Char
	{
		float Float;
		char Char[4];
	};
}

void Bruker::InitializeBrukertion(std::string tims_dll_path)
{
	Bruker::timsDll = LoadLibraryA(tims_dll_path.c_str());

	if (timsDll)
	{
		Bruker::tims_open = (TIMS_OPEN)GetProcAddress(Bruker::timsDll, "tims_open");
		Bruker::tims_close = (TIMS_CLOSE)GetProcAddress(Bruker::timsDll, "tims_close");
		Bruker::tims_get_last_error_string = (TIMS_GET_LAST_ERROR_STRING)GetProcAddress(Bruker::timsDll, "tims_get_last_error_string");
		Bruker::tims_has_recalibrated_state = (TIMS_HAS_RECALIBRATED_STATE)GetProcAddress(Bruker::timsDll, "tims_has_recalibrated_state");
		Bruker::tims_read_scans_v2 = (TIMS_READ_SCANS_V2)GetProcAddress(Bruker::timsDll, "tims_read_scans_v2");
		Bruker::tims_index_to_mz = (TIMS_INDEX_TO_MZ)GetProcAddress(Bruker::timsDll, "tims_index_to_mz");
		Bruker::tims_mz_to_index = (TIMS_MZ_TO_INDEX)GetProcAddress(Bruker::timsDll, "tims_mz_to_index");
		Bruker::tims_scannum_to_oneoverk0 = (TIMS_SCANNUM_TO_ONEOVERK0)GetProcAddress(Bruker::timsDll, "tims_scannum_to_oneoverk0");
		Bruker::tims_oneoverk0_to_scannum = (TIMS_ONEOVERK0_TO_SCANNUM)GetProcAddress(Bruker::timsDll, "tims_oneoverk0_to_scannum");
		Bruker::tims_scannum_to_voltage = (TIMS_SCANNUM_TO_VOLTAGE)GetProcAddress(Bruker::timsDll, "tims_scannum_to_voltage");
		Bruker::tims_voltage_to_scannum = (TIMS_VOLTAGE_TO_SCANNUM)GetProcAddress(Bruker::timsDll, "tims_voltage_to_scannum");
	}
	else
	{
		printf("Can not find %s. Please cheak you file or directory.\n", tims_dll_path.c_str());
	}
}

class TIMSData
{
public:
	TIMSData(const std::string& analysis_directory, bool use_recalibrated_state);

	~TIMSData()
	{
		Bruker::tims_close(this->handle);
		FreeLibrary(Bruker::timsDll);
	}

	int initial_frame_buffer_size;

	void readScans(__int64 frame_id,
		uint32_t scan_begin, uint32_t scan_end,
		std::vector< std::vector< std::vector<double> > >& result);

	void readSpectrum(__int64 frame_id, uint32_t scan_begin, uint32_t scan_end,
		std::vector< std::vector<double> >& mz_inten_pair);

	void indexToMz(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data);
	void mzToIndex(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data);
	void voltageToScanNum(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data);
	void scanNumToVoltage(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data);
	void scanNumToOneOverK0(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data);
	void oneOverK0ToScanNum(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data);

private:
	uint64_t handle;
	void throwLastTimsDataError();
};

TIMSData::TIMSData(const std::string& analysis_directory, bool use_recalibrated_state)
{
	this->handle = Bruker::tims_open(analysis_directory.c_str(), use_recalibrated_state);

	if (this->handle == 0)
	{
		this->throwLastTimsDataError();
		exit(0);
	}

	this->initial_frame_buffer_size = 128;
}

//==============================================================
// Output: list of pair <index, intensity>
//==============================================================
void TIMSData::readScans(__int64 frame_id,
	uint32_t scan_begin, uint32_t scan_end,
	std::vector< std::vector< std::vector<double> > >& result)
{
	// buffer growing loop
	std::vector<uint32_t> buff;
	while (true)
	{
		buff.resize(this->initial_frame_buffer_size, 0);

		int len = this->initial_frame_buffer_size * 4;

		uint32_t require_len = Bruker::tims_read_scans_v2(this->handle, frame_id, scan_begin, scan_end, buff.data(), len);

		if (require_len == 0)
		{
			this->throwLastTimsDataError();
		}

		if (require_len > len)
		{
			if (require_len > 16777216)
			{
				throw std::runtime_error("Maximum expected frame size exceeded.\n");
			}
			this->initial_frame_buffer_size = require_len / 4 + 1;
			buff.clear();
		}
		else
		{
			break;
		}
	}

	uint32_t delta = scan_end - scan_begin;

	uint32_t npeaks;
	for (uint32_t i = scan_begin; i < scan_end; ++i)
	{
		npeaks = buff[i - scan_begin];

		std::vector< std::vector<double> > indices_intensities(2);
		indices_intensities[0].insert(indices_intensities[0].begin(),
			buff.begin() + delta, buff.begin() + delta + npeaks);

		delta += npeaks;

		indices_intensities[1].insert(indices_intensities[1].begin(),
			buff.begin() + delta, buff.begin() + delta + npeaks);

		delta += npeaks;

		result.push_back(indices_intensities);
	}
}

void TIMSData::readSpectrum(__int64 frame_id, uint32_t scan_begin, uint32_t scan_end,
	std::vector< std::vector<double> >& mz_inten_pair)
{
	std::vector< std::vector< std::vector<double> > > scans;
	this->readScans(frame_id, scan_begin, scan_end, scans);

	std::vector<double> allindex;
	std::vector<double> allintensity;

	for (uint32_t i = 0; i < scans.size(); ++i)
	{
		if (scans[i][0].size() > 0)
		{
			allindex.insert(allindex.end(), scans[i][0].begin(), scans[i][0].end());
			allintensity.insert(allintensity.end(), scans[i][1].begin(), scans[i][1].end());
		}
	}
	std::vector<double> allmz(allindex.size(), 0.0);
	this->indexToMz(frame_id, allindex, allmz);

	mz_inten_pair.push_back(allmz);
	mz_inten_pair.push_back(allintensity);
}

void TIMSData::indexToMz(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data)
{
	uint32_t success = Bruker::tims_index_to_mz(this->handle,
		frame_id, in_data.data(), out_data.data(), (uint32_t)in_data.size());

	if (success == 0)
	{
		this->throwLastTimsDataError();
	}
}

void TIMSData::mzToIndex(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data)
{
	uint32_t success = Bruker::tims_mz_to_index(this->handle,
		frame_id, in_data.data(), out_data.data(), (uint32_t)in_data.size());

	if (success == 0)
	{
		this->throwLastTimsDataError();
	}
}

void TIMSData::scanNumToVoltage(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data)
{
	uint32_t success = Bruker::tims_scannum_to_voltage(this->handle,
		frame_id, in_data.data(), out_data.data(), (uint32_t)in_data.size());

	if (success == 0)
	{
		this->throwLastTimsDataError();
	}
}

void TIMSData::voltageToScanNum(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data)
{
	uint32_t success = Bruker::tims_voltage_to_scannum(this->handle,
		frame_id, in_data.data(), out_data.data(), (uint32_t)in_data.size());

	if (success == 0)
	{
		this->throwLastTimsDataError();
	}
}

void TIMSData::scanNumToOneOverK0(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data)
{
	uint32_t success = Bruker::tims_scannum_to_oneoverk0(this->handle,
		frame_id, in_data.data(), out_data.data(), (uint32_t)in_data.size());

	if (success == 0)
	{
		this->throwLastTimsDataError();
	}
}

void TIMSData::oneOverK0ToScanNum(uint64_t frame_id, std::vector<double>& in_data, std::vector<double>& out_data)
{
	uint32_t success = Bruker::tims_oneoverk0_to_scannum(this->handle,
		frame_id, in_data.data(), out_data.data(), (uint32_t)in_data.size());

	if (success == 0)
	{
		this->throwLastTimsDataError();
	}
}

void TIMSData::throwLastTimsDataError()
{
	//===========================================================
	// Throw last TimsData error string as an exception.
	//===========================================================
	uint32_t len = Bruker::tims_get_last_error_string(0, 0);

	char* buf = (char*)malloc(len * sizeof(char));
	Bruker::tims_get_last_error_string(buf, len);

	std::cerr << "Error occurred!" << '\n';
	exit(0);
}

void DearOMG::GetBrukerTDFBaseInfo(std::string inputFolder)
{
	Bruker::InitializeBrukertion("timsdata.dll");

	TIMSData tims_data(inputFolder.c_str(), false);

	int OK;
	int nRow;
	int nCol;
	int nFrame;
	char** pResult;
	sqlite3* connect_sqlite3;
	sqlite3_stmt* stmt = NULL;

	std::string tdf_file = inputFolder + "/analysis.tdf";

	OK = sqlite3_open(tdf_file.c_str(), &connect_sqlite3);

	if (OK != SQLITE_OK)
	{
		std::cerr << "Something was wrong when open analysis.tdf. Please cheak your file "
			<< inputFolder + "/analysis.tdf or directory." << '\n';
	}

	std::string sqlCommand = "SELECT Key, Value FROM GlobalMetadata";
	OK = sqlite3_get_table(connect_sqlite3, sqlCommand.c_str(), &pResult, &nRow, &nCol, NULL);

	if (OK != SQLITE_OK)
	{
		std::cerr << "Something was wrong when execute " << sqlCommand << std::endl;
		exit(0);
	}

	std::string key = "";
	std::string value = "";

	for (int i = 0; i < nRow; ++i)
	{
		key = pResult[2 * i];
		value = pResult[2 * i + 1];

		if (key == "SchemaType")
		{
			protHeader.schemaType = value;
		}
		if (key == "InstrumentName")
		{
			protHeader.msModel = value;
		}
	}

	sqlite3_free_table(pResult);

	sqlCommand = "select count(*) from Frames";
	OK = sqlite3_get_table(connect_sqlite3, sqlCommand.c_str(), &pResult, &nRow, &nCol, NULL);

	if (OK != SQLITE_OK)
	{
		std::cerr << "Something was wrong when execute " << sqlCommand << std::endl;
		exit(0);
	}

	nFrame = atoi(pResult[1]);
	sqlite3_free_table(pResult);

	__int64 lower_frame = 1;
	__int64 upper_frame = nFrame;

	std::vector<float> RT;
	std::vector<__int32> msms;
	std::vector<__int32> num_scans;

	sqlite3_exec(connect_sqlite3, "begin;", 0, 0, 0);

	sqlCommand = "select NumScans, Time, MsMsType from Frames where Id=?;";

	sqlite3_prepare_v2(connect_sqlite3, sqlCommand.c_str(), -1, &stmt, 0);

	for (__int64 frame_id = lower_frame; frame_id <= upper_frame; ++frame_id)
	{
		sqlite3_reset(stmt);

		sqlite3_bind_int(stmt, 1, frame_id);

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			num_scans.push_back(sqlite3_column_int(stmt, 0));
			RT.push_back((float)sqlite3_column_double(stmt, 1));
			msms.push_back(sqlite3_column_int(stmt, 2));
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(connect_sqlite3, "commit;", 0, 0, 0);

	std::ostringstream startTime;
	startTime << std::setprecision(2) << std::fixed;

	startTime << "PT" << RT[0] << "S";
	protHeader.startTime = startTime.str();

	std::ostringstream endTime;
	endTime << std::setprecision(2) << std::fixed;

	endTime << "PT" << RT.back() << "S";
	protHeader.endTime = endTime.str();

	if (msms.size() != nFrame)
	{
		std::cout << "Error in select NumScans, Time, MsMsType" << std::endl;
		exit(0);
	}

	protHeader.scanCount = std::to_string(nFrame);

	int mobilityScans = 0;
	std::vector<uint32_t> ook0_init;

	std::vector<float> mobilityValues;

	for (__int64 frame_id = lower_frame; frame_id <= upper_frame; ++frame_id)
	{
		std::vector<double> scan_number_axis(num_scans[frame_id - 1]);
		std::vector<double> ook0_axis(num_scans[frame_id - 1]);

		if (num_scans[frame_id - 1] > mobilityScans)
		{
			mobilityScans = num_scans[frame_id - 1];
		}

		for (int i = 0; i < num_scans[frame_id - 1]; ++i)
		{
			scan_number_axis[i] = (double)i;
		}

		tims_data.scanNumToOneOverK0(frame_id, scan_number_axis, ook0_axis);

		if (frame_id == 1)
		{
			for (int i = 0; i < ook0_axis.size(); ++i)
			{
				ook0_init.push_back((uint32_t)(ook0_axis[i] * 1e6));

				mobilityValues.push_back((float)ook0_axis[i]);
			}
		}
		else
		{
			for (int i = 0; i < ook0_axis.size(); ++i)
			{
				if ((uint32_t)(ook0_axis[i] * 1e6) != ook0_init[i])
				{
					std::cout << "Error in different mobility values of different frames.\n" <<
						"GPMZ does not support this diaPASEF file." << std::endl;
					exit(0);
				}
			}
		}
	}

	Bruker::Float2Char float2char;

	std::vector<char> mobilityCharArr;
	for (int i = 0; i < mobilityValues.size(); ++i)
	{
		float2char.Float = mobilityValues[i];
		for (int j = 0; j < 4; ++j)
		{
			mobilityCharArr.push_back(float2char.Char[j]);
		}
	}

	std::vector<char> mobilityBase64Code;
	Base64Encode(mobilityCharArr, mobilityBase64Code);

	protHeader.mobolityValues.assign(mobilityBase64Code.begin(), mobilityBase64Code.end());

	sqlite3_close(connect_sqlite3);
}

void DearOMG::LoadBrukerTDFFile(std::string inputFolder)
{
	Bruker::InitializeBrukertion("timsdata.dll");

	TIMSData tims_data(inputFolder.c_str(), false);

	int OK;
	int nRow;
	int nCol;
	int nFrame;
	char** pResult;
	sqlite3* connect_sqlite3;
	sqlite3_stmt* stmt = NULL;

	std::string tdf_file = inputFolder + "/analysis.tdf";

	OK = sqlite3_open(tdf_file.c_str(), &connect_sqlite3);

	if (OK != SQLITE_OK)
	{
		std::cerr << "Something was wrong when open analysis.tdf. Please cheak your file "
			<< inputFolder + "/analysis.tdf or directory." << '\n';
	}

	std::string sqlCommand = "select count(*) from Frames";
	OK = sqlite3_get_table(connect_sqlite3, sqlCommand.c_str(), &pResult, &nRow, &nCol, NULL);

	if (OK != SQLITE_OK)
	{
		std::cerr << "Something was wrong when execute " << sqlCommand << std::endl;
		exit(0);
	}

	nFrame = atoi(pResult[1]);
	sqlite3_free_table(pResult);

	std::cout << "This timsTOF .tdf file includes " << nFrame << " frames." << std::endl;

	__int64 lower_frame = 1;
	__int64 upper_frame = nFrame;

	std::vector<float> RT;
	std::vector<__int32> msms;
	std::vector<__int32> num_scans;

	sqlite3_exec(connect_sqlite3, "begin;", 0, 0, 0);

	sqlCommand = "select NumScans, Time, MsMsType from Frames where Id=?;";

	sqlite3_prepare_v2(connect_sqlite3, sqlCommand.c_str(), -1, &stmt, 0);

	for (__int64 frame_id = lower_frame; frame_id <= upper_frame; ++frame_id)
	{
		sqlite3_reset(stmt);

		sqlite3_bind_int(stmt, 1, frame_id);

		while (sqlite3_step(stmt) == SQLITE_ROW)
		{
			num_scans.push_back(sqlite3_column_int(stmt, 0));
			RT.push_back((float)sqlite3_column_double(stmt, 1));
			msms.push_back(sqlite3_column_int(stmt, 2));
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(connect_sqlite3, "commit;", 0, 0, 0);

	if (msms.size() != nFrame)
	{
		std::cout << "Error in select NumScans, Time, MsMsType" << std::endl;
		exit(0);
	}

	__int32 msmsType = -1;
	for (int i = 0; i < msms.size(); ++i)
	{
		if (msms[i] > msmsType)
		{
			msmsType = msms[i];
		}
	}

	if (msmsType != 2 && msmsType != 8 && msmsType != 9)
	{
		std::cout << "mzSQL does not support msmsType=" << msmsType << std::endl;
		exit(0);
	}

	if (msmsType == 2)
	{
		sqlCommand = "select TriggerMass, IsolationWidth, CollisionEnergy " \
			"from FrameMsMsInfo where Frame=?;";

		sqlite3_prepare_v2(connect_sqlite3, sqlCommand.c_str(), -1, &stmt, 0);
	}
	if (msmsType == 8)
	{
		sqlCommand = "select IsolationMz, IsolationWidth, \
					ScanNumBegin, ScanNumEnd, CollisionEnergy from PasefFrameMsMsInfo \
					where Frame=? ORDER BY IsolationMz ASC;";
		sqlite3_prepare_v2(connect_sqlite3, sqlCommand.c_str(), -1, &stmt, 0);
	}
	if (msmsType == 9)// new tdf 5.1 has pasef scan msms = 9
	{
		sqlCommand = "SELECT IsolationMz, IsolationWidth, \
					ScanNumBegin, ScanNumEnd, CollisionEnergy, Frame FROM DiaFrameMsMsWindows \
					INNER JOIN DiaFrameMsMsInfo ON \
					DiaFrameMsMsWindows.WindowGroup = DiaFrameMsMsInfo.WindowGroup \
					WHERE Frame=? ORDER BY IsolationMz ASC;";
		sqlite3_prepare_v2(connect_sqlite3, sqlCommand.c_str(), -1, &stmt, 0);
	}

	__int32 scan_start;
	__int32 scan_end;

	float width;
	float center;
	int cycle_id = -1;
	float collisionEnergy;

	uint32_t scanIndex = 0;
	int checkPoint = (int)(0.01 * upper_frame * 2);

	sqlite3_exec(connect_sqlite3, "begin;", 0, 0, 0);

	for (__int64 frame_id = lower_frame; frame_id <= upper_frame; ++frame_id)
	{
		if (msms[frame_id - 1] == 0) // MS1
		{
			++cycle_id;
			//std::cout << cycle_id << std::endl;
		}
		if (cycle_id == -1) continue;

		if (scanIndex == 0)
		{
			std::cout << "Process: ";
		}

		if (scanIndex % checkPoint == 0)
		{
			std::cout << scanIndex / checkPoint << "..." << std::flush;
		}

		// obtain scans in each frame
		std::vector< std::vector< std::vector<double> > > scans;
		tims_data.readScans(frame_id, 0, num_scans[frame_id - 1], scans);

		if (msms[frame_id - 1] == 0) // MS1
		{
			ProtData protData;

			protData.scanIndex = scanIndex;
			++scanIndex;

			protData.msLevel = 1;
			protData.rt = RT[frame_id - 1];

			protData.precursorMz = -1.0f;
			protData.collisionEnergy = -1.0f;

			protData.spectrum.resize(2);

			for (int i = 0; i < scans.size(); ++i)
			{
				if (scans[i][0].size() == 0) continue;

				std::vector<double> mz(scans[i][0].size());

				tims_data.indexToMz(frame_id, scans[i][0], mz);

				protData.spectrum[0].insert(protData.spectrum[0].end(),
					mz.begin(), mz.end());

				protData.spectrum[1].insert(protData.spectrum[1].end(),
					scans[i][1].begin(), scans[i][1].end());

				protData.mobilityIndex.insert(protData.mobilityIndex.end(),
					mz.size(), (uint32_t)i);
			}

			//std::cout << "size: " <<  spectrumData.spectrum[0].size() << std::endl;
			protQueue.push(protData);

		}

		if (msms[frame_id - 1] == 2)
		{
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, frame_id);

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				center = (float)sqlite3_column_double(stmt, 0);
				//width = (float)sqlite3_column_double(stmt, 1);
				collisionEnergy = (float)sqlite3_column_double(stmt, 2);

				ProtData protData;

				protData.scanIndex = scanIndex;
				++scanIndex;

				protData.msLevel = 2;
				protData.cycleId = cycle_id;
				protData.rt = RT[frame_id - 1];

				protData.precursorMz = center;
				protData.collisionEnergy = collisionEnergy;

				protData.spectrum.resize(2);

				for (int i = 0; i < scans.size(); ++i)
				{
					if (scans[i][0].size() == 0) continue;

					std::vector<double> mz(scans[i][0].size());

					tims_data.indexToMz(frame_id, scans[i][0], mz);

					protData.spectrum[0].insert(protData.spectrum[0].end(),
						mz.begin(), mz.end());

					protData.spectrum[1].insert(protData.spectrum[1].end(),
						scans[i][1].begin(), scans[i][1].end());

					protData.mobilityIndex.insert(protData.mobilityIndex.end(),
						mz.size(), (uint32_t)i);
				}

				protQueue.push(protData);
			}
		}

		if (msms[frame_id - 1] == 8 || msms[frame_id - 1] == 9)
		{
			sqlite3_reset(stmt);
			sqlite3_bind_int(stmt, 1, frame_id);

			while (sqlite3_step(stmt) == SQLITE_ROW)
			{
				center = (float)sqlite3_column_double(stmt, 0);
				//width = (float)sqlite3_column_double(stmt, 1);
				scan_start = sqlite3_column_int(stmt, 2);
				scan_end = sqlite3_column_int(stmt, 3);
				collisionEnergy = (float)sqlite3_column_double(stmt, 4);

				if (scan_end == scans.size()) scan_end = scans.size() - 1;

				ProtData protData;

				protData.scanIndex = scanIndex;
				++scanIndex;

				protData.msLevel = 2;
				protData.rt = RT[frame_id - 1];

				protData.precursorMz = center;
				protData.collisionEnergy = collisionEnergy;

				protData.spectrum.resize(2);

				for (int i = scan_start; i <= scan_end; ++i)
				{
					if (scans[i][0].size() == 0) continue;

					std::vector<double> mz(scans[i][0].size());

					tims_data.indexToMz(frame_id, scans[i][0], mz);

					protData.spectrum[0].insert(protData.spectrum[0].end(),
						mz.begin(), mz.end());

					protData.spectrum[1].insert(protData.spectrum[1].end(),
						scans[i][1].begin(), scans[i][1].end());

					protData.mobilityIndex.insert(protData.mobilityIndex.end(),
						mz.size(), (uint32_t)i);
				}



				protQueue.push(protData);

				//std::cout << center << '\t' << width << '\t' <<
				//	scan_start << '\t' << collisionEnergy << '\t' << scan_end << std::endl;

			}

			//std::cout << "\n\n";
		}
	}
	sqlite3_finalize(stmt);
	sqlite3_exec(connect_sqlite3, "commit;", 0, 0, 0);
	sqlite3_close(connect_sqlite3);

	std::cout << "Done!\n" << std::flush;

	ProtData protData;
	protData.stop = true;

	protQueue.push(protData);
}

#endif
