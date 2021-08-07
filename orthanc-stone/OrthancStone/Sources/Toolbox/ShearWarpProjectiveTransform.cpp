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


#include "ShearWarpProjectiveTransform.h"

#include "ImageGeometry.h"
#include "Extent2D.h"
#include "FiniteProjectiveCamera.h"
#include "GeometryToolbox.h"

#include <Images/PixelTraits.h>
#include <Images/ImageProcessing.h>
#include <OrthancException.h>
#include <Logging.h>

#include <boost/numeric/ublas/matrix_proxy.hpp>
#include <boost/math/special_functions/round.hpp>
#include <cassert>


namespace OrthancStone
{
  static bool IsValidShear(const Matrix& M_shear)
  {
    return (LinearAlgebra::IsCloseToZero(M_shear(0, 1)) &&
            LinearAlgebra::IsCloseToZero(M_shear(1, 0)) &&
            LinearAlgebra::IsCloseToZero(M_shear(2, 0)) &&
            LinearAlgebra::IsCloseToZero(M_shear(2, 1)) &&
            LinearAlgebra::IsNear(1.0,   M_shear(2, 2)) &&
            LinearAlgebra::IsCloseToZero(M_shear(2, 3)) &&
            LinearAlgebra::IsCloseToZero(M_shear(3, 0)) &&
            LinearAlgebra::IsCloseToZero(M_shear(3, 1)) &&
            LinearAlgebra::IsNear(1.0,   M_shear(3, 3)));
  }


