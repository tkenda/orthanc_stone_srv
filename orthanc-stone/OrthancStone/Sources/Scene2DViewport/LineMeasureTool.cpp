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

#include "LineMeasureTool.h"
#include "MeasureToolsToolbox.h"
#include "EditLineMeasureTracker.h"
#include "LayerHolder.h"
#include "../StoneException.h"

#include <Logging.h>

#include <boost/make_shared.hpp>

namespace OrthancStone
{
  LineMeasureTool::LineMeasureTool(
    boost::weak_ptr<IViewport> viewport):
    MeasureTool(viewport),
#if ORTHANC_STONE_ENABLE_OUTLINED_TEXT == 1
    layerHolder_(boost::shared_ptr<LayerHolder>(new LayerHolder(viewport,1,5))),
#else
    layerHolder_(boost::shared_ptr<LayerHolder>(new LayerHolder(viewport,1,1))),
#endif
    baseLayerIndex_(0),
    lineHighlightArea_(LineHighlightArea_None)
  {

  }

  boost::shared_ptr<LineMeasureTool> LineMeasureTool::Create(boost::weak_ptr<IViewport> viewport)
  {
    boost::shared_ptr<LineMeasureTool> obj(new LineMeasureTool(viewport));
    obj->MeasureTool::PostConstructor();
    obj->RefreshScene();
    return obj;
  }

  LineMeasureTool::~LineMeasureTool()
  {
    // this measuring tool is a RABI for the corresponding visual layers
    // stored in the 2D scene
    Disable();
    RemoveFromScene();
  }

  void LineMeasureTool::RemoveFromScene()
  {
    if (layerHolder_->AreLayersCreated() && IsSceneAlive())
    {
      layerHolder_->DeleteLayers();
    }
  }
  
  void LineMeasureTool::SetStart(ScenePoint2D start)
  {
    start_ = start;
    RefreshScene();
  }

  void LineMeasureTool::SetEnd(ScenePoint2D end)
  {
    end_ = end;
    RefreshScene();
  }

  void LineMeasureTool::Set(ScenePoint2D start, ScenePoint2D end)
  {
    start_ = start;
    end_ = end;
    RefreshScene();
  }

  void LineMeasureTool::SetLineHighlightArea(LineHighlightArea area)
  {
    if (lineHighlightArea_ != area)
    {
      lineHighlightArea_ = area;
      RefreshScene();
    }
  }

  std::string LineMeasureTool::GetDescription()
  {
    std::stringstream ss;
    ss << "LineMeasureTool. Start = " << start_ << " End = " << end_;
    return ss.str();
  }

  void LineMeasureTool::ResetHighlightState()
  {
    SetLineHighlightArea(LineHighlightArea_None);
  }
 
  void LineMeasureTool::Highlight(ScenePoint2D p)
  {
    LineHighlightArea lineHighlightArea = LineHitTest(p);
    SetLineHighlightArea(lineHighlightArea);
  }

  LineMeasureTool::LineHighlightArea LineMeasureTool::LineHitTest(ScenePoint2D p)
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    if (lock.get() != NULL)
    {
      ViewportController& controller = lock->GetController();
      const Scene2D& scene = controller.GetScene();

      const double pixelToScene = scene.GetCanvasToSceneTransform().ComputeZoom();
      const double SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD = 
        pixelToScene * HIT_TEST_MAX_DISTANCE_CANVAS_COORD * 
        pixelToScene * HIT_TEST_MAX_DISTANCE_CANVAS_COORD;

      const double sqDistanceFromStart = 
        ScenePoint2D::SquaredDistancePtPt(p, start_);
    
      if (sqDistanceFromStart <= SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD)
        return LineHighlightArea_Start;
    
      const double sqDistanceFromEnd = ScenePoint2D::SquaredDistancePtPt(p, end_);

      if (sqDistanceFromEnd <= SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD)
        return LineHighlightArea_End;

      const double sqDistanceFromPtSegment = 
        ScenePoint2D::SquaredDistancePtSegment(start_, end_, p);
    
      if (sqDistanceFromPtSegment <= SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD)
        return LineHighlightArea_Segment;
    }

