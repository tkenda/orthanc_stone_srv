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


#include "DicomInstanceParameters.h"

#include "../Scene2D/ColorTextureSceneLayer.h"
#include "../Scene2D/FloatTextureSceneLayer.h"
#include "GeometryToolbox.h"
#include "ImageToolbox.h"

#include <Images/Image.h>
#include <Images/ImageProcessing.h>
#include <Logging.h>
#include <OrthancException.h>
#include <Toolbox.h>


namespace OrthancStone
{
  static void ExtractFrameOffsets(Vector& target,
                                  const Orthanc::DicomMap& dicom,
                                  unsigned int numberOfFrames)
  {
    // http://dicom.nema.org/medical/Dicom/2016a/output/chtml/part03/sect_C.8.8.3.2.html

    std::string increment;

    if (dicom.LookupStringValue(increment, Orthanc::DICOM_TAG_FRAME_INCREMENT_POINTER, false))
    {
      Orthanc::Toolbox::ToUpperCase(increment);

      // We only support volumes where the FrameIncrementPointer (0028,0009) (required) contains 
      // the "Grid Frame Offset Vector" tag (DICOM_TAG_GRID_FRAME_OFFSET_VECTOR)
      if (increment != "3004,000C")
      {
        LOG(WARNING) << "Bad value for the FrameIncrementPointer tags in a multiframe image";
        target.resize(0);
        return;
      }
    }

    if (!LinearAlgebra::ParseVector(target, dicom, Orthanc::DICOM_TAG_GRID_FRAME_OFFSET_VECTOR) ||
        target.size() != numberOfFrames)
    {
      LOG(ERROR) << "The frame offset information (GridFrameOffsetVector (3004,000C)) is missing in a multiframe image";

      // DO NOT use ".clear()" here, as the "Vector" class doesn't behave like std::vector!
      target.resize(0);
    }
  }


