// Copyright 2009-2022 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "sycl_engine.h"
#include "../gpu/gpu_autoexposure.h"
#include "../gpu/gpu_input_process.h"
#include "../gpu/gpu_output_process.h"
#include "../gpu/gpu_image_copy.h"
#include "sycl_conv_gen9.h"
#include "sycl_conv_xehpg.h"
#include "sycl_conv_xehpc.h"
#include "sycl_pool.h"
#include "sycl_upsample.h"

namespace oidn {

  SYCLEngine::SYCLEngine(const Ref<SYCLDevice>& device,
                         const sycl::queue& queue)
    : device(device.get()),
      syclContext(queue.get_context()),
      syclDevice(queue.get_device()),
      queue(queue)
  {
    maxWorkGroupSize = syclDevice.get_info<sycl::info::device::max_work_group_size>();
  }

  std::shared_ptr<Conv> SYCLEngine::newConv(const ConvDesc& desc)
  {
    switch (device->getArch())
    {
    case SYCLArch::XeHPG:
      return std::make_shared<SYCLConvXeHPG>(this, desc);
    case SYCLArch::XeHPC:
      return std::make_shared<SYCLConvXeHPC>(this, desc);
    default:
      return std::make_shared<SYCLConvGen9>(this, desc);
    }
  }

  std::shared_ptr<Pool> SYCLEngine::newPool(const PoolDesc& desc)
  {
    return std::make_shared<SYCLPool>(this, desc);
  }

  std::shared_ptr<Upsample> SYCLEngine::newUpsample(const UpsampleDesc& desc)
  {
    return std::make_shared<SYCLUpsample>(this, desc);
  }

  std::shared_ptr<Autoexposure> SYCLEngine::newAutoexposure(const ImageDesc& srcDesc)
  {
    if (maxWorkGroupSize >= 1024)
      return std::make_shared<GPUAutoexposure<SYCLEngine, 1024>>(this, srcDesc);
    else if (maxWorkGroupSize >= 512)
      return std::make_shared<GPUAutoexposure<SYCLEngine, 512>>(this, srcDesc);
    else
      return std::make_shared<GPUAutoexposure<SYCLEngine, 256>>(this, srcDesc);
  }

  std::shared_ptr<InputProcess> SYCLEngine::newInputProcess(const InputProcessDesc& desc)
  {
    return std::make_shared<GPUInputProcess<SYCLEngine, half, TensorLayout::Chw16c>>(this, desc);
  }

  std::shared_ptr<OutputProcess> SYCLEngine::newOutputProcess(const OutputProcessDesc& desc)
  {
    return std::make_shared<GPUOutputProcess<SYCLEngine, half, TensorLayout::Chw16c>>(this, desc);
  }

  std::shared_ptr<ImageCopy> SYCLEngine::newImageCopy()
  {
    return std::make_shared<GPUImageCopy<SYCLEngine>>(this);
  }

  void* SYCLEngine::malloc(size_t byteSize, Storage storage)
  {
    switch (storage)
    {
    case Storage::Undefined:
    case Storage::Host:
      return sycl::aligned_alloc_host(memoryAlignment,
                                      byteSize,
                                      syclContext);

    case Storage::Device:
      return sycl::aligned_alloc_device(memoryAlignment,
                                        byteSize,
                                        syclDevice,
                                        syclContext);

    case Storage::Managed:
      return sycl::aligned_alloc_shared(memoryAlignment,
                                        byteSize,
                                        syclDevice,
                                        syclContext);

    default:
      throw Exception(Error::InvalidArgument, "invalid storage mode");
    }
  }

  void SYCLEngine::free(void* ptr, Storage storage)
  {
    sycl::free(ptr, syclContext);
  }

  void SYCLEngine::memcpy(void* dstPtr, const void* srcPtr, size_t byteSize)
  {
    lastEvent = queue.memcpy(dstPtr, srcPtr, byteSize, getDepEvents());
    wait();
  }

  Storage SYCLEngine::getPointerStorage(const void* ptr)
  {
    switch (sycl::get_pointer_type(ptr, syclContext))
    {
      case sycl::usm::alloc::host:
        return Storage::Host;

      case sycl::usm::alloc::device:
        return sycl::get_pointer_device(ptr, syclContext) == syclDevice ? Storage::Device : Storage::Undefined;
      
      case sycl::usm::alloc::shared:
        return Storage::Managed;
      
      default:
        return Storage::Undefined;
    }
  }

  void SYCLEngine::submitHostFunc(std::function<void()>&& f)
  {
    lastEvent = queue.submit([&](sycl::handler& cgh) {
      cgh.depends_on(getDepEvents()),
      cgh.host_task(f);
    });
  }
  
  void SYCLEngine::submitBarrier()
  { 
    lastEvent = queue.submit([&](sycl::handler& cgh) {
      cgh.depends_on(getDepEvents()),
      //cgh.ext_oneapi_barrier(); // FIXME: hangs, workaround: SYCL_PI_LEVEL_ZERO_USE_MULTIPLE_COMMANDLIST_BARRIERS=0
      cgh.single_task([](){});    // FIXME: should switch to ext_oneapi_barrier when it gets fixed
    });
  }

  void SYCLEngine::wait()
  {
    if (lastEvent)
    {
      lastEvent.value().wait_and_throw();
      lastEvent.reset();
    }
  }

} // namespace oidn