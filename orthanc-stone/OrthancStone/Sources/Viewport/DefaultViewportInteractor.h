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

#include "IViewportInteractor.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

namespace OrthancStone
{
  class DefaultViewportInteractor : public IViewportInteractor
  {
  private:
    // Index of the layer whose windowing is altered by grayscale windowing action
    int          windowingLayer_;
    MouseAction  leftButtonAction_;
    MouseAction  middleButtonAction_;
    MouseAction  rightButtonAction_;

    IFlexiblePointerTracker* CreateTrackerInternal(boost::weak_ptr<IViewport> viewport,
                                                   MouseAction action,
                                                   const PointerEvent& event,
                                                   unsigned int viewportWidth,
                                                   unsigned int viewportHeight);
    
  public:
    DefaultViewportInteractor() :
      windowingLayer_(0),
      leftButtonAction_(MouseAction_GrayscaleWindowing),
      middleButtonAction_(MouseAction_Pan),
      rightButtonAction_(MouseAction_Zoom)
    {
    }

    int GetWindowingLayer() const
    {
      return windowingLayer_;
    }

    void SetWindowingLayer(int layerIndex)
    {
      windowingLayer_ = layerIndex;
    }

    MouseAction GetLeftButtonAction() const
    {
      return leftButtonAction_;
    }

    void SetLeftButtonAction(MouseAction action)
    {
      leftButtonAction_ = action;
    }

    MouseAction GetMiddleButtonAction() const
    {
      return middleButtonAction_;
    }

    void SetMiddleButtonAction(MouseAction action)
    {
      middleButtonAction_ = action;
    }

    MouseAction GetRightButtonAction() const
    {
      return rightButtonAction_;
    }

    void SetRightButtonAction(MouseAction action)
    {
      rightButtonAction_ = action;
    }
    
    virtual IFlexiblePointerTracker* CreateTracker(boost::weak_ptr<IViewport> viewport,
                                                   const PointerEvent& event,
                                                   unsigned int viewportWidth,
                                                   unsigned int viewportHeight) ORTHANC_OVERRIDE;

    virtual bool HasMouseHover() const ORTHANC_OVERRIDE
    {
      return false;
    }

    virtual void HandleMouseHover(IViewport& viewport,
                                  const PointerEvent& event) ORTHANC_OVERRIDE;
  };
}
