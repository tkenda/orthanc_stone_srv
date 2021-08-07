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


#include "OrthancSeriesVolumeProgressiveLoader.h"

#include "../StoneException.h"
#include "../Loaders/ILoadersContext.h"
#include "../Loaders/BasicFetchingItemsSorter.h"
#include "../Loaders/BasicFetchingStrategy.h"
#include "../Toolbox/GeometryToolbox.h"
#include "../Volumes/DicomVolumeImageMPRSlicer.h"

#include <Compatibility.h>
#include <Images/ImageProcessing.h>
#include <OrthancException.h>

#if STONE_TIME_BLOCKING_OPS
# include <boost/date_time/posix_time/posix_time.hpp>
#endif

namespace OrthancStone
{
  class OrthancSeriesVolumeProgressiveLoader::ExtractedSlice : public DicomVolumeImageMPRSlicer::Slice
  {
  private:
    const OrthancSeriesVolumeProgressiveLoader&  that_;

  public:
    ExtractedSlice(const OrthancSeriesVolumeProgressiveLoader& that,
                   const CoordinateSystem3D& plane) :
      DicomVolumeImageMPRSlicer::Slice(*that.volume_, plane),
      that_(that)
    {
      if (IsValid())
      {
        if (GetProjection() == VolumeProjection_Axial)
        {
          // For coronal and sagittal projections, we take the global
          // revision of the volume because even if a single slice changes,
          // this means the projection will yield a different result --> 
          // we must increase the revision as soon as any slice changes 
          SetRevision(that_.seriesGeometry_.GetSliceRevision(GetSliceIndex()));
        }
      
        if (that_.strategy_.get() != NULL &&
            GetProjection() == VolumeProjection_Axial)
        {
          that_.strategy_->SetCurrent(GetSliceIndex());
        }
      }
    }
  };
    
