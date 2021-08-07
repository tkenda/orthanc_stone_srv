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


#include "CairoColorTextureRenderer.h"

#include "../ColorTextureSceneLayer.h"

namespace OrthancStone
{
  namespace Internals
  {
    CairoColorTextureRenderer::CairoColorTextureRenderer(ICairoContextProvider& target,
                                                         const ISceneLayer& layer) :
      target_(target)
    {
      Update(layer);
    }

    
    void CairoColorTextureRenderer::Update(const ISceneLayer& layer)
    {
      const ColorTextureSceneLayer& l = dynamic_cast<const ColorTextureSceneLayer&>(layer);

      texture_.Copy(l.GetTexture(), true);
      textureTransform_ = l.GetTransform();
      isLinearInterpolation_ = l.IsLinearInterpolation();
    }


    void CairoColorTextureRenderer::RenderColorTexture(ICairoContextProvider& target,
                                                       const AffineTransform2D& transform,
                                                       CairoSurface& texture,
                                                       const AffineTransform2D& textureTransform,
                                                       bool isLinearInterpolation)
    {
      cairo_t* cr = target.GetCairoContext();

      AffineTransform2D t =
        AffineTransform2D::Combine(transform, textureTransform);
      Matrix h = t.GetHomogeneousMatrix();
      
      cairo_save(cr);

      cairo_matrix_t m;
      cairo_matrix_init(&m, h(0, 0), h(1, 0), h(0, 1), h(1, 1), h(0, 2), h(1, 2));
      cairo_transform(cr, &m);

      cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
      cairo_set_source_surface(cr, texture.GetObject(), 0, 0);

      if (isLinearInterpolation)
      {
        cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_BILINEAR);
      }
      else
      {
        cairo_pattern_set_filter(cairo_get_source(cr), CAIRO_FILTER_NEAREST);
      }

      cairo_paint(cr);

      cairo_restore(cr);
    }
  }
}
