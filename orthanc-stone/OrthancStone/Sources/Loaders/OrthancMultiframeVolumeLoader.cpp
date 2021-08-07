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


#include "OrthancMultiframeVolumeLoader.h"

#include <Endianness.h>
#include <Toolbox.h>

#if STONE_TIME_BLOCKING_OPS
# include <boost/date_time/posix_time/posix_time.hpp>
#endif

namespace OrthancStone
{
  class OrthancMultiframeVolumeLoader::LoadRTDoseGeometry : public LoaderStateMachine::State
  {
  private:
    std::unique_ptr<Orthanc::DicomMap>  dicom_;

  public:
    LoadRTDoseGeometry(OrthancMultiframeVolumeLoader& that,
                       Orthanc::DicomMap* dicom) :
      State(that),
      dicom_(dicom)
    {
      if (dicom == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }

    }

    virtual void Handle(const OrthancRestApiCommand::SuccessMessage& message) ORTHANC_OVERRIDE
    {
      // Complete the DICOM tags with just-received "Grid Frame Offset Vector"
      std::string s = Orthanc::Toolbox::StripSpaces(message.GetAnswer());
      dicom_->SetValue(Orthanc::DICOM_TAG_GRID_FRAME_OFFSET_VECTOR, s, false);

      GetLoader<OrthancMultiframeVolumeLoader>().SetGeometry(*dicom_);
    }      
  };


