﻿/*
 * File: groupedgain.cpp
 * Project: lib
 * Created Date: 07/09/2018
 * Author: Shun Suzuki
 * -----
 * Last Modified: 04/09/2019
 * Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
 * -----
 * Copyright (c) 2018-2019 Hapis Lab. All rights reserved.
 * 
 */

#include <stdio.h>
#include <map>

#include "autd3.hpp"
#include "controller.hpp"
#include "gain.hpp"
#include "privdef.hpp"

using namespace autd;

GainPtr GroupedGain::Create(std::map<int, GainPtr> gainmap)
{
	auto gain = CreateHelper<GroupedGain>();
	gain->_gainmap = gainmap;
	gain->_geometry = nullptr;
	return gain;
}

void GroupedGain::build()
{
	if (this->built())
		return;
	auto geo = this->geometry();
	if (geo == nullptr)
	{
		throw runtime_error("Geometry is required to build Gain");
	}

	this->_data.clear();

	const auto ndevice = geo->numDevices();
	for (int i = 0; i < ndevice; i++)
	{
		this->_data[geo->deviceIdForDeviceIdx(i)].resize(NUM_TRANS_IN_UNIT);
	}

	for (std::pair<int, GainPtr> p : this->_gainmap)
	{
		auto g = p.second;
		g->SetGeometry(geo);
		g->build();
	}

	for (int i = 0; i < ndevice; i++)
	{
		auto groupId = geo->GroupIDForDeviceID(i);
		if (_gainmap.count(groupId))
		{
			auto data = _gainmap[groupId]->data();
			this->_data[i] = data[i];
		}
		else
		{
			this->_data[i] = std::vector<uint16_t>(NUM_TRANS_IN_UNIT, 0x0000);
		}
	}

	this->_built = true;
}
