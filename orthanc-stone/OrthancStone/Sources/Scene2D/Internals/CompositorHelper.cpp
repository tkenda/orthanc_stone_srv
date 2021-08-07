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


#include "CompositorHelper.h"

#include <OrthancException.h>

namespace OrthancStone
{
  namespace Internals
  {
    class CompositorHelper::Item : public boost::noncopyable
    {
    private:
      std::unique_ptr<ILayerRenderer>  renderer_;
      const ISceneLayer&             layer_;
      uint64_t                       layerIdentifier_;
      uint64_t                       lastRevision_;

    public:
      Item(ILayerRenderer* renderer,     // Takes ownership
           const ISceneLayer& layer,
           uint64_t layerIdentifier) :
        renderer_(renderer),
        layer_(layer),
        layerIdentifier_(layerIdentifier),
        lastRevision_(layer.GetRevision())
      {
        if (renderer == NULL)
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
        }
      }

      ILayerRenderer& GetRenderer() const
      {
        assert(renderer_.get() != NULL);
        return *renderer_;
      }

      const ISceneLayer& GetLayer() const
      {
        return layer_;
      }

      uint64_t GetLayerIdentifier() const
      {
        return layerIdentifier_;
      }

      uint64_t GetLastRevision() const
      {
        return lastRevision_;
      }

      void UpdateRenderer()
      {
        assert(renderer_.get() != NULL);
        renderer_->Update(layer_);
        lastRevision_ = layer_.GetRevision();
      }
    };


    void CompositorHelper::Visit(const Scene2D& scene,
                                 const ISceneLayer& layer,
                                 uint64_t layerIdentifier,
                                 int depth)
    {
      // "Visit()" is only applied to layers existing in the scene
      assert(scene.HasLayer(depth)); 

      Content::iterator found = content_.find(depth);

      assert(found == content_.end() ||
             found->second != NULL);

      if (found == content_.end() ||
          found->second->GetLayerIdentifier() != layerIdentifier)
      {
        // This is the first time this layer is rendered, or the layer
        // is not the same as before
        if (found != content_.end())
        {
          delete found->second;
          content_.erase(found);
        }

        // the returned renderer can be NULL in case of an unknown layer
        // or a NullLayer
        std::unique_ptr<ILayerRenderer> renderer(factory_.Create(layer));

        if (renderer.get() != NULL)
        {
          renderer->Render(sceneTransform_, canvasWidth_, canvasHeight_);
          content_[depth] = new Item(renderer.release(), layer, layerIdentifier);
        }
      }
      else
      {
        // This layer has already been rendered
        assert(found->second->GetLastRevision() <= layer.GetRevision());

        if (found->second->GetLastRevision() < layer.GetRevision())
        {
          found->second->UpdateRenderer();
        }

        found->second->GetRenderer().Render(sceneTransform_, canvasWidth_, canvasHeight_);
      }

      // Check invariants
      assert(content_.find(depth) == content_.end() ||
             (content_[depth]->GetLayerIdentifier() == layerIdentifier &&
              content_[depth]->GetLastRevision() == layer.GetRevision()));
    }


    CompositorHelper::~CompositorHelper()
    {
      for (Content::iterator it = content_.begin(); it != content_.end(); ++it)
      {
        assert(it->second != NULL);
        delete it->second;
      }
    }

  
    void CompositorHelper::Refresh(const Scene2D& scene,
                                   unsigned int canvasWidth,
                                   unsigned int canvasHeight)
    {
      /**
       * Safeguard mechanism to enforce the fact that the same scene
       * is always used with the compositor. Note that the safeguard
       * is not 100% bullet-proof, as a new scene might reuse the same
       * address as a previous scene.
       **/
      if (lastScene_ != NULL &&
          lastScene_ != &scene)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                        "ICompositor::ResetScene() should have been called");
      }

      lastScene_ = &scene;

      // Bring coordinate (0,0) to the center of the canvas
      AffineTransform2D offset = AffineTransform2D::CreateOffset(
        static_cast<double>(canvasWidth) / 2.0,
        static_cast<double>(canvasHeight) / 2.0);

      sceneTransform_ = AffineTransform2D::Combine(offset, scene.GetSceneToCanvasTransform());
      canvasWidth_ = canvasWidth;
      canvasHeight_ = canvasHeight;
      scene.Apply(*this);
    }
  }
}
