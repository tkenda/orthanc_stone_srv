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

#include "../Viewport/IViewport.h"

#include <boost/noncopyable.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace OrthancStone
{
  class PolylineSceneLayer;
  class TextSceneLayer;

  /**
  This class holds the indices of a set a layer and supplies
  getters to the concrete layer objects. Sounds very ad hoc, and it is.
  */
  class LayerHolder : public boost::noncopyable
  {
  public:
    /**
    This ctor merely stores the scene and layer counts. No layer creation
    performed at this time
    */
    LayerHolder(
      boost::weak_ptr<IViewport> viewport,
      int polylineLayerCount, int textLayerCount, int infoTextCount = 0);

    /**
    This actually creates the layers
    */
    void CreateLayers();

    /**
    This creates the layers if they are not created yet. Can be useful in 
    some scenarios
    */
    void CreateLayersIfNeeded();

    /**
    Whether the various text and polylines layers have all been created or 
    none at all
    */
    bool AreLayersCreated() const;

    /**
    This removes the layers from the scene
    */
    void DeleteLayers();

    /**
    This removes the layers from the scene if they are already created
    */
    void DeleteLayersIfNeeded();

    /**
    Please note that the returned pointer belongs to the scene.Don't you dare
    storing or deleting it, you fool!

    This throws if the index is not valid or if the layers are not created or
    have been deleted
    */
    PolylineSceneLayer* GetPolylineLayer(int index = 0);

    /**
    Please note that the returned pointer belongs to the scene. Don't you dare
    storing or deleting it, you fool!

    This throws if the index is not valid or if the layers are not created or
    have been deleted
    */
    TextSceneLayer* GetTextLayer(int index = 0);

    //TextSceneLayer* GetTextLayer(int index = 0);

  private:

    /**
    This will return a scoped lock to the viewport.
    If the viewport does not exist anymore, then nullptr is returned.
    */
    IViewport::ILock* GetViewportLock();

    int GetPolylineLayerIndex(int index = 0);
    int GetTextLayerIndex(int index = 0);
    int GetInfoTextLayerIndex(int index = 0);

    int textLayerCount_;
    int polylineLayerCount_;
    int infoTextCount_;
    boost::weak_ptr<IViewport> viewport_;
    int baseLayerIndex_;
  };
}

