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

#include "ViewportController.h"

#include "../StoneException.h"   // For ORTHANC_ASSERT
#include "MeasureCommands.h"
#include "UndoStack.h"

#include <boost/make_shared.hpp>

namespace OrthancStone
{
  ViewportController::ViewportController(boost::weak_ptr<IViewport> viewport)
    : scene_(new Scene2D)
    , viewport_(viewport)
    , canvasToSceneFactor_(1)
  {
    // undoStack_ is not default-initialized, which basically means empty.
    // The controller must be able to cope with this.
  }

  ViewportController::~ViewportController()
  {
  }

  void ViewportController::PushCommand(
    boost::shared_ptr<MeasureCommand> command)
  {
    boost::shared_ptr<UndoStack> undoStack = undoStackW_.lock();
    if (undoStack.get() != NULL)
    {
      undoStack->PushCommand(command);
    }
    else
    {
      LOG(ERROR) << "Internal error: no undo stack!";
    }
  }

  void ViewportController::Undo()
  {
    boost::shared_ptr<UndoStack> undoStack = undoStackW_.lock();
    if (undoStack.get() != NULL)
    {
      undoStack->Undo();
    }
    else
    {
      LOG(ERROR) << "Internal error: no undo stack!";
    }
  }

  void ViewportController::Redo()
  {
    boost::shared_ptr<UndoStack> undoStack = undoStackW_.lock();
    if (undoStack.get() != NULL)
    {
      undoStack->Redo();
    }
    else
    {
      LOG(ERROR) << "Internal error: no undo stack!";
    }
  }

  bool ViewportController::CanUndo() const
  {
    boost::shared_ptr<UndoStack> undoStack = undoStackW_.lock();
    if (undoStack.get() != NULL)
    {
      return undoStack->CanUndo();
    }
    else
    {
      return false;
    }
  }

  bool ViewportController::CanRedo() const
  {
    boost::shared_ptr<UndoStack> undoStack = undoStackW_.lock();
    if (undoStack.get() != NULL)
    {
      return undoStack->CanRedo();
    }
    else
    {
      return false;
    }
  }
  
  std::vector<boost::shared_ptr<MeasureTool> > 
    ViewportController::HitTestMeasureTools(ScenePoint2D p)
  {
    std::vector<boost::shared_ptr<MeasureTool> > ret;
    
    for (size_t i = 0; i < measureTools_.size(); ++i)
    {
      if (measureTools_[i]->HitTest(p))
        ret.push_back(measureTools_[i]);
    }
    return ret;
  }
  
  void ViewportController::ResetMeasuringToolsHighlight()
  {
    for (size_t i = 0; i < measureTools_.size(); ++i)
    {
      measureTools_[i]->ResetHighlightState();
    }
  }

  AffineTransform2D 
    ViewportController::GetCanvasToSceneTransform() const
  {
    return scene_->GetCanvasToSceneTransform();
  }

  AffineTransform2D 
    ViewportController::GetSceneToCanvasTransform() const
  {
    return scene_->GetSceneToCanvasTransform();
  }

  void ViewportController::SetSceneToCanvasTransform(
    const AffineTransform2D& transform)
  {
    scene_->SetSceneToCanvasTransform(transform);

    canvasToSceneFactor_ = scene_->GetCanvasToSceneTransform().ComputeZoom();
    BroadcastMessage(SceneTransformChanged(*this));
  }

  void ViewportController::BroadcastGrayscaleWindowingChanged(double windowingCenter,
                                                              double windowingWidth)
  {
    BroadcastMessage(GrayscaleWindowingChanged(*this, windowingCenter, windowingWidth));
  }

  void ViewportController::FitContent(unsigned int viewportWidth,
                                      unsigned int viewportHeight)
  {
    scene_->FitContent(viewportWidth, viewportHeight);
    canvasToSceneFactor_ = scene_->GetCanvasToSceneTransform().ComputeZoom();
    BroadcastMessage(SceneTransformChanged(*this));
  }

  void ViewportController::AddMeasureTool(
    boost::shared_ptr<MeasureTool> measureTool)
  {
    ORTHANC_ASSERT(std::find(measureTools_.begin(), 
                             measureTools_.end(), 
                             measureTool) == measureTools_.end(), 
                  "Duplicate measure tool");
    measureTools_.push_back(measureTool);
  }

  void ViewportController::RemoveMeasureTool(
    boost::shared_ptr<MeasureTool> measureTool)
  {
    ORTHANC_ASSERT(std::find(measureTools_.begin(), 
                             measureTools_.end(), 
                             measureTool) != measureTools_.end(), 
                  "Measure tool not found");
    measureTools_.erase(
      std::remove(measureTools_.begin(), measureTools_.end(), measureTool), 
      measureTools_.end());
  }

  double ViewportController::GetCanvasToSceneFactor() const
  {
    return canvasToSceneFactor_;
  }

  double ViewportController::GetHandleSideLengthS() const
  {
    return HANDLE_SIDE_LENGTH_CANVAS_COORD * GetCanvasToSceneFactor();
  }

  double ViewportController::GetAngleToolArcRadiusS() const
  {
    return ARC_RADIUS_CANVAS_COORD * GetCanvasToSceneFactor();
  }

  double ViewportController::GetHitTestMaximumDistanceS() const
  {
    return HIT_TEST_MAX_DISTANCE_CANVAS_COORD * GetCanvasToSceneFactor();
  }

  double ViewportController::GetAngleTopTextLabelDistanceS() const
  {
    return TEXT_CENTER_DISTANCE_CANVAS_COORD * GetCanvasToSceneFactor();
  }


  void ViewportController::HandleMousePress(IViewportInteractor&  interactor,
                                            const PointerEvent&   event,
                                            unsigned int          viewportWidth,
                                            unsigned int          viewportHeight)
  {
    if (activeTracker_)
    {
      // We are dealing with a multi-stage tracker (that is made of several 
      // interactions)
      activeTracker_->PointerDown(event);

      if (!activeTracker_->IsAlive())
      {
        activeTracker_.reset();
      }
    }
    else
    {
      // Check whether there is already a measure tool at that position
      for (size_t i = 0; i < measureTools_.size(); ++i)
      {
        if (measureTools_[i]->HitTest(event.GetMainPosition()))
        {
          activeTracker_ = measureTools_[i]->CreateEditionTracker(event);
          return;
        }
      }

      // No measure tool, create new tracker from the interactor
      activeTracker_.reset(interactor.CreateTracker(viewport_, 
                                                    event, 
                                                    viewportWidth, 
                                                    viewportHeight));
    }
  }

  bool ViewportController::HandleMouseMove(const PointerEvent& event)
  {
    if (activeTracker_)
    {
      activeTracker_->PointerMove(event);
      return true;
    }
    else
    {
      return false;
    }
  }

  void ViewportController::HandleMouseRelease(const PointerEvent& event)
  {
    if (activeTracker_)
    {
      activeTracker_->PointerUp(event);

      if (!activeTracker_->IsAlive())
      {
        activeTracker_.reset();
      }
    }
  }
}
