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

#include "../Scene2D/PolylineSceneLayer.h"
#include "../Scene2D/Scene2D.h"
#include "../Scene2D/ScenePoint2D.h"
#include "../Scene2D/TextSceneLayer.h"
#include "MeasureTool.h"

#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <vector>
#include <cmath>

namespace OrthancStone
{
  class LineMeasureTool : public MeasureTool
  {
  public:
    static boost::shared_ptr<LineMeasureTool> Create(boost::weak_ptr<IViewport> viewport);

    ~LineMeasureTool();

    void SetStart(ScenePoint2D start);
    void SetEnd(ScenePoint2D end);
    void Set(ScenePoint2D start, ScenePoint2D end);


    virtual bool HitTest(ScenePoint2D p) ORTHANC_OVERRIDE;
    virtual void Highlight(ScenePoint2D p) ORTHANC_OVERRIDE;
    virtual void ResetHighlightState() ORTHANC_OVERRIDE;
    virtual boost::shared_ptr<IFlexiblePointerTracker> CreateEditionTracker(const PointerEvent& e) ORTHANC_OVERRIDE;
    virtual MeasureToolMemento* CreateMemento() const ORTHANC_OVERRIDE;
    virtual void SetMemento(const MeasureToolMemento& memento) ORTHANC_OVERRIDE;
    virtual std::string GetDescription() ORTHANC_OVERRIDE;

    enum LineHighlightArea
    {
      LineHighlightArea_None,
      LineHighlightArea_Start,
      LineHighlightArea_End,
      LineHighlightArea_Segment
    };


    LineHighlightArea LineHitTest(ScenePoint2D p);

  private:
    explicit LineMeasureTool(boost::weak_ptr<IViewport> viewport);

    virtual void        RefreshScene() ORTHANC_OVERRIDE;
    void                RemoveFromScene();
    void                SetLineHighlightArea(LineHighlightArea area);

  private:

  private:
    ScenePoint2D                    start_;
    ScenePoint2D                    end_;
    boost::shared_ptr<LayerHolder>  layerHolder_;
    int                             baseLayerIndex_;
    LineHighlightArea               lineHighlightArea_;
  };

  class LineMeasureToolMemento : public MeasureToolMemento
  {
  public:
    ScenePoint2D                    start_;
    ScenePoint2D                    end_;
  };

}

