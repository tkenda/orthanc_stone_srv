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


#include "Glyph.h"

#include <OrthancException.h>


namespace OrthancStone
{
  Glyph::Glyph(const Glyph& other) : 
    width_(other.width_),
    height_(other.height_),
    offsetLeft_(other.offsetLeft_),
    offsetTop_(other.offsetTop_),
    advanceX_(other.advanceX_),
    lineHeight_(other.lineHeight_)
  {
  }
  
    
  Glyph::Glyph(unsigned int width,
               unsigned int height,
               int offsetLeft,
               int offsetTop,
               int advanceX,
               unsigned int lineHeight) :
    width_(width),
    height_(height),
    offsetLeft_(offsetLeft),
    offsetTop_(offsetTop),
    advanceX_(advanceX),
    lineHeight_(lineHeight)
  {
  }

  
  void Glyph::SetPayload(Orthanc::IDynamicObject* payload)  // Takes ownership
  {
    if (payload == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else
    {
      payload_.reset(payload);
    }
  }

      
  const Orthanc::IDynamicObject& Glyph::GetPayload() const
  {
    if (payload_.get() == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return *payload_;
    }
  }

  
  Orthanc::IDynamicObject* Glyph::ReleasePayload()
  {
    if (payload_.get() == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return payload_.release();
    }
  }
}
