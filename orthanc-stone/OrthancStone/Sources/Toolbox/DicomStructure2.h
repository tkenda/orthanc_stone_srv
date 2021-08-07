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

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

#include "DicomStructurePolygon2.h"
#include "DicomStructureSetUtils.h"

namespace OrthancStone
{

  /*
    A structure has a color, a name, a set of slices..

    Each slice is a polygon.
  */
  struct DicomStructure2
  {
    DicomStructure2() :
      red_(0), green_(0), blue_(0), sliceThickness_(0), state_(Building) {}

    void AddPolygon(const DicomStructurePolygon2& polygon);

    /**
    Once all polygons have been added, this method will determine:
    - the slice orientation (through the normal vector)
    - the spacing between slices (slice thickness)

    it will also set up the info required to efficiently compute plane
    intersections later on.
    */
    void ComputeDependentProperties();

    /**
    Being given a plane that is PARALLEL to the set of polygon structures, this 
    returns a pointer to the polygon located at that position (if it is closer
    than thickness/2) or NULL if there is none.

    TODO: use sorted vector to improve

    DO NOT STORE THE RETURNED POINTER!
    */
    const DicomStructurePolygon2* GetPolygonClosestToSlice(const CoordinateSystem3D& plane) const;

    Vector GetNormal() const;

    Color GetColor() const
    {
      return Color(red_, green_, blue_);
    }

    bool IsValid() const
    {
      return state_ == Valid;
    }

    /**
    This method is used to project the 3D structure on a 2D plane.

    A structure is a stack of polygons, representing a volume.

    We need to compute the intersection between this volume and the supplied 
    cutting plane (the "slice"). This is more than a cutting plane: it is also
    a 2D-coordinate system (the plane has axes vectors)

    The cutting plane is always parallel to the plane defined by two of the
    world coordinate system axes.
    
    The result is a set of closed polygons.

    If the cut is parallel to the polygons, we pick the polygon closest to 
    the slice, project it on the slice and return it in slice coordinates.

    If the cut is perpendicular to the polygons, for each polygon, we compute 
    the intersection between the cutting plane and the polygon slab (imaginary 
    volume created by extruding the polygon above and below its plane by 
    thickness/2) :
    - each slab, intersected by the plane, gives a set of 0..* rectangles \
      (only one if the polygon is convex)
    - when doing this for the whole stack of slabs, we get a set of rectangles:
      To compute these rectangles, for each polygon, we compute the intersection
      between :
       - the line defined by the intersection of the polygon plane and the cutting
         plane
       - the polygon itself
      This yields 0 or 2*K points along the line C. These are turned into K
      rectangles by taking two consecutive points along the line and extruding 
      this segment by sliceThickness/2 in the orientation of the polygon normal,
      in both directions.

    Then, once this list of rectangles is computed, we need to group the 
    connected rectangles together. Connected, here, means sharing at least part
    of an edge --> union/find data structures and algorithm.
    */
    bool Project(std::vector< std::pair<Point2D, Point2D> >& polygons, const CoordinateSystem3D& plane) const;

    std::string                         interpretation_;
    std::string                         name_;
    uint8_t                             red_;
    uint8_t                             green_;
    uint8_t                             blue_;
  
    /** Internal */
    const std::vector<DicomStructurePolygon2>& GetPolygons() const
    {
      return polygons_;
    }

    /** Internal */
    double GetSliceThickness() const
    {
      return sliceThickness_;
    }

  private:
    enum State
    {
      Building,
      NormalComputed,
      Valid, // When normal components AND slice thickness are computed
      Invalid
    };

    void ComputeNormal();
    void ComputeSliceThickness();

    std::vector<DicomStructurePolygon2> polygons_;
    Vector3D                            normal_;
    double                              sliceThickness_;

    /*
      After creation (and while polygons are added), state is Building.
      After ComputeDependentProperties() is called, state can either be
      Valid or Invalid. In any case, the object becomes immutable.
    */
    State state_;
  };
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2

