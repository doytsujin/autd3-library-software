// File: wrapper.hpp
// Project: capi
// Created Date: 09/06/2020
// Author: Shun Suzuki
// -----
// Last Modified: 03/07/2020
// Modified By: Shun Suzuki (suzuki@hapis.k.u-tokyo.ac.jp)
// -----
// Copyright (c) 2020 Hapis Lab. All rights reserved.
//

#pragma once

#include "autd3.hpp"

typedef struct {
  autd::GainPtr ptr;
} GainWrapper;

typedef struct {
  autd::ModulationPtr ptr;
} ModulationWrapper;

typedef struct {
  autd::SequencePtr ptr;
} SequenceWrapper;

typedef struct {
  autd::LinkPtr ptr;
} LinkWrapper;

typedef struct {
  autd::ControllerPtr ptr;
} ControllerWrapper;

typedef struct {
  autd::link::EtherCATAdapters adapters;
} EtherCATAdaptersWrapper;

typedef struct {
  autd::FirmwareInfoList list;
} FirmwareInfoListWrapper;

GainWrapper* GainCreate(autd::GainPtr ptr) { return new GainWrapper{ptr = ptr}; }
void GainDelete(GainWrapper* ptr) { delete ptr; }

ModulationWrapper* ModulationCreate(autd::ModulationPtr ptr) { return new ModulationWrapper{ptr = ptr}; }
void ModulationDelete(ModulationWrapper* ptr) { delete ptr; }

SequenceWrapper* SequencePtrCreate(autd::SequencePtr ptr) { return new SequenceWrapper{ptr = ptr}; }
void SequenceDelete(SequenceWrapper* ptr) { delete ptr; }

LinkWrapper* LinkCreate(autd::LinkPtr ptr) { return new LinkWrapper{ptr = ptr}; }
void LinkDelete(LinkWrapper* ptr) { delete ptr; }

ControllerWrapper* ControllerCreate(autd::ControllerPtr ptr) { return new ControllerWrapper{ptr = ptr}; }
void ControllerDelete(ControllerWrapper* ptr) { delete ptr; }

EtherCATAdaptersWrapper* EtherCATAdaptersCreate(autd::link::EtherCATAdapters adapters) { return new EtherCATAdaptersWrapper{adapters = adapters}; }
void EtherCATAdaptersDelete(EtherCATAdaptersWrapper* ptr) { delete ptr; }

FirmwareInfoListWrapper* FirmwareInfoListCreate(autd::FirmwareInfoList list) { return new FirmwareInfoListWrapper{list = list}; }
void FirmwareInfoListDelete(FirmwareInfoListWrapper* ptr) { delete ptr; }
