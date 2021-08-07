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


#include "Scene2D.h"

#include <OrthancException.h>


namespace OrthancStone
{
  class Scene2D::Item
  {
  private:
    std::unique_ptr<ISceneLayer>  layer_;
    uint64_t                    identifier_;

  public:
    Item(ISceneLayer* layer,
         uint64_t identifier) :
      layer_(layer),
      identifier_(identifier)
    {
      if (layer == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
    }

    ISceneLayer& GetLayer() const
    {
      if (layer_.get() == NULL)
      {
        LOG(ERROR) << "Scene2D::Item::GetLayer(): (layer_.get() == NULL)";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return *layer_;
      }
    }

    ISceneLayer* ReleaseLayer()
    {
      if (layer_.get() == NULL)
      {
        LOG(ERROR) << "Scene2D::Item::ReleaseLayer(): (layer_.get() == NULL)";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return layer_.release();
      }
    }

    uint64_t GetIdentifier() const
    {
      return identifier_;
    }
  };
  
  
  Scene2D::Scene2D(const Scene2D& other) 
    : sceneToCanvas_(other.sceneToCanvas_)
    , canvasToScene_(other.canvasToScene_)
    , layerCounter_(0)
  {
    for (Content::const_iterator it = other.content_.begin();
         it != other.content_.end(); ++it)
    {
      content_[it->first] = new Item(it->second->GetLayer().Clone(), layerCounter_++);
    }
  }

    
  Scene2D::~Scene2D()
  {
    for (Content::iterator it = content_.begin(); 
         it != content_.end(); ++it)
    {
      assert(it->second != NULL);
      delete it->second;
    }
  }


  ISceneLayer& Scene2D::SetLayer(int depth,
                                 ISceneLayer* layer)  // Takes ownership
  {
    LOG(TRACE) << "SetLayer(" << depth << ", " << reinterpret_cast<intptr_t>(layer) << ")";
    std::unique_ptr<Item> item(new Item(layer, layerCounter_++));

    if (layer == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }

    Content::iterator found = content_.find(depth);

    if (found == content_.end())
    {
      content_[depth] = item.release();
    }
    else
    {
      assert(found->second != NULL);
      delete found->second;
      found->second = item.release();
    }

    return *layer;
  }


  void Scene2D::DeleteLayer(int depth)
  {

    Content::iterator found = content_.find(depth);

    if (found != content_.end())
    {
      LOG(TRACE) << "DeleteLayer --found-- (" << depth << ")";
      assert(found->second != NULL);
      delete found->second;
      content_.erase(found);
    }    
  }

  
  bool Scene2D::HasLayer(int depth) const
  {
    return (content_.find(depth) != content_.end());
  }


  ISceneLayer& Scene2D::GetLayer(int depth) const
  {
    Content::const_iterator found = content_.find(depth);

    if (found == content_.end())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      assert(found->second != NULL);
      return found->second->GetLayer();
    }
  }

  
  int Scene2D::GetMinDepth() const
  {
    if (content_.size() == 0)
      return 0;
    else
      return content_.begin()->first;
  }


  int Scene2D::GetMaxDepth() const
  {
    if (content_.size() == 0)
      return 0;
    else
      return content_.rbegin()->first;
  }

  ISceneLayer* Scene2D::ReleaseLayer(int depth)
  {
    Content::iterator found = content_.find(depth);

    if (found == content_.end())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      assert(found->second != NULL);

      std::unique_ptr<ISceneLayer> layer(found->second->ReleaseLayer());
      assert(layer.get() != NULL);

      content_.erase(found);

      return layer.release();
    }    
  }

  void Scene2D::Apply(IVisitor& visitor) const
  {
    for (Content::const_iterator it = content_.begin(); 
         it != content_.end(); ++it)
    {
      assert(it->second != NULL);
      visitor.Visit(*this, it->second->GetLayer(), it->second->GetIdentifier(), it->first);
    }
  }


  void Scene2D::SetSceneToCanvasTransform(const AffineTransform2D& transform)
  {
    // Make sure the transform is invertible before making any change
    AffineTransform2D inverse = AffineTransform2D::Invert(transform);

    sceneToCanvas_ = transform;
    canvasToScene_ = inverse;
  }

  void Scene2D::GetBoundingBox(Extent2D& target) const
  {
    target.Clear();

    for (Content::const_iterator it = content_.begin();
         it != content_.end(); ++it)
    {
      assert(it->second != NULL);

      Extent2D tmp;
      it->second->GetLayer().GetBoundingBox(tmp);
      target.Union(tmp);
    }
  }

  void Scene2D::FitContent(unsigned int canvasWidth,
                           unsigned int canvasHeight)
  {
    Extent2D extent;

    GetBoundingBox(extent);

    if (!extent.IsEmpty())
    {
      double zoomX = static_cast<double>(canvasWidth) / extent.GetWidth();
      double zoomY = static_cast<double>(canvasHeight) / extent.GetHeight();

      double zoom = std::min(zoomX, zoomY);
      if (LinearAlgebra::IsCloseToZero(zoom))
      {
        zoom = 1;
      }

      double panX = extent.GetCenterX();
      double panY = extent.GetCenterY();

      // Bring the center of the scene to (0,0)
      AffineTransform2D t1 = AffineTransform2D::CreateOffset(-panX, -panY);
      
      // Scale the scene
      AffineTransform2D t2 = AffineTransform2D::CreateScaling(zoom, zoom);

      SetSceneToCanvasTransform(AffineTransform2D::Combine(t2, t1));
    }
  }
}
