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


#include "CairoPolylineRenderer.h"

#include "../PolylineSceneLayer.h"

namespace OrthancStone
{
  namespace Internals
  {
    void CairoPolylineRenderer::Render(const AffineTransform2D& transform,
                                       unsigned int canvasWidth,
                                       unsigned int canvasHeight)
    {
      const PolylineSceneLayer& layer = GetLayer<PolylineSceneLayer>();
      
      cairo_t* cr = GetCairoContext();

      cairo_set_line_width(cr, layer.GetThickness());
      
      for (size_t i = 0; i < layer.GetChainsCount(); i++)
      {
        const Color& color = layer.GetColor(i);
        cairo_set_source_rgb(cr, color.GetRedAsFloat(),
                             color.GetGreenAsFloat(),
                             color.GetBlueAsFloat());

        const PolylineSceneLayer::Chain& chain = layer.GetChain(i);

        if (!chain.empty())
        {
          for (size_t j = 0; j < chain.size(); j++)
          {
            ScenePoint2D p = chain[j].Apply(transform);

            if (j == 0)
            {
              cairo_move_to(cr, p.GetX(), p.GetY());
            }
            else
            {
              cairo_line_to(cr, p.GetX(), p.GetY());
            }
          }

          if (layer.IsClosedChain(i))
          {
            ScenePoint2D p = chain[0].Apply(transform);
            cairo_line_to(cr, p.GetX(), p.GetY());
          }
        }

        cairo_stroke(cr);
      }
    }
  }
}
