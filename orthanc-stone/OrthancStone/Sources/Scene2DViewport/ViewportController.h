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

#include "PredeclaredTypes.h"

#include "../Messages/IObservable.h"
#include "../Scene2D/Scene2D.h"
#include "../Scene2DViewport/IFlexiblePointerTracker.h"
#include "../Viewport/IViewportInteractor.h"

#include <Compatibility.h>

#include <boost/enable_shared_from_this.hpp>
#include <stack>

namespace OrthancStone
{
  class UndoStack;

  const double ARC_RADIUS_CANVAS_COORD = 30.0;
  const double TEXT_CENTER_DISTANCE_CANVAS_COORD = 90;

  const double HANDLE_SIDE_LENGTH_CANVAS_COORD = 10.0;
  const double HIT_TEST_MAX_DISTANCE_CANVAS_COORD = 15.0;

  const uint8_t TEXT_COLOR_RED = 0;
  const uint8_t TEXT_COLOR_GREEN = 223;
  const uint8_t TEXT_COLOR_BLUE = 81;

  const uint8_t TOOL_ANGLE_LINES_COLOR_RED = 0;
  const uint8_t TOOL_ANGLE_LINES_COLOR_GREEN = 183;
  const uint8_t TOOL_ANGLE_LINES_COLOR_BLUE = 17;
                     
  const uint8_t TOOL_ANGLE_LINES_HL_COLOR_RED = 0;
  const uint8_t TOOL_ANGLE_LINES_HL_COLOR_GREEN = 17;
  const uint8_t TOOL_ANGLE_LINES_HL_COLOR_BLUE = 183;

  const uint8_t TOOL_LINES_COLOR_RED = 0;
  const uint8_t TOOL_LINES_COLOR_GREEN = 223;
  const uint8_t TOOL_LINES_COLOR_BLUE = 21;

  const uint8_t TOOL_LINES_HL_COLOR_RED = 0;
  const uint8_t TOOL_LINES_HL_COLOR_GREEN = 21;
  const uint8_t TOOL_LINES_HL_COLOR_BLUE = 223;

  const uint8_t TEXT_OUTLINE_COLOR_RED = 0;
  const uint8_t TEXT_OUTLINE_COLOR_GREEN = 56;
  const uint8_t TEXT_OUTLINE_COLOR_BLUE = 21;

  /**
  This object is responsible for hosting a scene, responding to messages from
  the model and updating the scene accordingly.

  It contains the list of active measuring tools as well as the stack
  where measuring tool commands are stored.

  The active tracker is also stored in the viewport controller.

  Each canvas or other GUI area where we want to display a 2D image, either 
  directly or through slicing must be assigned a ViewportController.
  */
  class ViewportController : 
    public IObservable,
    public boost::enable_shared_from_this<ViewportController>
  {
  public:
    ORTHANC_STONE_DEFINE_ORIGIN_MESSAGE(__FILE__, __LINE__, \
                                        SceneTransformChanged, \
                                        ViewportController);

    class GrayscaleWindowingChanged : public OriginMessage<ViewportController>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);
      
    private:
      double  windowingCenter_;
      double  windowingWidth_;
      
    public:
      GrayscaleWindowingChanged(const ViewportController& origin,
                                double windowingCenter,
                                double windowingWidth) :
        OriginMessage(origin),
        windowingCenter_(windowingCenter),
        windowingWidth_(windowingWidth)        
      {
      }

      double GetWindowingCenter() const
      {
        return windowingCenter_;
      }

