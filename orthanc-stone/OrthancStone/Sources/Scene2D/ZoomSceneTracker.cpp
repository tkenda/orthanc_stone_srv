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


#include "ZoomSceneTracker.h"
#include "../Scene2DViewport/ViewportController.h"

namespace OrthancStone
{
  ZoomSceneTracker::ZoomSceneTracker(boost::weak_ptr<IViewport> viewport,
                                     const PointerEvent& event,
                                     unsigned int canvasHeight)
    : OneGesturePointerTracker(viewport)
    , clickY_(event.GetMainPosition().GetY())
    , aligner_(viewport, event.GetMainPosition())
  {
    
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    originalSceneToCanvas_ = lock->GetController().GetSceneToCanvasTransform();

    if (canvasHeight <= 3)
    {
      active_ = false;
    }
    else
    {
      normalization_ = 1.0 / static_cast<double>(canvasHeight - 1);
      active_ = true;
    }
  }
  
  void ZoomSceneTracker::PointerMove(const PointerEvent& event)
  {
    static const double MIN_ZOOM = -4;
    static const double MAX_ZOOM = 4;
      
    if (active_)
    {
      double y = event.GetMainPosition().GetY();
      
      // In the range [-1,1]
      double dy = static_cast<double>(y - clickY_) * normalization_;  
      
      double z;

      // Linear interpolation from [-1, 1] to [MIN_ZOOM, MAX_ZOOM]
      if (dy < -1.0)
      {
        z = MIN_ZOOM;
      }
      else if (dy > 1.0)
      {
        z = MAX_ZOOM;
      }
      else
      {
        z = MIN_ZOOM + (MAX_ZOOM - MIN_ZOOM) * (dy + 1.0) / 2.0;
      }

      double zoom = pow(2.0, z);

      std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
      lock->GetController().SetSceneToCanvasTransform(
        AffineTransform2D::Combine(
          AffineTransform2D::CreateScaling(zoom, zoom),
          originalSceneToCanvas_));
      aligner_.Apply();
      lock->Invalidate();
    }
  }

  void ZoomSceneTracker::Cancel()
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    lock->GetController().SetSceneToCanvasTransform(originalSceneToCanvas_);
    lock->Invalidate();
  }
}
