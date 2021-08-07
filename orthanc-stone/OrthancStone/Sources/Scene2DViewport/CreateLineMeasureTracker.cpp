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

#include "CreateLineMeasureTracker.h"
#include "CreateLineMeasureCommand.h"

#include <OrthancException.h>

namespace OrthancStone
{
  CreateLineMeasureTracker::CreateLineMeasureTracker(
    boost::weak_ptr<IViewport>          viewport,
    const PointerEvent&             e)
    : CreateMeasureTracker(viewport)
  {
    ScenePoint2D point;
    
    {
      std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
      ViewportController& controller = lock->GetController();
      point = e.GetMainPosition().Apply(controller.GetScene().GetCanvasToSceneTransform());
    }
    
    command_.reset(new CreateLineMeasureCommand(viewport, point));
  }

  CreateLineMeasureTracker::~CreateLineMeasureTracker()
  {

  }

  void CreateLineMeasureTracker::PointerMove(const PointerEvent& event)
  {
    if (!alive_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
        "Internal error: wrong state in CreateLineMeasureTracker::"
        "PointerMove: active_ == false");
    }

    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    ViewportController& controller = lock->GetController();

    ScenePoint2D scenePos = event.GetMainPosition().Apply(
      controller.GetScene().GetCanvasToSceneTransform());
      
    //LOG(TRACE) << "scenePos.GetX() = " << scenePos.GetX() << "     " <<
    //  "scenePos.GetY() = " << scenePos.GetY();
      
    /*CreateLineMeasureTracker* concreteThis =
      dynamic_cast<CreateLineMeasureTracker*>(this);
      assert(concreteThis != NULL);*/

    GetCommand()->SetEnd(scenePos);
  }

  void CreateLineMeasureTracker::PointerUp(const PointerEvent& e)
  {
    // TODO: the current app does not prevent multiple PointerDown AND
    // PointerUp to be sent to the tracker.
    // Unless we augment the PointerEvent structure with the button index, 
    // we cannot really tell if this pointer up event matches the initial
    // pointer down event. Let's make it simple for now.
    alive_ = false;
  }

  void CreateLineMeasureTracker::PointerDown(const PointerEvent& e)
  {
    LOG(WARNING) << "Additional touches (fingers, pen, mouse buttons...) "
      "are ignored when the line measure creation tracker is active";
  }

  boost::shared_ptr<CreateLineMeasureCommand> CreateLineMeasureTracker::GetCommand()
  {
    return boost::dynamic_pointer_cast<CreateLineMeasureCommand>(command_);
  }
}
