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

#include "../OrthancStone.h"

#if !defined(ORTHANC_ENABLE_DCMTK)
#  error The macro ORTHANC_ENABLE_DCMTK must be defined
#endif

#include "DicomStructureSetUtils.h"
#include "CoordinateSystem3D.h"
#include "Extent2D.h"
#include "OrthancDatasets/FullOrthancDataset.h"
#include "../Scene2D/Color.h"
#include "../Scene2D/PolylineSceneLayer.h"

#if ORTHANC_ENABLE_DCMTK == 1
#  include <DicomParsing/ParsedDicomFile.h>
#endif

//#define USE_BOOST_UNION_FOR_POLYGONS 1


#include <list>

namespace OrthancStone
{
  class DicomStructureSet : public boost::noncopyable
  {
  private:
    struct ReferencedSlice
    {
      std::string          seriesInstanceUid_;
      CoordinateSystem3D   geometry_;
      double               thickness_;

      ReferencedSlice()
      {
      }
      
      ReferencedSlice(const std::string& seriesInstanceUid,
                      const CoordinateSystem3D& geometry,
                      double thickness) :
        seriesInstanceUid_(seriesInstanceUid),
        geometry_(geometry),
        thickness_(thickness)
      {
      }
    };

    typedef std::map<std::string, ReferencedSlice>  ReferencedSlices;
    
    typedef std::vector<Vector>  Points;

    class Polygon
    {
    private:
      std::string         sopInstanceUid_;
      bool                hasSlice_;
      CoordinateSystem3D  geometry_;
      double              projectionAlongNormal_;
      double              sliceThickness_;  // In millimeters
      Points              points_;
      Extent2D            extent_;

      void CheckPointIsOnSlice(const Vector& v) const;
      bool IsPointOnSliceIfAny(const Vector& v) const;

    public:
      explicit Polygon(const std::string& sopInstanceUid) :
        sopInstanceUid_(sopInstanceUid),
        hasSlice_(false),
        projectionAlongNormal_(0),
        sliceThickness_(0)
      {
      }

      void Reserve(size_t n)
      {
        points_.reserve(n);
      }

      void AddPoint(const Vector& v);

      bool UpdateReferencedSlice(const ReferencedSlices& slices);

      bool IsOnSlice(const CoordinateSystem3D& geometry) const;

      const Vector& GetGeometryOrigin() const
      {
        return geometry_.GetOrigin();
      }

      const std::string& GetSopInstanceUid() const
      {
        return sopInstanceUid_;
      }

      const Points& GetPoints() const
      {
        return points_;
      }

      double GetSliceThickness() const
      {
        return sliceThickness_;
      }

      bool Project(double& x1,
                   double& y1,
                   double& x2,
                   double& y2,
                   const CoordinateSystem3D& slice) const;
    };

    typedef std::list<Polygon>  Polygons;

    struct Structure
    {
      std::string   name_;
      std::string   interpretation_;
      Polygons      polygons_;
      uint8_t       red_;
      uint8_t       green_;
      uint8_t       blue_;
    };

    typedef std::vector<Structure>  Structures;

    Structures        structures_;
    ReferencedSlices  referencedSlices_;

    void Setup(const IDicomDataset& dataset);
    
    const Structure& GetStructure(size_t index) const;

    Structure& GetStructure(size_t index);
  
    bool ProjectStructure(
#if USE_BOOST_UNION_FOR_POLYGONS == 1
      std::vector< std::vector<Point2D> >& polygons,
#else
      std::vector< std::pair<Point2D, Point2D> >& segments,
#endif
      const Structure& structure,
      const CoordinateSystem3D& slice) const;

  public:
    explicit DicomStructureSet(const FullOrthancDataset& instance)
    {
      Setup(instance);
    }

#if ORTHANC_ENABLE_DCMTK == 1
    explicit DicomStructureSet(Orthanc::ParsedDicomFile& instance);
#endif

    size_t GetStructuresCount() const
    {
      return structures_.size();
    }

    Vector GetStructureCenter(size_t index) const;

    const std::string& GetStructureName(size_t index) const;

    const std::string& GetStructureInterpretation(size_t index) const;

    Color GetStructureColor(size_t index) const;

    // TODO - remove
    void GetStructureColor(uint8_t& red,
                           uint8_t& green,
                           uint8_t& blue,
                           size_t index) const;

    void GetReferencedInstances(std::set<std::string>& instances);

    void AddReferencedSlice(const std::string& sopInstanceUid,
                            const std::string& seriesInstanceUid,
                            const CoordinateSystem3D& geometry,
                            double thickness);

    void AddReferencedSlice(const Orthanc::DicomMap& dataset);

    void CheckReferencedSlices();

    Vector GetNormal() const;

#if USE_BOOST_UNION_FOR_POLYGONS == 1
    bool ProjectStructure(std::vector< std::vector<Point2D> >& polygons,
                          size_t index,
                          const CoordinateSystem3D& slice) const
    {
      return ProjectStructure(polygons, GetStructure(index), slice);
    }
#else
    bool ProjectStructure(std::vector< std::pair<Point2D, Point2D> >& segments,
                          size_t index,
                          const CoordinateSystem3D& slice) const
    {
      return ProjectStructure(segments, GetStructure(index), slice);
    }
#endif

    void ProjectOntoLayer(PolylineSceneLayer& layer,
                          const CoordinateSystem3D& plane,
                          size_t structureIndex,
                          const Color& color) const;

    void ProjectOntoLayer(PolylineSceneLayer& layer,
                          const CoordinateSystem3D& plane,
                          size_t structureIndex) const
    {
      ProjectOntoLayer(layer, plane, structureIndex, GetStructureColor(structureIndex));
    }
  };
}
