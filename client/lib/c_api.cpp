﻿/*
 * File: c_api.cpp
 * Project: lib
 * Created Date: 07/02/2018
 * Author: Shun Suzuki
 * -----
 * Last Modified: 04/09/2019
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2018-2019 Hapis Lab. All rights reserved.
 * 
 */

#include "autd3.hpp"
#include "privdef.hpp"
#include "autd3_c_api.h"
#include <errno.h>
#include <windows.h>
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable \
				: ALL_CODE_ANALYSIS_WARNINGS)
#include <Eigen/Geometry>
#pragma warning(pop)

using namespace autd;
using namespace Eigen;

#pragma region Controller
void AUTDCreateController(AUTDControllerHandle *out)
{
	auto *cnt = new Controller;
	*out = cnt;
}
int AUTDOpenController(AUTDControllerHandle handle, const char *location)
{
	auto *cnt = static_cast<Controller *>(handle);
	cnt->Open(LinkType::ETHERCAT, std::string(location));
	if (!cnt->isOpen())
		return ENXIO;
	return 0;
}
int AUTDAddDevice(AUTDControllerHandle handle, float x, float y, float z, float rz1, float ry, float rz2, int groupId)
{
	auto *cnt = static_cast<Controller *>(handle);
	return cnt->geometry()->AddDevice(Vector3f(x, y, z), Vector3f(rz1, ry, rz2), groupId);
}
int AUTDAddDeviceQuaternion(AUTDControllerHandle handle, float x, float y, float z, float qua_w, float qua_x, float qua_y, float qua_z, int groupId)
{
	auto *cnt = static_cast<Controller *>(handle);
	return cnt->geometry()->AddDeviceQuaternion(Vector3f(x, y, z), Quaternionf(qua_w, qua_x, qua_y, qua_z), groupId);
}
void AUTDDelDevice(AUTDControllerHandle handle, int devId)
{
	auto *cnt = static_cast<Controller *>(handle);
	cnt->geometry()->DelDevice(devId);
}
void AUTDCloseController(AUTDControllerHandle handle)
{
	auto *cnt = static_cast<Controller *>(handle);
	cnt->Close();
}
void AUTDFreeController(AUTDControllerHandle handle)
{
	auto *cnt = static_cast<Controller *>(handle);
	delete cnt;
}
void AUTDSetSilentMode(AUTDControllerHandle handle, bool mode)
{
	auto *cnt = static_cast<Controller *>(handle);
	cnt->SetSilentMode(mode);
}
#pragma endregion

#pragma region Property
bool AUTDIsOpen(AUTDControllerHandle handle)
{
	Controller *cnt = static_cast<Controller *>(handle);
	return cnt->isOpen();
}
bool AUTDIsSilentMode(AUTDControllerHandle handle)
{
	Controller *cnt = static_cast<Controller *>(handle);
	return cnt->silentMode();
}
int AUTDNumDevices(AUTDControllerHandle handle)
{
	Controller *cnt = static_cast<Controller *>(handle);
	return cnt->geometry()->numDevices();
}
int AUTDNumTransducers(AUTDControllerHandle handle)
{
	Controller *cnt = static_cast<Controller *>(handle);
	return cnt->geometry()->numTransducers();
}
float AUTDFreqency(AUTDControllerHandle handle)
{
	Controller *cnt = static_cast<Controller *>(handle);
	return cnt->geometry()->frequency();
}
size_t AUTDRemainingInBuffer(AUTDControllerHandle handle)
{
	Controller *cnt = static_cast<Controller *>(handle);
	return cnt->remainingInBuffer();
}
#pragma endregion