  static void ComputeShearParameters(double& scaling,
                                     double& offsetX,
                                     double& offsetY,
                                     const Matrix& shear,
                                     double z)
  {
    // Check out: ../../Resources/Computations/ComputeShearParameters.py
    
    if (!LinearAlgebra::IsShearMatrix(shear))
    {
      LOG(ERROR) << "Not a valid shear matrix";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
    
    scaling = 1.0 / (shear(3,2) * z + 1.0);
    offsetX = shear(0,2) * z * scaling;
    offsetY = shear(1,2) * z * scaling;
  }  


  ShearWarpProjectiveTransform::
  ShearWarpProjectiveTransform(const Matrix& M_view,
                               //const Matrix& P,           // Permutation applied to the volume
                               unsigned int volumeWidth,
                               unsigned int volumeHeight,
                               unsigned int volumeDepth,
                               double pixelSpacingX,
                               double pixelSpacingY,
                               unsigned int imageWidth,
                               unsigned int imageHeight)
  {
    eye_o.resize(4);

    {
      // Find back the camera center given the "M_view" matrix
      const double m11 = M_view(0, 0);
      const double m12 = M_view(0, 1);
      const double m13 = M_view(0, 2);
      const double m14 = M_view(0, 3);
      const double m21 = M_view(1, 0);
      const double m22 = M_view(1, 1);
      const double m23 = M_view(1, 2);
      const double m24 = M_view(1, 3);
      const double m41 = M_view(3, 0);
      const double m42 = M_view(3, 1);
      const double m43 = M_view(3, 2);
      const double m44 = M_view(3, 3);

      // Equations (A.8) to (A.11) on page 203. Also check out
      // "Finding the camera center" in "Multiple View Geometry in
      // Computer Vision - 2nd edition", page 163.
      const double vx[9] = { m12, m13, m14, m22, m23, m24, m42, m43, m44 };
      const double vy[9] = { m11, m13, m14, m21, m23, m24, m41, m43, m44 };
      const double vz[9] = { m11, m12, m14, m21, m22, m24, m41, m42, m44 };
      const double vw[9] = { m11, m12, m13, m21, m22, m23, m41, m42, m43 };

      Matrix m;

      LinearAlgebra::FillMatrix(m, 3, 3, vx);
      eye_o[0] = -LinearAlgebra::ComputeDeterminant(m);

      LinearAlgebra::FillMatrix(m, 3, 3, vy);
      eye_o[1] = LinearAlgebra::ComputeDeterminant(m);

      LinearAlgebra::FillMatrix(m, 3, 3, vz);
      eye_o[2] = -LinearAlgebra::ComputeDeterminant(m);

      LinearAlgebra::FillMatrix(m, 3, 3, vw);
      eye_o[3] = LinearAlgebra::ComputeDeterminant(m);

      if (LinearAlgebra::IsCloseToZero(eye_o[3]))
      {
        LOG(ERROR) << "The shear-warp projective transform is not applicable to affine cameras";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
    }

#if 0
    // Assume "T_shift = I" (the eye does not lie on plane k = 0)
    const Matrix T_shift = LinearAlgebra::IdentityMatrix(4);
    
    // Equation (A.13) on page 204, given that the inverse of a
    // permutation matrix is its transpose (TODO CHECK). If no T_shift
    // or permutation P is applied, M'_view == M_view
    const Matrix MM_view = LinearAlgebra::Product(
      M_view,
      LinearAlgebra::Transpose(P),
      LinearAlgebra::InvertScalingTranslationMatrix(T_shift));
#else
    // This is a shortcut, as we take "T_shift = I" and "P = I"
    const Matrix MM_view = M_view;
#endif

    // Equation (A.14) on page 207
    Matrix MM_shear = LinearAlgebra::IdentityMatrix(4);
    MM_shear(0, 2) = -eye_o[0] / eye_o[2];
    MM_shear(1, 2) = -eye_o[1] / eye_o[2];
    MM_shear(3, 2) = -eye_o[3] / eye_o[2];


    // Compute the extent of the intermediate image
    Extent2D extent;
    double maxScaling = 1;

    {
      // Compute the shearing factors of the two extreme planes of the
      // volume (z=0 and z=volumeDepth)
      double scaling, offsetX, offsetY;
      ComputeShearParameters(scaling, offsetX, offsetY, MM_shear, 0);

      if (scaling > 0)
      {
        extent.AddPoint(offsetX, offsetY);
        extent.AddPoint(offsetX + static_cast<double>(volumeWidth) * scaling,
                        offsetY + static_cast<double>(volumeHeight) * scaling);

        if (scaling > maxScaling)
        {
          maxScaling = scaling;
        }
      }

      ComputeShearParameters(scaling, offsetX, offsetY, MM_shear, volumeDepth);

      if (scaling > 0)
      {
        extent.AddPoint(offsetX, offsetY);
        extent.AddPoint(offsetX + static_cast<double>(volumeWidth) * scaling,
                        offsetY + static_cast<double>(volumeHeight) * scaling);

        if (scaling > maxScaling)
        {
          maxScaling = scaling;
        }
      }
    }
      
    if (LinearAlgebra::IsCloseToZero(extent.GetWidth()) ||
        LinearAlgebra::IsCloseToZero(extent.GetHeight()))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    intermediateWidth_ = 
      static_cast<unsigned int>(std::ceil(extent.GetWidth() / maxScaling));
    intermediateHeight_ = 
      static_cast<unsigned int>(std::ceil(extent.GetHeight() / maxScaling));

    // This is the product "T * S" in Equation (A.16) on page 209
    Matrix TS = LinearAlgebra::Product(
      GeometryToolbox::CreateTranslationMatrix(
        static_cast<double>(intermediateWidth_) / 2.0,
        static_cast<double>(intermediateHeight_) / 2.0, 0),
      GeometryToolbox::CreateScalingMatrix(
        1.0 / maxScaling, 1.0 / maxScaling, 1),
      GeometryToolbox::CreateTranslationMatrix(
        -extent.GetCenterX(), -extent.GetCenterY(), 0));
    
    // This is Equation (A.16) on page 209. WARNING: There is an
    // error in Lacroute's thesis: "inv(MM_shear)" is used instead
    // of "MM_shear".
    M_shear = LinearAlgebra::Product(TS, MM_shear);

    if (!IsValidShear(M_shear))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    // This is Equation (A.17) on page 209
    Matrix tmp;
    LinearAlgebra::InvertMatrix(tmp, M_shear);
    M_warp = LinearAlgebra::Product(MM_view, tmp);

    // Intrinsic parameters of the camera
    k_ = LinearAlgebra::ZeroMatrix(3, 4);
    k_(0, 0) = 1.0 / pixelSpacingX;
    k_(0, 3) = static_cast<double>(imageWidth) / 2.0;
    k_(1, 1) = 1.0 / pixelSpacingY;
    k_(1, 3) = static_cast<double>(imageHeight) / 2.0;
    k_(2, 3) = 1.0;
  }


  FiniteProjectiveCamera *ShearWarpProjectiveTransform::CreateCamera() const
  {
    Matrix p = LinearAlgebra::Product(k_, M_warp, M_shear);
    return new FiniteProjectiveCamera(p);
  }
  

  void ShearWarpProjectiveTransform::ComputeShearOnSlice(double& a11,
                                                         double& b1,
                                                         double& a22,
                                                         double& b2,
                                                         double& shearedZ,
                                                         const double sourceZ)
  {
    // Check out: ../../Resources/Computations/ComputeShearOnSlice.py
    assert(IsValidShear(M_shear));

    const double s11 = M_shear(0, 0);
    const double s13 = M_shear(0, 2);
    const double s14 = M_shear(0, 3);
    const double s22 = M_shear(1, 1);
    const double s23 = M_shear(1, 2);
    const double s24 = M_shear(1, 3);
    const double s43 = M_shear(3, 2);

    double scaling = 1.0 / (s43 * sourceZ + 1.0);
    shearedZ = sourceZ * scaling;

    a11 = s11 * scaling;
    a22 = s22 * scaling;

    b1 = (s13 * sourceZ + s14) * scaling;
    b2 = (s23 * sourceZ + s24) * scaling;
  }


  Matrix ShearWarpProjectiveTransform::CalibrateView(const Vector& camera,
                                                     const Vector& principalPoint,
                                                     double angle)
  {
    if (camera.size() != 3 ||
        principalPoint.size() != 3)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    const double sid = boost::numeric::ublas::norm_2(camera - principalPoint);
      
    Matrix a;
    GeometryToolbox::AlignVectorsWithRotation(a, camera - principalPoint,
                                              LinearAlgebra::CreateVector(0, 0, -1));

    Matrix r = LinearAlgebra::Product(GeometryToolbox::CreateRotationMatrixAlongZ(angle), a);

    a = LinearAlgebra::ZeroMatrix(4, 4);
    boost::numeric::ublas::subrange(a, 0, 3, 0, 3) = r;

    const Vector v = LinearAlgebra::Product(r, -camera);
    a(0, 3) = v[0];
    a(1, 3) = v[1];
    a(2, 3) = v[2];
    a(3, 3) = 1;

    Matrix perspective = LinearAlgebra::ZeroMatrix(4, 4);
    // https://stackoverflow.com/questions/5267866/calculation-of-a-perspective-transformation-matrix
    perspective(0, 0) = sid;
    perspective(1, 1) = sid;
    perspective(2, 2) = sid;
    perspective(3, 2) = 1;

    Matrix M_view = LinearAlgebra::Product(perspective, a);
    assert(M_view.size1() == 4 &&
           M_view.size2() == 4);

    {
      // Sanity checks
      Vector p1 = LinearAlgebra::CreateVector(camera[0], camera[1], camera[2], 1.0);
      Vector p2 = LinearAlgebra::CreateVector(principalPoint[0], principalPoint[1], principalPoint[2], 1.0);
      
      Vector v1 = LinearAlgebra::Product(M_view, p1);
      Vector v2 = LinearAlgebra::Product(M_view, p2);

      if (!LinearAlgebra::IsCloseToZero(v1[3]) ||  // Must be mapped to singularity (w=0)
          LinearAlgebra::IsCloseToZero(v2[3]))
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }

      // The principal point must be mapped to (0,0,z,1)
      v2 /= v2[3];
      if (!LinearAlgebra::IsCloseToZero(v2[0]) ||
          !LinearAlgebra::IsCloseToZero(v2[1]))
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }      
    }

    return M_view;
  }