  DicomInstanceParameters::Data::Data(const Orthanc::DicomMap& dicom)
  {
    if (!dicom.LookupStringValue(studyInstanceUid_, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, false) ||
        !dicom.LookupStringValue(seriesInstanceUid_, Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, false) ||
        !dicom.LookupStringValue(sopInstanceUid_, Orthanc::DICOM_TAG_SOP_INSTANCE_UID, false))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
        
    std::string s;
    if (dicom.LookupStringValue(s, Orthanc::DICOM_TAG_SOP_CLASS_UID, false))
    {
      sopClassUid_ = StringToSopClassUid(s);
    }
    else
    {
      sopClassUid_ = SopClassUid_Other;
    }

    uint32_t n;
    if (dicom.ParseUnsignedInteger32(n, Orthanc::DICOM_TAG_NUMBER_OF_FRAMES))
    {
      hasNumberOfFrames_ = true;
      numberOfFrames_ = n;
    }
    else
    {
      hasNumberOfFrames_ = false;
      numberOfFrames_ = 1;
    }

    if (!dicom.HasTag(Orthanc::DICOM_TAG_COLUMNS) ||
        !dicom.GetValue(Orthanc::DICOM_TAG_COLUMNS).ParseFirstUnsignedInteger(width_))
    {
      width_ = 0;
    }    

    if (!dicom.HasTag(Orthanc::DICOM_TAG_ROWS) ||
        !dicom.GetValue(Orthanc::DICOM_TAG_ROWS).ParseFirstUnsignedInteger(height_))
    {
      height_ = 0;
    }


    bool sliceThicknessPresent = true;
    if (!dicom.ParseDouble(sliceThickness_, Orthanc::DICOM_TAG_SLICE_THICKNESS))
    {
      if (numberOfFrames_ > 1)
      {
        LOG(INFO) << "The (non-madatory) slice thickness information is missing in a multiframe image";
      }
      
      sliceThickness_ = 100.0 * std::numeric_limits<double>::epsilon();
      sliceThicknessPresent = false;
    }

    hasPixelSpacing_ = GeometryToolbox::GetPixelSpacing(pixelSpacingX_, pixelSpacingY_, dicom);

    std::string position, orientation;
    if (dicom.LookupStringValue(position, Orthanc::DICOM_TAG_IMAGE_POSITION_PATIENT, false) &&
        dicom.LookupStringValue(orientation, Orthanc::DICOM_TAG_IMAGE_ORIENTATION_PATIENT, false))
    {
      geometry_ = CoordinateSystem3D(position, orientation);
    }

    // Must be AFTER setting "numberOfFrames_"
    if (numberOfFrames_ > 1)
    {
      ExtractFrameOffsets(frameOffsets_, dicom, numberOfFrames_);

      // if the slice thickness is unknown, we try to infer it from the sequence of grid frame offsets
      // this only works if:
      // - the first offset is 0.0 (case (a) of http://dicom.nema.org/medical/Dicom/2017c/output/chtml/part03/sect_C.8.8.3.2.html)
      // - the offsets are all equal, to some small tolerance
      // - the offsets is positive (increasing throughout the frames)
      if (!sliceThicknessPresent)
      {
        if (frameOffsets_.size() >= 2)
        {
          double sliceThickness = frameOffsets_[1] - frameOffsets_[0];
          
          if (sliceThickness > 0)
          {
            bool sameSized = true;
            
            for (size_t i = 2; i < frameOffsets_.size(); ++i)
            {
              double currentThickness = frameOffsets_[i] - frameOffsets_[i-1];
              if (!LinearAlgebra::IsNear(sliceThickness, currentThickness))
              {
                LOG(ERROR) << "Unable to extract slice thickness from GridFrameOffsetVector (3004,000C) (reason: varying spacing)";
                sameSized = false;
                break;
              }
            }
            
            if (sameSized)
            {
              sliceThickness_ = sliceThickness;
              LOG(INFO) << "SliceThickness was not specified in the Dicom but was inferred from GridFrameOffsetVector (3004,000C).";
            }
          }
        }
        else
        {
          LOG(ERROR) << "Unable to extract slice thickness from GridFrameOffsetVector (3004,000C) (reason: GridFrameOffsetVector not present or too small)";
        }
      }
    }
    else
    {
      frameOffsets_.resize(0);
    }

    if (sopClassUid_ == SopClassUid_RTDose)
    {
      static const Orthanc::DicomTag DICOM_TAG_DOSE_UNITS(0x3004, 0x0002);

      if (!dicom.LookupStringValue(doseUnits_, DICOM_TAG_DOSE_UNITS, false))
      {
        LOG(ERROR) << "Tag DoseUnits (0x3004, 0x0002) is missing in " << sopInstanceUid_;
        doseUnits_.clear();
      }
    }

    if (dicom.ParseDouble(rescaleIntercept_, Orthanc::DICOM_TAG_RESCALE_INTERCEPT) &&
        dicom.ParseDouble(rescaleSlope_, Orthanc::DICOM_TAG_RESCALE_SLOPE))
    {
      if (sopClassUid_ == SopClassUid_RTDose)
      {
        LOG(INFO) << "DOSE HAS Rescale*: rescaleIntercept_ = " << rescaleIntercept_ << " rescaleSlope_ = " << rescaleSlope_;
        // WE SHOULD NOT TAKE THE RESCALE VALUE INTO ACCOUNT IN THE CASE OF DOSES
        hasRescale_ = false;
      }
      else
      {
        hasRescale_ = true;
      }
      
    }
    else
    {
      hasRescale_ = false;
    }

    if (dicom.ParseDouble(doseGridScaling_, Orthanc::DICOM_TAG_DOSE_GRID_SCALING))
    {
      if (sopClassUid_ == SopClassUid_RTDose)
      {
        LOG(INFO) << "DOSE HAS DoseGridScaling: doseGridScaling_ = " << doseGridScaling_;
      }
    }
    else
    {
      doseGridScaling_ = 1.0;
      if (sopClassUid_ == SopClassUid_RTDose)
      {
        LOG(ERROR) << "Tag DoseGridScaling (0x3004, 0x000e) is missing in " << sopInstanceUid_ << " doseGridScaling_ will be set to 1.0";
      }
    }

    bool ok = false;
    
    if (LinearAlgebra::ParseVector(windowingPresetCenters_, dicom, Orthanc::DICOM_TAG_WINDOW_CENTER) &&
        LinearAlgebra::ParseVector(windowingPresetWidths_, dicom, Orthanc::DICOM_TAG_WINDOW_WIDTH))
    {
      if (windowingPresetCenters_.size() == windowingPresetWidths_.size())
      {
        ok = true;
      }
      else
      {
        LOG(ERROR) << "Mismatch in the number of preset windowing widths/centers, ignoring this";
        ok = false;
      }
    }

    if (!ok)
    {
      // Don't use "Vector::clear()", as it has not the same meaning as "std::vector::clear()"
      windowingPresetCenters_.resize(0);
      windowingPresetWidths_.resize(0);
    }      

    // This computes the "IndexInSeries" metadata from Orthanc (check
    // out "Orthanc::ServerIndex::Store()")
    hasIndexInSeries_ = (
      dicom.ParseUnsignedInteger32(indexInSeries_, Orthanc::DICOM_TAG_INSTANCE_NUMBER) ||
      dicom.ParseUnsignedInteger32(indexInSeries_, Orthanc::DICOM_TAG_IMAGE_INDEX));

    if (!dicom.LookupStringValue(
          frameOfReferenceUid_, Orthanc::DICOM_TAG_FRAME_OF_REFERENCE_UID, false))
    {
      frameOfReferenceUid_.clear();
    }

    if (!dicom.HasTag(Orthanc::DICOM_TAG_INSTANCE_NUMBER) ||
        !dicom.GetValue(Orthanc::DICOM_TAG_INSTANCE_NUMBER).ParseInteger32(instanceNumber_))
    {
      instanceNumber_ = 0;
    }
  }