#pragma region Gain
void AUTDFocalPointGain(AUTDGainPtr *gain, float x, float y, float z)
{
	auto *g = FocalPointGain::Create(Vector3f(x, y, z));
	*gain = g;
}
void AUTDGroupedGain(AUTDGainPtr *gain, int *groupIDs, AUTDGainPtr *gains, int size)
{
	std::map<int, Gain *> gainmap;

	for (size_t i = 0; i < size; i++)
	{
		auto id = groupIDs[i];
		auto gain_id = gains[i];
		auto *g = static_cast<Gain *>(gain_id);
		gainmap[id] = g;
	}

	auto *ggain = GroupedGain::Create(gainmap);

	*gain = ggain;
}
void AUTDBesselBeamGain(AUTDGainPtr *gain, float x, float y, float z, float n_x, float n_y, float n_z, float theta_z)
{
	auto *g = BesselBeamGain::Create(Vector3f(x, y, z), Vector3f(n_x, n_y, n_z), theta_z);
	*gain = g;
}
void AUTDPlaneWaveGain(AUTDGainPtr *gain, float n_x, float n_y, float n_z)
{
	auto *g = PlaneWaveGain::Create(Vector3f(n_x, n_y, n_z));
	*gain = g;
}
void AUTDMatlabGain(AUTDGainPtr *gain, const char *filename, const char *varname)
{
	auto *g = MatlabGain::Create(std::string(filename), std::string(varname));
	*gain = g;
}
void AUTDCustomGain(AUTDGainPtr *gain, uint16_t *data, int dataLength)
{
	auto *g = CustomGain::Create(data, dataLength);
	*gain = g;
}
void AUTDHoloGain(AUTDGainPtr *gain, float *points, float *amps, int size)
{

	MatrixX3f holo(size, 3);
	VectorXf amp(size);
	for (int i = 0; i < size; i++)
	{
		holo(i, 0) = points[3 * i];
		holo(i, 1) = points[3 * i + 1];
		holo(i, 2) = points[3 * i + 2];
		amp(i) = amps[i];
	}

	auto *g = HoloGainSdp::Create(holo, amp);
	*gain = g;
}
void AUTDDoubleGain(AUTDGainPtr *gain, float *points, float *amps)
{
	Vector3f p1 = Vector3f(points[0], points[1], points[2]);
	Vector3f p2 = Vector3f(points[3], points[4], points[5]);

	auto *g = DoubleGain::Create(p1, p2, amps[0], amps[1]);
	*gain = g;
}
void AUTDNullGain(AUTDGainPtr *gain)
{
	auto *g = NullGain::Create();
	*gain = g;
}
void AUTDDeleteGain(AUTDGainPtr gain)
{
	auto *g = static_cast<Gain *>(gain);
	delete g;
}
#pragma endregion

#pragma region Modulation
void AUTDModulation(AUTDModulationPtr *mod, uint8_t amp)
{
	auto *m = Modulation::Create(amp);
	*mod = m;
}
void AUTDRawPCMModulation(AUTDModulationPtr *mod, const char *filename, float sampFreq)
{
	auto *m = RawPCMModulation::Create(std::string(filename), sampFreq);
	*mod = m;
}
void AUTDSawModulation(AUTDModulationPtr *mod, float freq)
{
	auto *m = SawModulation::Create(freq);
	*mod = m;
}
void AUTDSineModulation(AUTDModulationPtr *mod, float freq, float amp, float offset)
{
	auto *m = SineModulation::Create(freq, amp, offset);
	*mod = m;
}
void AUTDDeleteModulation(AUTDModulationPtr mod)
{
	auto *m = static_cast<Modulation *>(mod);
	delete m;
}
#pragma endregion

#pragma region LowLevelInterface
void AUTDAppendGain(AUTDControllerHandle handle, AUTDGainPtr gain)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto *g = static_cast<Gain *>(gain);
	cnt->AppendGain(g);
}

