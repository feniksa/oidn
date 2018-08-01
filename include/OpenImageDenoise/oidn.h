// ======================================================================== //
// Copyright 2009-2018 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include <stddef.h>

#if defined(__cplusplus)
extern "C" {
#endif

#ifndef OIDN_API
#if defined(_WIN32) && !defined(OIDN_STATIC_LIB)
#  define OIDN_API __declspec(dllimport)
#else
#  define OIDN_API
#endif
#endif

enum OIDNFormat
{
  OIDN_FORMAT_UNDEFINED,

  OIDN_FORMAT_FLOAT,
  OIDN_FORMAT_FLOAT2,
  OIDN_FORMAT_FLOAT3,
  OIDN_FORMAT_FLOAT3_SRGB,
};

enum OIDNBufferType
{
  OIDN_BUFFER_TYPE_INPUT,
  OIDN_BUFFER_TYPE_INPUT_ALBEDO,
  OIDN_BUFFER_TYPE_INPUT_NORMAL,
  OIDN_BUFFER_TYPE_OUTPUT,
};

enum OIDNFilterType
{
  OIDN_FILTER_TYPE_AUTOENCODER_LDR
};

// Device
typedef struct OIDNDeviceImpl* OIDNDevice;

OIDN_API OIDNDevice oidnNewDevice();

OIDN_API void oidnRetainDevice(OIDNDevice device);

OIDN_API void oidnReleaseDevice(OIDNDevice device);

// Buffer
typedef struct OIDNBufferImpl* OIDNBuffer;

OIDN_API OIDNBuffer oidnNewSharedBuffer(OIDNDevice device, void* ptr, size_t byteSize);

OIDN_API void oidnRetainBuffer(OIDNBuffer buffer);

OIDN_API void oidnReleaseBuffer(OIDNBuffer buffer);

// Filter
typedef struct OIDNFilterImpl* OIDNFilter;

OIDN_API OIDNFilter oidnNewFilter(OIDNDevice device, enum OIDNFilterType type);

OIDN_API void oidnRetainFilter(OIDNFilter filter);

OIDN_API void oidnReleaseFilter(OIDNFilter filter);

OIDN_API void oidnSetFilterBuffer2D(OIDNFilter filter, enum OIDNBufferType type, unsigned int slot,
                                    enum OIDNFormat format,
                                    OIDNBuffer buffer, size_t byteOffset, size_t byteStride,
                                    size_t width, size_t height);

OIDN_API void oidnSetSharedFilterBuffer2D(OIDNFilter filter, enum OIDNBufferType type, unsigned int slot,
                                          enum OIDNFormat format,
                                          const void* ptr, size_t byteOffset, size_t byteStride,
                                          size_t width, size_t height);

OIDN_API void oidnCommitFilter(OIDNFilter filter);

OIDN_API void oidnExecuteFilter(OIDNFilter filter);

#if defined(__cplusplus)
}
#endif