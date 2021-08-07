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


#include "InfoPanelSceneLayer.h"

#include <Images/Image.h>
#include <OrthancException.h>

namespace OrthancStone
{
  InfoPanelSceneLayer::InfoPanelSceneLayer(const Orthanc::ImageAccessor& texture,
                                           BitmapAnchor anchor,
                                           bool isLinearInterpolation,
                                           bool applySceneRotation) :
    texture_(Orthanc::Image::Clone(texture)),
    anchor_(anchor),
    isLinearInterpolation_(isLinearInterpolation),
    applySceneRotation_(applySceneRotation)
  {
    if (texture_->GetFormat() != Orthanc::PixelFormat_RGBA32 &&
        texture_->GetFormat() != Orthanc::PixelFormat_RGB24)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }
  }


  void InfoPanelSceneLayer::ComputeAnchorLocation(int& x,
                                                  int& y,
                                                  BitmapAnchor anchor,
                                                  unsigned int textureWidth,
                                                  unsigned int textureHeight,
                                                  unsigned int canvasWidth,
                                                  unsigned int canvasHeight)
  {
    int tw = static_cast<int>(textureWidth);
    int th = static_cast<int>(textureHeight);
    int cw = static_cast<int>(canvasWidth);
    int ch = static_cast<int>(canvasHeight);
    
    switch (anchor)
    {
      case BitmapAnchor_TopLeft:
      case BitmapAnchor_CenterLeft:
      case BitmapAnchor_BottomLeft:
        x = 0;
        break;
          
      case BitmapAnchor_TopCenter:
      case BitmapAnchor_Center:
      case BitmapAnchor_BottomCenter:
        x = (cw - tw) / 2;
        break;
          
      case BitmapAnchor_TopRight:
      case BitmapAnchor_CenterRight:
      case BitmapAnchor_BottomRight:
        x = cw - tw;
        break;
          
      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    switch (anchor)
    {
      case BitmapAnchor_TopLeft:
      case BitmapAnchor_TopCenter:
      case BitmapAnchor_TopRight:
        y = 0;
        break;
          
      case BitmapAnchor_CenterLeft:
      case BitmapAnchor_Center:
      case BitmapAnchor_CenterRight:
        y = (ch - th) / 2;
        break;
          
      case BitmapAnchor_BottomLeft:
      case BitmapAnchor_BottomCenter:
      case BitmapAnchor_BottomRight:
        y = ch - th;
        break;
          
      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
  }
}
