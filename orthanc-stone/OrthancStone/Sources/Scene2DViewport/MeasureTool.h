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

#include "../Messages/ObserverBase.h"
#include "../Scene2D/PolylineSceneLayer.h"
#include "../Scene2D/Scene2D.h"
#include "../Scene2D/ScenePoint2D.h"
#include "../Scene2D/TextSceneLayer.h"
#include "../Scene2DViewport/PredeclaredTypes.h"
#include "../Scene2DViewport/ViewportController.h"

#include <boost/weak_ptr.hpp>

#include <vector>
#include <cmath>

namespace OrthancStone
{
  class IFlexiblePointerTracker;
  class MeasureToolMemento;

  class MeasureTool : public ObserverBase<MeasureTool>
  {
  private:
    bool     enabled_;


  protected:
    explicit MeasureTool(boost::weak_ptr<IViewport> viewport);

    void PostConstructor();

    /**
       The measuring tool may exist in a standalone fashion, without any available
       scene (because the controller is dead or dying). This call allows to check 
       before accessing the scene.
    */
    bool IsSceneAlive() const;
    
    /**
       This is the meat of the tool: this method must [create (if needed) and]
       update the layers and their data according to the measure tool kind and
       current state. This is repeatedly called during user interaction
    */
    virtual void RefreshScene() = 0;

    /**
       enabled_ is not accessible by subclasses because there is a state machine
       that we do not wanna mess with
    */
    bool IsEnabled() const;

    /**
       Protected to allow sub-classes to use this weak pointer in factory methods
       (pass them to created objects)
    */
    boost::weak_ptr<IViewport> viewport_;

    /**
    This will return a scoped lock to the viewport. 
    If the viewport does not exist anymore, then nullptr is returned.
    */
    IViewport::ILock* GetViewportLock();
    IViewport::ILock* GetViewportLock() const;

  public:
    virtual ~MeasureTool()
    {
    }

    /**
       Enabled tools are rendered in the scene.
    */
    void Enable();

    /**
       Disabled tools are not rendered in the scene. This is useful to be able
       to use them as their own memento in command stacks (when a measure tool
       creation command has been undone, the measure remains alive in the
       command object but is disabled so that it can be redone later on easily)
    */
    void Disable();

    /**
       This method is called when the scene transform changes. It allows to 
       recompute the visual elements whose content depend upon the scene transform
    */
    void OnSceneTransformChanged(
      const ViewportController::SceneTransformChanged& message);
    
    /**
       This function must be implemented by the measuring tool to return whether
       a given point in scene coords is close to the measuring tool.

       This is used for mouse hover highlighting.

       It is assumed that if the pointer position leads to this function returning
       true, then a click at that position will return a tracker to edit the 
       measuring tool
    */
    virtual bool HitTest(ScenePoint2D p) = 0;

    /**
       This method must return a memento the captures the tool state (not including
       the highlighting state
    */
    virtual MeasureToolMemento* CreateMemento() const = 0;

    /**
       This method must apply the supplied memento (this requires RTTI to check
       the type)
    */
    virtual void SetMemento(const MeasureToolMemento& memento) = 0;

    /**
       This must create an edition tracker suitable for the supplied click position,
       or an empty pointer if no hit test (although this should have been checked
       first)
    */
    virtual boost::shared_ptr<IFlexiblePointerTracker> CreateEditionTracker(const PointerEvent& e) = 0;

    /**
       Will change the measuring tool to provide visual feedback on the GUI 
       element that is in the pointer hit zone
    */
    virtual void Highlight(ScenePoint2D p) = 0;

    /**
       This function must reset the visual highlighted hot zone feedback
    */
    virtual void ResetHighlightState() = 0;

    /**
       A description of the measuring tool, useful in debug logs
    */
    virtual std::string GetDescription() = 0;
  };

  class MeasureToolMemento : public boost::noncopyable
  {
  public:
    virtual ~MeasureToolMemento()
    {
    }
  };

}

//extern void TrackerSample_SetInfoDisplayMessage(
//  std::string key, std::string value);
