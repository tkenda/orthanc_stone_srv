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

#include "EditAngleMeasureTracker.h"
#include "EditAngleMeasureCommand.h"

#include "../StoneException.h"

namespace OrthancStone
{
  EditAngleMeasureTracker::EditAngleMeasureTracker(
    boost::shared_ptr<MeasureTool>  measureTool,
    boost::weak_ptr<IViewport> viewport,
    const PointerEvent& e)
    : EditMeasureTracker(viewport, e)
  {
    ScenePoint2D scenePos;
    
    {
      std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
      ViewportController& controller = lock->GetController();
      scenePos = e.GetMainPosition().Apply(controller.GetScene().GetCanvasToSceneTransform());
    }
    
    modifiedZone_ = dynamic_cast<AngleMeasureTool&>(*measureTool).AngleHitTest(scenePos);
    command_.reset(new EditAngleMeasureCommand(measureTool, viewport));
  }

  EditAngleMeasureTracker::~EditAngleMeasureTracker()
  {

  }

  void EditAngleMeasureTracker::PointerMove(const PointerEvent& e)
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    
    ViewportController& controller = lock->GetController();
    const Scene2D& scene = controller.GetScene();

    ScenePoint2D scenePos = e.GetMainPosition().Apply(
      scene.GetCanvasToSceneTransform());

    ScenePoint2D delta = scenePos - GetOriginalClickPosition();

    const AngleMeasureToolMemento& memento =
      dynamic_cast<const AngleMeasureToolMemento&>(command_->GetMementoOriginal());

    switch (modifiedZone_)
    {
      case AngleMeasureTool::AngleHighlightArea_Center:
      {
        ScenePoint2D newCenter = memento.center_ + delta;
        GetCommand()->SetCenter(newCenter);
        break;
      }
      
      case AngleMeasureTool::AngleHighlightArea_Side1:
      case AngleMeasureTool::AngleHighlightArea_Side2:
      {
        ScenePoint2D newCenter = memento.center_ + delta;
        ScenePoint2D newSide1End = memento.side1End_ + delta;
        ScenePoint2D newSide2End = memento.side2End_ + delta;
        GetCommand()->SetCenter(newCenter);
        GetCommand()->SetSide1End(newSide1End);
        GetCommand()->SetSide2End(newSide2End);
        break;
      }
      
      case AngleMeasureTool::AngleHighlightArea_Side1End:
      {
        ScenePoint2D newSide1End = memento.side1End_ + delta;
        GetCommand()->SetSide1End(newSide1End);
        break;
      }

      case AngleMeasureTool::AngleHighlightArea_Side2End:
      {
        ScenePoint2D newSide2End = memento.side2End_ + delta;
        GetCommand()->SetSide2End(newSide2End);
        break;
      }
      
      default:
        LOG(WARNING) << "Warning: please retry the measuring tool editing operation!";
        break;
    }
  }

  void EditAngleMeasureTracker::PointerUp(const PointerEvent& e)
  {
    alive_ = false;
  }

  void EditAngleMeasureTracker::PointerDown(const PointerEvent& e)
  {
    LOG(WARNING) << "Additional touches (fingers, pen, mouse buttons...) "
      "are ignored when the edit angle tracker is active";
  }

  boost::shared_ptr<EditAngleMeasureCommand> EditAngleMeasureTracker::GetCommand()
  {
    boost::shared_ptr<EditAngleMeasureCommand> ret = boost::dynamic_pointer_cast<EditAngleMeasureCommand>(command_);
    ORTHANC_ASSERT(ret.get() != NULL, "Internal error in EditAngleMeasureTracker::GetCommand()");
    return ret;
  }
}
