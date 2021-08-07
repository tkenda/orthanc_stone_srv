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

#include "../Scene2D.h"
#include "../ScenePoint2D.h"

#include <Compatibility.h>  // For ORTHANC_OVERRIDE

#include <boost/noncopyable.hpp>
#include <map>

namespace OrthancStone
{
  namespace Internals
  {
    class CompositorHelper : protected Scene2D::IVisitor
    {
    public:
      class ILayerRenderer : public boost::noncopyable
      {
      public:
        virtual ~ILayerRenderer()
        {
        }

        virtual void Render(const AffineTransform2D& transform,
                            unsigned int canvasWidth,
                            unsigned int canvasHeight) = 0;

        // "Update()" is only called if the type of the layer has not changed
        virtual void Update(const ISceneLayer& layer) = 0;
      };

      class IRendererFactory : public boost::noncopyable
      {
      public:
        virtual ~IRendererFactory()
        {
        }

        virtual ILayerRenderer* Create(const ISceneLayer& layer) = 0;
      };

    private:
      class Item;

      typedef std::map<int, Item*>  Content;

      IRendererFactory&  factory_;
      Content            content_;
      const Scene2D*     lastScene_;   // This is only a safeguard, don't use it!

      // Only valid during a call to Refresh()
      AffineTransform2D  sceneTransform_;
      unsigned int       canvasWidth_;
      unsigned int       canvasHeight_;
      
    protected:
      virtual void Visit(const Scene2D& scene,
                         const ISceneLayer& layer,
                         uint64_t layerIdentifier,
                         int depth) ORTHANC_OVERRIDE;

    public:
      explicit CompositorHelper(IRendererFactory& factory) :
        factory_(factory),
        lastScene_(NULL),
        canvasWidth_(0),
        canvasHeight_(0)
      {
      }

      ~CompositorHelper();

      void Refresh(const Scene2D& scene,
                   unsigned int canvasWidth,
                   unsigned int canvasHeight);
    };
  }
}
