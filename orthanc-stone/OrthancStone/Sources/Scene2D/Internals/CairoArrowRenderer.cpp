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


#include "CairoArrowRenderer.h"

#include "../ArrowSceneLayer.h"

namespace OrthancStone
{
  namespace Internals
  {
    void CairoArrowRenderer::Render(const AffineTransform2D& transform,
                                    unsigned int canvasWidth,
                                    unsigned int canvasHeight)
    {
      const ArrowSceneLayer& layer = GetLayer<ArrowSceneLayer>();
      
      cairo_t* cr = GetCairoContext();

      cairo_set_line_width(cr, layer.GetThickness());

      const Color& color = layer.GetColor();
      cairo_set_source_rgb(cr, color.GetRedAsFloat(),
                           color.GetGreenAsFloat(),
                           color.GetBlueAsFloat());

      const double zoom = transform.ComputeZoom();

      ScenePoint2D axis = layer.GetB() - layer.GetA();
      double n = ScenePoint2D::SquaredMagnitude(axis);
      
      if (!LinearAlgebra::IsCloseToZero(n))
      {
        axis = axis * layer.GetArrowLength() / (sqrt(n) * zoom);

        const ScenePoint2D a = layer.GetA().Apply(transform);
        const ScenePoint2D b = layer.GetB().Apply(transform);

        cairo_move_to(cr, a.GetX(), a.GetY());
        cairo_line_to(cr, b.GetX(), b.GetY());

        AffineTransform2D r1 = AffineTransform2D::CreateRotation(layer.GetArrowAngle());
        const ScenePoint2D c = (layer.GetA() + axis.Apply(r1)).Apply(transform);
        
        AffineTransform2D r2 = AffineTransform2D::CreateRotation(-layer.GetArrowAngle());
        const ScenePoint2D d = (layer.GetA() + axis.Apply(r2)).Apply(transform);
        
        cairo_move_to(cr, a.GetX(), a.GetY());
        cairo_line_to(cr, c.GetX(), c.GetY());

        cairo_move_to(cr, a.GetX(), a.GetY());
        cairo_line_to(cr, d.GetX(), d.GetY());

        cairo_stroke(cr);
      }
    }
  }
}
