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


#include "CairoContext.h"

#include <Logging.h>
#include <OrthancException.h>


namespace OrthancStone
{
  CairoContext::CairoContext(CairoSurface& surface) :
    width_(surface.GetWidth()),
    height_(surface.GetHeight())
  {
    context_ = cairo_create(surface.GetObject());
    if (!context_)
    {
      LOG(ERROR) << "Cannot create Cairo drawing context";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }


  CairoContext::~CairoContext()
  {
    if (context_ != NULL)
    {
      cairo_destroy(context_);
      context_ = NULL;
    }
  }


  void CairoContext::SetSourceColor(uint8_t red,
                                    uint8_t green,
                                    uint8_t blue)
  {
    cairo_set_source_rgb(context_,
                         static_cast<float>(red) / 255.0f,
                         static_cast<float>(green) / 255.0f,
                         static_cast<float>(blue) / 255.0f);
  }


  class CairoContext::AlphaSurface : public boost::noncopyable
  {
  private:
    cairo_surface_t  *surface_;

  public:
    AlphaSurface(unsigned int width,
                 unsigned int height)
    {
      surface_ = cairo_image_surface_create(CAIRO_FORMAT_A8, width, height);
      
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
    }
    
    ~AlphaSurface()
    {
      cairo_surface_destroy(surface_);
    }

    void GetAccessor(Orthanc::ImageAccessor& target)
    {
      target.AssignWritable(Orthanc::PixelFormat_Grayscale8,
                            cairo_image_surface_get_width(surface_),
                            cairo_image_surface_get_height(surface_),
                            cairo_image_surface_get_stride(surface_),
                            cairo_image_surface_get_data(surface_));
    }

    void Blit(cairo_t* cr,
              double x,
              double y)
    {
      cairo_surface_mark_dirty(surface_);
      cairo_mask_surface(cr, surface_, x, y);
      cairo_fill(cr);
    }
  };


  void CairoContext::DrawText(const Orthanc::Font& font,
                              const std::string& text,
                              double x,
                              double y,
                              BitmapAnchor anchor)
  {
    // Render a bitmap containing the text
    unsigned int width, height;
    font.ComputeTextExtent(width, height, text);
    
    AlphaSurface surface(width, height);

    Orthanc::ImageAccessor accessor;
    surface.GetAccessor(accessor);
    font.Draw(accessor, text, 0, 0, 255);

    // Correct the text location given the anchor location
    double deltaX, deltaY;
    ComputeAnchorTranslation(deltaX, deltaY, anchor, width, height);

    // Cancel zoom/rotation before blitting the text onto the surface
    double pixelX = x;
    double pixelY = y;
    cairo_user_to_device(context_, &pixelX, &pixelY);

    cairo_save(context_);
    cairo_identity_matrix(context_);

    // Blit the text bitmap
    surface.Blit(context_, pixelX + deltaX, pixelY + deltaY);
    cairo_restore(context_);
  }
}
