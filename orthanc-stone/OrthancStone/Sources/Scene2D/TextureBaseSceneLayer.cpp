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


#include "TextureBaseSceneLayer.h"

#include <Logging.h>
#include <OrthancException.h>

namespace OrthancStone
{
  void TextureBaseSceneLayer::CheckNoManualTransform() const
  {
    if (transform_.get() != NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls,
                                      "A transform has been manually set, first call ClearTransform()");
    }
  }


  void TextureBaseSceneLayer::SetTexture(Orthanc::ImageAccessor* texture)
  {
    if (texture == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else
    {
      texture_.reset(texture);
      IncrementRevision();
    }
  }


  void TextureBaseSceneLayer::CopyParameters(const TextureBaseSceneLayer& other)
  {
    originX_ = other.originX_;
    originY_ = other.originY_;
    pixelSpacingX_ = other.pixelSpacingX_;
    pixelSpacingY_ = other.pixelSpacingY_;
    angle_ = other.angle_;
    isLinearInterpolation_ = other.isLinearInterpolation_;
    flipX_ = other.flipX_;
    flipY_ = other.flipY_;

    if (other.transform_.get() != NULL)
    {
      transform_.reset(new AffineTransform2D(*other.transform_));
    }
  }


  TextureBaseSceneLayer::TextureBaseSceneLayer() :
    originX_(0),
    originY_(0),
    pixelSpacingX_(1),
    pixelSpacingY_(1),
    angle_(0),
    isLinearInterpolation_(false),
    flipX_(false),
    flipY_(false),
    revision_(0)
  {
    if (pixelSpacingX_ <= 0 ||
        pixelSpacingY_ <= 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }

  
  void TextureBaseSceneLayer::SetOrigin(double x,
                                        double y)
  {
    CheckNoManualTransform();
    
    originX_ = x;
    originY_ = y;
    IncrementRevision();
  }


  void TextureBaseSceneLayer::SetPixelSpacing(double sx,
                                              double sy)
  {
    CheckNoManualTransform();
    
    if (sx <= 0 ||
        sy <= 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      pixelSpacingX_ = sx;
      pixelSpacingY_ = sy;
      IncrementRevision();
    }
  }

  
  void TextureBaseSceneLayer::SetAngle(double angle)
  {
    CheckNoManualTransform();
    
    angle_ = angle;
    IncrementRevision();
  }

  
  void TextureBaseSceneLayer::SetLinearInterpolation(bool isLinearInterpolation)
  {
    isLinearInterpolation_ = isLinearInterpolation;
    IncrementRevision();
  }
    

  void TextureBaseSceneLayer::SetFlipX(bool flip)
  {
    CheckNoManualTransform();
    
    flipX_ = flip;
    IncrementRevision();
  }
  
    
  void TextureBaseSceneLayer::SetFlipY(bool flip)
  {
    CheckNoManualTransform();
    
    flipY_ = flip;
    IncrementRevision();
  }
    

  const Orthanc::ImageAccessor& TextureBaseSceneLayer::GetTexture() const
  {
    if (!HasTexture())
    {
      LOG(ERROR) << "TextureBaseSceneLayer::GetTexture(): (!HasTexture())";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return *texture_;
    }
  }

  
  void TextureBaseSceneLayer::SetTransform(const AffineTransform2D& transform)
  {
    transform_.reset(new AffineTransform2D(transform));
    IncrementRevision();    
  }
  

  void TextureBaseSceneLayer::ClearTransform()
  {
    transform_.reset(NULL);
    IncrementRevision();    
  }


  void TextureBaseSceneLayer::SetCuttingPlaneTransform(const CoordinateSystem3D& cuttingPlane,
                                                       const Vector& origin,
                                                       const Vector& pixelOffsetX,
                                                       const Vector& pixelOffsetY)
  {
    /**
     * Shift from the center of the voxel (DICOM convention for 3D
     * slices) to the corner of the voxel, because 2D textures are
     * expressed relatively to their borders. (*)
     **/
    Vector p = origin + cuttingPlane.GetOrigin() - 0.5 * pixelOffsetX - 0.5 * pixelOffsetY;
    
    double x0, y0, x1, y1, x2, y2;
    cuttingPlane.ProjectPoint(x0, y0, p);
    cuttingPlane.ProjectPoint(x1, y1, p + pixelOffsetX);
    cuttingPlane.ProjectPoint(x2, y2, p + pixelOffsetY);

    /**

       A = [ a11 a12 ; a21 a22 ]
       
       (1) A * (0 ; 0) + (b1 ; b2) = (x0 ; y0)
       (2) A * (1 ; 0) + (b1 ; b2) = (x1 ; y1)
       (3) A * (0 ; 1) + (b1 ; b2) = (x2 ; y2)

       (2-1) A * (1 ; 0) = (x1 - x0 ; y1 - y0) <=> (a11 ; a21) = (x1 - x0 ; y1 - y0)
       (3-1) A * (0 ; 1) = (x2 - x0 ; y2 - y0) <=> (a12 ; a22) = (x2 - x0 ; y2 - y0)

    **/

    Matrix m(3, 3);
    m(0, 0) = x1 - x0;  // a11
    m(0, 1) = x2 - x0;  // a12
    m(0, 2) = x0;       // b1
    m(1, 0) = y1 - y0;  // a21
    m(1, 1) = y2 - y0;  // a22
    m(1, 2) = y0;       // b2
    m(2, 0) = 0;
    m(2, 1) = 0;
    m(2, 2) = 1;

    SetTransform(AffineTransform2D(m));
  }


  AffineTransform2D TextureBaseSceneLayer::GetTransform() const
  {
    if (transform_.get() == NULL)
    {
      unsigned int width = 0;
      unsigned int height = 0;

      if (texture_.get() != NULL)
      {
        width = texture_->GetWidth();
        height = texture_->GetHeight();
      }
    
      return AffineTransform2D::Combine(
        AffineTransform2D::CreateOffset(originX_, originY_),
        AffineTransform2D::CreateRotation(angle_),
        AffineTransform2D::CreateScaling(pixelSpacingX_, pixelSpacingY_),
        AffineTransform2D::CreateOffset(-0.5, -0.5),  // (*)
        AffineTransform2D::CreateFlip(flipX_, flipY_, width, height));
    }
    else
    {
      return *transform_;
    }
  }

  
  void TextureBaseSceneLayer::GetBoundingBox(Extent2D& target) const
  {
    target.Clear();
    
    if (texture_.get() != NULL)
    {
      const AffineTransform2D t = GetTransform();

      double x, y;

      x = 0;
      y = 0;
      t.Apply(x, y);
      target.AddPoint(x, y);

      x = static_cast<double>(texture_->GetWidth());
      y = 0;
      t.Apply(x, y);
      target.AddPoint(x, y);

      x = 0;
      y = static_cast<double>(texture_->GetHeight());
      t.Apply(x, y);
      target.AddPoint(x, y);

      x = static_cast<double>(texture_->GetWidth());
      y = static_cast<double>(texture_->GetHeight());
      t.Apply(x, y);
      target.AddPoint(x, y);    
    }
  }
}
