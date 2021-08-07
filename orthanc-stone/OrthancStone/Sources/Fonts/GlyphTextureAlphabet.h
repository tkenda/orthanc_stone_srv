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

#include "GlyphBitmapAlphabet.h"

#include <Images/ImageAccessor.h>

namespace OrthancStone
{
  class GlyphTextureAlphabet : public boost::noncopyable
  {
  public:
    class TextureLocation : public Orthanc::IDynamicObject
    {
    private:
      unsigned int  x_;
      unsigned int  y_;

    public:
      TextureLocation(unsigned int x,
                      unsigned int y) :
        x_(x),
        y_(y)
      {
      }

      unsigned int GetX() const
      {
        return x_;
      }

      unsigned int GetY() const
      {
        return y_;
      }
    };

  private:
    class GlyphSizeVisitor;
    class TextureGenerator;
    class RenderTextVisitor;
    
    GlyphAlphabet                          alphabet_;
    std::unique_ptr<Orthanc::ImageAccessor>  texture_;
    unsigned int                           textureWidth_;
    unsigned int                           textureHeight_;
    
  public:
    explicit GlyphTextureAlphabet(const GlyphBitmapAlphabet& sourceAlphabet);
    
    const Orthanc::ImageAccessor& GetTexture() const;
    
    Orthanc::ImageAccessor* ReleaseTexture();

    Orthanc::ImageAccessor* RenderText(const std::string& utf8);

    const GlyphAlphabet& GetAlphabet() const
    {
      return alphabet_;
    }

    unsigned int GetTextureWidth() const
    {
      return textureWidth_;
    }

    unsigned int GetTextureHeight() const
    {
      return textureHeight_;
    }
  };
}
