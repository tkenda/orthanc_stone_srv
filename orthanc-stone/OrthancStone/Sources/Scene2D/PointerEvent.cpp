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


#include "PointerEvent.h"

#include <OrthancException.h>

namespace OrthancStone
{
  PointerEvent::PointerEvent() :
    button_(MouseButton_None),
    hasAltModifier_(false),
    hasControlModifier_(false),
    hasShiftModifier_(false)
  {
  }

  
  ScenePoint2D PointerEvent::GetMainPosition() const
  {
    if (positions_.empty())
    {
      return ScenePoint2D(0, 0);
    }
    else
    {
      return positions_[0];
    }
  }
    

  ScenePoint2D PointerEvent::GetPosition(size_t index) const
  {
    if (index < positions_.size())
    {
      return positions_[index];
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }
}
