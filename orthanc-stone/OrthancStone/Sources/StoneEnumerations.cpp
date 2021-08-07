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


#include "StoneEnumerations.h"

#include <Logging.h>
#include <OrthancException.h>
#include <Toolbox.h>

namespace OrthancStone
{  
  SopClassUid StringToSopClassUid(const std::string& source)
  {
    std::string s = Orthanc::Toolbox::StripSpaces(source);

    if (s == "1.2.840.10008.5.1.4.1.1.481.2")
    {
      return SopClassUid_RTDose;
    }
    else if (s == "1.2.840.10008.5.1.4.1.1.481.5")
    {
      return SopClassUid_RTPlan;
    }
    else if (s == "1.2.840.10008.5.1.4.1.1.481.3")
    {
      return SopClassUid_RTStruct;
    }
    else if (s == "1.2.840.10008.5.1.4.1.1.104.1")
    {
      return SopClassUid_EncapsulatedPdf;
    }
    else if (s == "1.2.840.10008.5.1.4.1.1.77.1.1.1")
    {
      return SopClassUid_VideoEndoscopicImageStorage;
    }      
    else if (s == "1.2.840.10008.5.1.4.1.1.77.1.2.1")
    {
      return SopClassUid_VideoMicroscopicImageStorage;
    }      
    else if (s == "1.2.840.10008.5.1.4.1.1.77.1.4.1")
    {
      return SopClassUid_VideoPhotographicImageStorage;
    }      
    else
    {
      //LOG(INFO) << "Other SOP class UID: " << source;
      return SopClassUid_Other;
    }
  }  


  void ComputeWindowing(float& targetCenter,
                        float& targetWidth,
                        ImageWindowing windowing,
                        float customCenter,
                        float customWidth)
  {
    switch (windowing)
    {
      case ImageWindowing_Custom:
        targetCenter = customCenter;
        targetWidth = customWidth;
        break;

      case ImageWindowing_Bone:
        targetCenter = 300;
        targetWidth = 2000;
        break;

      case ImageWindowing_Lung:
        targetCenter = -600;
        targetWidth = 1600;
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }

  
  void ComputeAnchorTranslation(double& deltaX,
                                double& deltaY,
                                BitmapAnchor anchor,
                                unsigned int bitmapWidth,
                                unsigned int bitmapHeight,
                                unsigned int border)
  {
    double dw = static_cast<double>(bitmapWidth);
    double dh = static_cast<double>(bitmapHeight);

    switch (anchor)
    {
      case BitmapAnchor_TopLeft:
        deltaX = 0;
        deltaY = 0;
        break;
        
      case BitmapAnchor_TopCenter:
        deltaX = -dw / 2.0;
        deltaY = 0;
        break;
        
      case BitmapAnchor_TopRight:
        deltaX = -dw;
        deltaY = 0;
        break;
        
      case BitmapAnchor_CenterLeft:
        deltaX = 0;
        deltaY = -dh / 2.0;
        break;
        
      case BitmapAnchor_Center:
        deltaX = -dw / 2.0;
        deltaY = -dh / 2.0;
        break;
        
      case BitmapAnchor_CenterRight:
        deltaX = -dw;
        deltaY = -dh / 2.0;
        break;
        
      case BitmapAnchor_BottomLeft:
        deltaX = 0;
        deltaY = -dh;
        break;
        
      case BitmapAnchor_BottomCenter:
        deltaX = -dw / 2.0;
        deltaY = -dh;
        break;
        
      case BitmapAnchor_BottomRight:
        deltaX = -dw;
        deltaY = -dh;
        break;
        
      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }    

    if (border != 0)
    {
      double b = static_cast<double>(border);

      switch (anchor)
      {
        case BitmapAnchor_TopLeft:
        case BitmapAnchor_TopCenter:
        case BitmapAnchor_TopRight:
          deltaY += b;
          break;
        
        case BitmapAnchor_BottomLeft:
        case BitmapAnchor_BottomCenter:
        case BitmapAnchor_BottomRight:
          deltaY -= b;
          break;
        
        default:
          break;
      }    

      switch (anchor)
      {
        case BitmapAnchor_TopLeft:
        case BitmapAnchor_CenterLeft:
        case BitmapAnchor_BottomLeft:
          deltaX += b;
          break;

        case BitmapAnchor_CenterRight:
        case BitmapAnchor_TopRight:
        case BitmapAnchor_BottomRight:
          deltaX -= b;
          break;
        
        default:
          break;
      }
    }
  }


  SeriesThumbnailType GetSeriesThumbnailType(SopClassUid sopClassUid)
  {
    switch (sopClassUid)
    {
      case SopClassUid_EncapsulatedPdf:
        return SeriesThumbnailType_Pdf;
        
      case SopClassUid_VideoEndoscopicImageStorage:
      case SopClassUid_VideoMicroscopicImageStorage:
      case SopClassUid_VideoPhotographicImageStorage:
        return SeriesThumbnailType_Video;

      default:
        return SeriesThumbnailType_Unsupported;
    }
  }
}
