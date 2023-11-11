// Copyright 2018 Intel Corporation
// SPDX-License-Identifier: Apache-2.0

#include "cpu_autoexposure.h"
#include "cpu_autoexposure_ispc.h"
#include "cpu_common.h"

OIDN_NAMESPACE_BEGIN

  CPUAutoexposure::CPUAutoexposure(const Ref<CPUEngine>& engine, const ImageDesc& srcDesc)
    : Autoexposure(srcDesc),
      engine(engine) {}

  void CPUAutoexposure::submit()
  {
    if (!src)
      throw std::logic_error("autoexposure source not set");
    if (!dst)
      throw std::logic_error("autoexposure destination not set");

    // Downsample the image to minimize sensitivity to noise
    ispc::ImageAccessor srcAcc = toISPC(*src);

    // Compute the average log luminance of the downsampled image
    using Sum = std::pair<float, int>;

    Sum sum =
      tbb::parallel_deterministic_reduce(
        tbb::blocked_range2d<int>(0, numBinsH, 0, numBinsW),
        Sum(0.f, 0),
        [&](const tbb::blocked_range2d<int>& r, Sum sum) -> Sum
        {
          // Iterate over bins
          for (int i = r.rows().begin(); i != r.rows().end(); ++i)
          {
            for (int j = r.cols().begin(); j != r.cols().end(); ++j)
            {
              // Compute the average luminance in the current bin
              const int beginH = int(ptrdiff_t(i)   * src->getH() / numBinsH);
              const int beginW = int(ptrdiff_t(j)   * src->getW() / numBinsW);
              const int endH   = int(ptrdiff_t(i+1) * src->getH() / numBinsH);
              const int endW   = int(ptrdiff_t(j+1) * src->getW() / numBinsW);

              const float L = ispc::autoexposureDownsample(srcAcc, beginH, endH, beginW, endW);

              // Accumulate the log luminance
              if (L > eps)
              {
                sum.first += math::log2(L);
                sum.second++;
              }
            }
          }

          return sum;
        },
        [](Sum a, Sum b) -> Sum { return Sum(a.first+b.first, a.second+b.second); }
      );

    *getDst() = (sum.second > 0) ? (key / math::exp2(sum.first / float(sum.second))) : 1.f;
  }

OIDN_NAMESPACE_END