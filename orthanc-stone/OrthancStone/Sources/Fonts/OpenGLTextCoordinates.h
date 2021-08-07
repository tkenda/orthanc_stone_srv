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

#include "GlyphTextureAlphabet.h"
#include "TextBoundingBox.h"

#include <vector>

namespace OrthancStone
{
  namespace OpenGL
  {
    class OpenGLTextCoordinates : protected GlyphAlphabet::ITextVisitor
    {
    private:
      TextBoundingBox     box_;
      float               width_;
      float               height_;
      std::vector<float>  renderingCoords_;
      std::vector<float>  textureCoords_;
      float               textureWidth_;
      float               textureHeight_;
    
    protected:
      virtual void Visit(uint32_t unicode,
                         int x,
                         int y,
                         unsigned int width,
                         unsigned int height,
                         const Orthanc::IDynamicObject* payload) ORTHANC_OVERRIDE;

    public:
      OpenGLTextCoordinates(const GlyphTextureAlphabet& alphabet,
                            const std::string& utf8);

      unsigned int GetTextWidth() const
      {
        return box_.GetWidth();
      }

      unsigned int GetTextHeight() const
      {
        return box_.GetHeight();
      }

      bool IsEmpty() const
      {
        return renderingCoords_.empty();
      }

      const std::vector<float>& GetRenderingCoords() const;

      const std::vector<float>& GetTextureCoords() const;
    };
  }
}
