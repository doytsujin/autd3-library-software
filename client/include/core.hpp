﻿// File: core.hpp
// Project: include
// Created Date: 11/04/2018
// Author: Shun Suzuki
// -----
// Last Modified: 29/04/2020
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2018-2020 Hapis Lab. All rights reserved.
//

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace autd {
namespace internal {
class Link;
}

namespace _utils {
class Vector3;
class Quaternion;
}  // namespace _utils
using _utils::Quaternion;
using _utils::Vector3;

enum class LinkType : int { ETHERCAT, TwinCAT, SOEM, EMULATOR };

class Controller;
class AUTDController;
class Geometry;
class Timer;

template <class T>
#if DLL_FOR_CAPI
static T* CreateHelper() {
  struct impl : T {
    impl() : T() {}
  };
  return new impl;
}
#else
static std::shared_ptr<T> CreateHelper() {
  struct impl : T {
    impl() : T() {}
  };
  auto p = std::make_shared<impl>();
  return std::move(p);
}
#endif

template <class T>
#if DLL_FOR_CAPI
static void DeleteHelper(T** ptr) {
  delete *ptr;
  *ptr = nullptr;
}
#else
static void DeleteHelper(std::shared_ptr<T>* ptr) {
}
#endif
}  // namespace autd
