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


#include "AffineTransform2D.h"

#include "ImageGeometry.h"

#include <Logging.h>
#include <OrthancException.h>

namespace OrthancStone
{
  AffineTransform2D::AffineTransform2D() :
    matrix_(LinearAlgebra::IdentityMatrix(3))
  {
  }

  
  AffineTransform2D::AffineTransform2D(const Matrix& m)
  {
    if (m.size1() != 3 ||
        m.size2() != 3)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageSize);
    }

    if (!LinearAlgebra::IsCloseToZero(m(2, 0)) ||
        !LinearAlgebra::IsCloseToZero(m(2, 1)) ||
        LinearAlgebra::IsCloseToZero(m(2, 2)))
    {
      LOG(ERROR) << "Cannot setup an AffineTransform2D with perspective effects";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    matrix_ = m / m(2, 2);
  }
    

  void AffineTransform2D::Apply(double& x /* inout */,
                                double& y /* inout */) const
  {
    Vector p;
    LinearAlgebra::AssignVector(p, x, y, 1);

    Vector q = LinearAlgebra::Product(matrix_, p);

    if (!LinearAlgebra::IsNear(q[2], 1.0))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
    else
    {
      x = q[0];
      y = q[1];
    }
  }


  void AffineTransform2D::Apply(Orthanc::ImageAccessor& target,
                                const Orthanc::ImageAccessor& source,
                                ImageInterpolation interpolation,
                                bool clear) const
  {
    assert(LinearAlgebra::IsNear(matrix_(2, 0), 0) &&
           LinearAlgebra::IsNear(matrix_(2, 1), 0) &&
           LinearAlgebra::IsNear(matrix_(2, 2), 1));

    ApplyAffineTransform(target, source,
                         matrix_(0, 0), matrix_(0, 1), matrix_(0, 2),
                         matrix_(1, 0), matrix_(1, 1), matrix_(1, 2),
                         interpolation, clear);
  }


  void AffineTransform2D::ConvertToOpenGLMatrix(float target[16],
                                                unsigned int canvasWidth,
                                                unsigned int canvasHeight) const
  {
    const AffineTransform2D t = AffineTransform2D::Combine(
      CreateOpenGLClipspace(canvasWidth, canvasHeight), *this);
    
    const Matrix source = t.GetHomogeneousMatrix();
  
    if (source.size1() != 3 ||
        source.size2() != 3)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    // "z" must be in the [-1,1] range, otherwise the texture does not show up
    float z = 0;

    // Embed the 3x3 affine transform of the 2D plane into a 4x4
    // matrix (3D) for OpenGL. The matrix must be transposed.

    target[0] = static_cast<float>(source(0, 0)); 
    target[1] = static_cast<float>(source(1, 0)); 
    target[2] = 0; 
    target[3] = static_cast<float>(source(2, 0));
    target[4] = static_cast<float>(source(0, 1)); 
    target[5] = static_cast<float>(source(1, 1));
    target[6] = 0;
    target[7] = static_cast<float>(source(2, 1));
    target[8] = 0; 
    target[9] = 0; 
    target[10] = -1; 
    target[11] = 0;
    target[12] = static_cast<float>(source(0, 2)); 
    target[13] = static_cast<float>(source(1, 2));
    target[14] = -z;
    target[15] = static_cast<float>(source(2, 2));
  }


  double AffineTransform2D::ComputeZoom() const
  {
    // Compute the length of the (0,0)-(1,1) diagonal (whose
    // length is sqrt(2)) instead of the (0,0)-(1,0) unit segment,
    // in order to cope with possible anisotropic zooming
        
    double x1 = 0;
    double y1 = 0;
    Apply(x1, y1);

    double x2 = 1;
    double y2 = 1;
    Apply(x2, y2);

    double dx = x2 - x1;
    double dy = y2 - y1;

    double zoom = sqrt(dx * dx + dy * dy) / sqrt(2.0);

    if (LinearAlgebra::IsCloseToZero(zoom))
    {
      return 1;  // Default value if transform is ill-conditioned 
    }
    else
    {
      return zoom;
    }
  }    


  AffineTransform2D AffineTransform2D::Invert(const AffineTransform2D& a)
  {
    AffineTransform2D t;
    LinearAlgebra::InvertMatrix(t.matrix_, a.matrix_);
    return t;
  }


  AffineTransform2D AffineTransform2D::Combine(const AffineTransform2D& a,
                                               const AffineTransform2D& b)
  {
    return AffineTransform2D(LinearAlgebra::Product(a.GetHomogeneousMatrix(),
                                                    b.GetHomogeneousMatrix()));
  }
  
      
  AffineTransform2D AffineTransform2D::Combine(const AffineTransform2D& a,
                                               const AffineTransform2D& b,
                                               const AffineTransform2D& c)
  {
    return AffineTransform2D(LinearAlgebra::Product(a.GetHomogeneousMatrix(),
                                                    b.GetHomogeneousMatrix(),
                                                    c.GetHomogeneousMatrix()));
  }
  
      
  AffineTransform2D AffineTransform2D::Combine(const AffineTransform2D& a,
                                               const AffineTransform2D& b,
                                               const AffineTransform2D& c,
                                               const AffineTransform2D& d)
  {
    return AffineTransform2D(LinearAlgebra::Product(a.GetHomogeneousMatrix(),
                                                    b.GetHomogeneousMatrix(),
                                                    c.GetHomogeneousMatrix(),
                                                    d.GetHomogeneousMatrix()));
  }
  
  AffineTransform2D AffineTransform2D::Combine(const AffineTransform2D& a,
                                               const AffineTransform2D& b,
                                               const AffineTransform2D& c,
                                               const AffineTransform2D& d,
                                               const AffineTransform2D& e)
  {
    return AffineTransform2D(LinearAlgebra::Product(a.GetHomogeneousMatrix(),
                                                    b.GetHomogeneousMatrix(),
                                                    c.GetHomogeneousMatrix(),
                                                    d.GetHomogeneousMatrix(),
                                                    e.GetHomogeneousMatrix()));
  }

  AffineTransform2D AffineTransform2D::CreateOffset(double dx,
                                                    double dy)
  {
    AffineTransform2D t;
    t.matrix_(0, 2) = dx;
    t.matrix_(1, 2) = dy;
      
    return t;
  }
  

  AffineTransform2D AffineTransform2D::CreateScaling(double sx,
                                                     double sy)
  {
    AffineTransform2D t;
    t.matrix_(0, 0) = sx;
    t.matrix_(1, 1) = sy;
      
    return t;
  }
  

  AffineTransform2D AffineTransform2D::CreateRotation(double angle)
  {
    double cosine = cos(angle);
    double sine = sin(angle);
      
    AffineTransform2D t;
    t.matrix_(0, 0) = cosine;
    t.matrix_(0, 1) = -sine;
    t.matrix_(1, 0) = sine;
    t.matrix_(1, 1) = cosine;

    return t;
  }

  AffineTransform2D AffineTransform2D::CreateRotation(double angle, // CW rotation
                                                      double cx,    // rotation center
                                                      double cy)    // rotation center
  {
    return Combine(
          CreateOffset(cx, cy),
          CreateRotation(angle),
          CreateOffset(-cx, -cy)
          );
  }

  AffineTransform2D AffineTransform2D::CreateOpenGLClipspace(unsigned int canvasWidth,
                                                             unsigned int canvasHeight)
  {
    AffineTransform2D t;
    t.matrix_(0, 0) = 2.0 / static_cast<double>(canvasWidth);
    t.matrix_(0, 2) = -1.0;
    t.matrix_(1, 1) = -2.0 / static_cast<double>(canvasHeight);
    t.matrix_(1, 2) = 1.0;
    
    return t;
  }

  
  AffineTransform2D AffineTransform2D::CreateFlip(bool flipX,
                                                  bool flipY,
                                                  unsigned int width,
                                                  unsigned int height)
  {
    AffineTransform2D t;
    t.matrix_(0, 0) = (flipX ? -1 : 1);
    t.matrix_(0, 2) = (flipX ? width : 0);
    t.matrix_(1, 1) = (flipY ? -1 : 1);
    t.matrix_(1, 2) = (flipY ? height : 0);

    return t;
  }
}