  template <Orthanc::PixelFormat SourceFormat,
            Orthanc::PixelFormat TargetFormat,
            bool MIP>
  static void ApplyAxialInternal(Orthanc::ImageAccessor& target,
                                 float& maxValue,
                                 const Matrix& M_view,
                                 const ImageBuffer3D& source,
                                 const VolumeImageGeometry& geometry,
                                 double pixelSpacing,
                                 unsigned int countSlices,
                                 ImageInterpolation shearInterpolation,
                                 ImageInterpolation warpInterpolation)
  {
    typedef Orthanc::PixelTraits<SourceFormat> SourceTraits;
    typedef Orthanc::PixelTraits<TargetFormat> TargetTraits;

    /**
     * Step 1: Precompute some information.
     **/
       
    if (target.GetFormat() != TargetFormat ||
        source.GetFormat() != SourceFormat ||
        !std::numeric_limits<float>::is_iec559 ||
        sizeof(float) != 4)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    if (countSlices > source.GetDepth())
    {
      countSlices = source.GetDepth();
    }

    if (countSlices == 0)
    {
      maxValue = 0;
      Orthanc::ImageProcessing::Set(target, 0);
      return;
    }
    
    LOG(INFO) << "Number of rendered slices: " << countSlices;


    /**
     * Step 2: Extract the shear-warp transform corresponding to
     * M_view.
     **/
    
    // Compute the "world" matrix that maps the source volume to the
    // (0,0,0)->(1,1,1) unit cube
    Vector origin = geometry.GetCoordinates(0, 0, 0);
    Vector ps = geometry.GetVoxelDimensions(VolumeProjection_Axial);
    Matrix world = LinearAlgebra::Product(
      GeometryToolbox::CreateScalingMatrix(1.0 / ps[0], 1.0 / ps[1], 1.0 / ps[2]),
      GeometryToolbox::CreateTranslationMatrix(-origin[0], -origin[1], -origin[2]));

    Matrix worldInv;
    LinearAlgebra::InvertMatrix(worldInv, world);

    ShearWarpProjectiveTransform shearWarp(LinearAlgebra::Product(M_view, worldInv),
                                           /*LinearAlgebra::IdentityMatrix(4),*/
                                           source.GetWidth(),
                                           source.GetHeight(),
                                           source.GetDepth(),
                                           pixelSpacing, pixelSpacing,
                                           target.GetWidth(), target.GetHeight());
    
    const unsigned int intermediateWidth = shearWarp.GetIntermediateWidth();
    const unsigned int intermediateHeight = shearWarp.GetIntermediateHeight();


    /**
     * Step 3: Apply the "shear" part of the transform to form the
     * intermediate image. The sheared images are accumulated into the
     * Float32 image "accumulator". The number of samples available
     * for each pixel is stored in the "counter" image.
     **/

    std::unique_ptr<Orthanc::ImageAccessor> accumulator, counter, intermediate;

    accumulator.reset(new Orthanc::Image(Orthanc::PixelFormat_Float32,
                                         intermediateWidth, intermediateHeight, false));
    counter.reset(new Orthanc::Image(Orthanc::PixelFormat_Grayscale16,
                                     intermediateWidth, intermediateHeight, false)); 
    intermediate.reset(new Orthanc::Image(SourceFormat, intermediateWidth, intermediateHeight, false));

    Orthanc::ImageProcessing::Set(*accumulator, 0);
    Orthanc::ImageProcessing::Set(*counter, 0);

    // Loop around the slices of the volume
    for (unsigned int i = 0; i <= countSlices; i++)
    {
      // (3.a) Compute the shear for this specific slice
      unsigned int z = static_cast<unsigned int>(
        boost::math::iround(static_cast<double>(i) /
                            static_cast<double>(countSlices) *
                            static_cast<double>(source.GetDepth() - 1)));
      
      double a11, b1, a22, b2, vz;
      shearWarp.ComputeShearOnSlice(a11, b1, a22, b2, vz, static_cast<double>(z) + 0.5);


      {
        // (3.b) Detect the "useful" portion of the intermediate image
        // for this slice (i.e. the bounding box where the source
        // slice is mapped to by the shear), so as to update "counter"
        Matrix a = LinearAlgebra::ZeroMatrix(3, 3);
        a(0,0) = a11;
        a(0,2) = b1;
        a(1,1) = a22;
        a(1,2) = b2;
        a(2,2) = 1;

        unsigned int x1, y1, x2, y2;
        if (GetProjectiveTransformExtent(x1, y1, x2, y2, a,
                                         source.GetWidth(), source.GetHeight(),
                                         intermediateWidth, intermediateHeight))
        {
          for (unsigned int y = y1; y <= y2; y++)
          {
            uint16_t* p = reinterpret_cast<uint16_t*>(counter->GetRow(y)) + x1;
            for (unsigned int x = x1; x <= x2; x++, p++)
            {
              if (MIP)
              {
                // TODO - In the case of MIP, "counter" could be
                // reduced to "PixelFormat_Grayscale8" to reduce
                // memory usage
                *p = 1;
              }
              else
              {
                *p += 1;
              }
            }
          }
        }
      }


      {
        // (3.c) Shear the source slice into a temporary image
        ImageBuffer3D::SliceReader reader(source, VolumeProjection_Axial, z);      
        ApplyAffineTransform(*intermediate, reader.GetAccessor(),
                             a11, 0,   b1,
                             0,   a22, b2,
                             shearInterpolation, true);
      }
      

      for (unsigned int y = 0; y < intermediateHeight; y++)
      {
        // (3.d) Accumulate the pixels of the sheared image into "accumulator"
        const typename SourceTraits::PixelType* p =
          reinterpret_cast<const typename SourceTraits::PixelType*>(intermediate->GetConstRow(y));

        float* q = reinterpret_cast<float*>(accumulator->GetRow(y));
      
        for (unsigned int x = 0; x < intermediateWidth; x++)
        {
          float pixel = SourceTraits::PixelToFloat(*p);
          
          if (MIP)
          {
            // Get maximum for MIP
            if (*q < pixel)
            {
              *q = pixel;
            }
          }
          else
          {
            *q += pixel;
          }
          
          p++;
          q++;
        }
      }
    }


    /**
     * Step 4: The intermediate image (that will be transformed by the
     * "warp") is now available as an accumulator image together with
     * a counter image. "Flatten" these two images into one.
     **/

    intermediate.reset(new Orthanc::Image
                       (TargetFormat, intermediateWidth, intermediateHeight, false));

    maxValue = 0;
    
    for (unsigned int y = 0; y < intermediateHeight; y++)
    {
      const float *qacc = reinterpret_cast<const float*>(accumulator->GetConstRow(y));
      const uint16_t *qcount = reinterpret_cast<const uint16_t*>(counter->GetConstRow(y));
      typename TargetTraits::PixelType *p =
        reinterpret_cast<typename TargetTraits::PixelType*>(intermediate->GetRow(y));

      for (unsigned int x = 0; x < intermediateWidth; x++)
      {
        if (*qcount == 0)
        {
          TargetTraits::SetZero(*p);
        }
        else
        {
          *p = static_cast<typename TargetTraits::PixelType>
            (*qacc / static_cast<float>(*qcount));

          if (*p > maxValue)
          {
            maxValue = *p;
          }
        }
        
        p++;
        qacc++;
        qcount++;
      }
    }

    // We don't need the accumulator images anymore
    accumulator.reset(NULL);
    counter.reset(NULL);

    
    /**
     * Step 6: Apply the "warp" part of the transform to map the
     * intermediate image to the final image.
     **/

    Matrix warp;

    {
      // (5.a) Compute the "warp" matrix by removing the 3rd row and
      // 3rd column from the GetWarp() matrix
      // Check out: ../../Resources/Computations/ComputeWarp.py
    
      Matrix fullWarp = LinearAlgebra::Product
        (shearWarp.GetIntrinsicParameters(), shearWarp.GetWarp());

      const double v[] = {
        fullWarp(0,0), fullWarp(0,1), fullWarp(0,3), 
        fullWarp(1,0), fullWarp(1,1), fullWarp(1,3), 
        fullWarp(2,0), fullWarp(2,1), fullWarp(2,3)
      };

      LinearAlgebra::FillMatrix(warp, 3, 3, v);
    }

    // (5.b) Apply the projective transform to the image
    ApplyProjectiveTransform(target, *intermediate, warp, warpInterpolation, true);
  }


