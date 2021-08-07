/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#include "ReferenceLineFactory.h"

#include "LineLayerRenderer.h"

namespace OrthancStone
{
  ReferenceLineFactory::ReferenceLineFactory(SliceViewerWidget& owner,
                                             SliceViewerWidget& sibling) :
    owner_(owner),
    sibling_(sibling),
    hasLayerIndex_(false)
  {
    style_.SetColor(0, 255, 0);
    slice_ = sibling.GetSlice();
    sibling_.Register(*this);
  }


  void ReferenceLineFactory::NotifySliceContentChange(const SliceViewerWidget& source,
                                               const SliceGeometry& slice)
  {
    if (&source == &sibling_)
    {
      SetSlice(slice);
    }
  }


  void ReferenceLineFactory::SetLayerIndex(size_t layerIndex)
  {
    hasLayerIndex_ = true;
    layerIndex_ = layerIndex;
  }


  void ReferenceLineFactory::SetStyle(const RenderStyle& style)
  {
    style_ = style;
  }


  RenderStyle ReferenceLineFactory::GetRenderStyle()
  {
    return style_;
  }


  void ReferenceLineFactory::SetSlice(const SliceGeometry& slice)
  {
    slice_ = slice;

    if (hasLayerIndex_)
    {
      owner_.InvalidateLayer(layerIndex_);
    }
  }


  ILayerRenderer* ReferenceLineFactory::CreateLayerRenderer(const SliceGeometry& viewportSlice)
  {
    Vector p, d;

    // Compute the line of intersection between the two slices
    if (!GeometryToolbox::IntersectTwoPlanes(p, d, 
                                             slice_.GetOrigin(), slice_.GetNormal(),
                                             viewportSlice.GetOrigin(), viewportSlice.GetNormal()))
    {
      // The two slice are parallel, don't try and display the intersection
      return NULL;
    }

    double x1, y1, x2, y2;
    viewportSlice.ProjectPoint(x1, y1, p);
    viewportSlice.ProjectPoint(x2, y2, p + 1000.0 * d);

    double sx1, sy1, sx2, sy2;
    owner_.GetView().GetSceneExtent(sx1, sy1, sx2, sy2);
        
    if (GeometryToolbox::ClipLineToRectangle(x1, y1, x2, y2, 
                                             x1, y1, x2, y2,
                                             sx1, sy1, sx2, sy2))
    {
      std::unique_ptr<ILayerRenderer> layer(new LineLayerRenderer(x1, y1, x2, y2));
      layer->SetLayerStyle(style_);
      return layer.release();
    }
    else
    {
      // Parallel slices
      return NULL;
    }
  }


  ISliceableVolume& ReferenceLineFactory::GetSourceVolume() const
  {
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
  }


  void ReferenceLineFactory::Configure(SliceViewerWidget& a,
                                       SliceViewerWidget& b)
  {
    {
      size_t layerIndex;
      ILayerRendererFactory& factory = a.AddLayer(layerIndex, new ReferenceLineFactory(a, b));
      dynamic_cast<ReferenceLineFactory&>(factory).SetLayerIndex(layerIndex);
    }

    {
      size_t layerIndex;
      ILayerRendererFactory& factory = b.AddLayer(layerIndex, new ReferenceLineFactory(b, a));
      dynamic_cast<ReferenceLineFactory&>(factory).SetLayerIndex(layerIndex);
    }
  }
}
