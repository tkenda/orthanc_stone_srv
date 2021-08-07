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

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

#include "DicomStructureSetSlicer2.h"

#include "../Toolbox/GeometryToolbox.h"
#include "../Volumes/IVolumeSlicer.h"
#include "../Scene2D/PolylineSceneLayer.h"

namespace OrthancStone
{
  DicomStructureSetSlicer2::DicomStructureSetSlicer2(boost::shared_ptr<DicomStructureSet2> structureSet)
    : structureSet_(structureSet)
  {}

  IVolumeSlicer::IExtractedSlice* DicomStructureSetSlicer2::ExtractSlice(const CoordinateSystem3D& cuttingPlane)
  {
    // revision is always the same, hence 0
    return new DicomStructureSetSlice2(structureSet_, 0, cuttingPlane);
  }

  DicomStructureSetSlice2::DicomStructureSetSlice2(
    boost::weak_ptr<DicomStructureSet2> structureSet, 
    uint64_t revision, 
    const CoordinateSystem3D& cuttingPlane) 
    : structureSet_(structureSet.lock())
    , isValid_(false)
  {
    bool opposite = false;

    if (structureSet_->GetStructuresCount() == 0)
    {
      isValid_ = false;
    }
    else
    {
      // some structures seen in real life have no polygons. We must be 
      // careful
      bool found = false;
      size_t curStructure = 0;
      while (!found && curStructure < structureSet_->GetStructuresCount())
      {
        if (structureSet_->GetStructure(curStructure).IsValid())
        {
          found = true;
          const Vector normal = structureSet_->GetStructure(0).GetNormal();
          isValid_ = (
            GeometryToolbox::IsParallelOrOpposite(opposite, normal, cuttingPlane.GetNormal()) ||
            GeometryToolbox::IsParallelOrOpposite(opposite, normal, cuttingPlane.GetAxisX()) ||
            GeometryToolbox::IsParallelOrOpposite(opposite, normal, cuttingPlane.GetAxisY()));
        }
      }
    }
  }

  ISceneLayer* DicomStructureSetSlice2::CreateSceneLayer(
    const ILayerStyleConfigurator* configurator,
    const CoordinateSystem3D& cuttingPlane)
  {
    assert(isValid_);

    std::unique_ptr<PolylineSceneLayer> layer(new PolylineSceneLayer);
    layer->SetThickness(2); // thickness of the on-screen line

    for (size_t i = 0; i < structureSet_->GetStructuresCount(); i++)
    {
      const DicomStructure2& structure = structureSet_->GetStructure(i);
      if (structure.IsValid())
      {
        const Color& color = structure.GetColor();

        std::vector< std::pair<Point2D, Point2D> > segments;

        if (structure.Project(segments, cuttingPlane))
        {
          for (size_t j = 0; j < segments.size(); j++)
          {
            PolylineSceneLayer::Chain chain;
            chain.resize(2);

            chain[0] = ScenePoint2D(segments[j].first.x, segments[j].first.y);
            chain[1] = ScenePoint2D(segments[j].second.x, segments[j].second.y);

            layer->AddChain(chain, false /* NOT closed */, color);
          }
        }
      }
    }
    return layer.release();
  }
}


#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2