  void OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::CheckSlice(
    size_t index, const DicomInstanceParameters& reference) const
  {
    const DicomInstanceParameters& slice = *slices_[index];
      
    if (!GeometryToolbox::IsParallel(
          reference.GetGeometry().GetNormal(),
          slice.GetGeometry().GetNormal()))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadGeometry,
                                      "A slice in the volume image is not parallel to the others");
    }

    if (reference.GetExpectedPixelFormat() != slice.GetExpectedPixelFormat())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat,
                                      "The pixel format changes across the slices of the volume image");
    }

    if (reference.GetImageInformation().GetWidth() != slice.GetImageInformation().GetWidth() ||
        reference.GetImageInformation().GetHeight() != slice.GetImageInformation().GetHeight())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageSize,
                                      "The width/height of slices are not constant in the volume image");
    }

    if (!LinearAlgebra::IsNear(reference.GetPixelSpacingX(), slice.GetPixelSpacingX()) ||
        !LinearAlgebra::IsNear(reference.GetPixelSpacingY(), slice.GetPixelSpacingY()))
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadGeometry,
                                      "The pixel spacing of the slices change across the volume image");
    }
  }

    
  void OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::CheckVolume() const
  {
    for (size_t i = 0; i < slices_.size(); i++)
    {
      assert(slices_[i] != NULL);
      if (slices_[i]->GetImageInformation().GetNumberOfFrames() != 1)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadGeometry,
                                        "This class does not support multi-frame images");
      }
    }

    if (slices_.size() != 0)
    {
      const DicomInstanceParameters& reference = *slices_[0];

      for (size_t i = 1; i < slices_.size(); i++)
      {
        CheckSlice(i, reference);
      }
    }
  }


  void OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::Clear()
  {
    for (size_t i = 0; i < slices_.size(); i++)
    {
      assert(slices_[i] != NULL);
      delete slices_[i];
    }

    slices_.clear();
    slicesRevision_.clear();
  }


  void OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::CheckSliceIndex(size_t index) const
  {
    if (!HasGeometry())
    {
      LOG(ERROR) << "OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::CheckSliceIndex(size_t index): (!HasGeometry())";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else if (index >= slices_.size())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      assert(slices_.size() == GetImageGeometry().GetDepth() &&
             slices_.size() == slicesRevision_.size());
    }
  }


  // WARNING: The payload of "slices" must be of class "DicomInstanceParameters"
  // (called with the slices created in LoadGeometry)
  void OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::ComputeGeometry(SlicesSorter& slices)
  {
    Clear();
      
    if (!slices.Sort())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange,
                                      "Cannot sort the 3D slices of a DICOM series");          
    }

    if (slices.GetSlicesCount() == 0)
    {
      geometry_.reset(new VolumeImageGeometry);
    }
    else
    {
      slices_.reserve(slices.GetSlicesCount());
      slicesRevision_.resize(slices.GetSlicesCount(), 0);

      for (size_t i = 0; i < slices.GetSlicesCount(); i++)
      {
        const DicomInstanceParameters& slice =
          dynamic_cast<const DicomInstanceParameters&>(slices.GetSlicePayload(i));
        slices_.push_back(new DicomInstanceParameters(slice));
      }

      CheckVolume();

      double spacingZ;

      if (slices.ComputeSpacingBetweenSlices(spacingZ))
      {
        LOG(TRACE) << "Computed spacing between slices: " << spacingZ << "mm";
      
        const DicomInstanceParameters& parameters = *slices_[0];

        geometry_.reset(new VolumeImageGeometry);
        geometry_->SetSizeInVoxels(parameters.GetImageInformation().GetWidth(),
                                   parameters.GetImageInformation().GetHeight(),
                                   static_cast<unsigned int>(slices.GetSlicesCount()));
        geometry_->SetAxialGeometry(slices.GetSliceGeometry(0));
        geometry_->SetVoxelDimensions(parameters.GetPixelSpacingX(),
                                      parameters.GetPixelSpacingY(), spacingZ);
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadGeometry,
                                        "The origins of the slices of a volume image are not regularly spaced");
     }
    }
  }


  const VolumeImageGeometry& OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::GetImageGeometry() const
  {
    if (!HasGeometry())
    {
      LOG(ERROR) << "OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::GetImageGeometry(): (!HasGeometry())";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      assert(slices_.size() == geometry_->GetDepth());
      return *geometry_;
    }
  }


  const DicomInstanceParameters& OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::GetSliceParameters(size_t index) const
  {
    CheckSliceIndex(index);
    return *slices_[index];
  }


  uint64_t OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::GetSliceRevision(size_t index) const
  {
    CheckSliceIndex(index);
    return slicesRevision_[index];
  }


  void OrthancSeriesVolumeProgressiveLoader::SeriesGeometry::IncrementSliceRevision(size_t index)
  {
    CheckSliceIndex(index);
    slicesRevision_[index] ++;
  }


  static unsigned int GetSliceIndexPayload(const OracleCommandBase& command)
  {
    assert(command.HasPayload());
    return dynamic_cast< const Orthanc::SingleValueObject<unsigned int>& >(command.GetPayload()).GetValue();
  }


  void OrthancSeriesVolumeProgressiveLoader::ScheduleNextSliceDownload()
  {
    assert(strategy_.get() != NULL);
      
    unsigned int sliceIndex = 0, quality = 0;
      
    if (strategy_->GetNext(sliceIndex, quality))
    {
      if (!progressiveQuality_)
      {
        ORTHANC_ASSERT(quality == QUALITY_00, "INTERNAL ERROR. quality != QUALITY_00 in "
                       << "OrthancSeriesVolumeProgressiveLoader::ScheduleNextSliceDownload");
      }

      const DicomInstanceParameters& slice = seriesGeometry_.GetSliceParameters(sliceIndex);
          
      const std::string& instance = slice.GetOrthancInstanceIdentifier();
      if (instance.empty())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }

      std::unique_ptr<OracleCommandBase> command;
        
      if (!progressiveQuality_ || quality == QUALITY_02)
      {
        std::unique_ptr<GetOrthancImageCommand> tmp(new GetOrthancImageCommand);
        // TODO: review the following comment. 
        // - Commented out by bgo on 2019-07-19 | reason: Alain has seen cases 
        //   where gzipping the uint16 image took 11 sec to produce 5mb. 
        //   The unzipped request was much much faster.
        // - Re-enabled on 2019-07-30. Reason: in Web Assembly, the browser 
        //   does not use the Accept-Encoding header and always requests
        //   compression. Furthermore, NOT 
        tmp->SetHttpHeader("Accept-Encoding", "gzip");
        tmp->SetHttpHeader("Accept", std::string(Orthanc::EnumerationToString(Orthanc::MimeType_Pam)));
        tmp->SetInstanceUri(instance, slice.GetExpectedPixelFormat());
        tmp->SetExpectedPixelFormat(slice.GetExpectedPixelFormat());
        //LOG(INFO) 
        //  << "OrthancSeriesVolumeProgressiveLoader.ScheduleNextSliceDownload()"
        //  << " sliceIndex = " << sliceIndex << " slice quality = " << quality 
        //  << " URI = " << tmp->GetUri();
        command.reset(tmp.release());
      }
      else // progressive mode is true AND quality is not final (different from QUALITY_02
      {
        std::unique_ptr<GetOrthancWebViewerJpegCommand> tmp(
          new GetOrthancWebViewerJpegCommand);

        // TODO: review the following comment. Commented out by bgo on 2019-07-19
        // (gzip for jpeg seems overkill)
        //tmp->SetHttpHeader("Accept-Encoding", "gzip");
        tmp->SetInstance(instance);
        tmp->SetQuality((quality == 0 ? 50 : 90)); // QUALITY_00 is Jpeg50 while QUALITY_01 is Jpeg90
        tmp->SetExpectedPixelFormat(slice.GetExpectedPixelFormat());
        LOG(TRACE)
          << "OrthancSeriesVolumeProgressiveLoader.ScheduleNextSliceDownload()"
          << " sliceIndex = " << sliceIndex << " slice quality = " << quality;
        command.reset(tmp.release());
      }

      command->AcquirePayload(new Orthanc::SingleValueObject<unsigned int>(sliceIndex));
      
      {
        std::unique_ptr<ILoadersContext::ILock> lock(loadersContext_.Lock());
        boost::shared_ptr<IObserver> observer(GetSharedObserver());
        lock->Schedule(observer, sliceSchedulingPriority_, command.release());
      }
    }
    else
    {
      // loading is finished!
      volumeImageReadyInHighQuality_ = true;
      BroadcastMessage(OrthancSeriesVolumeProgressiveLoader::VolumeImageReadyInHighQuality(*this));
    }
  }

