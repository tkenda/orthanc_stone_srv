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


#include "OpenGLTextCoordinates.h"

#include <OrthancException.h>

namespace OrthancStone
{
  namespace OpenGL
  {
    void OpenGLTextCoordinates::Visit(uint32_t unicode,
                                      int x,
                                      int y,
                                      unsigned int width,
                                      unsigned int height,
                                      const Orthanc::IDynamicObject* payload)
    {
      // Rendering coordinates
      float rx1 = static_cast<float>(x - box_.GetLeft());
      float ry1 = static_cast<float>(y - box_.GetTop());
      float rx2 = rx1 + static_cast<float>(width);
      float ry2 = ry1 + static_cast<float>(height);

      // Texture coordinates
      assert(payload != NULL);
      const GlyphTextureAlphabet::TextureLocation& location =
        *dynamic_cast<const GlyphTextureAlphabet::TextureLocation*>(payload);

      float tx1 = location.GetX() / textureWidth_;
      float ty1 = location.GetY() / textureHeight_;
      float tx2 = tx1 + (static_cast<float>(width) / textureWidth_);
      float ty2 = ty1 + (static_cast<float>(height) / textureHeight_);

      const float rpos[6][2] = {
        { rx1, ry1 },
        { rx1, ry2 },
        { rx2, ry1 },
        { rx2, ry1 },
        { rx1, ry2 },
        { rx2, ry2 }
      };

      const float tpos[6][2] = {
        { tx1, ty1 },
        { tx1, ty2 },
        { tx2, ty1 },
        { tx2, ty1 },
        { tx1, ty2 },
        { tx2, ty2 }
      };

      for (unsigned int i = 0; i < 6; i++)
      {
        renderingCoords_.push_back(rpos[i][0]);
        renderingCoords_.push_back(rpos[i][1]);
        textureCoords_.push_back(tpos[i][0]);
        textureCoords_.push_back(tpos[i][1]);
      }
    }


    OpenGLTextCoordinates::OpenGLTextCoordinates(const GlyphTextureAlphabet& alphabet,
                                                 const std::string& utf8) :
      box_(alphabet.GetAlphabet(), utf8),
      textureWidth_(static_cast<float>(alphabet.GetTextureWidth())),
      textureHeight_(static_cast<float>(alphabet.GetTextureHeight()))
    {
      if (textureWidth_ <= 0 ||
          textureHeight_ <= 0)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
      
      width_ = static_cast<float>(box_.GetWidth());
      height_ = static_cast<float>(box_.GetHeight());

      // Each character is made of two 2D triangles (= 2 * 3 * 2 = 12)
      renderingCoords_.reserve(box_.GetCharactersCount() * 12);
      textureCoords_.reserve(box_.GetCharactersCount() * 12);

      alphabet.GetAlphabet().Apply(*this, utf8);
    }


    const std::vector<float>& OpenGLTextCoordinates::GetRenderingCoords() const
    {
      assert(renderingCoords_.size() == textureCoords_.size());
      return renderingCoords_;
    }

    
    const std::vector<float>& OpenGLTextCoordinates::GetTextureCoords() const
    {
      assert(renderingCoords_.size() == textureCoords_.size());
      return textureCoords_;
    }
  }
}
