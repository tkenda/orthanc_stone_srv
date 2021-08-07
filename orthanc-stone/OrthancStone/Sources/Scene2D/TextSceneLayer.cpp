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


#include "TextSceneLayer.h"

#include <Compatibility.h>

namespace OrthancStone
{
  TextSceneLayer::TextSceneLayer() :
    x_(0),
    y_(0),
    fontIndex_(0),
    anchor_(BitmapAnchor_Center),
    border_(0),
    revision_(0)
  {
  }


  ISceneLayer* TextSceneLayer::Clone() const
  {
    std::unique_ptr<TextSceneLayer> cloned(new TextSceneLayer);
    cloned->SetColor(GetColor());
    cloned->x_ = x_;
    cloned->y_ = y_;
    cloned->utf8_ = utf8_;
    cloned->fontIndex_ = fontIndex_;
    cloned->anchor_ = anchor_;
    cloned->border_ = border_;
    return cloned.release();
  }

  void TextSceneLayer::SetPosition(double x,
                                   double y)
  {
    if (x != x_ || y != y_)
    {
      x_ = x;
      y_ = y;
      revision_++;
    }
  }

  void TextSceneLayer::SetText(const std::string& utf8)
  {
    if (utf8 != utf8_)
    {
      utf8_ = utf8;
      revision_++;
    }
  }

  void TextSceneLayer::SetFontIndex(size_t fontIndex)
  {
    if (fontIndex != fontIndex_)
    {
      fontIndex_ = fontIndex;
      revision_++;
    }
  }

  void TextSceneLayer::SetAnchor(BitmapAnchor anchor)
  {
    if (anchor != anchor_)
    {
      anchor_ = anchor;
      revision_++;
    }
  }

  void TextSceneLayer::SetBorder(unsigned int border)
  {
    if (border != border_)
    {
      border_ = border;
      revision_++;
    }
  }
}
