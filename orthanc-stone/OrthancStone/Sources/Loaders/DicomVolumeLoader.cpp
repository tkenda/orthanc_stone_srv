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


#include "DicomVolumeLoader.h"

#include <Images/ImageProcessing.h>

namespace OrthancStone
{
  DicomVolumeLoader::DicomVolumeLoader(boost::shared_ptr<SeriesFramesLoader>& framesLoader,
                                       bool computeRange) :
    framesLoader_(framesLoader),
    isValid_(false),
    started_(false),
    remaining_(0)
  {
    volume_.reset(new DicomVolumeImage);

    const SeriesOrderedFrames& frames = framesLoader_->GetOrderedFrames();

    if (frames.IsRegular3DVolume() &&
        frames.GetFramesCount() > 0)
    {
      // TODO - Is "0" the good choice for the reference frame?
      // Shouldn't we use "count - 1" depending on the direction
      // of the normal?
      const DicomInstanceParameters& parameters = frames.GetInstanceParameters(0);

      CoordinateSystem3D plane(frames.GetInstance(0));

      VolumeImageGeometry geometry;
      geometry.SetSizeInVoxels(parameters.GetImageInformation().GetWidth(),
                               parameters.GetImageInformation().GetHeight(),
                               static_cast<unsigned int>(frames.GetFramesCount()));
      geometry.SetAxialGeometry(plane);

      double spacing;
      if (parameters.GetSopClassUid() == SopClassUid_RTDose)
      {
        if (!parameters.ComputeRegularSpacing(spacing))
        {
          LOG(WARNING) << "Unable to compute the spacing in a RT-DOSE instance";
          spacing = frames.GetSpacingBetweenSlices();
        }
      }
      else
      {
        spacing = frames.GetSpacingBetweenSlices();
      }

      geometry.SetVoxelDimensions(parameters.GetPixelSpacingX(),
                                  parameters.GetPixelSpacingY(), spacing);
      volume_->Initialize(geometry, parameters.GetExpectedPixelFormat(), computeRange);
      volume_->GetPixelData().Clear();
      volume_->SetDicomParameters(parameters);

      remaining_ = frames.GetFramesCount();
      isValid_ = true;
    }
    else
    {
      LOG(WARNING) << "Not a regular 3D volume";
    }
  }


  void DicomVolumeLoader::Handle(const SeriesFramesLoader::FrameLoadedMessage& message)
  {
    if (remaining_ == 0 ||
        !message.HasUserPayload())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    if (message.GetImage().GetWidth() != volume_->GetPixelData().GetWidth() ||
        message.GetImage().GetHeight() != volume_->GetPixelData().GetHeight())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageSize);
    }

    if (message.GetImage().GetFormat() != volume_->GetPixelData().GetFormat())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    if (message.GetFrameIndex() >= volume_->GetPixelData().GetDepth())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    size_t frameIndex = dynamic_cast<const Orthanc::SingleValueObject<size_t>&>
    (message.GetUserPayload()).GetValue();

    {
      ImageBuffer3D::SliceWriter writer(volume_->GetPixelData(), 
                                        VolumeProjection_Axial, 
                                        static_cast<unsigned int>(frameIndex));
      
      Orthanc::ImageProcessing::Copy(writer.GetAccessor(), message.GetImage());
    }

    volume_->IncrementRevision();

    {
      VolumeUpdatedMessage updated(*this, 
                                   static_cast<unsigned int>(frameIndex));
      
      BroadcastMessage(updated);
    }

    remaining_--;

    if (remaining_ == 0)
    {
      VolumeReadyMessage ready(*this);
      BroadcastMessage(ready);
    }
  }


  DicomVolumeLoader::Factory::Factory(LoadedDicomResources& instances) :
    framesFactory_(instances),
    computeRange_(false)
  {
  }

  DicomVolumeLoader::Factory::Factory(const SeriesMetadataLoader::SuccessMessage& metadata) :
    framesFactory_(metadata.GetInstances()),
    computeRange_(false)
  {
    SetDicomDir(metadata.GetDicomDirPath(), metadata.GetDicomDir());  // Only useful for DICOMDIR sources
  }


  boost::shared_ptr<IObserver> DicomVolumeLoader::Factory::Create(ILoadersContext::ILock& context)
  { 
    boost::shared_ptr<SeriesFramesLoader> frames =
      boost::dynamic_pointer_cast<SeriesFramesLoader>(framesFactory_.Create(context));

    boost::shared_ptr<DicomVolumeLoader> volume(new DicomVolumeLoader(frames, computeRange_));
    volume->Register<SeriesFramesLoader::FrameLoadedMessage>(*frames, &DicomVolumeLoader::Handle);

    return volume;
  }

  void DicomVolumeLoader::Start(int priority,
                                const DicomSource& source)
  {
    if (started_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }

    started_ = true;

    if (IsValid())
    {
      for (size_t i = 0; i < GetOrderedFrames().GetFramesCount(); i++)
      {
        framesLoader_->ScheduleLoadFrame(priority, source, i, source.GetQualityCount() - 1,
                                         new Orthanc::SingleValueObject<size_t>(i));
      }
    }
    else
    {
      VolumeReadyMessage ready(*this);
      BroadcastMessage(ready);
    }
  }
}
