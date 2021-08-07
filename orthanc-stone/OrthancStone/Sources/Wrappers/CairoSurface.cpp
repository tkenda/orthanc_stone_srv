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


#include "CairoSurface.h"

#include <Logging.h>
#include <OrthancException.h>
#include <Images/ImageProcessing.h>

namespace OrthancStone
{
  void CairoSurface::Release()
  {
    if (surface_)
    {
      cairo_surface_destroy(surface_);
      surface_ = NULL;
    }
  }


  void CairoSurface::Allocate(unsigned int width,
                              unsigned int height,
                              bool hasAlpha)
  {
    Release();

    hasAlpha_ = hasAlpha;

    surface_ = cairo_image_surface_create
      (hasAlpha ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24, width, height);
    if (!surface_)
    {
      // Should never occur
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    if (cairo_surface_status(surface_) != CAIRO_STATUS_SUCCESS)
    {
      LOG(ERROR) << "Cannot create a Cairo surface";
      cairo_surface_destroy(surface_);
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    width_ = width;
    height_ = height;
    pitch_ = cairo_image_surface_get_stride(surface_);
    buffer_ = cairo_image_surface_get_data(surface_);
  }


  CairoSurface::CairoSurface(Orthanc::ImageAccessor& accessor,
                             bool hasAlpha) :
    hasAlpha_(hasAlpha)
  {
    if (accessor.GetFormat() != Orthanc::PixelFormat_BGRA32)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }      

    width_ = accessor.GetWidth();
    height_ = accessor.GetHeight();
    pitch_ = accessor.GetPitch();
    buffer_ = accessor.GetBuffer();

    surface_ = cairo_image_surface_create_for_data
      (reinterpret_cast<unsigned char*>(buffer_),
       hasAlpha_ ? CAIRO_FORMAT_ARGB32 : CAIRO_FORMAT_RGB24,
       width_, height_, pitch_);
    if (!surface_)
    {
      // Should never occur
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    if (cairo_surface_status(surface_) != CAIRO_STATUS_SUCCESS)
    {
      LOG(ERROR) << "Bad pitch for a Cairo surface";
      cairo_surface_destroy(surface_);
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }


  void CairoSurface::SetSize(unsigned int width,
                             unsigned int height,
                             bool hasAlpha)
  {
    if (hasAlpha_ != hasAlpha ||
        width_ != width ||
        height_ != height)
    {
      Allocate(width, height, hasAlpha);
    }
  }


  void CairoSurface::Copy(const CairoSurface& other)
  {
    SetSize(other.GetWidth(), other.GetHeight(), other.HasAlpha());
    
    Orthanc::ImageAccessor source, target;

    other.GetReadOnlyAccessor(source);
    GetWriteableAccessor(target);

    Orthanc::ImageProcessing::Copy(target, source);

    cairo_surface_mark_dirty(surface_);
  }


  void CairoSurface::Copy(const Orthanc::ImageAccessor& source,
                          bool hasAlpha)
  {
    SetSize(source.GetWidth(), source.GetHeight(), hasAlpha);

    Orthanc::ImageAccessor target;
    GetWriteableAccessor(target);

    Orthanc::ImageProcessing::Convert(target, source);

    cairo_surface_mark_dirty(surface_);
  }


  void CairoSurface::GetReadOnlyAccessor(Orthanc::ImageAccessor& target) const
  {
    target.AssignReadOnly(Orthanc::PixelFormat_BGRA32, width_, height_, pitch_, buffer_);
  }
  

  void CairoSurface::GetWriteableAccessor(Orthanc::ImageAccessor& target)
  {
    target.AssignWritable(Orthanc::PixelFormat_BGRA32, width_, height_, pitch_, buffer_);
  }
}
