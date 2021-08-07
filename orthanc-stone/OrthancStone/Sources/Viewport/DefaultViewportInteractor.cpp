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

#include "DefaultViewportInteractor.h"

#include "../Scene2D/GrayscaleWindowingSceneTracker.h"
#include "../Scene2D/PanSceneTracker.h"
#include "../Scene2D/RotateSceneTracker.h"
#include "../Scene2D/ZoomSceneTracker.h"

#include <OrthancException.h>

namespace OrthancStone
{
  IFlexiblePointerTracker* DefaultViewportInteractor::CreateTrackerInternal(
    boost::weak_ptr<IViewport> viewport,
    MouseAction action,
    const PointerEvent& event,
    unsigned int viewportWidth,
    unsigned int viewportHeight)
  {
    switch (action)
    {
      case MouseAction_None:
        return NULL;

      case MouseAction_Rotate:
        return new RotateSceneTracker(viewport, event);

      case MouseAction_GrayscaleWindowing:
        return new GrayscaleWindowingSceneTracker(
          viewport, windowingLayer_, event, viewportWidth, viewportHeight);

      case MouseAction_Pan:
        return new PanSceneTracker(viewport, event);
      
      case MouseAction_Zoom:
        return new ZoomSceneTracker(viewport, event, viewportHeight);
      
      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  IFlexiblePointerTracker* DefaultViewportInteractor::CreateTracker(
    boost::weak_ptr<IViewport>  viewport,
    const PointerEvent&         event,
    unsigned int                viewportWidth,
    unsigned int                viewportHeight)
  {
    MouseAction action;
    
    switch (event.GetMouseButton())
    {
      case MouseButton_Left:
        action = leftButtonAction_;
        break;

      case MouseButton_Middle:
        action = middleButtonAction_;
        break;
      
      case MouseButton_Right:
        action = rightButtonAction_;
        break;

      default:
        return NULL;
    }

    return CreateTrackerInternal(viewport, action, event, viewportWidth, viewportHeight);
  }


  void DefaultViewportInteractor::HandleMouseHover(IViewport& viewport,
                                                   const PointerEvent& event)
  {
    // "HasMouseOver()" returns "false"
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
  }
}
