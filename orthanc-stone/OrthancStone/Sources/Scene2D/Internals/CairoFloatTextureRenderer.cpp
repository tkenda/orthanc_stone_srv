/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this program. If not, see
 * <http://www.gnu.org/licenses/>.
 **/


#include "CairoFloatTextureRenderer.h"

#include "CairoColorTextureRenderer.h"
#include "../FloatTextureSceneLayer.h"

namespace OrthancStone
{
  namespace Internals
  {
    void CairoFloatTextureRenderer::Update(const ISceneLayer& layer)
    {
      const FloatTextureSceneLayer& l = dynamic_cast<const FloatTextureSceneLayer&>(layer);

      textureTransform_ = l.GetTransform();
      isLinearInterpolation_ = l.IsLinearInterpolation();

      float windowCenter, windowWidth;
      l.GetWindowing(windowCenter, windowWidth);

      const float a = windowCenter - windowWidth / 2.0f;
      const float slope = 256.0f / windowWidth;

      const Orthanc::ImageAccessor& source = l.GetTexture();
      const unsigned int width = source.GetWidth();
      const unsigned int height = source.GetHeight();
      texture_.SetSize(width, height, false);

      Orthanc::ImageAccessor target;
      texture_.GetWriteableAccessor(target);

      assert(source.GetFormat() == Orthanc::PixelFormat_Float32 &&
             target.GetFormat() == Orthanc::PixelFormat_BGRA32 &&
             sizeof(float) == 4);

      static const float LOG_NORMALIZATION = 255.0f / log(1.0f + 255.0f);
      
      for (unsigned int y = 0; y < height; y++)
      {
        const float* p = reinterpret_cast<const float*>(source.GetConstRow(y));
        uint8_t* q = reinterpret_cast<uint8_t*>(target.GetRow(y));

        for (unsigned int x = 0; x < width; x++)
        {
          float v = (*p - a) * slope;
          if (v <= 0)
          {
            v = 0;
          }
          else if (v >= 255)
          {
            v = 255;
          }

          if (l.IsApplyLog())
          {
            // https://theailearner.com/2019/01/01/log-transformation/
            v = LOG_NORMALIZATION * log(1.0f + static_cast<float>(v));
          }

          assert(v >= 0.0f && v <= 255.0f);

          uint8_t vv = static_cast<uint8_t>(v);

          if (l.IsInverted())
          {
            vv = 255 - vv;
          }

          q[0] = vv;
          q[1] = vv;
          q[2] = vv;

          p++;
          q += 4;
        }
      }
    }

      
    void CairoFloatTextureRenderer::Render(const AffineTransform2D& transform,
                                           unsigned int canvasWidth,
                                           unsigned int canvasHeight)
    {
      CairoColorTextureRenderer::RenderColorTexture(target_, transform, texture_,
                                                    textureTransform_, isLinearInterpolation_);
    }
  }
}
