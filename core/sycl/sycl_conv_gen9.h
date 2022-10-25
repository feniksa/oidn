// Copyright 2009-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#pragma once

#include "../conv.h"
#include "sycl_engine.h"

namespace oidn {

  class SYCLConvGen9 : public Conv
  {
  public:
    SYCLConvGen9(const Ref<SYCLEngine>& engine, const ConvDesc& desc);
    void submit() override;

  private:
    Ref<SYCLEngine> engine;
  };

} // namespace oidn