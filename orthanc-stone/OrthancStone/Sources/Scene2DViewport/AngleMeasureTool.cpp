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

#include "AngleMeasureTool.h"
#include "MeasureToolsToolbox.h"
#include "EditAngleMeasureTracker.h"
#include "LayerHolder.h"
#include "../StoneException.h"

#include <Logging.h>

#include <boost/math/constants/constants.hpp>
#include <boost/make_shared.hpp>

//// <HACK>
//// REMOVE THIS
//#ifndef NDEBUG
//extern void 
//TrackerSample_SetInfoDisplayMessage(std::string key, std::string value);
//#endif
//// </HACK>

namespace OrthancStone
{
  // the params in the LayerHolder ctor specify the number of polyline and text
  // layers
  AngleMeasureTool::AngleMeasureTool(
    boost::weak_ptr<IViewport> viewport)
    : MeasureTool(viewport)
#if ORTHANC_STONE_ENABLE_OUTLINED_TEXT == 1
    , layerHolder_(boost::shared_ptr<LayerHolder>(new LayerHolder(viewport,1,5)))
#else
    , layerHolder_(boost::shared_ptr<LayerHolder>(new LayerHolder(viewport,1,1)))
#endif
    , angleHighlightArea_(AngleHighlightArea_None)
  {
  }

  boost::shared_ptr<AngleMeasureTool> AngleMeasureTool::Create(boost::weak_ptr<IViewport> viewport)
  {
    boost::shared_ptr<AngleMeasureTool> obj(new AngleMeasureTool(viewport));
    obj->MeasureTool::PostConstructor();
    obj->RefreshScene();
    return obj;
  }

  AngleMeasureTool::~AngleMeasureTool()
  {
    // this measuring tool is a RABI for the corresponding visual layers
    // stored in the 2D scene
    Disable();
    RemoveFromScene();
  }

  void AngleMeasureTool::RemoveFromScene()
  {
    if (layerHolder_->AreLayersCreated() && IsSceneAlive())
    {
      layerHolder_->DeleteLayers();
    }
  }

  void AngleMeasureTool::SetSide1End(ScenePoint2D pt)
  {
    side1End_ = pt;
    RefreshScene();
  }

  void AngleMeasureTool::SetSide2End(ScenePoint2D pt)
  {
    side2End_ = pt;
    RefreshScene();
  }

  void AngleMeasureTool::SetAngleHighlightArea(AngleHighlightArea area)
  {
    if (angleHighlightArea_ != area)
    {
      angleHighlightArea_ = area;
      RefreshScene();
    }
  }

  void AngleMeasureTool::ResetHighlightState()
  {
    SetAngleHighlightArea(AngleHighlightArea_None);
  }


  MeasureToolMemento* AngleMeasureTool::CreateMemento() const
  {
    std::unique_ptr<AngleMeasureToolMemento> memento(new AngleMeasureToolMemento());
    memento->center_ = center_;
    memento->side1End_ = side1End_;
    memento->side2End_ = side2End_;
    return memento.release();
  }
  
  void AngleMeasureTool::SetMemento(const MeasureToolMemento& mementoBase)
  {
    const AngleMeasureToolMemento& memento = dynamic_cast<const AngleMeasureToolMemento&>(mementoBase);
    center_   = memento.center_;
    side1End_ = memento.side1End_;
    side2End_ = memento.side2End_;
    RefreshScene();
  }

  std::string AngleMeasureTool::GetDescription()
  {
    std::stringstream ss;
    ss << "AngleMeasureTool. Center = " << center_ << " Side1End = " 
       << side1End_ << " Side2End = " << side2End_;
    return ss.str();
  }

  void AngleMeasureTool::Highlight(ScenePoint2D p)
  {
    AngleHighlightArea angleHighlightArea = AngleHitTest(p);
    SetAngleHighlightArea(angleHighlightArea);
  }

  AngleMeasureTool::AngleHighlightArea AngleMeasureTool::AngleHitTest(ScenePoint2D p) const
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    const ViewportController& controller = lock->GetController();
    const Scene2D& scene = controller.GetScene();
    
    const double pixelToScene = scene.GetCanvasToSceneTransform().ComputeZoom();

    const double SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD = 
      pixelToScene * HIT_TEST_MAX_DISTANCE_CANVAS_COORD * 
      pixelToScene * HIT_TEST_MAX_DISTANCE_CANVAS_COORD;

    {
      const double sqDistanceFromSide1End = 
        ScenePoint2D::SquaredDistancePtPt(p, side1End_);

      if (sqDistanceFromSide1End <= SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD)
        return AngleHighlightArea_Side1End;
    }