  static std::string GetSopClassUid(const Orthanc::DicomMap& dicom)
  {
    std::string s;
    if (!dicom.LookupStringValue(s, Orthanc::DICOM_TAG_SOP_CLASS_UID, false))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                      "DICOM file without SOP class UID");
    }
    else
    {
      return s;
    }
  }
    

  class OrthancMultiframeVolumeLoader::LoadGeometry : public State
  {
  public:
    explicit LoadGeometry(OrthancMultiframeVolumeLoader& that) :
      State(that)
    {
    }
      
    virtual void Handle(const OrthancRestApiCommand::SuccessMessage& message)
    {
      OrthancMultiframeVolumeLoader& loader = GetLoader<OrthancMultiframeVolumeLoader>();
        
      Json::Value body;
      message.ParseJsonBody(body);
        
      if (body.type() != Json::objectValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadJson, "JSON body should be an object value");
      }

      std::unique_ptr<Orthanc::DicomMap> dicom(new Orthanc::DicomMap);
      dicom->FromDicomAsJson(body);

      if (StringToSopClassUid(GetSopClassUid(*dicom)) == SopClassUid_RTDose)
      {
        // Download the "Grid Frame Offset Vector" DICOM tag, that is
        // mandatory for RT-DOSE, but is too long to be returned by default
          
        std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
        command->SetCallerName("OrthancMultiframeVolumeLoader::LoadGeometry");
        command->SetUri("/instances/" + loader.GetInstanceId() + "/content/" +
                        Orthanc::DICOM_TAG_GRID_FRAME_OFFSET_VECTOR.Format());
        command->AcquirePayload(new LoadRTDoseGeometry(loader, dicom.release()));

        Schedule(command.release());
      }
      else
      {
        loader.SetGeometry(*dicom);
      }
    }
  };

  class OrthancMultiframeVolumeLoader::LoadTransferSyntax : public State
  {
  public:
    explicit LoadTransferSyntax(OrthancMultiframeVolumeLoader& that) :
      State(that)
    {
    }
      
    virtual void Handle(const OrthancRestApiCommand::SuccessMessage& message)
    {
      GetLoader<OrthancMultiframeVolumeLoader>().SetTransferSyntax(message.GetAnswer());
    }
  };

  class OrthancMultiframeVolumeLoader::LoadUncompressedPixelData : public State
  {
  public:
    explicit LoadUncompressedPixelData(OrthancMultiframeVolumeLoader& that) :
      State(that)
    {
    }
      
    virtual void Handle(const OrthancRestApiCommand::SuccessMessage& message)
    {
      GetLoader<OrthancMultiframeVolumeLoader>().SetUncompressedPixelData(message.GetAnswer());
    }
  };

  const std::string& OrthancMultiframeVolumeLoader::GetInstanceId() const
  {
    if (IsActive())
    {
      return instanceId_;
    }
    else
    {
      LOG(ERROR) << "OrthancMultiframeVolumeLoader::GetInstanceId(): (!IsActive())";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
  }

  void OrthancMultiframeVolumeLoader::ScheduleFrameDownloads()
  {
    if (transferSyntaxUid_.empty() ||
        !volume_->HasGeometry())
    {
      return;
    }
    /*
      1.2.840.10008.1.2	Implicit VR Endian: Default Transfer Syntax for DICOM
      1.2.840.10008.1.2.1	Explicit VR Little Endian
      1.2.840.10008.1.2.2	Explicit VR Big Endian

      See https://www.dicomlibrary.com/dicom/transfer-syntax/
    */
    if (transferSyntaxUid_ == "1.2.840.10008.1.2" ||
        transferSyntaxUid_ == "1.2.840.10008.1.2.1" ||
        transferSyntaxUid_ == "1.2.840.10008.1.2.2")
    {
      std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
      command->SetCallerName("OrthancMultiframeVolumeLoader::ScheduleFrameDownloads");
      command->SetHttpHeader("Accept-Encoding", "gzip");
      command->SetUri("/instances/" + instanceId_ + "/content/" +
                      Orthanc::DICOM_TAG_PIXEL_DATA.Format() + "/0");
      command->AcquirePayload(new LoadUncompressedPixelData(*this));
      Schedule(command.release());
    }
    else
    {
      throw Orthanc::OrthancException(
        Orthanc::ErrorCode_NotImplemented,
        "No support for multiframe instances with transfer syntax: " + transferSyntaxUid_);
    }
  }

  void OrthancMultiframeVolumeLoader::SetTransferSyntax(const std::string& transferSyntax)
  {
    transferSyntaxUid_ = Orthanc::Toolbox::StripSpaces(transferSyntax);
    ScheduleFrameDownloads();
  }

  void OrthancMultiframeVolumeLoader::SetGeometry(const Orthanc::DicomMap& dicom)
  {
    DicomInstanceParameters parameters(dicom);
    volume_->SetDicomParameters(parameters);
      
    Orthanc::PixelFormat format;
    if (!parameters.GetImageInformation().ExtractPixelFormat(format, true))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }

    double spacingZ;
    switch (parameters.GetSopClassUid())
    {
      case SopClassUid_RTDose:
        spacingZ = parameters.GetSliceThickness();
        break;

      default:
        throw Orthanc::OrthancException(
          Orthanc::ErrorCode_NotImplemented,
          "No support for multiframe instances with SOP class UID: " + GetSopClassUid(dicom));
    }

    const unsigned int width = parameters.GetImageInformation().GetWidth();
    const unsigned int height = parameters.GetImageInformation().GetHeight();
    const unsigned int depth = parameters.GetImageInformation().GetNumberOfFrames();

    {
      VolumeImageGeometry geometry;
      geometry.SetSizeInVoxels(width, height, depth);
      geometry.SetAxialGeometry(parameters.GetGeometry());
      geometry.SetVoxelDimensions(parameters.GetPixelSpacingX(),
                                  parameters.GetPixelSpacingY(), spacingZ);
      volume_->Initialize(geometry, format, true /* Do compute range */);
    }

    volume_->GetPixelData().Clear();

    ScheduleFrameDownloads();



    BroadcastMessage(DicomVolumeImage::GeometryReadyMessage(*volume_));
  }


  ORTHANC_FORCE_INLINE
  static void CopyPixel(uint32_t& target, const void* source)
  {
    // TODO - check alignement?
    target = le32toh(*reinterpret_cast<const uint32_t*>(source));
  }

  ORTHANC_FORCE_INLINE
    static void CopyPixel(uint16_t& target, const void* source)
  {
    // TODO - check alignement?
    target = le16toh(*reinterpret_cast<const uint16_t*>(source));
  }

  ORTHANC_FORCE_INLINE
    static void CopyPixel(int16_t& target, const void* source)
  {
    // byte swapping is the same for unsigned and signed integers
    // (the sign bit is always stored with the MSByte)
    uint16_t* targetUp = reinterpret_cast<uint16_t*>(&target);
    CopyPixel(*targetUp, source);
  }

  template <typename T>
  void OrthancMultiframeVolumeLoader::CopyPixelDataAndComputeDistribution(
    const std::string& pixelData, std::map<T, PixelCount>& distribution)
  {
#if STONE_TIME_BLOCKING_OPS
    boost::posix_time::ptime timerStart = boost::posix_time::microsec_clock::universal_time();
#endif

    ImageBuffer3D& target = volume_->GetPixelData();
      
    const unsigned int bpp = target.GetBytesPerPixel();
    const unsigned int width = target.GetWidth();
    const unsigned int height = target.GetHeight();
    const unsigned int depth = target.GetDepth();

    if (pixelData.size() != bpp * width * height * depth)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                      "The pixel data has not the proper size");
    }

    if (pixelData.empty())
    {
      return;
    }

    // first pass to initialize map