/**
   This is called in response to GET "/series/XXXXXXXXXXXXX/instances-tags"
*/
  void OrthancSeriesVolumeProgressiveLoader::LoadGeometry(const OrthancRestApiCommand::SuccessMessage& message)
  {
#if STONE_TIME_BLOCKING_OPS
    boost::posix_time::ptime timerStart = boost::posix_time::microsec_clock::universal_time();
#endif

    Json::Value body;
    message.ParseJsonBody(body);
      
    if (body.type() != Json::objectValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadJson, "JSON body should be an object value");
    }

    {
      Json::Value::Members instances = body.getMemberNames();

      SlicesSorter slices;
        
      for (size_t i = 0; i < instances.size(); i++)
      {
        Orthanc::DicomMap dicom;
        dicom.FromDicomAsJson(body[instances[i]]);

        std::unique_ptr<DicomInstanceParameters> instance(new DicomInstanceParameters(dicom));
        instance->SetOrthancInstanceIdentifier(instances[i]);

        // the 3D plane corresponding to the slice
        CoordinateSystem3D geometry = instance->GetGeometry();
        slices.AddSlice(geometry, instance.release());

        if (slicePostProcessor_)
          slicePostProcessor_->ProcessCTDicomSlice(dicom);
      }

      seriesGeometry_.ComputeGeometry(slices);
    }

    size_t slicesCount = seriesGeometry_.GetImageGeometry().GetDepth();

    if (slicesCount == 0)
    {
      volume_->Initialize(seriesGeometry_.GetImageGeometry(), Orthanc::PixelFormat_Grayscale8);
    }
    else
    {
      const DicomInstanceParameters& parameters = seriesGeometry_.GetSliceParameters(0);
        
      volume_->Initialize(seriesGeometry_.GetImageGeometry(), parameters.GetExpectedPixelFormat());
      volume_->SetDicomParameters(parameters);
      volume_->GetPixelData().Clear();

      // If we are in progressive mode, the Fetching strategy will first request QUALITY_00, then QUALITY_01, then
      // QUALITY_02... Otherwise, it's only QUALITY_00
      unsigned int maxQuality = QUALITY_00;
      if (progressiveQuality_)
        maxQuality = QUALITY_02;

      unsigned int initialSlice = 0;
      if (startCenter_)
          initialSlice = static_cast<unsigned int>(slicesCount) / 2;

      strategy_.reset(new BasicFetchingStrategy(
        sorter_->CreateSorter(static_cast<unsigned int>(slicesCount)),
        maxQuality,
        initialSlice));

      assert(simultaneousDownloads_ != 0);
      for (unsigned int i = 0; i < simultaneousDownloads_; i++)
      {
        ScheduleNextSliceDownload();
      }
    }

    slicesQuality_.resize(slicesCount, 0);

    BroadcastMessage(DicomVolumeImage::GeometryReadyMessage(*volume_));
    
