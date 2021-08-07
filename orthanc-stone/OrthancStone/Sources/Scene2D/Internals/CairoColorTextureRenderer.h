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


#pragma once

#include "../../Wrappers/CairoSurface.h"
#include "CompositorHelper.h"
#include "ICairoContextProvider.h"

namespace OrthancStone
{
  namespace Internals
  {
    class CairoColorTextureRenderer : public CompositorHelper::ILayerRenderer
    {
    private:
      ICairoContextProvider&  target_;
      CairoSurface            texture_;
      AffineTransform2D       textureTransform_;
      bool                    isLinearInterpolation_;
    
    public:
      CairoColorTextureRenderer(ICairoContextProvider& target,
                                const ISceneLayer& layer);

      virtual void Update(const ISceneLayer& layer) ORTHANC_OVERRIDE;
    
      virtual void Render(const AffineTransform2D& transform,
                          unsigned int canvasWidth,
                          unsigned int canvasHeight) ORTHANC_OVERRIDE
      {
        RenderColorTexture(target_, transform, texture_,
                           textureTransform_, isLinearInterpolation_);
      }

      static void RenderColorTexture(ICairoContextProvider& target,
                                     const AffineTransform2D& transform,
                                     CairoSurface& texture,
                                     const AffineTransform2D& textureTransform,
                                     bool isLinearInterpolation);
    };
  }
}