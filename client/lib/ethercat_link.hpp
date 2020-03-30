﻿// File: ethercat_link.hpp
// Project: lib
// Created Date: 01/06/2016
// Author: Seki Inoue
// -----
// Last Modified: 30/03/2020
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2016-2020 Hapis Lab. All rights reserved.
//

#pragma once

#include <stdio.h>

#include <memory>
#include <string>
#include <vector>

#if WIN32
#include <codeanalysis\warnings.h>
#pragma warning(push)
#pragma warning(disable : ALL_CODE_ANALYSIS_WARNINGS)
#endif
#include <AdsLib.h>
#if WIN32
#pragma warning(pop)
#endif

#include "link.hpp"

#ifdef _WINDOWS
#define NOMINMAX
#include <Windows.h>
#include <winnt.h>
#else
typedef void *HMODULE;
#endif

namespace autd {
namespace internal {
class EthercatLink : public Link {
 public:
  void Open(std::string location) override;
  virtual void Open(std::string ams_net_id, std::string ipv4addr);
  void Close() override;
  virtual void Send(size_t size, std::unique_ptr<uint8_t[]> buf);
  std::vector<uint8_t> Read() override;
  bool is_open() final;

 protected:
  long _port = 0L;  // NOLINT
  AmsNetId _netId;
};

class LocalEthercatLink : public EthercatLink {
 public:
  void Open(std::string location = "") final;
  void Close() final;
  void Send(size_t size, std::unique_ptr<uint8_t[]> buf) final;
  std::vector<uint8_t> Read() final;

 private:
  HMODULE lib = nullptr;
};
}  // namespace internal
}  // namespace autd
