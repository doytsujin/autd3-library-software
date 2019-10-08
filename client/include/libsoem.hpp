/*
 * File: libsoem.hpp
 * Project: include
 * Created Date: 24/08/2019
 * Author: Shun Suzuki
 * -----
 * Last Modified: 06/10/2019
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2019 Hapis Lab. All rights reserved.
 * 
 */

#pragma once

#include <memory>
#include <vector>
#include <string>

constexpr auto TRANS_NUM = 249;
constexpr auto MOD_SIZE = 124;
constexpr auto OUTPUT_FRAME_SIZE = TRANS_NUM * 2 + MOD_SIZE + 4;
constexpr auto INPUT_FRAME_SIZE = 2;

constexpr uint32_t CYCLE_TIME_MILLI_SEC = 1;
constexpr uint32_t CYCLE_TIME_MICRO_SEC = CYCLE_TIME_MILLI_SEC * 1000;
constexpr uint32_t CYCLE_TIME_NANO_SEC = CYCLE_TIME_MICRO_SEC * 1000;

using namespace std;

namespace libsoem
{
class SOEMController
{
public:
	SOEMController();
	~SOEMController();

	void Open(const char *ifname, size_t devNum);
	void Send(size_t size, unique_ptr<uint8_t[]> buf);
	void Close();

private:
	class impl;
	shared_ptr<impl> _pimpl;
};

struct EtherCATAdapterInfo
{
public:
	EtherCATAdapterInfo(){};
	EtherCATAdapterInfo(const EtherCATAdapterInfo &info)
	{
		desc = info.desc;
		name = info.name;
	}
	static vector<EtherCATAdapterInfo> EnumerateAdapters();

	shared_ptr<string> desc;
	shared_ptr<string> name;
};
} // namespace libsoem