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

#include <Images/Image.h>

namespace OrthancStone
{
  // Helpers methods to render text in bitmaps.
  // Compared to the GlyphBitmapAlphabet::RenderText, these methods do not need a
  // code page.
  class TextRenderer : public boost::noncopyable
  {
  public:
    // simply renders text in GrayScale8 with a black background and a white text
    static Orthanc::ImageAccessor* Render(const std::string& ttf,
                                          unsigned int fontSize,
                                          const std::string& utf8String);

    // renders text in "color" with alpha in a RGBA32 image
    static Orthanc::ImageAccessor* RenderWithAlpha(const std::string& ttf,
                                                   unsigned int fontSize,
                                                   const std::string& utf8String,
                                                   uint8_t foreground);

    //    // renders text in color + a border with alpha in a RGBA32 image
    //    static Orthanc::ImageAccessor* RenderWithAlpha(const std::string& ttf,
    //                                                   unsigned int fontSize,
    //                                                   const std::string& utf8String,
    //                                                   uint8_t foreground,
    //                                                   uint8_t borderColor);
  };
}
