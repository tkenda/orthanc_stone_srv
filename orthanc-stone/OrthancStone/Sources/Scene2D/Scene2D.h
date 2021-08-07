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

#include "ISceneLayer.h"
#include "../Toolbox/AffineTransform2D.h"
#include "../Messages/IObservable.h"
#include "../Messages/IMessage.h"

#include <map>

namespace OrthancStone
{
  class Scene2D : public boost::noncopyable
  {
  public:
    class IVisitor : public boost::noncopyable
    {
    public:
      virtual ~IVisitor()
      {
      }

      virtual void Visit(const Scene2D& scene,
                         const ISceneLayer& layer,
                         uint64_t layerIdentifier,
                         int depth) = 0;
    };

  private:
    class Item;
    
    typedef std::map<int, Item*>  Content;

    Content            content_;
    AffineTransform2D  sceneToCanvas_;
    AffineTransform2D  canvasToScene_;
    uint64_t           layerCounter_;

    Scene2D(const Scene2D& other);
    
  public:
    Scene2D() : layerCounter_(0)
    {
    }
    
    ~Scene2D();

    Scene2D* Clone() const
    {
      return new Scene2D(*this);
    }

    ISceneLayer& SetLayer(int depth,
                          ISceneLayer* layer);  // Takes ownership

    /**
    Removes the layer at specified depth and deletes the underlying object
    */
    void DeleteLayer(int depth);

    bool HasLayer(int depth) const;

    ISceneLayer& GetLayer(int depth) const;

    /**
    Returns the minimum depth among all layers or 0 if there are no layers
    */
    int GetMinDepth() const;

    /**
    Returns the minimum depth among all layers or 0 if there are no layers
    */
    int GetMaxDepth() const;

    /**
    Removes the layer at specified depth and transfers the object 
    ownership to the caller
    */
    ISceneLayer* ReleaseLayer(int depth);

    void Apply(IVisitor& visitor) const;

    const AffineTransform2D& GetSceneToCanvasTransform() const
    {
      return sceneToCanvas_;
    }

    const AffineTransform2D& GetCanvasToSceneTransform() const
    {
      return canvasToScene_;
    }

    void SetSceneToCanvasTransform(const AffineTransform2D& transform);

    void FitContent(unsigned int canvasWidth,
                    unsigned int canvasHeight);

    void GetBoundingBox(Extent2D& target) const;
  };
}
