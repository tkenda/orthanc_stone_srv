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


#include "CairoLookupTableTextureRenderer.h"

#include "CairoColorTextureRenderer.h"
#include "../LookupTableTextureSceneLayer.h"

#include <OrthancException.h>

namespace OrthancStone
{
  namespace Internals
  {
    void CairoLookupTableTextureRenderer::Update(const ISceneLayer& layer)
    {
      const LookupTableTextureSceneLayer& l = dynamic_cast<const LookupTableTextureSceneLayer&>(layer);

      textureTransform_ = l.GetTransform();
      isLinearInterpolation_ = l.IsLinearInterpolation();

      const Orthanc::ImageAccessor& source = l.GetTexture();
      const unsigned int width = source.GetWidth();
      const unsigned int height = source.GetHeight();
      texture_.SetSize(width, height, true /* alpha channel is enabled */);

      Orthanc::ImageAccessor target;
      texture_.GetWriteableAccessor(target);
      l.Render(target);
      
      cairo_surface_mark_dirty(texture_.GetObject());
    }

    void CairoLookupTableTextureRenderer::Render(const AffineTransform2D& transform,
                                                 unsigned int canvasWidth,
                                                 unsigned int canvasHeight)
    {
      CairoColorTextureRenderer::RenderColorTexture(target_, transform, texture_,
                                                    textureTransform_, isLinearInterpolation_);
    }
  }
}