  const Orthanc::DicomImageInformation& DicomInstanceParameters::GetImageInformation() const
  {
    assert(tags_.get() != NULL);
    
    if (imageInformation_.get() == NULL)
    {
      const_cast<DicomInstanceParameters&>(*this).imageInformation_.
        reset(new Orthanc::DicomImageInformation(GetTags()));

      assert(imageInformation_->GetWidth() == GetWidth());
      assert(imageInformation_->GetHeight() == GetHeight());
      assert(imageInformation_->GetNumberOfFrames() == GetNumberOfFrames());
    }

    assert(imageInformation_.get() != NULL);
    return *imageInformation_;
  }
  

  CoordinateSystem3D  DicomInstanceParameters::GetFrameGeometry(unsigned int frame) const
  {
    if (frame >= data_.numberOfFrames_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else if (data_.frameOffsets_.empty())
    {
      return data_.geometry_;
    }
    else
    {
      assert(data_.frameOffsets_.size() == data_.numberOfFrames_);

      return CoordinateSystem3D(
        data_.geometry_.GetOrigin() + data_.frameOffsets_[frame] * data_.geometry_.GetNormal(),
        data_.geometry_.GetAxisX(),
        data_.geometry_.GetAxisY());
    }
  }


  bool DicomInstanceParameters::IsPlaneWithinSlice(unsigned int frame,
                                                   const CoordinateSystem3D& plane) const
  {
    if (frame >= data_.numberOfFrames_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    CoordinateSystem3D tmp = data_.geometry_;

    if (frame != 0)
    {
      tmp = GetFrameGeometry(frame);
    }

    double distance;

    return (CoordinateSystem3D::ComputeDistance(distance, tmp, plane) &&
            distance <= data_.sliceThickness_ / 2.0);
  }


  bool DicomInstanceParameters::IsColor() const
  {
    Orthanc::PhotometricInterpretation photometric =
      GetImageInformation().GetPhotometricInterpretation();
    
    return (photometric != Orthanc::PhotometricInterpretation_Monochrome1 &&
            photometric != Orthanc::PhotometricInterpretation_Monochrome2);
  }


  void DicomInstanceParameters::ApplyRescaleAndDoseScaling(Orthanc::ImageAccessor& image,
                                                           bool useDouble) const
  {
    if (image.GetFormat() != Orthanc::PixelFormat_Float32)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    double factor = data_.doseGridScaling_;
    double offset = 0.0;

    if (data_.hasRescale_)
    {
      factor *= data_.rescaleSlope_;
      offset = data_.rescaleIntercept_;
    }

    if (!LinearAlgebra::IsNear(factor, 1) ||
        !LinearAlgebra::IsNear(offset, 0))
    {
      const unsigned int width = image.GetWidth();
      const unsigned int height = image.GetHeight();
        
      for (unsigned int y = 0; y < height; y++)
      {
        float* p = reinterpret_cast<float*>(image.GetRow(y));

        if (useDouble)
        {
          // Slower, accurate implementation using double
          for (unsigned int x = 0; x < width; x++, p++)
          {
            double value = static_cast<double>(*p);
            *p = static_cast<float>(value * factor + offset);
          }
        }
        else
        {
          // Fast, approximate implementation using float
          for (unsigned int x = 0; x < width; x++, p++)
          {
            *p = (*p) * static_cast<float>(factor) + static_cast<float>(offset);
          }
        }
      }
    }
  }

  double DicomInstanceParameters::GetRescaleIntercept() const
  {
    if (data_.hasRescale_)
    {
      return data_.rescaleIntercept_;
    }
    else
    {
      LOG(ERROR) << "DicomInstanceParameters::GetRescaleIntercept(): !data_.hasRescale_";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  double DicomInstanceParameters::GetRescaleSlope() const
  {
    if (data_.hasRescale_)
    {
      return data_.rescaleSlope_;
    }
    else
    {
      LOG(ERROR) << "DicomInstanceParameters::GetRescaleSlope(): !data_.hasRescale_";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  Orthanc::PixelFormat DicomInstanceParameters::GetExpectedPixelFormat() const
  {
    if (GetSopClassUid() == SopClassUid_RTDose)
    {
      switch (GetImageInformation().GetBitsStored())
      {
        case 16:
          return Orthanc::PixelFormat_Grayscale16;

        case 32:
          return Orthanc::PixelFormat_Grayscale32;

        default:
          return Orthanc::PixelFormat_Grayscale16;  // Rough guess
      } 
    }
    else if (IsColor())
    {
      return Orthanc::PixelFormat_RGB24;
    }
    else if (GetImageInformation().IsSigned())
    {
      return Orthanc::PixelFormat_SignedGrayscale16;
    }
    else
    {
      return Orthanc::PixelFormat_Grayscale16;  // Rough guess
    }
  }


  size_t DicomInstanceParameters::GetWindowingPresetsCount() const
  {
    assert(data_.windowingPresetCenters_.size() == data_.windowingPresetWidths_.size());
    return data_.windowingPresetCenters_.size();
  }
  

  float DicomInstanceParameters::GetWindowingPresetCenter(size_t i) const
  {
    if (i < GetWindowingPresetsCount())
    {
      return static_cast<float>(data_.windowingPresetCenters_[i]);
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  float DicomInstanceParameters::GetWindowingPresetWidth(size_t i) const
  {
    if (i < GetWindowingPresetsCount())
    {
      return static_cast<float>(data_.windowingPresetWidths_[i]);
    }
    else
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  static void GetWindowingBounds(float& low,
                                 float& high,
                                 double center,  // in
                                 double width)   // in
  {
    low = static_cast<float>(center - width / 2.0);
    high = static_cast<float>(center + width / 2.0);
  }

  
  void DicomInstanceParameters::GetWindowingPresetsUnion(float& center,
                                                         float& width) const
  {
    assert(tags_.get() != NULL);
    size_t s = GetWindowingPresetsCount();

    if (s > 0)
    {
      // Use the largest windowing given all the preset windowings
      // that are available in the DICOM tags
      float low, high;
      GetWindowingBounds(low, high, GetWindowingPresetCenter(0), GetWindowingPresetWidth(0));

      for (size_t i = 1; i < s; i++)
      {
        float a, b;
        GetWindowingBounds(a, b, GetWindowingPresetCenter(i), GetWindowingPresetWidth(i));
        low = std::min(low, a);
        high = std::max(high, b);
      }

      assert(low <= high);

      if (LinearAlgebra::IsNear(low, high))
      {
        // Cannot infer a suitable windowing from the available tags
        center = 128.0f;
        width = 256.0f;
      }
      else
      {
        center = (low + high) / 2.0f;
        width = (high - low);
      }
    }
    else
    {
      float a, b;
      if (tags_->ParseFloat(a, Orthanc::DICOM_TAG_SMALLEST_IMAGE_PIXEL_VALUE) &&
          tags_->ParseFloat(b, Orthanc::DICOM_TAG_LARGEST_IMAGE_PIXEL_VALUE) &&
          a < b)
      {
        center = (a + b) / 2.0f;
        width = (b - a);
      }
      else
      {
        // Cannot infer a suitable windowing from the available tags
        center = 128.0f;
        width = 256.0f;
      }
    }
  }


  Orthanc::ImageAccessor* DicomInstanceParameters::ConvertToFloat(const Orthanc::ImageAccessor& pixelData) const
  {
    std::unique_ptr<Orthanc::Image> converted(new Orthanc::Image(Orthanc::PixelFormat_Float32, 
                                                                 pixelData.GetWidth(), 
                                                                 pixelData.GetHeight(),
                                                                 false));
    Orthanc::ImageProcessing::Convert(*converted, pixelData);

                                                   
    // Correct rescale slope/intercept if need be
    //ApplyRescaleAndDoseScaling(*converted, (pixelData.GetFormat() == Orthanc::PixelFormat_Grayscale32));
    ApplyRescaleAndDoseScaling(*converted, false);

    return converted.release();
  }


  TextureBaseSceneLayer* DicomInstanceParameters::CreateTexture
  (const Orthanc::ImageAccessor& pixelData) const
  {
    // {
    //   const Orthanc::ImageAccessor& source = pixelData;
    //   const void* sourceBuffer = source.GetConstBuffer();
    //   intptr_t sourceBufferInt = reinterpret_cast<intptr_t>(sourceBuffer);
    //   int sourceWidth = source.GetWidth();
    //   int sourceHeight = source.GetHeight();
    //   int sourcePitch = source.GetPitch();

    //   // TODO: turn error into trace below
    //   LOG(ERROR) << "ConvertGrayscaleToFloat | source:"
    //     << " W = " << sourceWidth << " H = " << sourceHeight
    //     << " P = " << sourcePitch << " B = " << sourceBufferInt
    //     << " B % 4 == " << sourceBufferInt % 4;
    // }

    assert(sizeof(float) == 4);

    Orthanc::PixelFormat sourceFormat = pixelData.GetFormat();

    if (sourceFormat != GetExpectedPixelFormat())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    if (sourceFormat == Orthanc::PixelFormat_RGB24)
    {
      // This is the case of a color image. No conversion has to be done.
      return new ColorTextureSceneLayer(pixelData);
    }
    else
    {
      // This is the case of a grayscale frame. Convert it to Float32.
      std::unique_ptr<FloatTextureSceneLayer> texture;

      if (pixelData.GetFormat() == Orthanc::PixelFormat_Float32)
      {
        texture.reset(new FloatTextureSceneLayer(pixelData));
      }
      else
      {
        std::unique_ptr<Orthanc::ImageAccessor> converted(ConvertToFloat(pixelData));
        texture.reset(new FloatTextureSceneLayer(*converted));
      }

      if (GetWindowingPresetsCount() > 0)
      {
        texture->SetCustomWindowing(GetWindowingPresetCenter(0), GetWindowingPresetWidth(0));
      }
      
      switch (GetImageInformation().GetPhotometricInterpretation())
      {
        case Orthanc::PhotometricInterpretation_Monochrome1:
          texture->SetInverted(true);
          break;
          
        case Orthanc::PhotometricInterpretation_Monochrome2:
          texture->SetInverted(false);
          break;

        default:
          break;
      }

      return texture.release();
    }
  }


  LookupTableTextureSceneLayer* DicomInstanceParameters::CreateLookupTableTexture
  (const Orthanc::ImageAccessor& pixelData) const
  {
    std::unique_ptr<FloatTextureSceneLayer> texture;

    if (pixelData.GetFormat() == Orthanc::PixelFormat_Float32)
    {
      return new LookupTableTextureSceneLayer(pixelData);
    }
    else
    {
      std::unique_ptr<Orthanc::ImageAccessor> converted(ConvertToFloat(pixelData));
      return new LookupTableTextureSceneLayer(*converted);
    }
  }

  
  unsigned int DicomInstanceParameters::GetIndexInSeries() const
  {
    if (data_.hasIndexInSeries_)
    {
      return data_.indexInSeries_;
    }
    else
    {
      LOG(ERROR) << "DicomInstanceParameters::GetIndexInSeries(): !data_.hasIndexInSeries_";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }


  double DicomInstanceParameters::ApplyRescale(double value) const
  {
    double factor = data_.doseGridScaling_;
    double offset = 0.0;

    if (data_.hasRescale_)
    {
      factor *= data_.rescaleSlope_;
      offset = data_.rescaleIntercept_;
    }

    return (value * factor + offset);
  }


  bool DicomInstanceParameters::ComputeRegularSpacing(double& spacing) const
  {
    if (data_.frameOffsets_.size() == 0)  // Not a RT-DOSE
    {
      return false;
    }
    else if (data_.frameOffsets_.size() == 1)
    {
      spacing = 1;   // Edge case: RT-DOSE with one single frame
      return true;
    }
    else
    {
      assert(data_.frameOffsets_.size() == GetNumberOfFrames());
      
      spacing = std::abs(data_.frameOffsets_[1] - data_.frameOffsets_[0]);

      for (size_t i = 1; i + 1 < data_.frameOffsets_.size(); i++)
      {
        double s = data_.frameOffsets_[i + 1] - data_.frameOffsets_[i];
        if (!LinearAlgebra::IsNear(spacing, s, 0.001))
        {
          return false;
        }
      }
      
      return true;
    }
  }

  
  void DicomInstanceParameters::SetPixelSpacing(double pixelSpacingX,
                                                double pixelSpacingY)
  {
    data_.hasPixelSpacing_ = true;
    data_.pixelSpacingX_ = pixelSpacingX;
    data_.pixelSpacingY_ = pixelSpacingY;
  }


  static const Json::Value* LookupDicomWebSingleValue(const Json::Value& dicomweb,
                                                      const std::string& tag,
                                                      const std::string& vr)
  {
    static const char* const VALUE = "Value";
    static const char* const VR = "vr";

    if (dicomweb.type() == Json::objectValue &&
        dicomweb.isMember(tag) &&
        dicomweb[tag].type() == Json::objectValue &&
        dicomweb[tag].isMember(VALUE) &&
        dicomweb[tag].isMember(VR) &&
        dicomweb[tag][VR].type() == Json::stringValue &&
        dicomweb[tag][VR].asString() == vr &&
        dicomweb[tag][VALUE].type() == Json::arrayValue &&
        dicomweb[tag][VALUE].size() == 1u)
    {
      return &dicomweb[tag][VALUE][0];
    }
    else
    {
      return NULL;
    }
  }


  void DicomInstanceParameters::EnrichUsingDicomWeb(const Json::Value& dicomweb)
  {
    /**
     * Use DICOM tag "SequenceOfUltrasoundRegions" (0018,6011) in
     * order to derive the pixel spacing on ultrasound (US) images
     **/
    
    if (!data_.hasPixelSpacing_)
    {
      const Json::Value* region = LookupDicomWebSingleValue(dicomweb, "00186011", "SQ");
      if (region != NULL)
      {
        const Json::Value* physicalUnitsXDirection = LookupDicomWebSingleValue(*region, "00186024", "US");
        const Json::Value* physicalUnitsYDirection = LookupDicomWebSingleValue(*region, "00186026", "US");
        const Json::Value* physicalDeltaX = LookupDicomWebSingleValue(*region, "0018602C", "FD");
        const Json::Value* physicalDeltaY = LookupDicomWebSingleValue(*region, "0018602E", "FD");
        
        if (physicalUnitsXDirection != NULL &&
            physicalUnitsYDirection != NULL &&
            physicalDeltaX != NULL &&
            physicalDeltaY != NULL &&
            physicalUnitsXDirection->type() == Json::intValue &&
            physicalUnitsYDirection->type() == Json::intValue &&
            physicalUnitsXDirection->asInt() == 0x0003 &&  // Centimeters
            physicalUnitsYDirection->asInt() == 0x0003 &&  // Centimeters
            physicalDeltaX->isNumeric() &&
            physicalDeltaY->isNumeric())
        {
          // Scene coordinates are expressed in millimeters => multiplication by 10
          SetPixelSpacing(10.0 * physicalDeltaX->asDouble(),
                          10.0 * physicalDeltaY->asDouble());
        }
      }
    }
  }
}
