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

#include <Compatibility.h>  // For std::unique_ptr<>

#include <boost/make_shared.hpp>


namespace OrthancStone
{
  class Application : public ObserverBase<Application>
  {
  private:
    ILoadersContext&                         context_;
    boost::shared_ptr<IViewport>             viewport_;
    boost::shared_ptr<DicomResourcesLoader>  dicomLoader_;
    boost::shared_ptr<SeriesFramesLoader>    framesLoader_;

    Application(ILoadersContext& context,
                boost::shared_ptr<IViewport> viewport) : 
      context_(context),
      viewport_(viewport)
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
   
      //message.GetResources()->GetResource(0).Print(stdout);

      {
        std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
        SeriesFramesLoader::Factory f(*message.GetResources());

        framesLoader_ = boost::dynamic_pointer_cast<SeriesFramesLoader>(f.Create(*lock));
        Register<SeriesFramesLoader::FrameLoadedMessage>(*framesLoader_, &Application::Handle);

        assert(message.HasUserPayload());
        const Orthanc::SingleValueObject<unsigned int>& payload =
          dynamic_cast<const Orthanc::SingleValueObject<unsigned int>&>(message.GetUserPayload());

        LOG(INFO) << "Loading pixel data of frame: " << payload.GetValue();
        framesLoader_->ScheduleLoadFrame(
          0, message.GetDicomSource(), payload.GetValue(),
          message.GetDicomSource().GetQualityCount() - 1 /* download best quality available */,
          NULL);
      }
    }

  public:
    static boost::shared_ptr<Application> Create(ILoadersContext& context,
                                                 boost::shared_ptr<IViewport> viewport)
    {
      boost::shared_ptr<Application> application(new Application(context, viewport));

      {
        std::unique_ptr<ILoadersContext::ILock> lock(context.Lock());
        application->dicomLoader_ = DicomResourcesLoader::Create(*lock);
      }

      application->Register<DicomResourcesLoader::SuccessMessage>(*application->dicomLoader_, &Application::Handle);

      return application;
    }

