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


#include "CoordinateSystem3D.h"

#include "LinearAlgebra.h"
#include "GeometryToolbox.h"

#include <Logging.h>
#include <Toolbox.h>
#include <OrthancException.h>

namespace OrthancStone
{
  void CoordinateSystem3D::CheckAndComputeNormal()
  {
    /**
     * DICOM expects normal vectors to define the axes: "The row and
     * column direction cosine vectors shall be normal, i.e., the dot
     * product of each direction cosine vector with itself shall be
     * unity."
     * http://dicom.nema.org/medical/dicom/current/output/chtml/part03/sect_C.7.6.2.html
     **/
    if (!LinearAlgebra::IsNear(boost::numeric::ublas::norm_2(axisX_), 1.0) ||
        !LinearAlgebra::IsNear(boost::numeric::ublas::norm_2(axisY_), 1.0))
    {
      LOG(WARNING) << "Invalid 3D geometry: Axes are not normal vectors";
      SetupCanonical();
    }
    
    /**
     * The vectors within "Image Orientation Patient" must be
     * orthogonal, according to the DICOM specification: "The row and
     * column direction cosine vectors shall be orthogonal, i.e.,
     * their dot product shall be zero."
     * http://dicom.nema.org/medical/dicom/current/output/chtml/part03/sect_C.7.6.2.html
     *
     * The "0.00001" threshold is needed for KNIX (on this sample
     * image, the inner product equals "0.000003", which is rejected
     * by "LinearAlgebra::IsCloseToZero()").
     **/
    else if (!LinearAlgebra::IsNear(0, boost::numeric::ublas::inner_prod(axisX_, axisY_), 0.00001))
    {
      LOG(WARNING) << "Invalid 3D geometry: Image orientation patient is not orthogonal";
      SetupCanonical();
    }
    else
    {
      LinearAlgebra::CrossProduct(normal_, axisX_, axisY_);

      d_ = -(normal_[0] * origin_[0] + normal_[1] * origin_[1] + normal_[2] * origin_[2]);

      // Just a sanity check, it should be useless by construction (*)
      assert(LinearAlgebra::IsNear(boost::numeric::ublas::norm_2(normal_), 1.0));
    }
  }


  void CoordinateSystem3D::SetupCanonical()
  {
    valid_ = false;

    LinearAlgebra::AssignVector(origin_, 0, 0, 0);
    LinearAlgebra::AssignVector(axisX_, 1, 0, 0);
    LinearAlgebra::AssignVector(axisY_, 0, 1, 0);
    LinearAlgebra::AssignVector(normal_, 0, 0, 1);
    d_ = 0;
  }


  CoordinateSystem3D::CoordinateSystem3D(const Vector& origin,
                                         const Vector& axisX,
                                         const Vector& axisY) :
    valid_(true),
    origin_(origin),
    axisX_(axisX),
    axisY_(axisY)
  {
    CheckAndComputeNormal();
  }


  void CoordinateSystem3D::Setup(const std::string& imagePositionPatient,
                                 const std::string& imageOrientationPatient)
  {
    valid_ = true;
    
    std::string tmpPosition = Orthanc::Toolbox::StripSpaces(imagePositionPatient);
    std::string tmpOrientation = Orthanc::Toolbox::StripSpaces(imageOrientationPatient);

    Vector orientation;
    if (!LinearAlgebra::ParseVector(origin_, tmpPosition) ||
        !LinearAlgebra::ParseVector(orientation, tmpOrientation) ||
        origin_.size() != 3 ||
        orientation.size() != 6)
    {
      LOG(WARNING) << "Bad 3D geometry: image position/orientation patient: \""
                   << tmpPosition << "\" / \"" << tmpOrientation << "\"";
      SetupCanonical();
    }
    else
    {
      axisX_.resize(3);
      axisX_[0] = orientation[0];
      axisX_[1] = orientation[1];
      axisX_[2] = orientation[2];

      axisY_.resize(3);
      axisY_[0] = orientation[3];
      axisY_[1] = orientation[4];
      axisY_[2] = orientation[5];

      CheckAndComputeNormal();
    }
  }   