#if 0
    {
      const uint8_t* source = reinterpret_cast<const uint8_t*>(pixelData.c_str());

      for (unsigned int z = 0; z < depth; z++)
      {
        for (unsigned int y = 0; y < height; y++)
        {
          for (unsigned int x = 0; x < width; x++)
          {
            T value;
            CopyPixel(value, source);
            distribution[value] = 0;
            source += bpp;
          }
        }
      }
    }
#endif

    {
      const uint8_t* source = reinterpret_cast<const uint8_t*>(pixelData.c_str());

      for (unsigned int z = 0; z < depth; z++)
      {
        ImageBuffer3D::SliceWriter writer(target, VolumeProjection_Axial, z);

        assert(writer.GetAccessor().GetWidth() == width &&
          writer.GetAccessor().GetHeight() == height);
#if 0
        for (unsigned int y = 0; y < height; y++)
        {
          assert(sizeof(T) == Orthanc::GetBytesPerPixel(target.GetFormat()));

          T* target = reinterpret_cast<T*>(writer.GetAccessor().GetRow(y));

          for (unsigned int x = 0; x < width; x++)
          {
            CopyPixel(*target, source);

            distribution[*target] += 1;

            target++;
            source += bpp;
          }
        }
#else
        // optimized version (fixed) as of 2020-04-15
        unsigned int pitch = writer.GetAccessor().GetPitch();
        T* targetAddrLine = reinterpret_cast<T*>(writer.GetAccessor().GetRow(0));
        assert(sizeof(T) == Orthanc::GetBytesPerPixel(target.GetFormat()));

        for (unsigned int y = 0; y < height; y++)
        {
          T* targetAddrPix = targetAddrLine;
          for (unsigned int x = 0; x < width; x++)
          {
            CopyPixel(*targetAddrPix, source);

            distribution[*targetAddrPix].count_ += 1;

            targetAddrPix++;
            source += bpp;
          }
          uint8_t* targetAddrLineBytes = reinterpret_cast<uint8_t*>(targetAddrLine) + pitch;
          targetAddrLine = reinterpret_cast<T*>(targetAddrLineBytes);
        }
#endif
      }
    }
#if STONE_TIME_BLOCKING_OPS
    boost::posix_time::ptime timerEnd = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration duration = timerEnd - timerStart;
    int64_t durationMs = duration.total_milliseconds();
    LOG(WARNING) << "OrthancMultiframeVolumeLoader::CopyPixelDataAndComputeDistribution took " << durationMs << " ms";
