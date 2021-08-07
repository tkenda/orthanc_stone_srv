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


#include "VolumeSceneLayerSource.h"

#include "../Scene2D/NullLayer.h"
#include "../Viewport/IViewport.h"
#include "../StoneException.h"
#include "../Scene2DViewport/ViewportController.h"

#include <OrthancException.h>

namespace OrthancStone
{
  static bool IsSameCuttingPlane(const CoordinateSystem3D& a,
                                 const CoordinateSystem3D& b)
  {
    // TODO - What if the normal is reversed?
    double distance;
    return (CoordinateSystem3D::ComputeDistance(distance, a, b) &&
            LinearAlgebra::IsCloseToZero(distance));
  }

  void VolumeSceneLayerSource::ClearLayer()
  {
    {
      std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
      if (lock.get())
      {
        ViewportController& controller = lock->GetController();
        Scene2D& scene = controller.GetScene();
        scene.DeleteLayer(layerDepth_);
      }
    }
    lastPlane_.reset(NULL);
  }

  IViewport::ILock* VolumeSceneLayerSource::GetViewportLock()
  {
    boost::shared_ptr<IViewport> viewport = viewport_.lock();
    if (viewport)
      return viewport->Lock();
    else
      return NULL;
  }

  IViewport::ILock* VolumeSceneLayerSource::GetViewportLock() const
  {
    boost::shared_ptr<IViewport> viewport = viewport_.lock();
    if (viewport)
      return viewport->Lock();
    else
      return NULL;
  }


  VolumeSceneLayerSource::VolumeSceneLayerSource(boost::weak_ptr<IViewport>  viewport,
                                                 int layerDepth,
                                                 const boost::shared_ptr<IVolumeSlicer>& slicer) :
    viewport_(viewport),
    layerDepth_(layerDepth),
    slicer_(slicer),
    lastRevision_(0),
    lastConfiguratorRevision_(0)
  {
    if (slicer == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }

    {
      std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
      ViewportController& controller = lock->GetController();
      Scene2D& scene = controller.GetScene();
      ORTHANC_ASSERT(!scene.HasLayer(layerDepth_));

      // we need to book the scene layer depth by adding a dummy layer
      std::unique_ptr<NullLayer> nullLayer(new NullLayer);
      scene.SetLayer(layerDepth_,nullLayer.release());
    }
  }

  VolumeSceneLayerSource::~VolumeSceneLayerSource()
  {
    ClearLayer();
  }

  void VolumeSceneLayerSource::RemoveConfigurator()
  {
    configurator_.reset();
    lastPlane_.reset();
  }


  void VolumeSceneLayerSource::SetConfigurator(ILayerStyleConfigurator* configurator)  // Takes ownership
  {
    if (configurator == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }

    configurator_.reset(configurator);

    // Invalidate the layer
    lastPlane_.reset(NULL);
  }


  ILayerStyleConfigurator& VolumeSceneLayerSource::GetConfigurator() const
  {
    if (configurator_.get() == NULL)
    {
      LOG(ERROR) << "VolumeSceneLayerSource::GetConfigurator(): (configurator_.get() == NULL)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
      
    return *configurator_;
  }


  void VolumeSceneLayerSource::Update(const CoordinateSystem3D& plane)
  {
    std::unique_ptr<IViewport::ILock> lock(GetViewportLock());
    if(lock)
    {
      ViewportController& controller = lock->GetController();
      Scene2D& scene = controller.GetScene();

      assert(slicer_.get() != NULL);
      std::unique_ptr<IVolumeSlicer::IExtractedSlice> slice(slicer_->ExtractSlice(plane));

      if (slice.get() == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);        
      }

      if (!slice->IsValid())
      {
        // The slicer cannot handle this cutting plane: Clear the layer
        ClearLayer();
      }
      else if (lastPlane_.get() != NULL &&
              IsSameCuttingPlane(*lastPlane_, plane) &&
              lastRevision_ == slice->GetRevision())
      {
        // The content of the slice has not changed: Don't update the
        // layer content, but possibly update its style

        if (configurator_.get() != NULL &&
            configurator_->GetRevision() != lastConfiguratorRevision_ &&
            scene.HasLayer(layerDepth_))
        {
          configurator_->ApplyStyle(scene.GetLayer(layerDepth_));
        }
      }
      else
      {
        LOG(TRACE) << "VolumeSceneLayerSource::Update -- Content has changed: An update is needed";
        // Content has changed: An update is needed
        lastPlane_.reset(new CoordinateSystem3D(plane));
        lastRevision_ = slice->GetRevision();

        std::unique_ptr<ISceneLayer> layer(slice->CreateSceneLayer(configurator_.get(), plane));
        if (layer.get() == NULL)
        {
          LOG(TRACE) << "VolumeSceneLayerSource::Update -- (layer.get() == NULL)";
          ClearLayer();
        }
        else
        {
          LOG(TRACE) << "VolumeSceneLayerSource::Update -- (layer.get() != NULL)";
          if (configurator_.get() != NULL)
          {
            lastConfiguratorRevision_ = configurator_->GetRevision();
            configurator_->ApplyStyle(*layer);
          }

          scene.SetLayer(layerDepth_, layer.release());
        }
      }
    }
  }
}