      double GetWindowingWidth() const
      {
        return windowingWidth_;
      }
    };

    explicit ViewportController(boost::weak_ptr<IViewport> viewport);

    ~ViewportController();

    /**
    This method returns the list of measure tools containing the supplied point
    (in scene coords). A tracker can then be requested from the chosen 
    measure tool, if needed
    */
    std::vector<boost::shared_ptr<MeasureTool> > HitTestMeasureTools(
      ScenePoint2D p);

    /**
    This function will traverse the measuring tools and will clear their 
    highlighted state
    */
    void ResetMeasuringToolsHighlight();

    /**
    With this method, the object takes ownership of the supplied tracker and
    updates it according to user interaction
    */
    void AcquireActiveTracker(const boost::shared_ptr<IFlexiblePointerTracker>& tracker)
    {
      activeTracker_ = tracker;
    }

    /** Forwarded to the underlying scene */
    AffineTransform2D GetCanvasToSceneTransform() const;

    /** Forwarded to the underlying scene */
    AffineTransform2D GetSceneToCanvasTransform() const;

    /** Forwarded to the underlying scene, and broadcasted to the observers */
    void SetSceneToCanvasTransform(const AffineTransform2D& transform);

    /** Info broadcasted to the observers */
    void BroadcastGrayscaleWindowingChanged(double windowingCenter,
                                            double windowingWidth);

    /** Forwarded to the underlying scene, and broadcasted to the observers */
    void FitContent(unsigned int viewportWidth,
                    unsigned int viewportHeight);

    /** Adds a new measure tool */
    void AddMeasureTool(boost::shared_ptr<MeasureTool> measureTool);

    /** Removes a measure tool or throws if it cannot be found */
    void RemoveMeasureTool(boost::shared_ptr<MeasureTool> measureTool);

    /**
    The square handle side length in *scene* coordinates
    */
    double GetHandleSideLengthS() const;

    /**
    The angle measure too arc  radius in *scene* coordinates

    Note: you might wonder why this is not part of the AngleMeasureTool itself,
    but we prefer to put all such constants in the same location, to ease 
    */
    double GetAngleToolArcRadiusS() const;

    /**
    The hit test maximum distance in *scene* coordinates.
    If a pointer event is less than GetHandleSideLengthS() to a GUI element,
    the hit test for this GUI element is seen as true
    */
    double GetHitTestMaximumDistanceS() const;

    /**
    Distance between the top of the angle measuring tool and the center of 
    the label showing the actual measure, in *scene* coordinates
    */
    double GetAngleTopTextLabelDistanceS() const;


    /** forwarded to the UndoStack */
    void PushCommand(boost::shared_ptr<MeasureCommand> command);

    /** forwarded to the UndoStack */
    void Undo();

    /** forwarded to the UndoStack */
    void Redo();

    /** forwarded to the UndoStack */
    bool CanUndo() const;

    /** forwarded to the UndoStack */
    bool CanRedo() const;


    // Must be expressed in canvas coordinates
    void HandleMousePress(IViewportInteractor& interactor,
                          const PointerEvent& event,
                          unsigned int viewportWidth,
                          unsigned int viewportHeight);

    // Must be expressed in canvas coordinates. Returns "true" if the
    // state has changed, so that "Invalidate()" can be called.
    bool HandleMouseMove(const PointerEvent& event);

    // Must be expressed in canvas coordinates
    void HandleMouseRelease(const PointerEvent& event);

    const Scene2D& GetScene() const
    {
      return *scene_;
    }

    Scene2D& GetScene()
    {
      return *scene_;
    }

    /**
    This method is used in a move pattern: when the ownership of the scene 
    managed by this viewport controller must be transferred to another 
    controller.
    */
    Scene2D* ReleaseScene()
    {
      return scene_.release();
    }

    /**
    This method is used when one wishes to replace the scene that is currently
    managed by the controller. The previous scene is deleted and the controller
    now has ownership of the new one.
    */
    void AcquireScene(Scene2D* scene)
    {
      scene_.reset(scene);
    }

    /**
    Sets the undo stack that is used by PushCommand, Undo...
    */
    void SetUndoStack(boost::weak_ptr<UndoStack> undoStackW)
    {
      undoStackW_ = undoStackW;
    }
    
    bool HasActiveTracker() const
    {
      return activeTracker_.get() != NULL;
    }

  private:
    double GetCanvasToSceneFactor() const;

    // The scene can be used by the higher-level objects (including the
    // measuring tools), possibly in their destructor, and so it must be 
    // deleted last (and, thus, constructed first)
    std::unique_ptr<Scene2D>   scene_;

    boost::weak_ptr<IViewport>                    viewport_;

    boost::weak_ptr<UndoStack>                    undoStackW_;  // Global stack, possibly shared by all viewports
    std::vector<boost::shared_ptr<MeasureTool> >  measureTools_;
    boost::shared_ptr<IFlexiblePointerTracker>    activeTracker_;  // TODO - Couldn't this be a "std::unique_ptr"?


    // this is cached
    double  canvasToSceneFactor_;    
  };
}
