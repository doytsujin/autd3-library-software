﻿// File: ethercat_link.cpp
// Project: lib
// Created Date: 01/06/2016
// Author: Seki Inoue
// -----
// Last Modified: 01/07/2020
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2016-2020 Hapis Lab. All rights reserved.
//

#include "twincat_link.hpp"

#if _WINDOWS
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)
#endif
#include <AdsLib.h>
#if _WINDOWS
#pragma warning(pop)
#define NOMINMAX
#include <Windows.h>
#include <winnt.h>
#else
typedef void *HMODULE;
#endif

#include <algorithm>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "../lib/ec_config.hpp"
#include "../lib/privdef.hpp"

constexpr uint32_t INDEX_GROUP = 0x3040030;
constexpr uint32_t INDEX_OFFSET_BASE = 0x81000000;
constexpr uint32_t INDEX_OFFSET_BASE_READ = 0x80000000;
constexpr uint16_t PORT = 301;

namespace autd::link {

static inline std::vector<std::string> split(const std::string &s, char delim) {
  std::vector<std::string> tokens;
  std::string token;
  for (char ch : s) {
    if (ch == delim) {
      if (!token.empty()) tokens.push_back(token);
      token.clear();
    } else {
      token += ch;
    }
  }
  if (!token.empty()) tokens.push_back(token);
  return tokens;
}

class TwinCATLinkImpl : public TwinCATLink {
 public:
  ~TwinCATLinkImpl() override{};
  std::string _ams_net_id;
  std::string _ipv4addr;
  long _port = 0L;  // NOLINT
  AmsNetId _netId;

 protected:
  void Open() override;
  void Close() override;
  void Send(size_t size, std::unique_ptr<uint8_t[]> buf) override;
  std::vector<uint8_t> Read(uint32_t buffer_len) override;
  bool is_open() final;
};

LinkPtr TwinCATLink::Create(std::string ams_net_id) { return Create("", ams_net_id); }

LinkPtr TwinCATLink::Create(std::string ipv4addr, std::string ams_net_id) {
  auto link = std::make_shared<TwinCATLinkImpl>();
  link->_ipv4addr = ipv4addr;
  link->_ams_net_id = ams_net_id;

  return link;
}

void TwinCATLinkImpl::Open() {
  auto octets = split(_ams_net_id, '.');
  if (octets.size() != 6) {
    throw std::runtime_error("Ams net id must have 6 octets");
  }
  if (_ipv4addr == "") {
    // Extract ipv6 addr from leading four octets of the ams net id.
    for (int i = 0; i < 3; i++) {
      _ipv4addr += octets[i] + ".";
    }
    _ipv4addr += octets[3];
  }
  this->_netId = {static_cast<uint8_t>(std::stoi(octets[0])), static_cast<uint8_t>(std::stoi(octets[1])), static_cast<uint8_t>(std::stoi(octets[2])),
                  static_cast<uint8_t>(std::stoi(octets[3])), static_cast<uint8_t>(std::stoi(octets[4])), static_cast<uint8_t>(std::stoi(octets[5]))};
  if (AdsAddRoute(this->_netId, _ipv4addr.c_str())) {
    std::cerr << "Error: Could not connect to remote." << std::endl;
    return;
  }
  // open a new ADS port
  this->_port = AdsPortOpenEx();
  if (!this->_port) {
    std::cerr << "Error: Failed to open a new ADS port." << std::endl;
  }
}

void TwinCATLinkImpl::Close() {
  this->_port = 0;
  AdsPortCloseEx(this->_port);
}

bool TwinCATLinkImpl::is_open() { return (this->_port > 0); }

void TwinCATLinkImpl::Send(size_t size, std::unique_ptr<uint8_t[]> buf) {
  const AmsAddr pAddr = {this->_netId, PORT};
  long ret = AdsSyncWriteReqEx(this->_port,  // NOLINT
                               &pAddr, INDEX_GROUP, INDEX_OFFSET_BASE, static_cast<uint32_t>(size), &buf[0]);
  if (ret > 0) {
    switch (ret) {
      case ADSERR_DEVICE_INVALIDSIZE:
        std::cerr << "The number of devices is invalid." << std::endl;
        break;
      default:
        std::cerr << "Error on sending data: " << std::hex << ret << std::endl;
    }
    throw static_cast<int>(ret);
  }
}

std::vector<uint8_t> TwinCATLinkImpl::Read(uint32_t buffer_len) {
  const AmsAddr pAddr = {this->_netId, PORT};
  const auto buffer = std::make_unique<uint8_t[]>(buffer_len);
  uint32_t read_bytes;
  auto ret = AdsSyncReadReqEx2(this->_port,  // NOLINT
                               &pAddr, INDEX_GROUP, INDEX_OFFSET_BASE_READ, buffer_len, &buffer[0], &read_bytes);

  if (ret > 0) {
    std::cerr << "Error on reading data: " << std::hex << ret << std::endl;
    throw static_cast<int>(ret);
  }

  std::vector<uint8_t> res(&buffer[0], &buffer[read_bytes]);
  return res;
}

class LocalTwinCATLinkImpl : public LocalTwinCATLink {
 public:
  ~LocalTwinCATLinkImpl() override {}
  std::string _ams_net_id;
  std::string _ipv4addr;
  long _port = 0L;  // NOLINT
  AmsNetId _netId;

