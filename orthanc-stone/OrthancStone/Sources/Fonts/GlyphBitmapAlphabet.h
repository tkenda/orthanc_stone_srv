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

#include "GlyphAlphabet.h"

#include <Images/ImageAccessor.h>

namespace OrthancStone
{
  class GlyphBitmapAlphabet : public boost::noncopyable
  {
  private:
    class RenderTextVisitor;

    GlyphAlphabet  alphabet_;

  public:
    const GlyphAlphabet& GetAlphabet() const
    {
      return alphabet_;
    }    
    
    void AddUnicodeCharacter(FontRenderer& renderer,
                             uint32_t unicode)
    {
      alphabet_.Register(renderer, unicode);
    }


#if ORTHANC_ENABLE_LOCALE == 1
    void LoadCodepage(FontRenderer& renderer,
                      Orthanc::Encoding codepage);
#endif
    
    
    Orthanc::ImageAccessor* RenderText(const std::string& utf8) const;
  };
}
