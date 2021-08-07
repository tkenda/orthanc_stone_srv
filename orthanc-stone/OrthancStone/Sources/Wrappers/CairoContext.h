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

#include "CairoSurface.h"
#include "../StoneEnumerations.h"

#include <Images/Font.h>

namespace OrthancStone
{
  // This is a RAII wrapper around the Cairo drawing context
  class CairoContext : public boost::noncopyable
  {
  private:
    class AlphaSurface;
    
    cairo_t*      context_;
    unsigned int  width_;
    unsigned int  height_;

  public:
    explicit CairoContext(CairoSurface& surface);

    ~CairoContext();

    cairo_t* GetObject()
    {
      return context_;
    }

    unsigned int GetWidth() const
    {
      return width_;
    }

    unsigned int GetHeight() const
    {
      return height_;
    }

    void SetSourceColor(uint8_t red,
                        uint8_t green,
                        uint8_t blue);

    void SetSourceColor(const uint8_t color[3])
    {
      SetSourceColor(color[0], color[1], color[2]);
    }

    void DrawText(const Orthanc::Font& font,
                  const std::string& text,
                  double x,
                  double y,
                  BitmapAnchor anchor);      
  };
}
