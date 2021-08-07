/**
 * Stone of Orthanc
 * Copyright (C) 2012-2016 Sebastien Jodogne, Medical Physics
 * Department, University Hospital of Liege, Belgium
 * Copyright (C) 2017-2021 Osimis S.A., Belgium
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License
 * as published by the Free Software Foundation, either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/


#pragma once

#include "../../../../OrthancStone/Sources/Loaders/DicomResourcesLoader.h"
#include "../../../../OrthancStone/Sources/Loaders/ILoadersContext.h"
#include "../../../../OrthancStone/Sources/Loaders/SeriesFramesLoader.h"
#include "../../../../OrthancStone/Sources/Loaders/SeriesThumbnailsLoader.h"
#include "../../../../OrthancStone/Sources/Scene2DViewport/ViewportController.h"
#include "../../../../OrthancStone/Sources/Viewport/IViewport.h"

#include <Compatibility.h>  // For std::unique_ptr<>

#include <boost/make_shared.hpp>


using OrthancStone::ILoadersContext;
using OrthancStone::ObserverBase;
using OrthancStone::IViewport;
using OrthancStone::DicomResourcesLoader;
using OrthancStone::SeriesFramesLoader;
using OrthancStone::TextureBaseSceneLayer;
using OrthancStone::DicomSource;
using OrthancStone::SeriesThumbnailsLoader;
using OrthancStone::LoadedDicomResources;
using OrthancStone::SeriesThumbnailType;
using OrthancStone::OracleScheduler;
using OrthancStone::OrthancRestApiCommand;
using OrthancStone::OracleScheduler;
using OrthancStone::OracleScheduler;
using OrthancStone::OracleScheduler;


class SdlSimpleViewerApplication : public ObserverBase<SdlSimpleViewerApplication>
{

public:
  static boost::shared_ptr<SdlSimpleViewerApplication> Create(ILoadersContext& context, boost::shared_ptr<IViewport> viewport)
  {
    boost::shared_ptr<SdlSimpleViewerApplication> application(new SdlSimpleViewerApplication(context, viewport));

    {
      std::unique_ptr<ILoadersContext::ILock> lock(context.Lock());
      application->dicomLoader_ = DicomResourcesLoader::Create(*lock);
    }

    application->Register<DicomResourcesLoader::SuccessMessage>(*application->dicomLoader_, &SdlSimpleViewerApplication::Handle);

    return application;
  }

  void LoadOrthancFrame(const DicomSource& source, const std::string& instanceId, unsigned int frame)
  {
    std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());

    dicomLoader_->ScheduleLoadOrthancResource(boost::make_shared<LoadedDicomResources>(Orthanc::DICOM_TAG_SOP_INSTANCE_UID),
                                              0, source, Orthanc::ResourceType_Instance, instanceId,
                                              new Orthanc::SingleValueObject<unsigned int>(frame));
  }

#if 0
  void LoadDicomWebFrame(const DicomSource& source,
                         const std::string& studyInstanceUid,
                         const std::string& seriesInstanceUid,
                         const std::string& sopInstanceUid,
                         unsigned int frame)
  {
    std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());

    // We first must load the "/metadata" to know the number of frames
    dicomLoader_->ScheduleGetDicomWeb(
      boost::make_shared<LoadedDicomResources>(Orthanc::DICOM_TAG_SOP_INSTANCE_UID), 0, source,
      "/studies/" + studyInstanceUid + "/series/" + seriesInstanceUid + "/instances/" + sopInstanceUid + "/metadata",
      new Orthanc::SingleValueObject<unsigned int>(frame));
  }
#endif 

  void FitContent()
  {
    std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
    lock->GetCompositor().FitContent(lock->GetController().GetScene());
    lock->Invalidate();
  }

  OrthancStone::Units GetUnits() const
  {
    return units_;
  }

private:
  ILoadersContext& context_;
  boost::shared_ptr<IViewport>             viewport_;
  boost::shared_ptr<DicomResourcesLoader>  dicomLoader_;
  boost::shared_ptr<SeriesFramesLoader>    framesLoader_;
  OrthancStone::Units                      units_;

  SdlSimpleViewerApplication(ILoadersContext& context,
                             boost::shared_ptr<IViewport> viewport) :
    context_(context),
    viewport_(viewport),
    units_(OrthancStone::Units_Pixels)
  {
  }

  void Handle(const SeriesFramesLoader::FrameLoadedMessage& message)
  {
    LOG(INFO) << "Frame decoded! "
      << message.GetImage().GetWidth() << "x" << message.GetImage().GetHeight()
      << " " << Orthanc::EnumerationToString(message.GetImage().GetFormat());

    std::unique_ptr<TextureBaseSceneLayer> layer(
      message.GetInstanceParameters().CreateTexture(message.GetImage()));
    layer->SetLinearInterpolation(true);
    layer->SetPixelSpacing(message.GetInstanceParameters().GetPixelSpacingX(),
                           message.GetInstanceParameters().GetPixelSpacingY());

    {
      std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
      lock->GetController().GetScene().SetLayer(0, layer.release());
      lock->GetCompositor().FitContent(lock->GetController().GetScene());
      lock->Invalidate();
    }
  }

  void Handle(const DicomResourcesLoader::SuccessMessage& message)
  {
    if (message.GetResources()->GetSize() != 1)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }

    OrthancStone::DicomInstanceParameters parameters(message.GetResources()->GetResource(0));
    if (parameters.HasPixelSpacing())
    {
      /**
       * TODO - Ultrasound (US) images store an equivalent to
       * "PixelSpacing" in the "SequenceOfUltrasoundRegions"
       * (0018,6011) sequence, cf. tags "PhysicalDeltaX" (0018,602c)
       * and "PhysicalDeltaY" (0018,602e) => This would require
       * parsing "message.GetResources()->GetSourceJson(0)"
       * => cf. "DicomInstanceParameters::EnrichUsingDicomWeb()"
       **/

      // std::cout << message.GetResources()->GetSourceJson(0).toStyledString();
      
      LOG(INFO) << "Using millimeters units, as the DICOM instance contains the PixelSpacing tag";
      units_ = OrthancStone::Units_Millimeters;
    }
    else
    {
      LOG(INFO) << "Using pixels units, as the DICOM instance does *not* contain the PixelSpacing tag";
    }
    
    //message.GetResources()->GetResource(0).Print(stdout);

    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
      SeriesFramesLoader::Factory f(*message.GetResources());

      framesLoader_ = boost::dynamic_pointer_cast<SeriesFramesLoader>(
        f.Create(*lock));
      
      Register<SeriesFramesLoader::FrameLoadedMessage>(
        *framesLoader_, &SdlSimpleViewerApplication::Handle);

      assert(message.HasUserPayload());

      const Orthanc::SingleValueObject<unsigned int>& payload =
        dynamic_cast<const Orthanc::SingleValueObject<unsigned int>&>(
          message.GetUserPayload());

      LOG(INFO) << "Loading pixel data of frame: " << payload.GetValue();
      framesLoader_->ScheduleLoadFrame(
        0, message.GetDicomSource(), payload.GetValue(),
        message.GetDicomSource().GetQualityCount() - 1 /* download best quality available */,
        NULL);
    }
  }

};

