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

#include "LayerHolder.h"
#include "../Scene2D/TextSceneLayer.h"
#include "../Scene2D/PolylineSceneLayer.h"
#include "../Scene2D/Scene2D.h"
#include "../Viewport/IViewport.h"
#include "../StoneException.h"
#include "ViewportController.h"

namespace OrthancStone
{
  LayerHolder::LayerHolder(
    boost::weak_ptr<IViewport> viewport,
    int        polylineLayerCount,
    int        textLayerCount,
    int        infoTextCount)
    : textLayerCount_(textLayerCount)
    , polylineLayerCount_(polylineLayerCount)
    , infoTextCount_(infoTextCount)
    , viewport_(viewport)
    , baseLayerIndex_(-1)
  {

  }

  IViewport::ILock* LayerHolder::GetViewportLock()
  {
    boost::shared_ptr<IViewport> viewport = viewport_.lock();
    if (viewport)
      return viewport->Lock();
    else
      return NULL;
  }

  void LayerHolder::CreateLayers()
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    ViewportController& controller = lock->GetController();
    Scene2D& scene = controller.GetScene();

    assert(baseLayerIndex_ == -1);

    baseLayerIndex_ = scene.GetMaxDepth() + 100;

    for (int i = 0; i < polylineLayerCount_; ++i)
    {
      std::unique_ptr<PolylineSceneLayer> layer(new PolylineSceneLayer());
      scene.SetLayer(baseLayerIndex_ + i, layer.release());
    }

    for (int i = 0; i < textLayerCount_; ++i)
    {
      std::unique_ptr<TextSceneLayer> layer(new TextSceneLayer());
      scene.SetLayer(baseLayerIndex_ + polylineLayerCount_ + i, layer.release());
    }
    lock->Invalidate();
  }

  void LayerHolder::CreateLayersIfNeeded()
  {
    if (baseLayerIndex_ == -1)
      CreateLayers();
  }

  bool LayerHolder::AreLayersCreated() const
  {
    return (baseLayerIndex_ != -1);
  }

  void LayerHolder::DeleteLayersIfNeeded()
  {
    if (baseLayerIndex_ != -1)
      DeleteLayers();
  }
  
  void LayerHolder::DeleteLayers()
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    if (lock)
    {
      Scene2D& scene = lock->GetController().GetScene();

      for (int i = 0; i < textLayerCount_ + polylineLayerCount_; ++i)
      {
        ORTHANC_ASSERT(scene.HasLayer(baseLayerIndex_ + i), "No layer");
        scene.DeleteLayer(baseLayerIndex_ + i);
      }
      baseLayerIndex_ = -1;
      lock->Invalidate();
    }
  }
  
  PolylineSceneLayer* LayerHolder::GetPolylineLayer(int index /*= 0*/)
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    const Scene2D& scene = lock->GetController().GetScene();

    ORTHANC_ASSERT(baseLayerIndex_ != -1);
    ORTHANC_ASSERT(scene.HasLayer(GetPolylineLayerIndex(index)));
    ISceneLayer* layer = &(scene.GetLayer(GetPolylineLayerIndex(index)));
      
    PolylineSceneLayer* concreteLayer =
      dynamic_cast<PolylineSceneLayer*>(layer);
      
    ORTHANC_ASSERT(concreteLayer != NULL);
    return concreteLayer;
  }

  TextSceneLayer* LayerHolder::GetTextLayer(int index /*= 0*/)
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    const Scene2D& scene = lock->GetController().GetScene();

    ORTHANC_ASSERT(baseLayerIndex_ != -1);
    ORTHANC_ASSERT(scene.HasLayer(GetTextLayerIndex(index)));
    ISceneLayer* layer = &(scene.GetLayer(GetTextLayerIndex(index)));
      
    TextSceneLayer* concreteLayer =
      dynamic_cast<TextSceneLayer*>(layer);
      
    ORTHANC_ASSERT(concreteLayer != NULL);
    return concreteLayer;
  }

  int LayerHolder::GetPolylineLayerIndex(int index /*= 0*/)
  {
    ORTHANC_ASSERT(index < polylineLayerCount_);
    return baseLayerIndex_ + index;
  }
  
  int LayerHolder::GetTextLayerIndex(int index /*= 0*/)
  {
    ORTHANC_ASSERT(index < textLayerCount_);

    // the text layers are placed right after the polyline layers
    // this means they are drawn ON TOP
    return baseLayerIndex_ + polylineLayerCount_ + index;
  }
}