#endif
  }

  template <typename T>
  void OrthancMultiframeVolumeLoader::ComputeMinMaxWithOutlierRejection(
    const std::map<T, PixelCount>& distribution)
  {
    if (distribution.size() == 0)
    {
      LOG(ERROR) << "ComputeMinMaxWithOutlierRejection -- Volume image empty.";
    }
    else
    {
      ImageBuffer3D& target = volume_->GetPixelData();

      const uint64_t width = target.GetWidth();
      const uint64_t height = target.GetHeight();
      const uint64_t depth = target.GetDepth();
      const uint64_t voxelCount = width * height * depth;

      // now that we have distribution[pixelValue] == numberOfPixelsWithValue
      // compute number of values and check (assertion) that it is equal to 
      // width * height * depth 
      {
        typename std::map<T, PixelCount>::const_iterator it = distribution.begin();
        uint64_t totalCount = 0;
        distributionRawMin_ = static_cast<float>(it->first);

        while (it != distribution.end())
        {
          T pixelValue = it->first;
          totalCount += it->second.count_;
          ++it;
          if (it == distribution.end())
            distributionRawMax_ = static_cast<float>(pixelValue);
        }
        LOG(INFO) << "Volume image. First distribution value = " 
          << static_cast<float>(distributionRawMin_) 
          << " | Last distribution value = " 
          << static_cast<float>(distributionRawMax_);

        if (totalCount != voxelCount)
        {
          LOG(ERROR) << "Internal error in dose distribution computation. TC (" 
            << totalCount << ") != VoxC (" << voxelCount;
          throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
        }
      }

      // compute the number of voxels to reject at each end of the distribution
      uint64_t endRejectionCount = static_cast<uint64_t>(
        outliersHalfRejectionRate_ * voxelCount);

      if (endRejectionCount > voxelCount)
      {
        LOG(ERROR) << "Internal error in dose distribution computation."
          << " endRejectionCount = " << endRejectionCount
          << " | voxelCount = " << voxelCount;
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }

      // this will contain the actual distribution minimum after outlier 
      // rejection
      T resultMin = 0;

      // then start from start and remove pixel values up to 
      // endRejectionCount voxels rejected
      {
        typename std::map<T, PixelCount>::const_iterator it = distribution.begin();
        
        uint64_t currentCount = 0;

        while (it != distribution.end())
        {
          T pixelValue = it->first;
          uint64_t count = it->second.count_;

          // if this pixelValue crosses the rejection threshold, let's set it
          // and exit the loop
          if ((currentCount <= endRejectionCount) &&
              (currentCount + count > endRejectionCount))
          {
            resultMin = pixelValue;
            break;
          }
          else
          {
            currentCount += count;
          }
          // and continue walking along the distribution
          ++it;
        }
      }

      // this will contain the actual distribution maximum after outlier 
      // rejection
      T resultMax = 0;
      // now start from END and remove pixel values up to 
      // endRejectionCount voxels rejected
      {
        typename std::map<T, PixelCount>::const_reverse_iterator it = distribution.rbegin();

        uint64_t currentCount = 0;

        while (it != distribution.rend())
        {
          T pixelValue = it->first;
          uint64_t count = it->second.count_;

          if ((currentCount <= endRejectionCount) &&
              (currentCount + count > endRejectionCount))
          {
            resultMax = pixelValue;
            break;
          }
          else
          {
            currentCount += count;
          }
          // and continue walking along the distribution
          ++it;
        }
      }
      if (resultMin > resultMax)
      {
        LOG(ERROR) << "Internal error in dose distribution computation! " << 
          "resultMin (" << resultMin << ") > resultMax (" << resultMax << ")";
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
      computedDistributionMin_ = static_cast<float>(resultMin);
      computedDistributionMax_ = static_cast<float>(resultMax);
    }
  }

  template <typename T>
  void OrthancMultiframeVolumeLoader::CopyPixelDataAndComputeMinMax(
    const std::string& pixelData)
  {
    std::map<T, PixelCount> distribution;
    CopyPixelDataAndComputeDistribution(pixelData, distribution);
    ComputeMinMaxWithOutlierRejection(distribution);
  }

  void OrthancMultiframeVolumeLoader::SetUncompressedPixelData(const std::string& pixelData)
  {
    switch (volume_->GetPixelData().GetFormat())
    {
      case Orthanc::PixelFormat_Grayscale32:
        CopyPixelDataAndComputeMinMax<uint32_t>(pixelData);
        break;
      case Orthanc::PixelFormat_Grayscale16:
        CopyPixelDataAndComputeMinMax<uint16_t>(pixelData);
        break;
      case Orthanc::PixelFormat_SignedGrayscale16:
        CopyPixelDataAndComputeMinMax<int16_t>(pixelData);
        break;
      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }

    volume_->IncrementRevision();

    pixelDataLoaded_ = true;
    BroadcastMessage(DicomVolumeImage::ContentUpdatedMessage(*volume_));
  }
  
  bool OrthancMultiframeVolumeLoader::HasGeometry() const
  {
    return volume_->HasGeometry();
  }

  const VolumeImageGeometry& OrthancMultiframeVolumeLoader::GetImageGeometry() const
  {
    return volume_->GetGeometry();
  }

  OrthancMultiframeVolumeLoader::OrthancMultiframeVolumeLoader(
    ILoadersContext& loadersContext,
    boost::shared_ptr<DicomVolumeImage> volume,
    float outliersHalfRejectionRate) 
    : LoaderStateMachine(loadersContext)
    , volume_(volume)
    , pixelDataLoaded_(false)
    , outliersHalfRejectionRate_(outliersHalfRejectionRate)
    , distributionRawMin_(0)
    , distributionRawMax_(0)
    , computedDistributionMin_(0)
    , computedDistributionMax_(0)
  {
    if (volume.get() == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
  }


  boost::shared_ptr<OrthancMultiframeVolumeLoader>
    OrthancMultiframeVolumeLoader::Create(
      ILoadersContext& loadersContext, 
      boost::shared_ptr<DicomVolumeImage> volume, 
      float outliersHalfRejectionRate /*= 0.0005*/)
  {
    boost::shared_ptr<OrthancMultiframeVolumeLoader> obj(
      new OrthancMultiframeVolumeLoader(
        loadersContext,
        volume,
        outliersHalfRejectionRate));
    obj->LoaderStateMachine::PostConstructor();
    return obj;
  }

  OrthancMultiframeVolumeLoader::~OrthancMultiframeVolumeLoader()
  {
    LOG(TRACE) << "OrthancMultiframeVolumeLoader::~OrthancMultiframeVolumeLoader()";
  }
  
  void OrthancMultiframeVolumeLoader::GetDistributionMinMax
  (float& minValue, float& maxValue) const
  {
    if (distributionRawMin_ == 0 && distributionRawMax_ == 0)
    {
      LOG(WARNING) << "GetDistributionMinMaxWithOutliersRejection called before computation!";
    }
    minValue = distributionRawMin_;
    maxValue = distributionRawMax_;
  }
  
  void OrthancMultiframeVolumeLoader::GetDistributionMinMaxWithOutliersRejection
    (float& minValue, float& maxValue) const
  {
    if (computedDistributionMin_ == 0 && computedDistributionMax_ == 0)
    {
      LOG(WARNING) << "GetDistributionMinMaxWithOutliersRejection called before computation!";
    }
    minValue = computedDistributionMin_;
    maxValue = computedDistributionMax_;
  }

  void OrthancMultiframeVolumeLoader::LoadInstance(const std::string& instanceId)
  {
    Start();

    instanceId_ = instanceId;

    {
      std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
      command->SetCallerName("OrthancMultiframeVolumeLoader::LoadInstance");
      command->SetHttpHeader("Accept-Encoding", "gzip");
      command->SetUri("/instances/" + instanceId + "/tags");
      command->AcquirePayload(new LoadGeometry(*this));
      Schedule(command.release());
    }

    {
      std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
      command->SetCallerName("OrthancMultiframeVolumeLoader::LoadInstance");
      command->SetUri("/instances/" + instanceId + "/metadata/TransferSyntax");
      command->AcquirePayload(new LoadTransferSyntax(*this));
      Schedule(command.release());
    }
  }
}
