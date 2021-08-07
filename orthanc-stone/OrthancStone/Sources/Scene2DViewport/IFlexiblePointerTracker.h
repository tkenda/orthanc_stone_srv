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

#include "PredeclaredTypes.h"

#include "../Scene2D/PointerEvent.h"


namespace OrthancStone
{
  /**
  This interface represents a flexible mouse tracker that can respond to 
  several events and is not automatically deleted upon mouse up or when touch
  interaction is suspended : for instance, a stateful tracker with a two-step 
  interaction like: click & drag --> mouse up --> drag --> mouse click 
  (for instance, for an angle measuring tracker or an ellipse tracker)
  */
  class IFlexiblePointerTracker : public boost::noncopyable
  {
  public:
    virtual ~IFlexiblePointerTracker() {}

    /**
    This method will be repeatedly called during user interaction
    */
    virtual void PointerMove(const PointerEvent& event) = 0;

    /**
    This method will be called when a touch/pointer is removed (mouse up, 
    pen lift, finger removed...)
    */
    virtual void PointerUp(const PointerEvent& event) = 0;

    /**
    This method will be called when a touch/pointer is added (mouse down, 
    pen or finger press)

    Important note: the initial pointer down that leads to creating the 
    tracker is NOT sent to the tracker.

    Thus, if you count the PointerDown vs PointerUp, there will be an extra
    PointerUp.
    */
    virtual void PointerDown(const PointerEvent& event) = 0;

    /**
    This method will be repeatedly called by the tracker owner (for instance,
    the application) to check whether the tracker must keep on receiving 
    interaction or if its job is done and it should be deleted.
    */
    virtual bool IsAlive() const = 0;

    /**
    This will be called if the tracker needs to be dismissed without committing
    its changes to the underlying model. If the model has been modified during
    tracker lifetime, it must be restored to its initial value
    */
    virtual void Cancel() = 0;
  };
}
