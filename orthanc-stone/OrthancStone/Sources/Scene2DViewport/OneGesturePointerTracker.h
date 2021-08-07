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

#include "IFlexiblePointerTracker.h"

#include "../Viewport/IViewport.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

namespace OrthancStone
{
  /**
  This base is class allows to write simple trackers that deal with single 
  drag gestures with only one touch. It is *not* suitable for multi-touch and
  multi-state trackers where various mouse operations need to be handled.

  In order to write such a tracker:
  - subclass this class
  - you may store the initial click/touch position in the constructor
  - implement PointerMove to react to pointer/touch events
  - implement Cancel to restore the state at initial tracker creation time

  */
  class OneGesturePointerTracker : public IFlexiblePointerTracker
  {
  private:
    bool  alive_;
    int   currentTouchCount_;

  protected:
    boost::weak_ptr<IViewport> viewport_;

    /**
    This will return a scoped lock to the viewport.
    If the viewport does not exist anymore, then nullptr is returned.
    */
    IViewport::ILock* GetViewportLock();

  public:
    explicit OneGesturePointerTracker(boost::weak_ptr<IViewport> viewport);
    
    virtual void PointerUp(const PointerEvent& event) ORTHANC_OVERRIDE;
    
    virtual void PointerDown(const PointerEvent& event) ORTHANC_OVERRIDE;
    
    virtual bool IsAlive() const ORTHANC_OVERRIDE;
  };
}