  CoordinateSystem3D::CoordinateSystem3D(const IDicomDataset& dicom)
  {
    std::string a, b;

    if (dicom.GetStringValue(a, Orthanc::DicomPath(Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT)) &&
        dicom.GetStringValue(b, Orthanc::DicomPath(Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT)))
    {
      Setup(a, b);
    }
    else
    {
      SetupCanonical();
    }
  }


  CoordinateSystem3D::CoordinateSystem3D(const Orthanc::DicomMap& dicom)
  {
    std::string a, b;

    if (dicom.LookupStringValue(a, Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, false) &&
        dicom.LookupStringValue(b, Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, false))
    {
      Setup(a, b);
    }
    else
    {
      SetupCanonical();
    }    
  }


  void CoordinateSystem3D::SetOrigin(const Vector& origin)
  {
    if (origin.size() != 3)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      origin_ = origin;
    }
  }


  Vector CoordinateSystem3D::MapSliceToWorldCoordinates(double x,
                                                        double y) const
  {
    return origin_ + x * axisX_ + y * axisY_;
  }


  double CoordinateSystem3D::ProjectAlongNormal(const Vector& point) const
  {
    return boost::numeric::ublas::inner_prod(point, normal_);
  }

  void CoordinateSystem3D::ProjectPoint2(double& offsetX, double& offsetY, const Vector& point) const
  {
    // Project the point onto the slice
    double projectionX,projectionY,projectionZ;
    GeometryToolbox::ProjectPointOntoPlane2(projectionX, projectionY, projectionZ, point, normal_, origin_);

    // As the axes are orthonormal vectors thanks to
    // CheckAndComputeNormal(), the following dot products give the
    // offset of the origin of the slice wrt. the origin of the
    // reference plane https://en.wikipedia.org/wiki/Vector_projection
    offsetX = axisX_[0] * (projectionX - origin_[0]) + axisX_[1] * (projectionY - origin_[1]) + axisX_[2] * (projectionZ - origin_[2]);
    offsetY = axisY_[0] * (projectionX - origin_[0]) + axisY_[1] * (projectionY - origin_[1]) + axisY_[2] * (projectionZ - origin_[2]);
  }

  void CoordinateSystem3D::ProjectPoint(double& offsetX,
                                        double& offsetY,
                                        const Vector& point) const
  {
    // Project the point onto the slice
    Vector projection;
    GeometryToolbox::ProjectPointOntoPlane(projection, point, normal_, origin_);

    // As the axes are orthonormal vectors thanks to
    // CheckAndComputeNormal(), the following dot products give the
    // offset of the origin of the slice wrt. the origin of the
    // reference plane https://en.wikipedia.org/wiki/Vector_projection
    offsetX = boost::numeric::ublas::inner_prod(axisX_, projection - origin_);
    offsetY = boost::numeric::ublas::inner_prod(axisY_, projection - origin_);
  }

  bool CoordinateSystem3D::IntersectSegment(Vector& p,
                                            const Vector& edgeFrom,
                                            const Vector& edgeTo) const
  {
    return GeometryToolbox::IntersectPlaneAndSegment(p, normal_, d_, edgeFrom, edgeTo);
  }


  bool CoordinateSystem3D::IntersectLine(Vector& p,
                                         const Vector& origin,
                                         const Vector& direction) const
  {
    return GeometryToolbox::IntersectPlaneAndLine(p, normal_, d_, origin, direction);
  }


  double CoordinateSystem3D::ComputeDistance(const Vector& p) const
  {
    /**
     * "normal_" is an unit vector (*) => sqrt(a_1^2+a_2^2+a_3^2) = 1,
     * and the denominator equals 1 by construction.
     * https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_plane#Closest_point_and_distance_for_a_hyperplane_and_arbitrary_point
     **/

    return std::abs(boost::numeric::ublas::inner_prod(p, normal_) + d_);
  }


  bool CoordinateSystem3D::ComputeDistance(double& distance,
                                           const CoordinateSystem3D& a,
                                           const CoordinateSystem3D& b)
  {
    bool opposite = false;   // Ignored

    if (GeometryToolbox::IsParallelOrOpposite(opposite, a.GetNormal(), b.GetNormal()))
    {
      distance = std::abs(a.ProjectAlongNormal(a.GetOrigin()) -
                          a.ProjectAlongNormal(b.GetOrigin()));
      return true;
    }
    else
    {
      return false;
    }
  }

  std::ostream& operator<< (std::ostream& s, const CoordinateSystem3D& that)
  {
    s << "origin: " << that.origin_ << " normal: " << that.normal_ 
      << " axisX: " << that.axisX_ << " axisY: " << that.axisY_ 
      << " D: " << that.d_;
    return s;
  }


  CoordinateSystem3D CoordinateSystem3D::NormalizeCuttingPlane(const CoordinateSystem3D& plane)
  {
    double ox, oy;
    plane.ProjectPoint(ox, oy, LinearAlgebra::CreateVector(0, 0, 0));

    CoordinateSystem3D normalized(plane);
    normalized.SetOrigin(plane.MapSliceToWorldCoordinates(ox, oy));
    return normalized;
  }


  CoordinateSystem3D CoordinateSystem3D::CreateFromPlaneGeneralForm(double a,
                                                                    double b,
                                                                    double c,
                                                                    double d)
  {
    /**
     * "a*x + b*y + c*z + d = 0" => The un-normalized normal is vector
     * (a,b,c). 
     **/

    Vector normal;
    LinearAlgebra::AssignVector(normal, a, b, c);

    
    /**
     * Choose the origin of plane, as the point that is closest to the
     * origin of the axes (0,0,0). 
     * https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_plane#Restatement_using_linear_algebra
     **/

    double squaredNorm = a * a + b * b + c * c;
    if (LinearAlgebra::IsCloseToZero(squaredNorm))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadGeometry, "Singular matrix");
    }        
    
    Vector origin = -d * normal / squaredNorm;


    /**
     * Select the X axis by computing a vector that is perpendicular
     * to the normal.
     *
     * "Exactly 1 and only 1 of the bools get set; b0/b1/b2 gets set
     * if dimension "i" has magnitude strictly less than all
     * subsequent dimensions and not greater than all previous
     * dimensions. We then have a unit vector with a single non-zero
     * dimension that corresponds to a dimension of minimum magnitude
     * in "normal". The cross product of this with "normal" is
     * orthogonal to "normal" by definition of cross product. Consider
     * now that the cross product is numerically unstable only when
     * the two vectors are very closely aligned. Consider that our
     * unit vector is large in only a single dimension and that that
     * dimension corresponds to the dimension where "normal" was
     * small. It's thus guaranteed to be loosely orthogonal to
     * "normal" before taking the cross product, with least
     * orthogonality in the case where all dimensions of "normal" are
     * equal. In this least-orthogonal case, we're still quite
     * orthogonal given that our unit vector has all but one dimension
     * 0 whereas "normal" has all equal. We thus avoid the unstable
     * case of taking the cross product of two nearly-aligned
     * vectors."  https://stackoverflow.com/a/43454629/881731
     **/

    bool b0 = (normal[0] <  normal[1]) && (normal[0] <  normal[2]);
    bool b1 = (normal[1] <= normal[0]) && (normal[1] <  normal[2]);
    bool b2 = (normal[2] <= normal[0]) && (normal[2] <= normal[1]);
    Vector swap = LinearAlgebra::CreateVector(b0 ? 1 : 0, b1 ? 1 : 0, b2 ? 1 : 0);
    
    Vector axisX;
    LinearAlgebra::CrossProduct(axisX, normal, swap);
    LinearAlgebra::NormalizeVector(axisX);

    
    /**
     * The Y axis follows as the cross-product of the normal and the X
     * axis.
     **/
    
    Vector axisY;
    LinearAlgebra::CrossProduct(axisY, axisX, normal);
    LinearAlgebra::NormalizeVector(axisY);

    return CoordinateSystem3D(origin, axisX, axisY);
  }


  CoordinateSystem3D CoordinateSystem3D::CreateFromThreePoints(const Vector& a,
                                                               const Vector& b,
                                                               const Vector& c)
  {
    Vector axisX = b - a;
    LinearAlgebra::NormalizeVector(axisX);
    
    Vector normal;
    LinearAlgebra::CrossProduct(normal, axisX, c - a);

    Vector axisY;
    LinearAlgebra::CrossProduct(axisY, axisX, normal);
    LinearAlgebra::NormalizeVector(axisY);

    return CoordinateSystem3D(a, axisX, axisY);
  }
}
