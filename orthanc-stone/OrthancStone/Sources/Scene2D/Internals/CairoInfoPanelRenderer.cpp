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


#include "CairoInfoPanelRenderer.h"

#include "../InfoPanelSceneLayer.h"

namespace OrthancStone
{
  namespace Internals
  {
    void CairoInfoPanelRenderer::Update(const ISceneLayer& layer)
    {
      const InfoPanelSceneLayer& l = dynamic_cast<const InfoPanelSceneLayer&>(layer);

      texture_.Copy(l.GetTexture(), true);
      anchor_ = l.GetAnchor();
      isLinearInterpolation_ = l.IsLinearInterpolation();
      applySceneRotation_ = l.ShouldApplySceneRotation();
    }

    void CairoInfoPanelRenderer::Render(const AffineTransform2D& transform,
                                        unsigned int canvasWidth,
                                        unsigned int canvasHeight)
    {
      int dx, dy;
      InfoPanelSceneLayer::ComputeAnchorLocation(
        dx, dy, anchor_, texture_.GetWidth(), texture_.GetHeight(),
        canvasWidth, canvasHeight);

      cairo_t* cr = target_.GetCairoContext();
      cairo_save(cr);

      if (applySceneRotation_)
      {
        // the transformation is as follows:
        // - originally, the image is aligned so that its top left corner
        // is at 0,0
        // - first, we translate the image by -w/2,-h/2 
        // - then we rotate it, so that the next rotation will make the  
        //   image rotate around its center.
        // - then, we translate the image by +w/2,+h/2 to put it
        //   back in place
        // - the fourth and last transform is the one that brings the 
        //   image to its desired anchored location.

        int32_t halfWidth =
          static_cast<int32_t>(0.5 * texture_.GetWidth());

        int32_t halfHeight =
          static_cast<int32_t>(0.5 * texture_.GetHeight());

        const Matrix& sceneTransformM = transform.GetHomogeneousMatrix();
        Matrix r;
        Matrix q;
        LinearAlgebra::RQDecomposition3x3(r, q, sceneTransformM);

        // first, put the scene rotation in a cairo matrix
        cairo_matrix_t m;
        cairo_matrix_init(
          &m, q(0, 0), q(1, 0), q(0, 1), q(1, 1), q(0, 2), q(1, 2));

        // now let's build the transform piece by piece
        // first translation (directly written in `t`)
        cairo_matrix_t t;
        cairo_matrix_init_identity(&t);
        cairo_matrix_translate(&t, -halfWidth, -halfHeight);

        // then the rotation
        cairo_matrix_multiply(&t, &t, &m);

        // then the second translation
        {
          cairo_matrix_t translation2;
          cairo_matrix_init_translate(&translation2, halfWidth, halfHeight);
          cairo_matrix_multiply(&t, &t, &m);
        }

        // then the last translation
        {
          cairo_matrix_t translation3;
          cairo_matrix_init_translate(&translation3, dx, dy);
          cairo_matrix_multiply(&t, &t, &translation3);
        }
        cairo_transform(cr, &t);
      } 
      else
      {
        cairo_matrix_t t;
        cairo_matrix_init_identity(&t);
        cairo_matrix_translate(&t, dx, dy);
        cairo_transform(cr, &t);
      }
      cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
      cairo_set_source_surface(cr, texture_.GetObject(), 0, 0);

      if (isLinearInterpolation_)
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
