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


#include "OneGesturePointerTracker.h"

#include <OrthancException.h>

#include "../StoneException.h"

namespace OrthancStone
{
  OneGesturePointerTracker::OneGesturePointerTracker(boost::weak_ptr<IViewport> viewport) :
    alive_(true),
    currentTouchCount_(1),
    viewport_(viewport)
  {
  }

  IViewport::ILock* OneGesturePointerTracker::GetViewportLock()
  {
    boost::shared_ptr<IViewport> viewport = viewport_.lock();
    if (viewport)
      return viewport->Lock();
    else
      return NULL;
  }

  void OneGesturePointerTracker::PointerUp(const PointerEvent& event)
  {
    // pointer up is only called for the LAST up event in case of a multi-touch
    // gesture
    ORTHANC_ASSERT(currentTouchCount_ > 0, "Wrong state in tracker");
    currentTouchCount_--;
    //LOG(TRACE) << "currentTouchCount_ becomes: " << currentTouchCount_;
    if (currentTouchCount_ == 0)
    {
      //LOG(TRACE) << "currentTouchCount_ == 0 --> alive_ = false";
      alive_ = false;
    }
  }

  void OneGesturePointerTracker::PointerDown(const PointerEvent& event)
  {
    // additional touches are not taken into account but we need to count 
    // the number of active touches
    currentTouchCount_++;
    //LOG(TRACE) << "currentTouchCount_ becomes: " << currentTouchCount_;

    /**
     * 2019-12-06 (SJO): Patch to have consistent behavior when mouse
     * leaves the canvas while the tracker is still active, then
     * button is released while out-of-canvas. Such an event is not
     * caught (at least in WebAssembly), so we delete the tracker on
     * the next click inside the canvas.
     **/
    alive_ = false;
  }

  bool OneGesturePointerTracker::IsAlive() const
  {
    return alive_;
  }
}