#if STONE_TIME_BLOCKING_OPS
      boost::posix_time::ptime timerEnd = boost::posix_time::microsec_clock::universal_time();
    boost::posix_time::time_duration duration = timerEnd - timerStart;
    int64_t durationMs = duration.total_milliseconds();
    LOG(WARNING) << "OrthancSeriesVolumeProgressiveLoader::LoadGeometry took " << durationMs << " ms";
#endif

  }


  void OrthancSeriesVolumeProgressiveLoader::SetSliceContent(unsigned int sliceIndex,
                                                             const Orthanc::ImageAccessor& image,
                                                             unsigned int quality)
  {
    ORTHANC_ASSERT(sliceIndex < slicesQuality_.size() &&
           slicesQuality_.size() == volume_->GetPixelData().GetDepth());
      
    if (!progressiveQuality_)
    {
      ORTHANC_ASSERT(quality                    == QUALITY_00);
      ORTHANC_ASSERT(slicesQuality_[sliceIndex] == QUALITY_00);
    }

    if (quality >= slicesQuality_[sliceIndex])
    {
      {
        ImageBuffer3D::SliceWriter writer(volume_->GetPixelData(), 
                                                        VolumeProjection_Axial, 
                                                        sliceIndex);
        
        Orthanc::ImageProcessing::Copy(writer.GetAccessor(), image);
      }

      volume_->IncrementRevision();
      seriesGeometry_.IncrementSliceRevision(sliceIndex);
      slicesQuality_[sliceIndex] = quality;

      BroadcastMessage(DicomVolumeImage::ContentUpdatedMessage(*volume_));
    }
    LOG(TRACE) << "SetSliceContent sliceIndex = " << sliceIndex << " -- will "
      << " now call ScheduleNextSliceDownload()";
    ScheduleNextSliceDownload();
  }

  void OrthancSeriesVolumeProgressiveLoader::LoadBestQualitySliceContent(
    const GetOrthancImageCommand::SuccessMessage& message)
  {
    unsigned int quality = QUALITY_00;
    if (progressiveQuality_)
      quality = QUALITY_02;

    SetSliceContent(GetSliceIndexPayload(message.GetOrigin()), 
                                         message.GetImage(),
                                         quality);
  }

  void OrthancSeriesVolumeProgressiveLoader::LoadJpegSliceContent(
    const GetOrthancWebViewerJpegCommand::SuccessMessage& message)
  {
    ORTHANC_ASSERT(progressiveQuality_, "INTERNAL ERROR: OrthancSeriesVolumeProgressiveLoader::LoadJpegSliceContent"
                   << " called while progressiveQuality_ is false!");

    LOG(TRACE) << "OrthancSeriesVolumeProgressiveLoader::LoadJpegSliceContent";
    unsigned int quality;
      
    switch (dynamic_cast<const GetOrthancWebViewerJpegCommand&>(message.GetOrigin()).GetQuality())
    {
      case 50:
        quality = QUALITY_00;
        break;

      case 90:
        quality = QUALITY_01;
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
      
    SetSliceContent(GetSliceIndexPayload(message.GetOrigin()), message.GetImage(), quality);
  }


  void  OrthancSeriesVolumeProgressiveLoader::SetMetadataSchedulingPriority(int p)
  {
    medadataSchedulingPriority_ = p;
  }

  int   OrthancSeriesVolumeProgressiveLoader::GetMetadataSchedulingPriority() const
  {
    return medadataSchedulingPriority_;
  }

  void  OrthancSeriesVolumeProgressiveLoader::SetSliceSchedulingPriority(int p)
  {
    sliceSchedulingPriority_ = p;
  }
    
  int   OrthancSeriesVolumeProgressiveLoader::GetSliceSchedulingPriority() const
  {
    return sliceSchedulingPriority_;
  }

  void  OrthancSeriesVolumeProgressiveLoader::SetSchedulingPriority(int p)
  {
    medadataSchedulingPriority_ = p;
    sliceSchedulingPriority_ = p;
  }

  OrthancSeriesVolumeProgressiveLoader::OrthancSeriesVolumeProgressiveLoader(
    ILoadersContext& loadersContext,
    boost::shared_ptr<DicomVolumeImage> volume,
    bool progressiveQuality)
    : loadersContext_(loadersContext)
    , active_(false)
    , progressiveQuality_(progressiveQuality)
    , startCenter_(false)
    , simultaneousDownloads_(4)
    , volume_(volume)
    , sorter_(new BasicFetchingItemsSorter::Factory)
    , volumeImageReadyInHighQuality_(false)
    , medadataSchedulingPriority_(0)
    , sliceSchedulingPriority_(0)
  {
  }

  boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> 
    OrthancSeriesVolumeProgressiveLoader::Create(
      ILoadersContext& loadersContext,
      boost::shared_ptr<DicomVolumeImage> volume,
      bool progressiveQuality)
  {
    std::unique_ptr<ILoadersContext::ILock> lock(loadersContext.Lock());

    boost::shared_ptr<OrthancSeriesVolumeProgressiveLoader> obj(
        new OrthancSeriesVolumeProgressiveLoader(
          loadersContext, volume, progressiveQuality));

    obj->Register<OrthancRestApiCommand::SuccessMessage>(
      lock->GetOracleObservable(),
      &OrthancSeriesVolumeProgressiveLoader::LoadGeometry);

    obj->Register<GetOrthancImageCommand::SuccessMessage>(
      lock->GetOracleObservable(),
      &OrthancSeriesVolumeProgressiveLoader::LoadBestQualitySliceContent);

    obj->Register<GetOrthancWebViewerJpegCommand::SuccessMessage>(
      lock->GetOracleObservable(),
      &OrthancSeriesVolumeProgressiveLoader::LoadJpegSliceContent);

    return obj;
  }


  OrthancSeriesVolumeProgressiveLoader::~OrthancSeriesVolumeProgressiveLoader()
  {
    LOG(TRACE) << "OrthancSeriesVolumeProgressiveLoader::~OrthancSeriesVolumeProgressiveLoader()";
  }

  void OrthancSeriesVolumeProgressiveLoader::SetStartCenter(bool startCenter)
  {
      startCenter_ = startCenter;
  }

  void OrthancSeriesVolumeProgressiveLoader::SetSimultaneousDownloads(unsigned int count)
  {
    if (active_)
    {
      LOG(ERROR) << "OrthancSeriesVolumeProgressiveLoader::SetSimultaneousDownloads(): (active_)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else if (count == 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);        
    }
    else
    {
      simultaneousDownloads_ = count;
    }
  }


  void OrthancSeriesVolumeProgressiveLoader::LoadSeries(const std::string& seriesId)
  {
    if (active_)
    {
      LOG(ERROR) << "OrthancSeriesVolumeProgressiveLoader::LoadSeries(const std::string& seriesId): (active_)";
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      active_ = true;

      std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
      command->SetUri("/series/" + seriesId + "/instances-tags");
      {
        std::unique_ptr<ILoadersContext::ILock> lock(loadersContext_.Lock());
        boost::shared_ptr<IObserver> observer(GetSharedObserver());
        lock->Schedule(observer, medadataSchedulingPriority_, command.release());
      }
    }
  }
  

  IVolumeSlicer::IExtractedSlice* 
  OrthancSeriesVolumeProgressiveLoader::ExtractSlice(const CoordinateSystem3D& cuttingPlane)
  {
    if (volume_->HasGeometry())
    {
      return new ExtractedSlice(*this, cuttingPlane);
    }
    else
    {
      return new IVolumeSlicer::InvalidSlice;
    }
  }
}
