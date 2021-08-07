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

#include "DicomStructure2.h"

#include "GeometryToolbox.h"
#include "DisjointDataSet.h"

#include <Logging.h>

namespace OrthancStone
{
  // see header
  //void DicomStructure2::ComputeNormal()
  //{
  //  try
  //  {
  //    if (polygons_.size() > 0)
  //    {

  //      // TODO: check all polygons are OK
  //      const DicomStructurePolygon2 polygon = polygons_[0];
  //      $$$$$$$$$$$$$$$$$
  //        state_ = NormalComputed;
  //    }
  //    else
  //    {
  //      // bogus! no polygons. Let's assign a "nothing here" value
  //      LinearAlgebra::AssignVector(normal_, 0, 0, 0);
  //      state_ = Invalid;
  //    }
  //  }
  //  catch (const Orthanc::OrthancException& e)
  //  {
  //    state_ = Invalid;
  //    if (e.HasDetails())
  //    {
  //      LOG(ERROR) << "OrthancException in ComputeNormal: " << e.What() << " Details: " << e.GetDetails();
  //    }
  //    else
  //    {
  //      LOG(ERROR) << "OrthancException in ComputeNormal: " << e.What();
  //    }
  //    throw;
  //  }
  //  catch (const std::exception& e)
  //  {
  //    state_ = Invalid;
  //    LOG(ERROR) << "std::exception in ComputeNormal: " << e.what();
  //    throw;
  //  }
  //  catch (...)
  //  {
  //    state_ = Invalid;
  //    LOG(ERROR) << "Unknown exception in ComputeNormal";
  //    throw;
  //  }
  //}