    void LoadOrthancFrame(const DicomSource& source,
                          const std::string& instanceId,
                          unsigned int frame)
    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());

      dicomLoader_->ScheduleLoadOrthancResource(
        boost::make_shared<LoadedDicomResources>(Orthanc::DICOM_TAG_SOP_INSTANCE_UID), 
        0, source, Orthanc::ResourceType_Instance, instanceId,
        new Orthanc::SingleValueObject<unsigned int>(frame));
    }

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

    void FitContent()
    {
      std::unique_ptr<IViewport::ILock> lock(viewport_->Lock());
      lock->GetCompositor().FitContent(lock->GetController().GetScene());
      lock->Invalidate();
    }
  };



  class IWebViewerLoadersObserver : public boost::noncopyable
  {
  public:
    virtual ~IWebViewerLoadersObserver()
    {
    }

    virtual void SignalSeriesUpdated(LoadedDicomResources& series) = 0;

    virtual void SignalThumbnailLoaded(const std::string& studyInstanceUid,
                                       const std::string& seriesInstanceUid,
                                       SeriesThumbnailType type) = 0;
  };
  

  class WebViewerLoaders : public ObserverBase<WebViewerLoaders>
  {
  private:
    static const int PRIORITY_ADD_RESOURCES = 0;
    static const int PRIORITY_THUMBNAILS = OracleScheduler::PRIORITY_LOW + 100;

    enum Type
    {
      Type_Orthanc = 1,
      Type_DicomWeb = 2
    };

    ILoadersContext&                           context_;
    std::unique_ptr<IWebViewerLoadersObserver>   observer_;
    bool                                       loadThumbnails_;
    DicomSource                                source_;
    std::set<std::string>                      scheduledSeries_;
    std::set<std::string>                      scheduledThumbnails_;
    std::set<std::string>                      scheduledStudies_;
    boost::shared_ptr<LoadedDicomResources>    loadedSeries_;
    boost::shared_ptr<LoadedDicomResources>    loadedStudies_;
    boost::shared_ptr<DicomResourcesLoader>    resourcesLoader_;
    boost::shared_ptr<SeriesThumbnailsLoader>  thumbnailsLoader_;

    WebViewerLoaders(ILoadersContext& context,
                     IWebViewerLoadersObserver* observer) :
      context_(context),
      observer_(observer),
      loadThumbnails_(false),
      loadedSeries_(boost::make_shared<LoadedDicomResources>(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID)),
      loadedStudies_(boost::make_shared<LoadedDicomResources>(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID))
    {
    }

    static Orthanc::IDynamicObject* CreatePayload(Type type)
    {
      return new Orthanc::SingleValueObject<Type>(type);
    }
    
    void HandleThumbnail(const SeriesThumbnailsLoader::SuccessMessage& message)
    {
      if (observer_.get() != NULL)
      {
        observer_->SignalThumbnailLoaded(message.GetStudyInstanceUid(),
                                         message.GetSeriesInstanceUid(),
                                         message.GetType());
      }
    }
    
    void HandleLoadedResources(const DicomResourcesLoader::SuccessMessage& message)
    {
      LoadedDicomResources series(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID);

      switch (dynamic_cast<const Orthanc::SingleValueObject<Type>&>(message.GetUserPayload()).GetValue())
      {
        case Type_DicomWeb:
        {          
          for (size_t i = 0; i < loadedSeries_->GetSize(); i++)
          {
            std::string study;
            if (loadedSeries_->GetResource(i).LookupStringValue(
                  study, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, false) &&
                loadedStudies_->HasResource(study))
            {
              Orthanc::DicomMap m;
              m.Assign(loadedSeries_->GetResource(i));
              loadedStudies_->MergeResource(m, study);
              series.AddResource(m);
            }
          }

          break;
        }

        case Type_Orthanc:
        {          
          for (size_t i = 0; i < message.GetResources()->GetSize(); i++)
          {
            series.AddResource(message.GetResources()->GetResource(i));
          }

          break;
        }

        default:
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }

      if (loadThumbnails_ &&
          (!source_.IsDicomWeb() ||
           source_.HasDicomWebRendered()))
      {
        for (size_t i = 0; i < series.GetSize(); i++)
        {
          std::string patientId, studyInstanceUid, seriesInstanceUid;
          if (series.GetResource(i).LookupStringValue(patientId, Orthanc::DICOM_TAG_PATIENT_ID, false) &&
              series.GetResource(i).LookupStringValue(studyInstanceUid, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, false) &&
              series.GetResource(i).LookupStringValue(seriesInstanceUid, Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, false) &&
              scheduledThumbnails_.find(seriesInstanceUid) == scheduledThumbnails_.end())
          {
            scheduledThumbnails_.insert(seriesInstanceUid);
            thumbnailsLoader_->ScheduleLoadThumbnail(source_, patientId, studyInstanceUid, seriesInstanceUid);
          }
        }
      }

      if (observer_.get() != NULL &&
          series.GetSize() > 0)
      {
        observer_->SignalSeriesUpdated(series);
      }
    }

    void HandleOrthancRestApi(const OrthancRestApiCommand::SuccessMessage& message)
    {
      Json::Value body;
      message.ParseJsonBody(body);

      if (body.type() != Json::arrayValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
      }
      else
      {
        for (Json::Value::ArrayIndex i = 0; i < body.size(); i++)
        {
          if (body[i].type() == Json::stringValue)
          {
            AddOrthancSeries(body[i].asString());
          }
          else
          {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
          }
        }
      }
    }

  public:
    static boost::shared_ptr<WebViewerLoaders> Create(ILoadersContext& context,
                                                      const DicomSource& source,
                                                      bool loadThumbnails,
                                                      IWebViewerLoadersObserver* observer)
    {
      boost::shared_ptr<WebViewerLoaders> application(new WebViewerLoaders(context, observer));
      application->source_ = source;
      application->loadThumbnails_ = loadThumbnails;

      {
        std::unique_ptr<ILoadersContext::ILock> lock(context.Lock());

        application->resourcesLoader_ = DicomResourcesLoader::Create(*lock);

        {
          SeriesThumbnailsLoader::Factory f;
          f.SetPriority(PRIORITY_THUMBNAILS);
          application->thumbnailsLoader_ = boost::dynamic_pointer_cast<SeriesThumbnailsLoader>(f.Create(*lock));
        }

        application->Register<OrthancRestApiCommand::SuccessMessage>(
          lock->GetOracleObservable(), &WebViewerLoaders::HandleOrthancRestApi);

        application->Register<DicomResourcesLoader::SuccessMessage>(
          *application->resourcesLoader_, &WebViewerLoaders::HandleLoadedResources);

        application->Register<SeriesThumbnailsLoader::SuccessMessage>(
          *application->thumbnailsLoader_, &WebViewerLoaders::HandleThumbnail);

        lock->AddLoader(application);
      }

      return application;
    }
    
    void AddDicomAllSeries()
    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());

      if (source_.IsDicomWeb())
      {
        resourcesLoader_->ScheduleGetDicomWeb(loadedSeries_, PRIORITY_ADD_RESOURCES, source_,
                                              "/series", CreatePayload(Type_DicomWeb));
        resourcesLoader_->ScheduleGetDicomWeb(loadedStudies_, PRIORITY_ADD_RESOURCES, source_,
                                              "/studies", CreatePayload(Type_DicomWeb));
      }
      else if (source_.IsOrthanc())
      {
        std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
        command->SetMethod(Orthanc::HttpMethod_Get);
        command->SetUri("/series");
        lock->Schedule(GetSharedObserver(), PRIORITY_ADD_RESOURCES, command.release());
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }
    }
    
    void AddDicomStudy(const std::string& studyInstanceUid)
    {
      // Avoid adding twice the same study
      if (scheduledStudies_.find(studyInstanceUid) == scheduledStudies_.end())
      {
        scheduledStudies_.insert(studyInstanceUid);

        if (source_.IsDicomWeb())
        {
          Orthanc::DicomMap filter;
          filter.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, studyInstanceUid, false);
          
          std::set<Orthanc::DicomTag> tags;
          
          {
            std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());      
            
            resourcesLoader_->ScheduleQido(loadedStudies_, PRIORITY_ADD_RESOURCES, source_,
                                           Orthanc::ResourceType_Study, filter, tags, CreatePayload(Type_DicomWeb));
            
            resourcesLoader_->ScheduleQido(loadedSeries_, PRIORITY_ADD_RESOURCES, source_,
                                           Orthanc::ResourceType_Series, filter, tags, CreatePayload(Type_DicomWeb));
          }
        }
        else if (source_.IsOrthanc())
        {
          std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
          command->SetMethod(Orthanc::HttpMethod_Post);
          command->SetUri("/tools/find");

          Json::Value body;
          body["Level"] = "Series";
          body["Query"] = Json::objectValue;
          body["Query"]["StudyInstanceUID"] = studyInstanceUid;
          command->SetBody(body);

          {
            std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());      
            lock->Schedule(GetSharedObserver(), PRIORITY_ADD_RESOURCES, command.release());
          }
        }
        else
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }        
      }
    }
    
    void AddDicomSeries(const std::string& studyInstanceUid,
                        const std::string& seriesInstanceUid)
    {
      std::set<Orthanc::DicomTag> tags;

      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());      

      if (scheduledStudies_.find(studyInstanceUid) == scheduledStudies_.end())
      {
        scheduledStudies_.insert(studyInstanceUid);
          
        if (source_.IsDicomWeb())
        {
          Orthanc::DicomMap filter;
          filter.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, studyInstanceUid, false);
          
          resourcesLoader_->ScheduleQido(loadedStudies_, PRIORITY_ADD_RESOURCES, source_,
                                         Orthanc::ResourceType_Study, filter, tags, CreatePayload(Type_DicomWeb));
        }
      }

      if (scheduledSeries_.find(seriesInstanceUid) == scheduledSeries_.end())
      {
        scheduledSeries_.insert(seriesInstanceUid);

        if (source_.IsDicomWeb())
        {
          Orthanc::DicomMap filter;
          filter.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, studyInstanceUid, false);
          filter.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, seriesInstanceUid, false);
          
          resourcesLoader_->ScheduleQido(loadedSeries_, PRIORITY_ADD_RESOURCES, source_,
                                         Orthanc::ResourceType_Series, filter, tags, CreatePayload(Type_DicomWeb));
        }
        else if (source_.IsOrthanc())
        {
          std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
          command->SetMethod(Orthanc::HttpMethod_Post);
          command->SetUri("/tools/find");

          Json::Value body;
          body["Level"] = "Series";
          body["Query"] = Json::objectValue;
          body["Query"]["StudyInstanceUID"] = studyInstanceUid;
          body["Query"]["SeriesInstanceUID"] = seriesInstanceUid;
          command->SetBody(body);

          lock->Schedule(GetSharedObserver(), PRIORITY_ADD_RESOURCES, command.release());
        }
        else
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
        }
      }
    }

    void AddOrthancStudy(const std::string& orthancId)
    {
      if (source_.IsOrthanc())
      {
        std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());      
        resourcesLoader_->ScheduleLoadOrthancResources(
          loadedSeries_, PRIORITY_ADD_RESOURCES, source_,
          Orthanc::ResourceType_Study, orthancId, Orthanc::ResourceType_Series,
          CreatePayload(Type_Orthanc));
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadParameterType,
                                        "Only applicable to Orthanc DICOM sources");
      }
    }

    void AddOrthancSeries(const std::string& orthancId)
    {
      if (source_.IsOrthanc())
      {
        std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());      
        resourcesLoader_->ScheduleLoadOrthancResource(
          loadedSeries_, PRIORITY_ADD_RESOURCES,
          source_, Orthanc::ResourceType_Series, orthancId,
          CreatePayload(Type_Orthanc));
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadParameterType,
                                        "Only applicable to Orthanc DICOM sources");
      }
    }
  };
}
