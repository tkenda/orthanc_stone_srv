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

#include "../Scene2D/Scene2D.h"
#include "../Viewport/IViewport.h"
#include "IVolumeSlicer.h"

#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  class IViewport;
  /**
     This class applies one "volume slicer" to a "3D volume", in order
     to create one "2D scene layer" that will be set onto the "2D
     scene". The style of the layer can be fine-tuned using a "layer
     style configurator". The class only changes the layer if the
     cutting plane has been modified since the last call to "Update()".
   */
  class VolumeSceneLayerSource : public boost::noncopyable
  {
  private:
    boost::weak_ptr<IViewport>                viewport_;
    int                                       layerDepth_;
    boost::shared_ptr<IVolumeSlicer>          slicer_;
    std::unique_ptr<ILayerStyleConfigurator>  configurator_;
    std::unique_ptr<CoordinateSystem3D>       lastPlane_;
    uint64_t                                  lastRevision_;
    uint64_t                                  lastConfiguratorRevision_;

    void ClearLayer();

    /**
    This will return a scoped lock to the viewport.
    If the viewport does not exist anymore, then nullptr is returned.
    */
    IViewport::ILock* GetViewportLock();
    IViewport::ILock* GetViewportLock() const;

  public:
    VolumeSceneLayerSource(boost::weak_ptr<IViewport>  viewport,
                           int layerDepth,
                           const boost::shared_ptr<IVolumeSlicer>& slicer);

    ~VolumeSceneLayerSource();

    const IVolumeSlicer& GetSlicer() const
    {
      return *slicer_;
    }

    void RemoveConfigurator();

    void SetConfigurator(ILayerStyleConfigurator* configurator);

    bool HasConfigurator() const
    {
      return configurator_.get() != NULL;
    }

    ILayerStyleConfigurator& GetConfigurator() const;

    /**
    Make sure the Scene2D is protected from concurrent accesses before 
    calling this method.

    If the scene that has been supplied to the ctor is part of an IViewport, 
    you can lock the whole viewport data (including scene) by means of the 
    IViewport::Lock method.
    */ 
    void Update(const CoordinateSystem3D& plane); 
  };
}
