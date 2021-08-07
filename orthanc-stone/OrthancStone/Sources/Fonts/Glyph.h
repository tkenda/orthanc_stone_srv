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

#include <Compatibility.h>
#include <IDynamicObject.h>

#include <memory>


namespace OrthancStone
{
  class Glyph : public boost::noncopyable
  {
  private:
    unsigned int   width_;
    unsigned int   height_;
    int            offsetLeft_;
    int            offsetTop_;
    int            advanceX_;
    unsigned int   lineHeight_;
      
    std::unique_ptr<Orthanc::IDynamicObject>  payload_;

  public:
    // WARNING: This does not copy the payload
    Glyph(const Glyph& other);
    
    Glyph(unsigned int width,
          unsigned int height,
          int offsetLeft,
          int offsetTop,
          int advanceX,
          unsigned int lineHeight);

    void SetPayload(Orthanc::IDynamicObject* payload);

    int GetOffsetLeft() const
    {
      return offsetLeft_;
    }

    int GetOffsetTop() const
    {
      return offsetTop_;
    }

    unsigned int GetWidth() const
    {
      return width_;
    }

    unsigned int GetHeight() const
    {
      return height_;
    }

    unsigned int GetAdvanceX() const
    {
      return advanceX_;
    }

    unsigned int GetLineHeight() const
    {
      return lineHeight_;
    }

    bool HasPayload() const
    {
      return payload_.get() != NULL;
    }
      
    const Orthanc::IDynamicObject& GetPayload() const;
      
    Orthanc::IDynamicObject* ReleasePayload();
  };
}