void AUTDAppendGainSync(AUTDControllerHandle handle, AUTDGainPtr gain)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto *g = static_cast<Gain *>(gain);
	cnt->AppendGainSync(g);
}
void AUTDAppendModulation(AUTDControllerHandle handle, AUTDModulationPtr mod)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto *m = static_cast<Modulation *>(mod);
	cnt->AppendModulation(m);
}
void AUTDAppendModulationSync(AUTDControllerHandle handle, AUTDModulationPtr mod)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto *m = static_cast<Modulation *>(mod);
	cnt->AppendModulationSync(m);
}
void AUTDAppendLateralGain(AUTDControllerHandle handle, AUTDGainPtr gain)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto *g = static_cast<Gain *>(gain);
	cnt->AppendLateralGain(g);
}
void AUTDStartLateralModulation(AUTDControllerHandle handle, float freq)
{
	auto *cnt = static_cast<Controller *>(handle);
	cnt->StartLateralModulation(freq);
}
void AUTDFinishLateralModulation(AUTDControllerHandle handle)
{
	auto *cnt = static_cast<Controller *>(handle);
	cnt->FinishLateralModulation();
}
void AUTDResetLateralGain(AUTDControllerHandle handle)
{
	auto *cnt = static_cast<Controller *>(handle);
	cnt->ResetLateralGain();
}
void AUTDSetGain(AUTDControllerHandle handle, int deviceIndex, int transIndex, int amp, int phase)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto g = TransducerTestGain::Create(deviceIndex * 249 + transIndex, amp, phase);
	cnt->AppendGainSync(g);
}
void AUTDFlush(AUTDControllerHandle handle)
{
	auto *cnt = static_cast<Controller *>(handle);
	cnt->Flush();
}
int AUTDDevIdForDeviceIdx(AUTDControllerHandle handle, int devIdx)
{
	auto *cnt = static_cast<Controller *>(handle);
	return cnt->geometry()->deviceIdForDeviceIdx(devIdx);
}
int AUTDDevIdForTransIdx(AUTDControllerHandle handle, int transIdx)
{
	auto *cnt = static_cast<Controller *>(handle);
	return cnt->geometry()->deviceIdForTransIdx(transIdx);
}
float *AUTDTransPosition(AUTDControllerHandle handle, int transIdx)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto pos = cnt->geometry()->position(transIdx);
	auto *array = new float[3];
	array[0] = pos[0];
	array[1] = pos[1];
	array[2] = pos[2];
	return array;
}
float *AUTDTransDirection(AUTDControllerHandle handle, int transIdx)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto dir = cnt->geometry()->direction(transIdx);
	auto *array = new float[3];
	array[0] = dir[0];
	array[1] = dir[1];
	array[2] = dir[2];
	return array;
}
double *GetAngleZYZ(double *rotationMatrix)
{
	Matrix3d rot;
	for (int i = 0; i < 9; i++)
	{
		rot(i / 3, i % 3) = rotationMatrix[i];
	}
	auto euler = rot.eulerAngles(2, 1, 2);
	auto *angleZYZ = new double[3];
	angleZYZ[0] = euler[0];
	angleZYZ[1] = euler[1];
	angleZYZ[2] = euler[2];
	return angleZYZ;
}
#pragma endregion

#pragma region HighLevelInterface
int AUTDSetFocalPoint(AUTDControllerHandle handle, float x, float y, float z, int amp)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto *g = FocalPointGain::Create(Vector3f(x, y, z));
	cnt->AppendGainSync(g);
	auto *m = Modulation::Create(amp);
	cnt->AppendModulationSync(m);
	return 0;
}
int AUTDSetFocalPointSine(AUTDControllerHandle handle, float x, float y, float z, float freq, float amp, float offset)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto *g = FocalPointGain::Create(Vector3f(x, y, z));
	cnt->AppendGainSync(g);
	auto *m = SineModulation::Create(freq, amp, offset);
	cnt->AppendModulationSync(m);
	return 0;
}
int AUTDSetFocalPointLM(AUTDControllerHandle handle, float x, float y, float z, float lmamp_x, float lmamp_y, float lmamp_z, float freq, uint8_t amp)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto lmamp = Vector3f(lmamp_x, lmamp_y, lmamp_z);
	auto g1 = FocalPointGain::Create(Vector3f(x, y, z) + lmamp);
	auto g2 = FocalPointGain::Create(Vector3f(x, y, z) - lmamp);
	cnt->AppendModulation(Modulation::Create(amp));
	cnt->ResetLateralGain();
	cnt->AppendLateralGain(g1);
	cnt->AppendLateralGain(g2);
	cnt->StartLateralModulation(freq);
	return 0;
}
int AUTDSetBesselBeam(AUTDControllerHandle handle, float x, float y, float z, float vec_x, float vec_y, float vec_z, float theta_z, int amp)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto *g = BesselBeamGain::Create(Vector3f(x, y, z), Vector3f(vec_x, vec_y, vec_z), theta_z);
	cnt->AppendGainSync(g);
	auto *m = Modulation::Create(amp);
	cnt->AppendModulationSync(m);
	return 0;
}
int AUTDSetBesselBeamSine(AUTDControllerHandle handle, float x, float y, float z, float vec_x, float vec_y, float vec_z, float theta_z, float freq, float amp, float offset)
{
	auto *cnt = static_cast<Controller *>(handle);
	auto g = BesselBeamGain::Create(Vector3f(x, y, z), Vector3f(vec_x, vec_y, vec_z), theta_z);
	cnt->AppendGainSync(g);
	auto m = SineModulation::Create(freq, amp, offset);
	cnt->AppendModulationSync(m);
	return 0;
}
#pragma endregion

#pragma region Debug
#ifdef UNITY_DEBUG
DebugLogFunc _debugLogFunc = nullptr;

void DebugLog(const char *msg)
{
	if (_debugLogFunc != nullptr)
		_debugLogFunc(msg);
}

void SetDebugLog(DebugLogFunc func)
{
	_debugLogFunc = func;
}

void DebugLogTest()
{
	DebugLog("Debug Log Test");
}
#endif

#pragma endregion