  template <Orthanc::PixelFormat SourceFormat,
            Orthanc::PixelFormat TargetFormat>
  static void ApplyAxialInternal2(Orthanc::ImageAccessor& target,
                                  float& maxValue,
                                  const Matrix& M_view,
                                  const ImageBuffer3D& source,
                                  const VolumeImageGeometry& geometry,
                                  bool mip,
                                  double pixelSpacing,
                                  unsigned int countSlices,
                                  ImageInterpolation shearInterpolation,
                                  ImageInterpolation warpInterpolation)
  {
    if (mip)
    {
      ApplyAxialInternal<SourceFormat, TargetFormat, true>
        (target, maxValue, M_view, source, geometry, pixelSpacing,
         countSlices, shearInterpolation, warpInterpolation);
    }
    else
    {
      ApplyAxialInternal<SourceFormat, TargetFormat, false>
        (target, maxValue, M_view, source, geometry, pixelSpacing,
         countSlices, shearInterpolation, warpInterpolation);
    } 
  }
  

  Orthanc::ImageAccessor*
  ShearWarpProjectiveTransform::ApplyAxial(float& maxValue,
                                           const Matrix& M_view,
                                           const ImageBuffer3D& source,
                                           const VolumeImageGeometry& geometry,
                                           Orthanc::PixelFormat targetFormat,
                                           unsigned int targetWidth,
                                           unsigned int targetHeight,
                                           bool mip,
                                           double pixelSpacing,
                                           unsigned int countSlices,
                                           ImageInterpolation shearInterpolation,
                                           ImageInterpolation warpInterpolation)
  {
    std::unique_ptr<Orthanc::ImageAccessor> target
      (new Orthanc::Image(targetFormat, targetWidth, targetHeight, false));
    
    if (source.GetFormat() == Orthanc::PixelFormat_Grayscale16 &&
        targetFormat == Orthanc::PixelFormat_Grayscale16)
    {
      ApplyAxialInternal2<Orthanc::PixelFormat_Grayscale16,
                          Orthanc::PixelFormat_Grayscale16>
        (*target, maxValue, M_view, source, geometry, mip, pixelSpacing,
         countSlices, shearInterpolation, warpInterpolation);
    }
    else if (source.GetFormat() == Orthanc::PixelFormat_SignedGrayscale16 &&
             targetFormat == Orthanc::PixelFormat_SignedGrayscale16)
    {
      ApplyAxialInternal2<Orthanc::PixelFormat_SignedGrayscale16,
                          Orthanc::PixelFormat_SignedGrayscale16>
        (*target, maxValue, M_view, source, geometry, mip, pixelSpacing,
         countSlices, shearInterpolation, warpInterpolation);
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }

    return target.release();
  }
}
