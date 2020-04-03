// File: simple_soem.cpp
// Project: example_soem
// Created Date: 24/08/2019
// Author: Shun Suzuki
// -----
// Last Modified: 03/04/2020
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2019-2020 Hapis Lab. All rights reserved.
//

#include <iostream>

#include "autd3.hpp"

using namespace std;

string GetAdapterName() {
  int size;
  auto adapters = autd::Controller::EnumerateAdapters(&size);
  for (auto i = 0; i < size; i++) {
    auto adapter = adapters[i];
    cout << "[" << i << "]: " << adapter.first << ", " << adapter.second << endl;
  }

  int index;
  cout << "Choose number: ";
  cin >> index;
  cin.ignore();

  return adapters[index].second;
}

int main() {
  auto autd = autd::Controller::Create();

  // AddDevice() must be called before Open(), and be called as many times as for the number of AUTDs connected.
  autd->geometry()->AddDevice(autd::Vector3(0, 0, 0), autd::Vector3(0, 0, 0));
  // autd->geometry()->AddDevice(autd::Vector3(0, 0, 0), autd::Vector3(0, 0, 0));

  // If you have already recognized the EtherCAT adapter name, you can write it directly like below.
  // autd->Open(autd::LinkType::SOEM, "\\Device\\NPF_{B5B631C6-ED16-4780-9C4C-3941AE8120A6}");
  auto ifname = GetAdapterName();
  autd->Open(autd::LinkType::SOEM, ifname);

  if (!autd->is_open()) return ENXIO;

  auto firm_info_list = autd->firmware_info_list();
  for (auto firm_info : firm_info_list) std::cout << firm_info << std::endl;

  // If you use more than one AUTD, call CalibrateModulation() only once after Open().
  // It takes several seconds and blocks the thread in the meantime.
  // autd->CalibrateModulation();

  auto m = autd::SineModulation::Create(150);  // 150Hz AM
  autd->AppendModulationSync(m);

  auto g = autd::FocalPointGain::Create(autd::Vector3(90, 70, 150));
  autd->AppendGainSync(g);

  cout << "press any key to finish..." << endl;
  getchar();

  cout << "disconnecting..." << endl;
  autd->Close();

  return 0;
}