  void DicomStructure2::ComputeSliceThickness()
  {
    if (state_ != NormalComputed)
    {
      LOG(ERROR) << "DicomStructure2::ComputeSliceThickness - state must be NormalComputed";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    if (polygons_.size() < 2)
    {
      // cannot compute thickness if there are not at least 2 slabs (structures)
      sliceThickness_ = 1.0;
      state_ = Invalid;
    }
    else
    {
      // normal can be (1,0,0), (0,1,0) or (0,0,1), nothing else.
      // these can be compared with == (exact double representation)
      if (normal_[0] == 1)
      {
        // in a single polygon, all the points have the same X
        sliceThickness_ = fabs(polygons_[0].GetPoint(0)[0] - polygons_[1].GetPoint(0)[0]);
      }
      else if (normal_[1] == 1)
      {
        // in a single polygon, all the points have the same X
        sliceThickness_ = fabs(polygons_[0].GetPoint(0)[1] - polygons_[1].GetPoint(0)[1]);
      }
      else if (normal_[2] == 1)
      {
        // in a single polygon, all the points have the same X
        sliceThickness_ = fabs(polygons_[0].GetPoint(0)[2] - polygons_[1].GetPoint(0)[2]);
      }
      else
      {
        ORTHANC_ASSERT(false);
        state_ = Invalid;
      }
    }
    state_ = Valid;
  }

  void DicomStructure2::AddPolygon(const DicomStructurePolygon2& polygon)
  {
    if (state_ != Building)
    {
      LOG(ERROR) << "DicomStructure2::AddPolygon - can only add polygon while building";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    polygons_.push_back(polygon);
  }

  void DicomStructure2::ComputeDependentProperties()
  {
    if (state_ != Building)
    {
      LOG(ERROR) << "DicomStructure2::ComputeDependentProperties - can only be called once";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    for (size_t i = 0; i < polygons_.size(); ++i)
    {
      // "compute" the polygon normal
      polygons_[i].ComputeDependentProperties();
    }
    if (polygons_.size() > 0)
    {
      normal_ = polygons_[0].GetNormal();
      state_ = NormalComputed;
    }
    else
    {
      LinearAlgebra::AssignVector(normal_, 0, 0, 0);
      state_ = Invalid; // THIS MAY HAPPEN !!! (for instance for instance 72c773ac-5059f2c4-2e6a9120-4fd4bca1-45701661 :) )
    }
    if (polygons_.size() >= 2)
      ComputeSliceThickness(); // this will change state_ from NormalComputed to Valid
  }

  Vector DicomStructure2::GetNormal() const
  {
    if (state_ != Valid && state_ != Invalid)
    {
      LOG(ERROR) << "DicomStructure2::GetNormal() -- please call ComputeDependentProperties first.";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    if (state_ == Invalid)
    {
      LOG(ERROR) << "DicomStructure2::GetNormal() -- The Dicom structure is invalid. The normal is set to 0,0,0";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    return normal_;
  }

  const DicomStructurePolygon2* DicomStructure2::GetPolygonClosestToSlice(
    const CoordinateSystem3D& plane) const
  {
    ORTHANC_ASSERT(state_ == Valid);

    // we assume 0,0,1 for now
    ORTHANC_ASSERT(LinearAlgebra::IsNear(plane.GetNormal()[0], 0.0));
    ORTHANC_ASSERT(LinearAlgebra::IsNear(plane.GetNormal()[1], 0.0));

    for (size_t i = 0; i < polygons_.size(); ++i)
    {
      const DicomStructurePolygon2& polygon = polygons_[i];

      // "height" of cutting plane
      double cutZ = plane.GetOrigin()[2];

      if (LinearAlgebra::IsNear(
        cutZ, polygon.GetZ(),
        sliceThickness_ / 2.0 /* in mm */))
        return &polygon;
    }
    return NULL;
  }


    bool DicomStructure2::Project(std::vector< std::pair<Point2D, Point2D> > & segments, const CoordinateSystem3D & plane) const
    {
      segments.clear();

      Vector normal = GetNormal();

      size_t totalRectCount = 0;

      // dummy var
      bool isOpposite = false;

      // This is an axial projection
      if (GeometryToolbox::IsParallelOrOpposite(isOpposite, normal, plane.GetNormal()))
      {
        const DicomStructurePolygon2* polygon = GetPolygonClosestToSlice(plane);
        if (polygon)
        {
          polygon->ProjectOnParallelPlane(segments, plane);
        }
      }
      else
      {
        // let's compute the dot product of the plane normal and the polygons
        // normal.
        double dot = LinearAlgebra::DotProduct(plane.GetNormal(), normal);

        if (LinearAlgebra::IsNear(dot, 0))
        {
          // Coronal or sagittal projection

          // vector of vector of rectangles that will be merged in a single big contour:

          // each polygon slab cut by a perpendicular plane yields 0..* rectangles
          std::vector< RtStructRectanglesInSlab > rectanglesForEachSlab;

          for (size_t i = 0; i < polygons_.size(); ++i)
          {
            // book an entry for this slab
            rectanglesForEachSlab.push_back(RtStructRectanglesInSlab());

            // let's compute the intersection between the polygon and the plane
            // intersections are in plane coords
            std::vector<Point2D> intersections;

            polygons_[i].ProjectOnConstantPlane(intersections, plane);

            // for each pair of intersections, we add a rectangle.
            if ((intersections.size() % 2) != 0)
            {
              LOG(WARNING) << "Odd number of intersections between structure "
                << name_ << ", polygon # " << i
                << " and plane where X axis is parallel to polygon normal vector";
            }

            size_t numRects = intersections.size() / 2;
            
            // we keep count of the total number of rects for vector pre-allocations
            totalRectCount += numRects;
            
            for (size_t iRect = 0; iRect < numRects; ++iRect)
            {
              RtStructRectangleInSlab rectangle;
              ORTHANC_ASSERT(LinearAlgebra::IsNear(intersections[2 * iRect].y, intersections[2 * iRect + 1].y));
              ORTHANC_ASSERT((2 * iRect + 1) < intersections.size());
              double x1 = intersections[2 * iRect].x;
              double x2 = intersections[2 * iRect + 1].x;
              double y1 = intersections[2 * iRect].y - sliceThickness_ * 0.5;
              double y2 = intersections[2 * iRect].y + sliceThickness_ * 0.5;

              rectangle.xmin = std::min(x1, x2);
              rectangle.xmax = std::max(x1, x2);
              rectangle.ymin = std::min(y1, y2);
              rectangle.ymax = std::max(y1, y2);

              // TODO: keep them sorted!!!!

              rectanglesForEachSlab.back().push_back(rectangle);
            }
          }
          // now we need to merge all the slabs into a set of polygons (1 or more)
          ConvertListOfSlabsToSegments(segments, rectanglesForEachSlab, totalRectCount);
        }
        else
        {
          // plane is not perpendicular to the polygons
          // 180.0 / [Math]::Pi = 57.2957795130823
          double acDot = 57.2957795130823 * acos(dot);
          LOG(ERROR) << "DicomStructure2::Project -- cutting plane must be "
            << "perpendicular to the structures, but dot product is: "
            << dot << " and (180/pi)*acos(dot) = " << acDot;
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
      }
      return segments.size() != 0;
    }
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2