    {
      const double sqDistanceFromSide2End = 
        ScenePoint2D::SquaredDistancePtPt(p, side2End_);

      if (sqDistanceFromSide2End <= SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD)
        return AngleHighlightArea_Side2End;
    }

    {
      const double sqDistanceFromCenter = 
        ScenePoint2D::SquaredDistancePtPt(p, center_);
      if (sqDistanceFromCenter <= SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD)
        return AngleHighlightArea_Center;
    }

    {
      const double sqDistanceFromSide1 = 
        ScenePoint2D::SquaredDistancePtSegment(center_, side1End_, p);

      if (sqDistanceFromSide1 <= SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD)
        return AngleHighlightArea_Side1;
    }

    {
      const double sqDistanceFromSide2 = 
        ScenePoint2D::SquaredDistancePtSegment(center_, side2End_, p);

      if (sqDistanceFromSide2 <= SQUARED_HIT_TEST_MAX_DISTANCE_SCENE_COORD)
        return AngleHighlightArea_Side2;
    }

    return AngleHighlightArea_None;
  }

  bool AngleMeasureTool::HitTest(ScenePoint2D p)
  {
    return AngleHitTest(p) != AngleHighlightArea_None;
  }


  boost::shared_ptr<IFlexiblePointerTracker> AngleMeasureTool::CreateEditionTracker(const PointerEvent& e)
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    ViewportController& controller = lock->GetController();
    const Scene2D& scene = controller.GetScene();

    ScenePoint2D scenePos = e.GetMainPosition().Apply(
      scene.GetCanvasToSceneTransform());

    if (!HitTest(scenePos))
    {
      return boost::shared_ptr<IFlexiblePointerTracker>(); // NULL
    }
    else
    {
      /**
         new EditLineMeasureTracker(
         boost::shared_ptr<LineMeasureTool> measureTool;
         MessageBroker & broker,
         boost::weak_ptr<IViewport>          viewport,
         const PointerEvent & e);
      */

      boost::shared_ptr<EditAngleMeasureTracker> editAngleMeasureTracker(
        new EditAngleMeasureTracker(shared_from_this(), viewport_, e));
      return editAngleMeasureTracker;
    }
  }

  void AngleMeasureTool::SetCenter(ScenePoint2D pt)
  {
    center_ = pt;
    RefreshScene();
  }
  
  void AngleMeasureTool::RefreshScene()
  {
    if (IsSceneAlive())
    {
      std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
      ViewportController& controller = lock->GetController();
      Scene2D& scene = controller.GetScene();

      if (IsEnabled())
      {
        layerHolder_->CreateLayersIfNeeded();

        {
          // Fill the polyline layer with the measurement lines
          PolylineSceneLayer* polylineLayer = layerHolder_->GetPolylineLayer(0);
          if (polylineLayer)
          {
            polylineLayer->ClearAllChains();

            const Color color(TOOL_ANGLE_LINES_COLOR_RED, 
                              TOOL_ANGLE_LINES_COLOR_GREEN, 
                              TOOL_ANGLE_LINES_COLOR_BLUE);

            const Color highlightColor(TOOL_ANGLE_LINES_HL_COLOR_RED, 
                                       TOOL_ANGLE_LINES_HL_COLOR_GREEN, 
                                       TOOL_ANGLE_LINES_HL_COLOR_BLUE);

            // sides
            {
              {
                PolylineSceneLayer::Chain chain;
                chain.push_back(side1End_);
                chain.push_back(center_);

                if ((angleHighlightArea_ == AngleHighlightArea_Side1) ||
                    (angleHighlightArea_ == AngleHighlightArea_Side2))
                {
                  polylineLayer->AddChain(chain, false, highlightColor);
                } 
                else
                {
                  polylineLayer->AddChain(chain, false, color);
                }
              }
              {
                PolylineSceneLayer::Chain chain;
                chain.push_back(side2End_);
                chain.push_back(center_);
                if ((angleHighlightArea_ == AngleHighlightArea_Side1) ||
                  (angleHighlightArea_ == AngleHighlightArea_Side2))
                {
                  polylineLayer->AddChain(chain, false, highlightColor);
                }
                else
                {
                  polylineLayer->AddChain(chain, false, color);
                }
              }
            }

            // Create the handles
            {
              {
                PolylineSceneLayer::Chain chain;
                //TODO: take DPI into account
                AddSquare(chain, controller.GetScene(), side1End_, 
                          controller.GetHandleSideLengthS());
              
                if (angleHighlightArea_ == AngleHighlightArea_Side1End)
                  polylineLayer->AddChain(chain, true, highlightColor);
                else
                  polylineLayer->AddChain(chain, true, color);
              
              }
              {
                PolylineSceneLayer::Chain chain;
                //TODO: take DPI into account
                AddSquare(chain, controller.GetScene(), side2End_, 
                          controller.GetHandleSideLengthS());

                if (angleHighlightArea_ == AngleHighlightArea_Side2End)
                  polylineLayer->AddChain(chain, true, highlightColor);
                else
                  polylineLayer->AddChain(chain, true, color);
              }
            }

            // Create the arc
            {
              PolylineSceneLayer::Chain chain;

              AddShortestArc(chain, side1End_, center_, side2End_,
                             controller.GetAngleToolArcRadiusS());
              if (angleHighlightArea_ == AngleHighlightArea_Center)
                polylineLayer->AddChain(chain, false, highlightColor);
              else
                polylineLayer->AddChain(chain, false, color);
            }
          }
        }
        {
          // Set the text layer

          double p1cAngle = atan2(
            side1End_.GetY() - center_.GetY(),
            side1End_.GetX() - center_.GetX());

          double p2cAngle = atan2(
            side2End_.GetY() - center_.GetY(),
            side2End_.GetX() - center_.GetX());

          double delta = NormalizeAngle(p2cAngle - p1cAngle);
          double theta = p1cAngle + delta / 2;

          double ox = controller.GetAngleTopTextLabelDistanceS() * cos(theta);
          double oy = controller.GetAngleTopTextLabelDistanceS() * sin(theta);

          double pointX = center_.GetX() + ox;
          double pointY = center_.GetY() + oy;

          char buf[64];
          double angleDeg = std::abs(RadiansToDegrees(delta));

          // http://www.ltg.ed.ac.uk/~richard/utf-8.cgi?input=00B0&mode=hex
          sprintf(buf, "%0.02f\xc2\xb0", angleDeg);

#if ORTHANC_STONE_ENABLE_OUTLINED_TEXT == 1
          SetTextLayerOutlineProperties(
            scene, layerHolder_, buf, ScenePoint2D(pointX, pointY), 0);
#else
          SetTextLayerProperties(
            scene, layerHolder_, buf, ScenePoint2D(pointX, pointY) , 0);
#endif

#if 0
          // TODO:make it togglable
          bool enableInfoDisplay = true;
          if (enableInfoDisplay)
          {
            TrackerSample_SetInfoDisplayMessage("center_.GetX()",
              boost::lexical_cast<std::string>(center_.GetX()));

            TrackerSample_SetInfoDisplayMessage("center_.GetY()",
              boost::lexical_cast<std::string>(center_.GetY()));

            TrackerSample_SetInfoDisplayMessage("side1End_.GetX()",
              boost::lexical_cast<std::string>(side1End_.GetX()));

            TrackerSample_SetInfoDisplayMessage("side1End_.GetY()",
              boost::lexical_cast<std::string>(side1End_.GetY()));

            TrackerSample_SetInfoDisplayMessage("side2End_.GetX()",
              boost::lexical_cast<std::string>(side2End_.GetX()));

            TrackerSample_SetInfoDisplayMessage("side2End_.GetY()",
              boost::lexical_cast<std::string>(side2End_.GetY()));

            TrackerSample_SetInfoDisplayMessage("p1cAngle (deg)",
              boost::lexical_cast<std::string>(RadiansToDegrees(p1cAngle)));

            TrackerSample_SetInfoDisplayMessage("delta (deg)",
              boost::lexical_cast<std::string>(RadiansToDegrees(delta)));

            TrackerSample_SetInfoDisplayMessage("theta (deg)",
              boost::lexical_cast<std::string>(RadiansToDegrees(theta)));

            TrackerSample_SetInfoDisplayMessage("p2cAngle (deg)",
              boost::lexical_cast<std::string>(RadiansToDegrees(p2cAngle)));

            TrackerSample_SetInfoDisplayMessage("ox (scene)",
              boost::lexical_cast<std::string>(ox));

            TrackerSample_SetInfoDisplayMessage("offsetY (scene)",
              boost::lexical_cast<std::string>(oy));

            TrackerSample_SetInfoDisplayMessage("pointX",
              boost::lexical_cast<std::string>(pointX));

            TrackerSample_SetInfoDisplayMessage("pointY",
              boost::lexical_cast<std::string>(pointY));

            TrackerSample_SetInfoDisplayMessage("angleDeg",
              boost::lexical_cast<std::string>(angleDeg));
          }
#endif
        }
      }
      else
      {
        RemoveFromScene();
      }
      lock->Invalidate();
    }
  }
}