    return LineHighlightArea_None;
  }

  bool LineMeasureTool::HitTest(ScenePoint2D p)
  {
    return LineHitTest(p) != LineHighlightArea_None;
  }

  boost::shared_ptr<IFlexiblePointerTracker> LineMeasureTool::CreateEditionTracker(const PointerEvent& e)
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    if (lock.get() != NULL)
    {
      ViewportController& controller = lock->GetController();
      const Scene2D& scene = controller.GetScene();

      ScenePoint2D scenePos = e.GetMainPosition().Apply(
        scene.GetCanvasToSceneTransform());

      if (!HitTest(scenePos))
      {
        return boost::shared_ptr<IFlexiblePointerTracker>();  // NULL
      }
      else
      {
        boost::shared_ptr<EditLineMeasureTracker> editLineMeasureTracker(
          new EditLineMeasureTracker(shared_from_this(), viewport_, e));
        return editLineMeasureTracker;
      }
    }
    else
    {
      return boost::shared_ptr<IFlexiblePointerTracker>();  // NULL
    }
  }

  MeasureToolMemento* LineMeasureTool::CreateMemento() const
  {
    std::unique_ptr<LineMeasureToolMemento> memento(new LineMeasureToolMemento());
    memento->start_ = start_;
    memento->end_ = end_;
    return memento.release();
  }

  void LineMeasureTool::SetMemento(const MeasureToolMemento& mementoBase)
  {
    const LineMeasureToolMemento& memento = dynamic_cast<const LineMeasureToolMemento&>(mementoBase);
    start_ = memento.start_;
    end_ = memento.end_;
    RefreshScene();
  }

  void LineMeasureTool::RefreshScene()
  {
    if (IsSceneAlive())
    {
      if (IsEnabled())
      {
        
        std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
        ViewportController& controller = lock->GetController();
        Scene2D& scene = controller.GetScene();

        layerHolder_->CreateLayersIfNeeded();
        {
          // Fill the polyline layer with the measurement line

          PolylineSceneLayer* polylineLayer = layerHolder_->GetPolylineLayer(0);
          if (polylineLayer)
          {
            polylineLayer->ClearAllChains();

            const Color color(TOOL_LINES_COLOR_RED, 
                              TOOL_LINES_COLOR_GREEN, 
                              TOOL_LINES_COLOR_BLUE);

            const Color highlightColor(TOOL_LINES_HL_COLOR_RED,
                                       TOOL_LINES_HL_COLOR_GREEN,
                                       TOOL_LINES_HL_COLOR_BLUE);

            {
              PolylineSceneLayer::Chain chain;
              chain.push_back(start_);
              chain.push_back(end_);
              if(lineHighlightArea_ == LineHighlightArea_Segment)
                polylineLayer->AddChain(chain, false, highlightColor);
              else
                polylineLayer->AddChain(chain, false, color);
            }

            // handles
            {
              {
                PolylineSceneLayer::Chain chain;
              
                //TODO: take DPI into account
                AddSquare(chain, controller.GetScene(), start_, 
                          controller.GetHandleSideLengthS());
              
                if (lineHighlightArea_ == LineHighlightArea_Start)
                  polylineLayer->AddChain(chain, true, highlightColor);
                else
                  polylineLayer->AddChain(chain, true, color);
              }

              {
                PolylineSceneLayer::Chain chain;
              
                //TODO: take DPI into account
                AddSquare(chain, controller.GetScene(), end_, 
                          controller.GetHandleSideLengthS());
              
                if (lineHighlightArea_ == LineHighlightArea_End)
                  polylineLayer->AddChain(chain, true, highlightColor);
                else
                  polylineLayer->AddChain(chain, true, color);
              }
            }
          }
        }
        {
          // Set the text layer propreties
          double deltaX = end_.GetX() - start_.GetX();
          double deltaY = end_.GetY() - start_.GetY();
          double squareDist = deltaX * deltaX + deltaY * deltaY;
          double dist = sqrt(squareDist);
          char buf[64];
          sprintf(buf, "%0.02f mm", dist);

          // TODO: for now we simply position the text overlay at the middle
          // of the measuring segment
          double midX = 0.5 * (end_.GetX() + start_.GetX());
          double midY = 0.5 * (end_.GetY() + start_.GetY());

          {

#if ORTHANC_STONE_ENABLE_OUTLINED_TEXT == 1
            SetTextLayerOutlineProperties(
              scene, layerHolder_, buf, ScenePoint2D(midX, midY), 0);
#else
            SetTextLayerProperties(
              scene, layerHolder_, buf, ScenePoint2D(midX, midY), 0);
#endif
            lock->Invalidate();
          }
        }
        lock->Invalidate();
      }
      else
      {
        RemoveFromScene();
      }
    }
  }
}
