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


#include "MacroSceneLayer.h"

#include <OrthancException.h>

#include <cassert>

namespace OrthancStone
{
  void MacroSceneLayer::CheckInvariant() const
  {
#if !defined(NDEBUG)
    // Only run the sanity check in debug mode
    size_t countRecycled = 0;

    for (size_t i = 0; i < layers_.size(); i++)
    {
      if (layers_[i] == NULL)
      {
        assert(recycledLayers_.find(i) != recycledLayers_.end());
        countRecycled++;
      }
      else
      {
        assert(recycledLayers_.find(i) == recycledLayers_.end());
      }
    }

    assert(countRecycled == recycledLayers_.size());
#endif
  }
  

  void MacroSceneLayer::Clear()
  {
    CheckInvariant();
    
    for (size_t i = 0; i < layers_.size(); i++)
    {
      if (layers_[i] != NULL)
      {
        delete layers_[i];
      }
    }

    layers_.clear();
    recycledLayers_.clear();
    
    BumpRevision();
  }

  
  size_t MacroSceneLayer::AddLayer(ISceneLayer* layer)
  {
    CheckInvariant();
    
    if (layer == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else
    {
      size_t index;

      if (recycledLayers_.empty())
      {
        index = layers_.size();
        layers_.push_back(layer);
      }
      else
      {
        index = *recycledLayers_.begin();
        assert(layers_[index] == NULL);
        layers_[index] = layer;
        recycledLayers_.erase(index);
      }
      
      BumpRevision();
      return index;
    }
  }

  
  void MacroSceneLayer::UpdateLayer(size_t index,
                                    ISceneLayer* layer)
  {
    CheckInvariant();
    
    if (layer == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else if (index >= layers_.size())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      if (layers_[index] == NULL)
      {
        assert(recycledLayers_.find(index) != recycledLayers_.end());
        recycledLayers_.erase(index);
      }
      else
      {
        assert(recycledLayers_.find(index) == recycledLayers_.end());
        delete layers_[index];
      }

      layers_[index] = layer;
      BumpRevision();
    }
  }    


  bool MacroSceneLayer::HasLayer(size_t index) const
  {
    CheckInvariant();
    
    if (index >= layers_.size())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      return (layers_[index] != NULL);
    }
  }
  

  void MacroSceneLayer::DeleteLayer(size_t index)
  {
    CheckInvariant();
    
    if (index >= layers_.size())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else if (layers_[index] == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentItem);
    }
    else
    {
      delete layers_[index];
      layers_[index] = NULL;

      assert(recycledLayers_.find(index) == recycledLayers_.end());
      recycledLayers_.insert(index);
    }
  }


  const ISceneLayer& MacroSceneLayer::GetLayer(size_t index) const
  {
    CheckInvariant();
    
    if (index >= layers_.size())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else if (layers_[index] == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InexistentItem);
    }
    else
    {
      return *layers_[index];
    }
  }


  ISceneLayer* MacroSceneLayer::Clone() const
  {
    CheckInvariant();
    
    std::unique_ptr<MacroSceneLayer> copy(new MacroSceneLayer);

    for (size_t i = 0; i < layers_.size(); i++)
    {
      if (layers_[i] == NULL)
      {
        copy->layers_.push_back(NULL);
      }
      else
      {
        copy->layers_.push_back(layers_[i]->Clone());
      }
    }

    copy->recycledLayers_ = recycledLayers_;

    return copy.release();
  }
  

  void MacroSceneLayer::GetBoundingBox(Extent2D& target) const
  {
    CheckInvariant();
    
    target.Clear();

    for (size_t i = 0; i < layers_.size(); i++)
    {
      if (layers_[i] != NULL)
      {
        Extent2D subextent;
        layers_[i]->GetBoundingBox(subextent);
        target.Union(subextent);
      }
    }
  }
}
