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

#include "EditLineMeasureTracker.h"
#include "EditLineMeasureCommand.h"

#include "../StoneException.h"


namespace OrthancStone
{
  EditLineMeasureTracker::EditLineMeasureTracker(
    boost::shared_ptr<MeasureTool>  measureTool,
    boost::weak_ptr<IViewport> viewport,
    const PointerEvent& e)
    : EditMeasureTracker(viewport, e)
  {
    ScenePoint2D scenePos;
    
    {
      std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
      Scene2D& scene = lock->GetController().GetScene();
      scenePos = e.GetMainPosition().Apply(scene.GetCanvasToSceneTransform());
    }
    
    modifiedZone_ = dynamic_cast<LineMeasureTool&>(*measureTool).LineHitTest(scenePos);
    command_.reset(new EditLineMeasureCommand(measureTool, viewport));
  }

  void EditLineMeasureTracker::PointerMove(const PointerEvent& e)
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    ViewportController& controller = lock->GetController();
    const Scene2D& scene = controller.GetScene();

    ScenePoint2D scenePos = e.GetMainPosition().Apply(
      scene.GetCanvasToSceneTransform());

    ScenePoint2D delta = scenePos - GetOriginalClickPosition();

    const LineMeasureToolMemento& memento =
      dynamic_cast<const LineMeasureToolMemento&>(command_->GetMementoOriginal());

    switch (modifiedZone_)
    {
      case LineMeasureTool::LineHighlightArea_Start:
      {
        ScenePoint2D newStart = memento.start_ + delta;
        GetCommand()->SetStart(newStart);
        break;
      }
      
      case LineMeasureTool::LineHighlightArea_End:
      {
        ScenePoint2D newEnd = memento.end_ + delta;
        GetCommand()->SetEnd(newEnd);
        break;
      }
      
      case LineMeasureTool::LineHighlightArea_Segment:
      {
        ScenePoint2D newStart = memento.start_ + delta;
        ScenePoint2D newEnd = memento.end_ + delta;
        GetCommand()->SetStart(newStart);
        GetCommand()->SetEnd(newEnd);
        break;
      }
      
      default:
        LOG(WARNING) << "Warning: please retry the measuring tool editing operation!";
        break;
    }
  }

  void EditLineMeasureTracker::PointerUp(const PointerEvent& e)
  {
    alive_ = false;
  }

  void EditLineMeasureTracker::PointerDown(const PointerEvent& e)
  {
    LOG(WARNING) << "Additional touches (fingers, pen, mouse buttons...) "
      "are ignored when the edit line tracker is active";
  }

  boost::shared_ptr<EditLineMeasureCommand> EditLineMeasureTracker::GetCommand()
  {
    boost::shared_ptr<EditLineMeasureCommand> ret = boost::dynamic_pointer_cast<EditLineMeasureCommand>(command_);
    ORTHANC_ASSERT(ret.get() != NULL, "Internal error in EditLineMeasureTracker::GetCommand()");
    return ret;
  }
}
