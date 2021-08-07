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


#include "TextBoundingBox.h"

namespace OrthancStone
{
  void TextBoundingBox::AddPoint(int x,
                                 int y)
  {
    left_ = std::min(left_, x);
    right_ = std::max(right_, x);
    top_ = std::min(top_, y);
    bottom_ = std::max(bottom_, y);
  }


  void TextBoundingBox::Clear()
  {
    left_ = 0;
    top_ = 0;
    right_ = 0;
    bottom_ = 0;
    countCharacters_ = 0;
  }


  void TextBoundingBox::Visit(uint32_t unicode,
                              int x,
                              int y,
                              unsigned int width,
                              unsigned int height,
                              const Orthanc::IDynamicObject* payload /* ignored */)
  {
    AddPoint(x, y);
    AddPoint(x + static_cast<int>(width),
             y + static_cast<int>(height));
    countCharacters_++;
  }


  TextBoundingBox::TextBoundingBox(const GlyphAlphabet& alphabet,
                                   const std::string& utf8)
  {
    Clear();
    alphabet.Apply(*this, utf8);
  }


  unsigned int TextBoundingBox::GetWidth() const
  {
    assert(left_ <= right_);
    return static_cast<unsigned int>(right_ - left_ + 1);
  }

  
  unsigned int TextBoundingBox::GetHeight() const
  {
    assert(top_ <= bottom_);
    return static_cast<unsigned int>(bottom_ - top_ + 1);
  }
}
