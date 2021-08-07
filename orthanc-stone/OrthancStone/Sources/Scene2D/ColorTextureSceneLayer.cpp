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


#include "ColorTextureSceneLayer.h"

#include <OrthancException.h>
#include <Images/Image.h>


namespace OrthancStone
{
  ColorTextureSceneLayer::ColorTextureSceneLayer(const Orthanc::ImageAccessor& texture)
  {
    if (texture.GetFormat() != Orthanc::PixelFormat_Grayscale8 &&
        texture.GetFormat() != Orthanc::PixelFormat_RGBA32 &&
        texture.GetFormat() != Orthanc::PixelFormat_RGB24)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }
      
    SetTexture(Orthanc::Image::Clone(texture));
  }


  ISceneLayer* ColorTextureSceneLayer::Clone() const
  {
    std::unique_ptr<ColorTextureSceneLayer> cloned(new ColorTextureSceneLayer(GetTexture()));
    cloned->CopyParameters(*this);
    return cloned.release();
  }
}
