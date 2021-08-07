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

#include "ScenePoint2D.h"

#include <boost/noncopyable.hpp>
#include <stdint.h>

namespace OrthancStone
{
  class PointerEvent : public boost::noncopyable
  {
  private:
    MouseButton                button_;
    std::vector<ScenePoint2D>  positions_;
    bool                       hasAltModifier_;
    bool                       hasControlModifier_;
    bool                       hasShiftModifier_;

  public:
    PointerEvent();

    ScenePoint2D GetMainPosition() const;
    
    void AddPosition(const ScenePoint2D& p)
    {
      positions_.push_back(p);
    }

    void AddPosition(double x,
                     double y)
    {
      positions_.push_back(ScenePoint2D(x, y));
    }

    size_t GetPositionsCount() const
    {
      return positions_.size();
    }
    
    ScenePoint2D GetPosition(size_t index) const;

    void SetAltModifier(bool value)
    {
      hasAltModifier_ = value;
    }

    bool HasAltModifier() const
    {
      return hasAltModifier_;
    }

    void SetControlModifier(bool value)
    {
      hasControlModifier_ = value;
    }

    bool HasControlModifier() const
    {
      return hasControlModifier_;
    }

    void SetShiftModifier(bool value)
    {
      hasShiftModifier_ = value;
    }

    bool HasShiftModifier() const
    {
      return hasShiftModifier_;
    }

    void SetMouseButton(MouseButton button)
    {
      button_ = button;
    }

    MouseButton GetMouseButton() const
    {
      return button_;
    }
  };
}
