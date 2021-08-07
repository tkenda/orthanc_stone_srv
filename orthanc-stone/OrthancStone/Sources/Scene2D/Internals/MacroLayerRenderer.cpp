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


#include "MacroLayerRenderer.h"
#include "../MacroSceneLayer.h"

namespace OrthancStone
{
  namespace Internals
  {
    void MacroLayerRenderer::Clear()
    {
      for (size_t i = 0; i < renderers_.size(); i++)
      {
        assert(renderers_[i] != NULL);
        delete renderers_[i];
      }

      renderers_.clear();
    }
  

    void MacroLayerRenderer::Render(const AffineTransform2D& transform,
                                    unsigned int canvasWidth,
                                    unsigned int canvasHeight)
    {
      for (size_t i = 0; i < renderers_.size(); i++)
      {
        assert(renderers_[i] != NULL);
        renderers_[i]->Render(transform, canvasWidth, canvasHeight);
      }
    }
    
      
    void MacroLayerRenderer::Update(const ISceneLayer& layer)
    {
      const MacroSceneLayer& macro = dynamic_cast<const MacroSceneLayer&>(layer);
      
      Clear();

      renderers_.reserve(macro.GetSize());

      for (size_t i = 0; i < macro.GetSize(); i++)
      {
        if (macro.HasLayer(i))
        {
          renderers_.push_back(factory_.Create(macro.GetLayer(i)));
        }
      }
    }
  }
}