 protected:
  void Open() final;
  void Close() final;
  void Send(size_t size, std::unique_ptr<uint8_t[]> buf) final;
  std::vector<uint8_t> Read(uint32_t buffer_len) final;
  bool is_open() final;

 private:
  HMODULE lib = nullptr;
};

LinkPtr LocalTwinCATLink::Create() {
  auto link = std::make_shared<LocalTwinCATLinkImpl>();
  return link;
}

bool LocalTwinCATLinkImpl::is_open() { return (this->_port > 0); }

#ifdef _WIN32
typedef long(_stdcall *TcAdsPortOpenEx)(void);                    // NOLINT
typedef long(_stdcall *TcAdsPortCloseEx)(long);                   // NOLINT
typedef long(_stdcall *TcAdsGetLocalAddressEx)(long, AmsAddr *);  // NOLINT
typedef long(_stdcall *TcAdsSyncWriteReqEx)(long, AmsAddr *,      // NOLINT
                                            unsigned long,        // NOLINT
                                            unsigned long,        // NOLINT
                                            unsigned long,        // NOLINT
                                            void *);              // NOLINT
typedef long(_stdcall *TcAdsSyncReadReqEx)(long, AmsAddr *,       // NOLINT
                                           unsigned long,         // NOLINT
                                           unsigned long,         // NOLINT
                                           unsigned long,         // NOLINT
                                           void *,                // NOLINT
                                           unsigned long *);      // NOLINT
#ifdef _WIN64
#define TCADS_AdsPortOpenEx "AdsPortOpenEx"
#define TCADS_AdsGetLocalAddressEx "AdsGetLocalAddressEx"
#define TCADS_AdsPortCloseEx "AdsPortCloseEx"
#define TCADS_AdsSyncWriteReqEx "AdsSyncWriteReqEx"
#define TCADS_AdsSyncReadReqEx "AdsSyncReadReqEx2"
#endif

void LocalTwinCATLinkImpl::Open() {
  this->lib = LoadLibrary("TcAdsDll.dll");
  if (lib == nullptr) {
    throw std::runtime_error("couldn't find TcADS-DLL.");
    return;
  }
  // open a new ADS port
  TcAdsPortOpenEx portOpen = (TcAdsPortOpenEx)GetProcAddress(this->lib, TCADS_AdsPortOpenEx);
  this->_port = (*portOpen)();
  if (!this->_port) {
    std::cerr << "Error: Failed to open a new ADS port." << std::endl;
  }
  AmsAddr addr;
  TcAdsGetLocalAddressEx getAddr = (TcAdsGetLocalAddressEx)GetProcAddress(this->lib, TCADS_AdsGetLocalAddressEx);
  long nErr = getAddr(this->_port, &addr);  // NOLINT
  if (nErr) std::cerr << "Error: AdsGetLocalAddress: " << nErr << std::endl;
  this->_netId = addr.netId;
}
void LocalTwinCATLinkImpl::Close() {
  this->_port = 0;
  TcAdsPortCloseEx portClose = (TcAdsPortCloseEx)GetProcAddress(this->lib, TCADS_AdsPortCloseEx);
  (*portClose)(this->_port);
}
void LocalTwinCATLinkImpl::Send(size_t size, std::unique_ptr<uint8_t[]> buf) {
  AmsAddr addr = {this->_netId, PORT};
  TcAdsSyncWriteReqEx write = (TcAdsSyncWriteReqEx)GetProcAddress(this->lib, TCADS_AdsSyncWriteReqEx);
  long ret = write(this->_port,  // NOLINT
                   &addr, INDEX_GROUP, INDEX_OFFSET_BASE,
                   static_cast<unsigned long>(size),  // NOLINT
                   &buf[0]);
  if (ret > 0) {
    // https://infosys.beckhoff.com/english.php?content=../content/1033/tcadscommon/html/tcadscommon_intro.htm&id=
    // 6 : target port not found
    std::cerr << "Error on sending data (local): " << std::hex << ret << std::endl;
    throw static_cast<int>(ret);
  }
}

std::vector<uint8_t> LocalTwinCATLinkImpl::Read(uint32_t buffer_len) {
  AmsAddr addr = {this->_netId, PORT};
  TcAdsSyncReadReqEx read = (TcAdsSyncReadReqEx)GetProcAddress(this->lib, TCADS_AdsSyncReadReqEx);

  const auto buffer = std::make_unique<uint8_t[]>(buffer_len);
  unsigned long read_bytes;     // NOLINT
  long ret = read(this->_port,  // NOLINT
                  &addr, INDEX_GROUP, INDEX_OFFSET_BASE_READ, buffer_len, &buffer[0], &read_bytes);

  if (ret > 0) {
    std::cerr << "Error on reading data: " << std::hex << ret << std::endl;
    throw static_cast<int>(ret);
  }

  std::vector<uint8_t> res(&buffer[0], &buffer[read_bytes]);
  return res;
}

#else
void LocalTwinCATLinkImpl::Open() {
  throw std::runtime_error(
      "Link to localhost has not been compiled. Rebuild this library on a "
      "Twincat3 host machine with TcADS-DLL.");
}
void LocalTwinCATLinkImpl::Close() {}
void LocalTwinCATLinkImpl::Send(size_t size, std::unique_ptr<uint8_t[]> buf) {}
std::vector<uint8_t> LocalTwinCATLinkImpl::Read(uint32_t buffer_len) { return std::vector<uint8_t>(); }
#endif  // TC_ADS

}  // namespace autd::link
