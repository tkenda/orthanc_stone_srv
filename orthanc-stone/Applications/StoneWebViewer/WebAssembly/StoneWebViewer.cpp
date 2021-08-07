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


#include <EmbeddedResources.h>
#include <emscripten.h>


#define DISPATCH_JAVASCRIPT_EVENT(name)                         \
  EM_ASM(                                                       \
    const customEvent = document.createEvent("CustomEvent");    \
    customEvent.initCustomEvent(name, false, false, undefined); \
    window.dispatchEvent(customEvent);                          \
    );


#define EXTERN_CATCH_EXCEPTIONS                         \
  catch (Orthanc::OrthancException& e)                  \
  {                                                     \
    LOG(ERROR) << "OrthancException: " << e.What();     \
    DISPATCH_JAVASCRIPT_EVENT("StoneException");        \
  }                                                     \
  catch (OrthancStone::StoneException& e)               \
  {                                                     \
    LOG(ERROR) << "StoneException: " << e.What();       \
    DISPATCH_JAVASCRIPT_EVENT("StoneException");        \
  }                                                     \
  catch (std::exception& e)                             \
  {                                                     \
    LOG(ERROR) << "Runtime error: " << e.what();        \
    DISPATCH_JAVASCRIPT_EVENT("StoneException");        \
  }                                                     \
  catch (...)                                           \
  {                                                     \
    LOG(ERROR) << "Native exception";                   \
    DISPATCH_JAVASCRIPT_EVENT("StoneException");        \
  }


// Orthanc framework includes
#include <Cache/MemoryObjectCache.h>
#include <DicomFormat/DicomArray.h>
#include <DicomParsing/ParsedDicomFile.h>
#include <Images/Image.h>
#include <Images/ImageProcessing.h>
#include <Images/JpegReader.h>
#include <Logging.h>

// Stone includes
#include <Loaders/DicomResourcesLoader.h>
#include <Loaders/SeriesMetadataLoader.h>
#include <Loaders/SeriesThumbnailsLoader.h>
#include <Messages/ObserverBase.h>
#include <Oracle/ParseDicomFromWadoCommand.h>
#include <Oracle/ParseDicomSuccessMessage.h>
#include <Scene2D/AnnotationsSceneLayer.h>
#include <Scene2D/ArrowSceneLayer.h>
#include <Scene2D/ColorTextureSceneLayer.h>
#include <Scene2D/FloatTextureSceneLayer.h>
#include <Scene2D/MacroSceneLayer.h>
#include <Scene2D/OsiriXLayerFactory.h>
#include <Scene2D/PolylineSceneLayer.h>
#include <Scene2D/TextSceneLayer.h>
#include <Scene2DViewport/ViewportController.h>
#include <StoneException.h>
#include <Toolbox/DicomInstanceParameters.h>
#include <Toolbox/GeometryToolbox.h>
#include <Toolbox/OsiriX/AngleAnnotation.h>
#include <Toolbox/OsiriX/CollectionOfAnnotations.h>
#include <Toolbox/OsiriX/LineAnnotation.h>
#include <Toolbox/OsiriX/TextAnnotation.h>
#include <Toolbox/SortedFrames.h>
#include <Viewport/DefaultViewportInteractor.h>

// WebAssembly includes
#include <WebAssemblyCairoViewport.h>
#include <WebAssemblyLoadersContext.h>
#include <WebGLViewport.h>


#include <boost/math/special_functions/round.hpp>
#include <boost/make_shared.hpp>
#include <stdio.h>


#if !defined(STONE_WEB_VIEWER_EXPORT)
// We are not running ParseWebAssemblyExports.py, but we're compiling the wasm
#  define STONE_WEB_VIEWER_EXPORT
#endif


#define FIX_LSD_479  1


enum STONE_WEB_VIEWER_EXPORT ThumbnailType
{
  ThumbnailType_Image,
    ThumbnailType_NoPreview,
    ThumbnailType_Pdf,
    ThumbnailType_Video,
    ThumbnailType_Loading,
    ThumbnailType_Unknown
    };


enum STONE_WEB_VIEWER_EXPORT DisplayedFrameQuality
{
  DisplayedFrameQuality_None,
    DisplayedFrameQuality_Low,
    DisplayedFrameQuality_High
    };


enum STONE_WEB_VIEWER_EXPORT WebViewerAction
{
  WebViewerAction_None,
    
    WebViewerAction_Windowing,
    WebViewerAction_Zoom,
    WebViewerAction_Pan,
    WebViewerAction_Rotate,
    WebViewerAction_Crosshair,
    
    WebViewerAction_CreateAngle,
    WebViewerAction_CreateCircle,
    WebViewerAction_CreateSegment,
    WebViewerAction_RemoveMeasure
    };
  


static OrthancStone::MouseAction ConvertWebViewerAction(int action)
{
  switch (action)
  {
    case WebViewerAction_Windowing:
      return OrthancStone::MouseAction_GrayscaleWindowing;
      
    case WebViewerAction_Zoom:
      return OrthancStone::MouseAction_Zoom;
      
    case WebViewerAction_Pan:
      return OrthancStone::MouseAction_Pan;
      
    case WebViewerAction_Rotate:
      return OrthancStone::MouseAction_Rotate;
      
    case WebViewerAction_None:
    case WebViewerAction_Crosshair:
    case WebViewerAction_CreateAngle:
    case WebViewerAction_CreateCircle:
    case WebViewerAction_CreateSegment:
    case WebViewerAction_RemoveMeasure:
      return OrthancStone::MouseAction_None;

    default:
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
  }
}



static const int PRIORITY_HIGH = -100;
static const int PRIORITY_LOW = 100;
static const int PRIORITY_NORMAL = 0;

static const unsigned int QUALITY_JPEG = 0;
static const unsigned int QUALITY_FULL = 1;

static const unsigned int DEFAULT_CINE_RATE = 30;



class VirtualSeries : public boost::noncopyable
{
private:
  class Item : public boost::noncopyable
  {
  private:
    std::string             seriesInstanceUid_;
    std::list<std::string>  sopInstanceUids_;

  public:
    Item(const std::string& seriesInstanceUid,
         const std::list<std::string>& sopInstanceUids) :
      seriesInstanceUid_(seriesInstanceUid),
      sopInstanceUids_(sopInstanceUids)
    {
    }

    const std::string& GetSeriesInstanceUid() const
    {
      return seriesInstanceUid_;
    }

    const std::list<std::string>& GetSopInstanceUids() const
    {
      return sopInstanceUids_;
    }
  };

  typedef std::map<std::string, Item*>  Content;

  Content  content_;

  const Item& GetItem(const std::string& id) const
  {
    Content::const_iterator found = content_.find(id);
    
    if (found == content_.end())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      assert(found->second != NULL);
      return *found->second;
    }
  }
  
public:
  ~VirtualSeries()
  {
    for (Content::iterator it = content_.begin(); it != content_.end(); ++it)
    {
      assert(it->second != NULL);
      delete it->second;
    }
  }
  
  std::string AddSingleInstance(const std::string& seriesInstanceUid,
                                const std::string& sopInstanceUid)
  {
    std::list<std::string> sopInstanceUids;
    sopInstanceUids.push_back(sopInstanceUid);
    return AddMultipleInstances(seriesInstanceUid, sopInstanceUids);
  }

  std::string AddMultipleInstances(const std::string& seriesInstanceUid,
                                   const std::list<std::string>& sopInstanceUids)
  {
    // Generate a unique identifier for this virtual series
    const std::string virtualSeriesId = "virtual-" + boost::lexical_cast<std::string>(content_.size());
    
    if (content_.find(virtualSeriesId) != content_.end())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      content_.insert(std::make_pair(virtualSeriesId, new Item(seriesInstanceUid, sopInstanceUids)));
      return virtualSeriesId;
    }
  }

  const std::string& GetSeriesInstanceUid(const std::string& id) const
  {
    return GetItem(id).GetSeriesInstanceUid();
  }

  const std::list<std::string>& GetSopInstanceUids(const std::string& id) const
  {
    return GetItem(id).GetSopInstanceUids();
  }
};



class ResourcesLoader : public OrthancStone::ObserverBase<ResourcesLoader>
{
public:
  class IObserver : public boost::noncopyable
  {
  public:
    virtual ~IObserver()
    {
    }

    virtual void SignalResourcesLoaded() = 0;

    virtual void SignalSeriesThumbnailLoaded(const std::string& studyInstanceUid,
                                             const std::string& seriesInstanceUid) = 0;

    virtual void SignalSeriesMetadataLoaded(const std::string& studyInstanceUid,
                                            const std::string& seriesInstanceUid) = 0;

    virtual void SignalSeriesPdfLoaded(const std::string& studyInstanceUid,
                                       const std::string& seriesInstanceUid,
                                       const std::string& pdf) = 0;

    virtual void SignalVirtualSeriesThumbnailLoaded(const std::string& virtualSeriesId,
                                                    const std::string& jpeg) = 0;
  };
  
private:
  OrthancStone::ILoadersContext&                           context_;
  std::unique_ptr<IObserver>                               observer_;
  OrthancStone::DicomSource                                source_;
  size_t                                                   pending_;
  boost::shared_ptr<OrthancStone::LoadedDicomResources>    studies_;
  boost::shared_ptr<OrthancStone::LoadedDicomResources>    series_;
  boost::shared_ptr<OrthancStone::DicomResourcesLoader>    resourcesLoader_;
  boost::shared_ptr<OrthancStone::SeriesThumbnailsLoader>  thumbnailsLoader_;
  boost::shared_ptr<OrthancStone::SeriesMetadataLoader>    metadataLoader_;
  std::set<std::string>                                    scheduledVirtualSeriesThumbnails_;
  VirtualSeries                                            virtualSeries_;

  explicit ResourcesLoader(OrthancStone::ILoadersContext& context,
                           const OrthancStone::DicomSource& source) :
    context_(context),
    source_(source),
    pending_(0),
    studies_(new OrthancStone::LoadedDicomResources(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID)),
    series_(new OrthancStone::LoadedDicomResources(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID))
  {
  }

  void Handle(const OrthancStone::DicomResourcesLoader::SuccessMessage& message)
  {
    const Orthanc::SingleValueObject<Orthanc::ResourceType>& payload =
      dynamic_cast<const Orthanc::SingleValueObject<Orthanc::ResourceType>&>(message.GetUserPayload());
    
    OrthancStone::LoadedDicomResources& dicom = *message.GetResources();
    
    LOG(INFO) << "resources loaded: " << dicom.GetSize()
              << ", " << Orthanc::EnumerationToString(payload.GetValue());

    if (payload.GetValue() == Orthanc::ResourceType_Series)
    {
      for (size_t i = 0; i < dicom.GetSize(); i++)
      {
        std::string studyInstanceUid, seriesInstanceUid;
        if (dicom.GetResource(i).LookupStringValue(
              studyInstanceUid, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, false) &&
            dicom.GetResource(i).LookupStringValue(
              seriesInstanceUid, Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, false))
        {
          thumbnailsLoader_->ScheduleLoadThumbnail(source_, "", studyInstanceUid, seriesInstanceUid);
          metadataLoader_->ScheduleLoadSeries(PRIORITY_LOW + 1, source_, studyInstanceUid, seriesInstanceUid);
        }
      }
    }

    if (pending_ == 0)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
    else
    {
      pending_ --;
      if (pending_ == 0 &&
          observer_.get() != NULL)
      {
        observer_->SignalResourcesLoaded();
      }
    }
  }

  void Handle(const OrthancStone::SeriesThumbnailsLoader::SuccessMessage& message)
  {
    if (observer_.get() != NULL)
    {
      observer_->SignalSeriesThumbnailLoaded(
        message.GetStudyInstanceUid(), message.GetSeriesInstanceUid());
    }
  }

  void Handle(const OrthancStone::SeriesMetadataLoader::SuccessMessage& message)
  {
    if (observer_.get() != NULL)
    {
      observer_->SignalSeriesMetadataLoaded(
        message.GetStudyInstanceUid(), message.GetSeriesInstanceUid());
    }
  }

  void FetchInternal(const std::string& patientId,
                     const std::string& studyInstanceUid,
                     const std::string& seriesInstanceUid)
  {
    // Firstly, load the study
    Orthanc::DicomMap filter;

    if (!patientId.empty())
    {
      filter.SetValue(Orthanc::DICOM_TAG_PATIENT_ID, patientId, false);
    }

    if (!studyInstanceUid.empty())
    {
      filter.SetValue(Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, studyInstanceUid, false);
    }

    std::set<Orthanc::DicomTag> tags;
    tags.insert(Orthanc::DICOM_TAG_STUDY_DESCRIPTION);  // Necessary for Orthanc DICOMweb plugin

    resourcesLoader_->ScheduleQido(
      studies_, PRIORITY_HIGH, source_, Orthanc::ResourceType_Study, filter, tags,
      new Orthanc::SingleValueObject<Orthanc::ResourceType>(Orthanc::ResourceType_Study));

    // Secondly, load the series
    if (!seriesInstanceUid.empty())
    {
      filter.SetValue(Orthanc::DICOM_TAG_SERIES_INSTANCE_UID, seriesInstanceUid, false);
    }
    
    tags.insert(Orthanc::DICOM_TAG_SERIES_NUMBER);  // Necessary for Google Cloud Platform
    
    resourcesLoader_->ScheduleQido(
      series_, PRIORITY_HIGH, source_, Orthanc::ResourceType_Series, filter, tags,
      new Orthanc::SingleValueObject<Orthanc::ResourceType>(Orthanc::ResourceType_Series));

    pending_ += 2;
  }
  

  class PdfInfo : public Orthanc::IDynamicObject
  {
  private:
    std::string  studyInstanceUid_;
    std::string  seriesInstanceUid_;

  public:
    PdfInfo(const std::string& studyInstanceUid,
            const std::string& seriesInstanceUid) :
      studyInstanceUid_(studyInstanceUid),
      seriesInstanceUid_(seriesInstanceUid)
    {
    }

    const std::string& GetStudyInstanceUid() const
    {
      return studyInstanceUid_;
    }

    const std::string& GetSeriesInstanceUid() const
    {
      return seriesInstanceUid_;
    }
  };


  void Handle(const OrthancStone::ParseDicomSuccessMessage& message)
  {
    const PdfInfo& info = dynamic_cast<const PdfInfo&>(message.GetOrigin().GetPayload());

    if (observer_.get() != NULL)
    {
      std::string pdf;
      if (message.GetDicom().ExtractPdf(pdf))
      {
        observer_->SignalSeriesPdfLoaded(info.GetStudyInstanceUid(), info.GetSeriesInstanceUid(), pdf);
      }
      else
      {
        LOG(ERROR) << "Unable to extract PDF from series: " << info.GetSeriesInstanceUid();
      }
    }
  }

  void FetchVirtualSeriesThumbnail(const std::string& virtualSeriesId,
                                   const std::string& studyInstanceUid,
                                   const std::string& seriesInstanceUid,
                                   const std::string& sopInstanceUid)
  {
    if (scheduledVirtualSeriesThumbnails_.find(virtualSeriesId) == scheduledVirtualSeriesThumbnails_.end())
    {
      scheduledVirtualSeriesThumbnails_.insert(virtualSeriesId);
      
      std::map<std::string, std::string> arguments;
      std::map<std::string, std::string> headers;
      arguments["viewport"] = (
        boost::lexical_cast<std::string>(thumbnailsLoader_->GetThumbnailWidth()) + "," +
        boost::lexical_cast<std::string>(thumbnailsLoader_->GetThumbnailHeight()));
      headers["Accept"] = Orthanc::MIME_JPEG;

      const std::string uri = ("studies/" + studyInstanceUid + "/series/" + seriesInstanceUid +
                               "/instances/" + sopInstanceUid + "/frames/1/rendered");

      {
        std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(context_.Lock());
        lock->Schedule(
          GetSharedObserver(), PRIORITY_LOW + 2, source_.CreateDicomWebCommand(
            uri, arguments, headers, new Orthanc::SingleValueObject<std::string>(virtualSeriesId)));
      }
    }
  }

  void HandleInstanceThumbnail(const OrthancStone::HttpCommand::SuccessMessage& message)
  {
    if (observer_.get() != NULL)
    {
      const std::string& virtualSeriesId =
        dynamic_cast<const Orthanc::SingleValueObject<std::string>&>(
          message.GetOrigin().GetPayload()).GetValue();
      observer_->SignalVirtualSeriesThumbnailLoaded(virtualSeriesId, message.GetAnswer());
    }
  }

public:
  static boost::shared_ptr<ResourcesLoader> Create(OrthancStone::ILoadersContext::ILock& lock,
                                                   const OrthancStone::DicomSource& source)
  {
    boost::shared_ptr<ResourcesLoader> loader(new ResourcesLoader(lock.GetContext(), source));

    loader->resourcesLoader_ = OrthancStone::DicomResourcesLoader::Create(lock);
    loader->thumbnailsLoader_ = OrthancStone::SeriesThumbnailsLoader::Create(lock, PRIORITY_LOW);
    loader->metadataLoader_ = OrthancStone::SeriesMetadataLoader::Create(lock);
    
    loader->Register<OrthancStone::DicomResourcesLoader::SuccessMessage>(
      *loader->resourcesLoader_, &ResourcesLoader::Handle);

    loader->Register<OrthancStone::SeriesThumbnailsLoader::SuccessMessage>(
      *loader->thumbnailsLoader_, &ResourcesLoader::Handle);

    loader->Register<OrthancStone::SeriesMetadataLoader::SuccessMessage>(
      *loader->metadataLoader_, &ResourcesLoader::Handle);

    loader->Register<OrthancStone::ParseDicomSuccessMessage>(
      lock.GetOracleObservable(), &ResourcesLoader::Handle);

    loader->Register<OrthancStone::HttpCommand::SuccessMessage>(
      lock.GetOracleObservable(), &ResourcesLoader::HandleInstanceThumbnail);

    return loader;
  }
  
  void FetchAllStudies()
  {
    FetchInternal("", "", "");
  }
  
  void FetchPatient(const std::string& patientId)
  {
    if (!patientId.empty())
    {
      FetchInternal(patientId, "", "");
    }
  }
  
  void FetchStudy(const std::string& studyInstanceUid)
  {
    if (!studyInstanceUid.empty())
    {
      FetchInternal("", studyInstanceUid, "");
    }
  }
  
  void FetchSeries(const std::string& studyInstanceUid,
                   const std::string& seriesInstanceUid)
  {
    if (!studyInstanceUid.empty() &&
        !seriesInstanceUid.empty())
    {
      FetchInternal("", studyInstanceUid, seriesInstanceUid);
    }
  }

  size_t GetStudiesCount() const
  {
    return studies_->GetSize();
  }

  size_t GetSeriesCount() const
  {
    return series_->GetSize();
  }

  void GetStudy(Orthanc::DicomMap& target,
                size_t i) const
  {
    target.Assign(studies_->GetResource(i));
  }

  void GetSeries(Orthanc::DicomMap& target,
                 size_t i) const
  {
    target.Assign(series_->GetResource(i));

    // Complement with the study-level tags
    std::string studyInstanceUid;
    if (target.LookupStringValue(studyInstanceUid, Orthanc::DICOM_TAG_STUDY_INSTANCE_UID, false) &&
        studies_->HasResource(studyInstanceUid))
    {
      studies_->MergeResource(target, studyInstanceUid);
    }
  }

  OrthancStone::SeriesThumbnailType GetSeriesThumbnail(std::string& image,
                                                       std::string& mime,
                                                       const std::string& seriesInstanceUid) const
  {
    return thumbnailsLoader_->GetSeriesThumbnail(image, mime, seriesInstanceUid);
  }

  void FetchSeriesMetadata(int priority,
                           const std::string& studyInstanceUid,
                           const std::string& seriesInstanceUid) const
  {
    metadataLoader_->ScheduleLoadSeries(priority, source_, studyInstanceUid, seriesInstanceUid);
  }

  bool IsSeriesComplete(const std::string& seriesInstanceUid) const
  {
    OrthancStone::SeriesMetadataLoader::Accessor accessor(*metadataLoader_, seriesInstanceUid);
    return accessor.IsComplete();
  }

  bool LookupVirtualSeries(std::map<std::string, unsigned int>& virtualSeries /* out */,
                           const std::string& seriesInstanceUid)
  {
    OrthancStone::SeriesMetadataLoader::Accessor accessor(*metadataLoader_, seriesInstanceUid);
    if (accessor.IsComplete() &&
        accessor.GetInstancesCount() >= 2)
    {
      bool hasMultiframe = false;
      
      for (size_t i = 0; i < accessor.GetInstancesCount(); i++)
      {
        OrthancStone::DicomInstanceParameters p(accessor.GetInstance(i));

        if (p.GetNumberOfFrames() > 1)
        {
          hasMultiframe = true;
        }
      }

      if (hasMultiframe)
      {
        std::string studyInstanceUid;
        std::list<std::string> instancesWithoutFrameNumber;
        
        for (size_t i = 0; i < accessor.GetInstancesCount(); i++)
        {
          OrthancStone::DicomInstanceParameters p(accessor.GetInstance(i));

          if (p.HasNumberOfFrames())
          {
            const std::string virtualSeriesId = virtualSeries_.AddSingleInstance(seriesInstanceUid, p.GetSopInstanceUid());
            if (virtualSeries.find(virtualSeriesId) != virtualSeries.end())
            {
              throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
            }
            else
            {
              virtualSeries[virtualSeriesId] = p.GetNumberOfFrames();
              FetchVirtualSeriesThumbnail(virtualSeriesId, p.GetStudyInstanceUid(), seriesInstanceUid, p.GetSopInstanceUid());
            }
          }
          else
          {
            studyInstanceUid = p.GetStudyInstanceUid();
            instancesWithoutFrameNumber.push_back(p.GetSopInstanceUid());
          }
        }

        if (!instancesWithoutFrameNumber.empty())
        {
          /**
           * Group together in a single "virtual series" all the DICOM
           * instances that have no value for the tag "NumberOfFrames"
           * (0028,0008). This can happen in US CINE series. New in
           * Stone Web viewer 2.1.
           * https://groups.google.com/g/orthanc-users/c/V-vOnlwj06A/m/2sPNwteYAAAJ
           **/
          const std::string virtualSeriesId = virtualSeries_.AddMultipleInstances(seriesInstanceUid, instancesWithoutFrameNumber);
          virtualSeries[virtualSeriesId] = instancesWithoutFrameNumber.size();
          FetchVirtualSeriesThumbnail(virtualSeriesId, studyInstanceUid, seriesInstanceUid, instancesWithoutFrameNumber.front());
        }

        return true;
      }
      else
      {
        return false;
      }
    }
    else
    {
      return false;
    }
  }

  bool SortSeriesFrames(OrthancStone::SortedFrames& target,
                        const std::string& seriesInstanceUid) const
  {
    OrthancStone::SeriesMetadataLoader::Accessor accessor(*metadataLoader_, seriesInstanceUid);
    
    if (accessor.IsComplete())
    {
      target.Clear();

      for (size_t i = 0; i < accessor.GetInstancesCount(); i++)
      {
        target.AddInstance(accessor.GetInstance(i));
      }

      target.Sort();
      
      return true;
    }
    else
    {
      return false;
    }
  }

  bool SortVirtualSeriesFrames(OrthancStone::SortedFrames& target,
                               const std::string& virtualSeriesId) const
  {
    const std::string& seriesInstanceUid = virtualSeries_.GetSeriesInstanceUid(virtualSeriesId);
    
    OrthancStone::SeriesMetadataLoader::Accessor accessor(*metadataLoader_, seriesInstanceUid);
    
    if (accessor.IsComplete())
    {
      const std::list<std::string>& sopInstanceUids = virtualSeries_.GetSopInstanceUids(virtualSeriesId);

      target.Clear();

      for (std::list<std::string>::const_iterator
             it = sopInstanceUids.begin(); it != sopInstanceUids.end(); ++it)
      {
        Orthanc::DicomMap instance;
        if (accessor.LookupInstance(instance, *it))
        {
          target.AddInstance(instance);
        }
        else
        {
          LOG(ERROR) << "Missing instance: " << *it;
        }
      }
      
      target.Sort();
      return true;
    }
    else
    {
      return false;
    }
  }

  size_t GetSeriesNumberOfFrames(const std::string& seriesInstanceUid) const
  {
    OrthancStone::SeriesMetadataLoader::Accessor accessor(*metadataLoader_, seriesInstanceUid);
    
    if (accessor.IsComplete())
    {
      size_t count = 0;
      
      for (size_t i = 0; i < accessor.GetInstancesCount(); i++)
      {
        uint32_t f;
        if (accessor.GetInstance(i).ParseUnsignedInteger32(f, Orthanc::DICOM_TAG_NUMBER_OF_FRAMES))
        {
          count += f;
        }
        else
        {
          count++;
        }
      }

      return count;
    }
    else
    {
      return 0;
    }
  }

  void AcquireObserver(IObserver* observer)
  {  
    observer_.reset(observer);
  }

  void FetchPdf(const std::string& studyInstanceUid,
                const std::string& seriesInstanceUid)
  {
    OrthancStone::SeriesMetadataLoader::Accessor accessor(*metadataLoader_, seriesInstanceUid);
    
    if (accessor.IsComplete())
    {
      if (accessor.GetInstancesCount() > 1)
      {
        LOG(INFO) << "Series with more than one instance, will show the first PDF: "
                  << seriesInstanceUid;
      }

      for (size_t i = 0; i < accessor.GetInstancesCount(); i++)
      {
        std::string sopClassUid, sopInstanceUid;
        if (accessor.GetInstance(i).LookupStringValue(sopClassUid, Orthanc::DICOM_TAG_SOP_CLASS_UID, false) &&
            accessor.GetInstance(i).LookupStringValue(sopInstanceUid, Orthanc::DICOM_TAG_SOP_INSTANCE_UID, false) &&
            sopClassUid == "1.2.840.10008.5.1.4.1.1.104.1")
        {
          std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(context_.Lock());
          lock->Schedule(
            GetSharedObserver(), PRIORITY_NORMAL, OrthancStone::ParseDicomFromWadoCommand::Create(
              source_, studyInstanceUid, seriesInstanceUid, sopInstanceUid,
              false /* no transcoding */, Orthanc::DicomTransferSyntax_LittleEndianExplicit /* dummy value */,
              new PdfInfo(studyInstanceUid, seriesInstanceUid)));
          
          return;
        }
      }

      LOG(WARNING) << "Series without a PDF: " << seriesInstanceUid;
    }
  }
};



class FramesCache : public boost::noncopyable
{
private:
  class CachedImage : public Orthanc::ICacheable
  {
  private:
    std::unique_ptr<Orthanc::ImageAccessor>  image_;
    unsigned int                             quality_;

  public:
    CachedImage(Orthanc::ImageAccessor* image,
                unsigned int quality) :
      image_(image),
      quality_(quality)
    {
      assert(image != NULL);
    }

    virtual size_t GetMemoryUsage() const ORTHANC_OVERRIDE
    {    
      assert(image_.get() != NULL);
      return (image_->GetBytesPerPixel() * image_->GetPitch() * image_->GetHeight());
    }

    const Orthanc::ImageAccessor& GetImage() const
    {
      assert(image_.get() != NULL);
      return *image_;
    }

    unsigned int GetQuality() const
    {
      return quality_;
    }
  };


  static std::string GetKey(const std::string& sopInstanceUid,
                            size_t frameNumber)
  {
    return sopInstanceUid + "|" + boost::lexical_cast<std::string>(frameNumber);
  }
  

  Orthanc::MemoryObjectCache  cache_;
  
public:
  FramesCache()
  {
    SetMaximumSize(100 * 1024 * 1024);  // 100 MB
    //SetMaximumSize(1);  // DISABLE CACHE
  }
  
  size_t GetMaximumSize()
  {
    return cache_.GetMaximumSize();
  }
    
  void SetMaximumSize(size_t size)
  {
    cache_.SetMaximumSize(size);
  }

  /**
   * Returns "true" iff the provided image has better quality than the
   * previously cached one, or if no cache was previously available.
   **/
  bool Acquire(const std::string& sopInstanceUid,
               size_t frameNumber,
               Orthanc::ImageAccessor* image /* transfer ownership */,
               unsigned int quality)
  {
    std::unique_ptr<Orthanc::ImageAccessor> protection(image);
    
    if (image == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }
    else if (image->GetFormat() != Orthanc::PixelFormat_Float32 &&
             image->GetFormat() != Orthanc::PixelFormat_RGB24)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    const std::string& key = GetKey(sopInstanceUid, frameNumber);

    bool invalidate = false;
    
    {
      /**
       * Access the previous cached entry, with side effect of tagging
       * it as the most recently accessed frame (update of LRU recycling)
       **/
      Orthanc::MemoryObjectCache::Accessor accessor(cache_, key, false /* unique lock */);

      if (accessor.IsValid())
      {
        const CachedImage& previous = dynamic_cast<const CachedImage&>(accessor.GetValue());
        
        // There is already a cached image for this frame
        if (previous.GetQuality() < quality)
        {
          // The previously stored image has poorer quality
          invalidate = true;
        }
        else
        {
          // No update in the quality, don't change the cache
          return false;   
        }
      }
      else
      {
        invalidate = false;
      }
    }

    if (invalidate)
    {
      cache_.Invalidate(key);
    }
        
    cache_.Acquire(key, new CachedImage(protection.release(), quality));
    return true;
  }

  class Accessor : public boost::noncopyable
  {
  private:
    Orthanc::MemoryObjectCache::Accessor accessor_;

    const CachedImage& GetCachedImage() const
    {
      if (IsValid())
      {
        return dynamic_cast<CachedImage&>(accessor_.GetValue());
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
    }
    
  public:
    Accessor(FramesCache& that,
             const std::string& sopInstanceUid,
             size_t frameNumber) :
      accessor_(that.cache_, GetKey(sopInstanceUid, frameNumber), false /* shared lock */)
    {
    }

    bool IsValid() const
    {
      return accessor_.IsValid();
    }

    const Orthanc::ImageAccessor& GetImage() const
    {
      return GetCachedImage().GetImage();
    }

    unsigned int GetQuality() const
    {
      return GetCachedImage().GetQuality();
    }
  };
};



class SeriesCursor : public boost::noncopyable
{
public:
  enum Action
  {
    Action_FastPlus,
    Action_Plus,
    Action_None,
    Action_Minus,
    Action_FastMinus
  };
  
private:
  std::vector<size_t>  prefetch_;
  int                  framesCount_;
  int                  currentFrame_;
  bool                 isCircularPrefetch_;
  int                  fastDelta_;
  Action               lastAction_;

  int ComputeNextFrame(int currentFrame,
                       Action action,
                       bool isCircular) const
  {
    if (framesCount_ == 0)
    {
      assert(currentFrame == 0);
      return 0;
    }

    int nextFrame = currentFrame;
    
    switch (action)
    {
      case Action_FastPlus:
        nextFrame += fastDelta_;
        break;

      case Action_Plus:
        nextFrame += 1;
        break;

      case Action_None:
        break;

      case Action_Minus:
        nextFrame -= 1;
        break;

      case Action_FastMinus:
        nextFrame -= fastDelta_;
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    if (isCircular)
    {
      while (nextFrame < 0)
      {
        nextFrame += framesCount_;
      }

      while (nextFrame >= framesCount_)
      {
        nextFrame -= framesCount_;
      }
    }
    else
    {
      if (nextFrame < 0)
      {
        nextFrame = 0;
      }
      else if (nextFrame >= framesCount_)
      {
        nextFrame = framesCount_ - 1;
      }
    }

    return nextFrame;
  }
  
  void UpdatePrefetch()
  {
    /**
     * This method will order the frames of the series according to
     * the number of "actions" (i.e. mouse wheels) that are necessary
     * to reach them, starting from the current frame. It is assumed
     * that once one action is done, it is more likely that the user
     * will do the same action just afterwards.
     **/
    
    prefetch_.clear();

    if (framesCount_ == 0)
    {
      return;
    }

    prefetch_.reserve(framesCount_);
    
    // Breadth-first search using a FIFO. The queue associates a frame
    // and the action that is the most likely in this frame
    typedef std::list< std::pair<int, Action> >  Queue;

    Queue queue;
    std::set<int>  visited;  // Frames that have already been visited

    queue.push_back(std::make_pair(currentFrame_, lastAction_));

    while (!queue.empty())
    {
      int frame = queue.front().first;
      Action previousAction = queue.front().second;
      queue.pop_front();

      if (visited.find(frame) == visited.end())
      {
        visited.insert(frame);
        prefetch_.push_back(frame);

        switch (previousAction)
        {
          case Action_None:
          case Action_Plus:
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_Plus, isCircularPrefetch_), Action_Plus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_Minus, isCircularPrefetch_), Action_Minus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_FastPlus, isCircularPrefetch_), Action_FastPlus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_FastMinus, isCircularPrefetch_), Action_FastMinus));
            break;
          
          case Action_Minus:
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_Minus, isCircularPrefetch_), Action_Minus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_Plus, isCircularPrefetch_), Action_Plus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_FastMinus, isCircularPrefetch_), Action_FastMinus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_FastPlus, isCircularPrefetch_), Action_FastPlus));
            break;

          case Action_FastPlus:
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_FastPlus, isCircularPrefetch_), Action_FastPlus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_FastMinus, isCircularPrefetch_), Action_FastMinus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_Plus, isCircularPrefetch_), Action_Plus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_Minus, isCircularPrefetch_), Action_Minus));
            break;
              
          case Action_FastMinus:
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_FastMinus, isCircularPrefetch_), Action_FastMinus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_FastPlus, isCircularPrefetch_), Action_FastPlus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_Minus, isCircularPrefetch_), Action_Minus));
            queue.push_back(std::make_pair(ComputeNextFrame(frame, Action_Plus, isCircularPrefetch_), Action_Plus));
            break;

          default:
            throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
        }
      }
    }

    assert(prefetch_.size() == framesCount_);
  }

  bool CheckFrameIndex(int frame) const
  {
    return ((framesCount_ == 0 && frame == 0) ||
            (framesCount_ > 0 && frame >= 0 && frame < framesCount_));
  }
  
public:
  explicit SeriesCursor(size_t framesCount) :
    framesCount_(framesCount),
    currentFrame_(framesCount / 2),  // Start at the middle frame    
    isCircularPrefetch_(false),
    lastAction_(Action_None)
  {
    SetFastDelta(framesCount / 20);
    UpdatePrefetch();
  }

  size_t GetFramesCount() const
  {
    return framesCount_;
  }

  void SetCircularPrefetch(bool isCircularPrefetch)
  {
    isCircularPrefetch_ = isCircularPrefetch;
    UpdatePrefetch();
  }

  void SetFastDelta(int delta)
  {
    fastDelta_ = (delta < 0 ? -delta : delta);

    if (fastDelta_ <= 0)
    {
      fastDelta_ = 1;
    }
  }

  void SetCurrentIndex(size_t frame)
  {
    if (frame >= framesCount_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      currentFrame_ = frame;
      lastAction_ = Action_None;
      UpdatePrefetch();
    }
  }

  size_t GetCurrentIndex() const
  {
    assert(CheckFrameIndex(currentFrame_));
    return static_cast<size_t>(currentFrame_);
  }

  void Apply(Action action,
             bool isCircular)
  {
    currentFrame_ = ComputeNextFrame(currentFrame_, action, isCircular);
    lastAction_ = action;
    UpdatePrefetch();
  }

  size_t GetPrefetchSize() const
  {
    assert(prefetch_.size() == framesCount_);
    return prefetch_.size();
  }

  size_t GetPrefetchIndex(size_t i) const
  {
    if (i >= prefetch_.size())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    else
    {
      assert(CheckFrameIndex(prefetch_[i]));
      return static_cast<size_t>(prefetch_[i]);
    }
  }
};




/**
 * Returns a clipped line (out: x1, y1, x2, y2), in the coordinate
 * system of "instance1". Note that the frame of reference UID is not
 * checked by this function.
 **/
static bool GetReferenceLineCoordinates(double& x1,
                                        double& y1,
                                        double& x2,
                                        double& y2,
                                        const OrthancStone::DicomInstanceParameters& instance1,
                                        unsigned int frame1,
                                        const OrthancStone::CoordinateSystem3D& plane2)
{
  if (instance1.GetWidth() == 0 &&
      instance1.GetHeight() == 0)
  {
    return false;
  }
  else
  {
    /**
     * Compute the 2D extent of the "instance1", expressed in
     * centimeters, in the 2D plane defined by this DICOM instance.
     *
     * In a multiframe image (cf. "ExtractFrameOffsets()"), the plane of
     * each frame is a translation of the plane of the first frame along
     * its normal. As a consequence, the extent is the same for each
     * frame, so we can ignore the frame number.
     **/
    OrthancStone::Extent2D extent;

    double ox = -instance1.GetPixelSpacingX() / 2.0;
    double oy = -instance1.GetPixelSpacingY() / 2.0;
    extent.AddPoint(ox, oy);
    extent.AddPoint(ox + instance1.GetPixelSpacingX() * static_cast<double>(instance1.GetWidth()),
                    oy + instance1.GetPixelSpacingY() * static_cast<double>(instance1.GetHeight()));

    const OrthancStone::CoordinateSystem3D c1 = instance1.GetFrameGeometry(frame1);
  
    OrthancStone::Vector direction, origin;
  
    if (!extent.IsEmpty() &&
        OrthancStone::GeometryToolbox::IntersectTwoPlanes(origin, direction,
                                                          c1.GetOrigin(), c1.GetNormal(),
                                                          plane2.GetOrigin(), plane2.GetNormal()))
    {
      double ax, ay, bx, by;
      c1.ProjectPoint(ax, ay, origin);
      c1.ProjectPoint(bx, by, origin + 100.0 * direction);
    
      return OrthancStone::GeometryToolbox::ClipLineToRectangle(
        x1, y1, x2, y2,
        ax, ay, bx, by,
        extent.GetX1(), extent.GetY1(), extent.GetX2(), extent.GetY2());
    }
    else
    {
      return false;
    }
  }
}



class StoneAnnotationsRegistry : public boost::noncopyable
{
private:
  class Index
  {
  private:
    std::string  sopInstanceUid_;
    size_t       frame_;

  public:
    Index(const std::string& sopInstanceUid,
          size_t frame) :
      sopInstanceUid_(sopInstanceUid),
      frame_(frame)
    {
    }

    const std::string& GetSopInstanceUid() const
    {
      return sopInstanceUid_;
    }

    size_t GetFrame() const
    {
      return frame_;
    }

    bool operator< (const Index& other) const
    {
      if (sopInstanceUid_ < other.sopInstanceUid_)
      {
        return true;
      }
      else if (sopInstanceUid_ > other.sopInstanceUid_)
      {
        return false;
      }
      else
      {
        return frame_ < other.frame_;
      }
    }
  };
  
  typedef std::map<Index, Json::Value*>  Content;

  Content  content_;

  void Clear()
  {
    for (Content::iterator it = content_.begin(); it != content_.end(); ++it)
    {
      assert(it->second != NULL);
      delete it->second;
    }

    content_.clear();
  }

  StoneAnnotationsRegistry()
  {
  }

public:
  ~StoneAnnotationsRegistry()
  {
    Clear();
  }

  static StoneAnnotationsRegistry& GetInstance()
  {
    static StoneAnnotationsRegistry singleton;
    return singleton;
  }
  
  void Save(const std::string& sopInstanceUid,
            size_t frame,
            const OrthancStone::AnnotationsSceneLayer& layer)
  {
    std::unique_ptr<Json::Value> serialized(new Json::Value);
    layer.Serialize(*serialized);

    const Index index(sopInstanceUid, frame);
    
    Content::iterator found = content_.find(index);
    if (found == content_.end())
    {
      content_[index] = serialized.release();
    }
    else
    {
      assert(found->second != NULL);
      delete found->second;
      found->second = serialized.release();
    }
  }

  void Load(OrthancStone::AnnotationsSceneLayer& layer,
            const std::string& sopInstanceUid,
            size_t frame) const
  {
    const Index index(sopInstanceUid, frame);
    
    Content::const_iterator found = content_.find(index);
    if (found == content_.end())
    {
      layer.Clear();
    }
    else
    {
      assert(found->second != NULL);
      layer.Unserialize(*found->second);
    }
  }
};



class ViewerViewport : public OrthancStone::ObserverBase<ViewerViewport>
{
public:
  class IObserver : public boost::noncopyable
  {
  public:
    virtual ~IObserver()
    {
    }

    virtual void SignalSeriesDetailsReady(const ViewerViewport& viewport) = 0;

    virtual void SignalFrameUpdated(const ViewerViewport& viewport,
                                    size_t currentFrame,
                                    size_t countFrames,
                                    DisplayedFrameQuality quality,
                                    unsigned int instanceNumber) = 0;

    // "click" is a 3D vector in world coordinates
    virtual void SignalCrosshair(const ViewerViewport& viewport,
                                 const OrthancStone::Vector& click) = 0;

    virtual void SignalSynchronizedBrowsing(const ViewerViewport& viewport,
                                            const OrthancStone::Vector& click,
                                            const OrthancStone::Vector& normal) = 0;

    virtual void SignalWindowingUpdated(const ViewerViewport& viewport,
                                        double windowingCenter,
                                        double windowingWidth) = 0;

    virtual void SignalStoneAnnotationsChanged(const ViewerViewport& viewport,
                                               const std::string& sopInstanceUid,
                                               size_t frame) = 0;

    virtual void SignalStoneAnnotationAdded(const ViewerViewport& viewport) = 0;

    virtual void SignalStoneAnnotationRemoved(const ViewerViewport& viewport) = 0;
  };

private:
  static const int LAYER_TEXTURE = 0;
  static const int LAYER_REFERENCE_LINES = 1;
  static const int LAYER_ANNOTATIONS_OSIRIX = 2;
  static const int LAYER_ANNOTATIONS_STONE = 3;

  
  class ICommand : public Orthanc::IDynamicObject
  {
  private:
    boost::shared_ptr<ViewerViewport>  viewport_;
    
  public:
    explicit ICommand(boost::shared_ptr<ViewerViewport> viewport) :
      viewport_(viewport)
    {
      if (viewport == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
    }
    
    virtual ~ICommand()
    {
    }

    ViewerViewport& GetViewport() const
    {
      assert(viewport_ != NULL);
      return *viewport_;
    }
    
    virtual void Handle(const OrthancStone::DicomResourcesLoader::SuccessMessage& message) const
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
    
    virtual void Handle(const OrthancStone::HttpCommand::SuccessMessage& message) const
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }

    virtual void Handle(const OrthancStone::ParseDicomSuccessMessage& message) const
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
    }
  };

  class LoadSeriesDetailsFromInstance : public ICommand
  {
  public:
    explicit LoadSeriesDetailsFromInstance(boost::shared_ptr<ViewerViewport> viewport) :
      ICommand(viewport)
    {
    }
    
    virtual void Handle(const OrthancStone::DicomResourcesLoader::SuccessMessage& message) const ORTHANC_OVERRIDE
    {
      if (message.GetResources()->GetSize() != 1)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
      }

      const Orthanc::DicomMap& dicom = message.GetResources()->GetResource(0);
      
      {
        OrthancStone::DicomInstanceParameters params(dicom);
        
        params.EnrichUsingDicomWeb(message.GetResources()->GetSourceJson(0));
        GetViewport().centralPixelSpacingX_ = params.GetPixelSpacingX();
        GetViewport().centralPixelSpacingY_ = params.GetPixelSpacingY();

        if (params.HasPixelSpacing())
        {
          GetViewport().stoneAnnotations_->SetUnits(OrthancStone::Units_Millimeters);
        }
        else
        {
          GetViewport().stoneAnnotations_->SetUnits(OrthancStone::Units_Pixels);
        }

        if (params.GetPixelSpacingX() != 0 &&
            params.GetPixelSpacingY() != 0 &&
            params.GetWidth() != 0 &&
            params.GetHeight())
        {
          GetViewport().centralPhysicalWidth_ = (params.GetPixelSpacingX() *
                                                 static_cast<double>(params.GetWidth()));
          GetViewport().centralPhysicalHeight_ = (params.GetPixelSpacingY() *
                                                  static_cast<double>(params.GetHeight()));
        }

        GetViewport().windowingPresetCenters_.resize(params.GetWindowingPresetsCount());
        GetViewport().windowingPresetWidths_.resize(params.GetWindowingPresetsCount());

        for (size_t i = 0; i < params.GetWindowingPresetsCount(); i++)
        {
          LOG(INFO) << "Preset windowing " << (i + 1) << "/" << params.GetWindowingPresetsCount()
                    << ": " << params.GetWindowingPresetCenter(i)
                    << "," << params.GetWindowingPresetWidth(i);

          GetViewport().windowingPresetCenters_[i] = params.GetWindowingPresetCenter(i);
          GetViewport().windowingPresetWidths_[i] = params.GetWindowingPresetWidth(i);
        }

        if (params.GetWindowingPresetsCount() == 0)
        {
          LOG(INFO) << "No preset windowing";
        }

        GetViewport().SetWindowingPreset();
      }

      uint32_t cineRate;
      if (dicom.ParseUnsignedInteger32(cineRate, Orthanc::DICOM_TAG_CINE_RATE) &&
          cineRate > 0)
      {
        /**
         * If we detect a cine sequence, start on the first frame
         * instead of on the middle frame.
         **/
        GetViewport().cursor_->SetCurrentIndex(0);
        GetViewport().cineRate_ = cineRate;
      }
      else
      {
        GetViewport().cineRate_ = DEFAULT_CINE_RATE;
      }

      GetViewport().Redraw();

      if (GetViewport().observer_.get() != NULL)
      {
        GetViewport().observer_->SignalSeriesDetailsReady(GetViewport());
      }
    }
  };


  class SetLowQualityFrame : public ICommand
  {
  private:
    std::string   sopInstanceUid_;
    unsigned int  frameNumber_;
    float         windowCenter_;
    float         windowWidth_;
    bool          isMonochrome1_;
    bool          isPrefetch_;
    
  public:
    SetLowQualityFrame(boost::shared_ptr<ViewerViewport> viewport,
                       const std::string& sopInstanceUid,
                       unsigned int frameNumber,
                       float windowCenter,
                       float windowWidth,
                       bool isMonochrome1,
                       bool isPrefetch) :
      ICommand(viewport),
      sopInstanceUid_(sopInstanceUid),
      frameNumber_(frameNumber),
      windowCenter_(windowCenter),
      windowWidth_(windowWidth),
      isMonochrome1_(isMonochrome1),
      isPrefetch_(isPrefetch)
    {
    }
    
    virtual void Handle(const OrthancStone::HttpCommand::SuccessMessage& message) const ORTHANC_OVERRIDE
    {
      std::unique_ptr<Orthanc::JpegReader> jpeg(new Orthanc::JpegReader);
      jpeg->ReadFromMemory(message.GetAnswer());

      std::unique_ptr<Orthanc::ImageAccessor> converted;
      
      switch (jpeg->GetFormat())
      {
        case Orthanc::PixelFormat_RGB24:
          converted.reset(jpeg.release());
          break;

        case Orthanc::PixelFormat_Grayscale8:
        {
          if (isMonochrome1_)
          {
            Orthanc::ImageProcessing::Invert(*jpeg);
          }

          converted.reset(new Orthanc::Image(Orthanc::PixelFormat_Float32, jpeg->GetWidth(),
                                             jpeg->GetHeight(), false));

          Orthanc::ImageProcessing::Convert(*converted, *jpeg);

          /**

             Orthanc::ImageProcessing::ShiftScale() computes "(x + offset) * scaling".
             The system to solve is thus:           

             (0 + offset) * scaling = windowingCenter - windowingWidth / 2     [a]
             (255 + offset) * scaling = windowingCenter + windowingWidth / 2   [b]

             Resolution:

             [b - a] => 255 * scaling = windowingWidth
             [a] => offset = (windowingCenter - windowingWidth / 2) / scaling

          **/

          const float scaling = windowWidth_ / 255.0f;
          const float offset = (OrthancStone::LinearAlgebra::IsCloseToZero(scaling) ? 0 :
                                (windowCenter_ - windowWidth_ / 2.0f) / scaling);

          Orthanc::ImageProcessing::ShiftScale(*converted, offset, scaling, false);
          break;
        }

        default:
          throw Orthanc::OrthancException(Orthanc::ErrorCode_NotImplemented);
      }

      assert(converted.get() != NULL);
      GetViewport().RenderCurrentSceneFromCommand(*converted, sopInstanceUid_, frameNumber_, DisplayedFrameQuality_Low);
      GetViewport().framesCache_->Acquire(sopInstanceUid_, frameNumber_, converted.release(), QUALITY_JPEG);

      if (isPrefetch_)
      {
        GetViewport().ScheduleNextPrefetch();
      }
    }
  };


  class SetFullDicomFrame : public ICommand
  {
  private:
    std::string   sopInstanceUid_;
    unsigned int  frameNumber_;
    int           priority_;
    bool          isPrefetch_;
    bool          serverSideTranscoding_;
    
  public:
    SetFullDicomFrame(boost::shared_ptr<ViewerViewport> viewport,
                      const std::string& sopInstanceUid,
                      unsigned int frameNumber,
                      int priority,
                      bool isPrefetch,
                      bool serverSideTranscoding) :
      ICommand(viewport),
      sopInstanceUid_(sopInstanceUid),
      frameNumber_(frameNumber),
      priority_(priority),
      isPrefetch_(isPrefetch),
      serverSideTranscoding_(serverSideTranscoding)
    {
    }
    
    virtual void Handle(const OrthancStone::ParseDicomSuccessMessage& message) const ORTHANC_OVERRIDE
    {
      std::unique_ptr<Orthanc::ImageAccessor> frame;
      
      try
      {
        frame.reset(message.GetDicom().DecodeFrame(frameNumber_));
      }
      catch (Orthanc::OrthancException& e)
      {
        if (e.GetErrorCode() == Orthanc::ErrorCode_NotImplemented)
        {
          if (!serverSideTranscoding_)
          {
            // If we haven't tried server-side rendering yet, give it a try
            LOG(INFO) << "Switching to server-side transcoding";
            GetViewport().serverSideTranscoding_ = true;
            GetViewport().ScheduleLoadFullDicomFrame(sopInstanceUid_, frameNumber_, priority_, isPrefetch_);
          }
          return;
        }
        else
        {
          throw;
        }
      }

      if (frame.get() == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
      }
      else
      {
        Apply(GetViewport(), message.GetDicom(), frame.release(), sopInstanceUid_, frameNumber_);

        if (isPrefetch_)
        {
          GetViewport().ScheduleNextPrefetch();
        }
      }
    }

    static void Apply(ViewerViewport& viewport,
                      const Orthanc::ParsedDicomFile& dicom,
                      Orthanc::ImageAccessor* f,
                      const std::string& sopInstanceUid,
                      unsigned int frameNumber)
    {
      std::unique_ptr<Orthanc::ImageAccessor> frameProtection(f);
      
      Orthanc::DicomMap tags;
      dicom.ExtractDicomSummary(tags, ORTHANC_STONE_MAX_TAG_LENGTH);

      std::unique_ptr<Orthanc::ImageAccessor> converted;
      
      if (frameProtection->GetFormat() == Orthanc::PixelFormat_RGB24)
      {
        converted.reset(frameProtection.release());
      }
      else
      {
        double a = 1;
        double b = 0;

        double doseScaling;
        if (tags.ParseDouble(doseScaling, Orthanc::DICOM_TAG_DOSE_GRID_SCALING))
        {
          a = doseScaling;
        }
      
        double rescaleIntercept, rescaleSlope;
        if (tags.ParseDouble(rescaleIntercept, Orthanc::DICOM_TAG_RESCALE_INTERCEPT) &&
            tags.ParseDouble(rescaleSlope, Orthanc::DICOM_TAG_RESCALE_SLOPE))
        {
          a *= rescaleSlope;
          b = rescaleIntercept;
        }

        converted.reset(new Orthanc::Image(Orthanc::PixelFormat_Float32, frameProtection->GetWidth(), frameProtection->GetHeight(), false));
        Orthanc::ImageProcessing::Convert(*converted, *frameProtection);
        Orthanc::ImageProcessing::ShiftScale2(*converted, b, a, false);        
      }

      assert(converted.get() != NULL);
      viewport.RenderCurrentSceneFromCommand(*converted, sopInstanceUid, frameNumber, DisplayedFrameQuality_High);
      viewport.framesCache_->Acquire(sopInstanceUid, frameNumber, converted.release(), QUALITY_FULL);
    }
  };


  class PrefetchItem
  {
  private:
    size_t   cursorIndex_;
    bool     isFullQuality_;

  public:
    PrefetchItem(size_t cursorIndex,
                 bool isFullQuality) :
      cursorIndex_(cursorIndex),
      isFullQuality_(isFullQuality)
    {
    }

    size_t GetCursorIndex() const
    {
      return cursorIndex_;
    }

    bool IsFullQuality() const
    {
      return isFullQuality_;
    }
  };
  

  std::unique_ptr<IObserver>                   observer_;
  OrthancStone::WebAssemblyLoadersContext&               context_;
  boost::shared_ptr<OrthancStone::WebAssemblyViewport>   viewport_;
  boost::shared_ptr<OrthancStone::DicomResourcesLoader> loader_;
  OrthancStone::DicomSource                    source_;
  boost::shared_ptr<FramesCache>               framesCache_;  
  std::unique_ptr<OrthancStone::SortedFrames>  frames_;
  std::unique_ptr<SeriesCursor>                cursor_;
  float                                        windowingCenter_;
  float                                        windowingWidth_;
  std::vector<float>                           windowingPresetCenters_;
  std::vector<float>                           windowingPresetWidths_;
  unsigned int                                 cineRate_;
  bool                                         inverted_;
  bool                                         flipX_;
  bool                                         flipY_;
  bool                                         fitNextContent_;
  std::list<PrefetchItem>                      prefetchQueue_;
  bool                                         serverSideTranscoding_;
  OrthancStone::Vector                         synchronizationOffset_;
  bool                                         synchronizationEnabled_;
  double                                       centralPhysicalWidth_;   // LSD-479
  double                                       centralPhysicalHeight_;
  double                                       centralPixelSpacingX_;
  double                                       centralPixelSpacingY_;

  bool         hasFocusOnInstance_;
  std::string  focusSopInstanceUid_;
  size_t       focusFrameNumber_;

  // The coordinates of OsiriX annotations are expressed in 3D world coordinates
  boost::shared_ptr<OrthancStone::OsiriX::CollectionOfAnnotations>  osiriXAnnotations_;

  // The coordinates of Stone annotations are expressed in 2D
  // coordinates of the current texture, with (0,0) corresponding to
  // the center of the top-left pixel
  boost::shared_ptr<OrthancStone::AnnotationsSceneLayer>  stoneAnnotations_;


  void ScheduleNextPrefetch()
  {
    while (!prefetchQueue_.empty())
    {
      size_t cursorIndex = prefetchQueue_.front().GetCursorIndex();
      bool isFullQuality = prefetchQueue_.front().IsFullQuality();
      prefetchQueue_.pop_front();
      
      const std::string sopInstanceUid = frames_->GetInstanceOfFrame(cursorIndex).GetSopInstanceUid();
      unsigned int frameNumber = frames_->GetFrameNumberInInstance(cursorIndex);

      {
        FramesCache::Accessor accessor(*framesCache_, sopInstanceUid, frameNumber);
        if (!accessor.IsValid() ||
            (isFullQuality && accessor.GetQuality() == 0))
        {
          if (isFullQuality)
          {
            ScheduleLoadFullDicomFrame(cursorIndex, PRIORITY_NORMAL, true);
          }
          else
          {
            ScheduleLoadRenderedFrame(cursorIndex, PRIORITY_NORMAL, true);
          }
          
          return;  // We have found a new frame to cache, stop the lookup loop
        }
      }
    }
  }
  
  
  void ClearViewport()
  {
    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());
      lock->GetController().GetScene().DeleteLayer(LAYER_TEXTURE);
      //lock->GetCompositor().Refresh(lock->GetController().GetScene());
      lock->Invalidate();
    }
  }


  /**
   * NB: "frame" is only used to estimate the memory size to store 1
   * frame, in order to avoid prefetching too much data.
   **/
  void SetupPrefetchAfterRendering(const Orthanc::ImageAccessor& frame,
                                   DisplayedFrameQuality quality)
  {
    const size_t cursorIndex = cursor_->GetCurrentIndex();

    // Prepare prefetching
    prefetchQueue_.clear();

    if (1)  // DISABLE PREFETCHING
    {
      const size_t frameSize = frame.GetPitch() * frame.GetHeight();
      size_t prefetchedSize = 0;
    
      for (size_t i = 0; i < cursor_->GetPrefetchSize() && i < 16 &&
             prefetchedSize <= framesCache_->GetMaximumSize() / 2; i++)
      {
        size_t a = cursor_->GetPrefetchIndex(i);
        if (a != cursorIndex)
        {
          prefetchQueue_.push_back(PrefetchItem(a, i < 2));
          prefetchedSize += frameSize;
        }
      }
    }

    ScheduleNextPrefetch();
    
    if (frames_.get() != NULL &&
        cursor_.get() != NULL &&
        observer_.get() != NULL)
    {
      const Orthanc::DicomMap& instance = frames_->GetInstanceOfFrame(cursor_->GetCurrentIndex()).GetTags();

      uint32_t instanceNumber;
      if (!instance.ParseUnsignedInteger32(instanceNumber, Orthanc::DICOM_TAG_INSTANCE_NUMBER))
      {
        instanceNumber = 0;
      }
      
      observer_->SignalFrameUpdated(*this, cursorIndex, frames_->GetFramesCount(), quality, instanceNumber);
    }
  }
  
  
  void RenderCurrentScene(const Orthanc::ImageAccessor& frame,
                          const OrthancStone::DicomInstanceParameters& instance,
                          size_t frameIndex,
                          const OrthancStone::CoordinateSystem3D& plane)
  {
    /**
     * IMPORTANT - DO NOT use "instance.GetWidth()" and
     * "instance.GetHeight()" in this method. Use the information from
     * "frame" instead. Indeed, the "instance" information is taken
     * from DICOMweb "/studies/.../series/.../metadata". But,
     * "SeriesMetadataExtrapolatedTags" includes the "Columns" and
     * "Rows" DICOM tags for performance, which make this information
     * unreliable if the series includes instances with varying sizes
     * (cf. LSD-479).
     **/
    
    bool isMonochrome1 = (instance.GetImageInformation().GetPhotometricInterpretation() ==
                          Orthanc::PhotometricInterpretation_Monochrome1);
      
    std::unique_ptr<OrthancStone::TextureBaseSceneLayer> layer;

    switch (frame.GetFormat())
    {
      case Orthanc::PixelFormat_RGB24:
        layer.reset(new OrthancStone::ColorTextureSceneLayer(frame));
        break;

      case Orthanc::PixelFormat_Float32:
      {
        std::unique_ptr<OrthancStone::FloatTextureSceneLayer> tmp(
          new OrthancStone::FloatTextureSceneLayer(frame));
        tmp->SetCustomWindowing(windowingCenter_, windowingWidth_);
        tmp->SetInverted(inverted_ ^ isMonochrome1);
        layer.reset(tmp.release());
        break;
      }

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_IncompatibleImageFormat);
    }

    assert(layer.get() != NULL);

    layer->SetLinearInterpolation(true);
    layer->SetFlipX(flipX_);
    layer->SetFlipY(flipY_);

    double pixelSpacingX, pixelSpacingY;

    if (instance.HasPixelSpacing())
    {
      pixelSpacingX = instance.GetPixelSpacingX();
      pixelSpacingY = instance.GetPixelSpacingY();
    }
    else
    {
      pixelSpacingX = centralPixelSpacingX_;
      pixelSpacingY = centralPixelSpacingY_;
    }

    if (FIX_LSD_479)
    {
      /**
       * Some series contain a first instance (secondary capture) that
       * is completely different from others wrt. to resolution and
       * pixel spacing. We make sure to rescale each frame to fit in a
       * square that corresponds to the extent of the frame in the
       * middle of the series.
       **/
      double physicalWidth = pixelSpacingX * static_cast<double>(frame.GetWidth()); 
      double physicalHeight = pixelSpacingY * static_cast<double>(frame.GetHeight());

      if (OrthancStone::LinearAlgebra::IsCloseToZero(physicalWidth) ||
          OrthancStone::LinearAlgebra::IsCloseToZero(physicalHeight))
      {
        // Numerical instability, don't try further processing
        layer->SetPixelSpacing(pixelSpacingX, pixelSpacingY);
      }
      else
      {
        double scale = std::max(centralPhysicalWidth_ / physicalWidth,
                                centralPhysicalHeight_ / physicalHeight);
        layer->SetPixelSpacing(pixelSpacingX * scale, pixelSpacingY * scale);
        layer->SetOrigin((centralPhysicalWidth_ - physicalWidth * scale) / 2.0,
                         (centralPhysicalHeight_ - physicalHeight * scale) / 2.0);
      }
    }
    else
    {
      layer->SetPixelSpacing(pixelSpacingX, pixelSpacingY);
    }

    std::unique_ptr<OrthancStone::MacroSceneLayer>  annotationsOsiriX;

    if (osiriXAnnotations_)
    {
      std::set<size_t> a;
      osiriXAnnotations_->LookupSopInstanceUid(a, instance.GetSopInstanceUid());
      if (plane.IsValid() &&
          !a.empty())
      {
        annotationsOsiriX.reset(new OrthancStone::MacroSceneLayer);
        // annotationsOsiriX->Reserve(a.size());

        OrthancStone::OsiriXLayerFactory factory;
        factory.SetColor(0, 255, 0);
          
        for (std::set<size_t>::const_iterator it = a.begin(); it != a.end(); ++it)
        {
          const OrthancStone::OsiriX::Annotation& annotation = osiriXAnnotations_->GetAnnotation(*it);
          annotationsOsiriX->AddLayer(factory.Create(annotation, plane));
        }
      }
    }

    StoneAnnotationsRegistry::GetInstance().Load(*stoneAnnotations_, instance.GetSopInstanceUid(), frameIndex);

    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());

      OrthancStone::Scene2D& scene = lock->GetController().GetScene();

      scene.SetLayer(LAYER_TEXTURE, layer.release());

      if (annotationsOsiriX.get() != NULL)
      {
        scene.SetLayer(LAYER_ANNOTATIONS_OSIRIX, annotationsOsiriX.release());
      }
      else
      {
        scene.DeleteLayer(LAYER_ANNOTATIONS_OSIRIX);
      }

      stoneAnnotations_->Render(scene);  // Necessary for "FitContent()" to work

      if (fitNextContent_)
      {
        lock->RefreshCanvasSize();
        lock->GetCompositor().FitContent(scene);
        stoneAnnotations_->Render(scene);
        fitNextContent_ = false;
      }
        
      //lock->GetCompositor().Refresh(scene);
      lock->Invalidate();
    }
  }


  void RenderCurrentSceneFromCommand(const Orthanc::ImageAccessor& frame,
                                     const std::string& loadedSopInstanceUid,
                                     unsigned int loadedFrameNumber,
                                     DisplayedFrameQuality quality)
  {
    if (cursor_.get() != NULL &&
        frames_.get() != NULL)
    {
      const size_t cursorIndex = cursor_->GetCurrentIndex();
      const OrthancStone::DicomInstanceParameters& instance = frames_->GetInstanceOfFrame(cursorIndex);
      const size_t frameNumber = frames_->GetFrameNumberInInstance(cursorIndex);

      // Only change the scene if the loaded frame still corresponds to the current cursor
      if (instance.GetSopInstanceUid() == loadedSopInstanceUid &&
          frameNumber == loadedFrameNumber)
      {
        const OrthancStone::CoordinateSystem3D plane = frames_->GetFrameGeometry(cursorIndex);
        
        if (quality == DisplayedFrameQuality_Low)
        {
          FramesCache::Accessor accessor(*framesCache_, instance.GetSopInstanceUid(), frameNumber);
          if (accessor.IsValid() &&
              accessor.GetQuality() == QUALITY_FULL)
          {
            // A high-res image was downloaded in between: Use this cached image instead of the low-res
            RenderCurrentScene(accessor.GetImage(), instance, frameNumber, plane);
            SetupPrefetchAfterRendering(frame, DisplayedFrameQuality_High);
          }
          else
          {
            // This frame is only available in low-res: Download the full DICOM
            RenderCurrentScene(frame, instance, frameNumber, plane);
            SetupPrefetchAfterRendering(frame, quality);

            /**
             * The command "SetupPrefetchAfterRendering()" must be
             * after "SetupPrefetchAfterRendering(quality)", as the
             * DICOM instance might already be cached by the oracle,
             * which makes a call to "observer_->SignalFrameUpdated()"
             * with a low quality, whereas the high quality is
             * available.
             **/
            ScheduleLoadFullDicomFrame(cursorIndex, PRIORITY_HIGH, false /* not a prefetch */);
          }
        }
        else
        {
          assert(quality == DisplayedFrameQuality_High);
          SetupPrefetchAfterRendering(frame, quality);
          RenderCurrentScene(frame, instance, frameNumber, plane);
        }
      }
    }
  }

  void ScheduleLoadFullDicomFrame(const std::string& sopInstanceUid,
                                  unsigned int frameNumber,
                                  int priority,
                                  bool isPrefetch)
  {
    if (frames_.get() != NULL)
    {
      std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(context_.Lock());
      lock->Schedule(
        GetSharedObserver(), priority, OrthancStone::ParseDicomFromWadoCommand::Create(
          source_, frames_->GetStudyInstanceUid(), frames_->GetSeriesInstanceUid(),
          sopInstanceUid, serverSideTranscoding_,
          Orthanc::DicomTransferSyntax_LittleEndianExplicit,
          new SetFullDicomFrame(GetSharedObserver(), sopInstanceUid, frameNumber, priority, isPrefetch, serverSideTranscoding_)));
    }
  }

  void ScheduleLoadFullDicomFrame(size_t cursorIndex,
                                  int priority,
                                  bool isPrefetch)
  {
    if (frames_.get() != NULL)
    {
      std::string sopInstanceUid = frames_->GetInstanceOfFrame(cursorIndex).GetSopInstanceUid();
      unsigned int frameNumber = frames_->GetFrameNumberInInstance(cursorIndex);
      ScheduleLoadFullDicomFrame(sopInstanceUid, frameNumber, priority, isPrefetch);
    }
  }

  void ScheduleLoadRenderedFrame(size_t cursorIndex,
                                 int priority,
                                 bool isPrefetch)
  {
    if (!source_.HasDicomWebRendered())
    {
      ScheduleLoadFullDicomFrame(cursorIndex, priority, isPrefetch);
    }
    else if (frames_.get() != NULL)
    {
      const OrthancStone::DicomInstanceParameters& instance = frames_->GetInstanceOfFrame(cursorIndex);
      unsigned int frameNumber = frames_->GetFrameNumberInInstance(cursorIndex);

      /**
       * If the full-resolution DICOM file is already available in the
       * cache of the oracle, bypass the loading of the "rendered" and
       * use the cached DICOM file.
       **/
      std::unique_ptr<OrthancStone::WebAssemblyOracle::CachedInstanceAccessor> accessor(
        context_.AccessCachedInstance(instance.GetSopInstanceUid()));

      if (accessor.get() != NULL &&
          accessor->IsValid())
      {
        try
        {
          std::unique_ptr<Orthanc::ImageAccessor> frame(accessor->GetDicom().DecodeFrame(frameNumber));
          SetFullDicomFrame::Apply(*this, accessor->GetDicom(), frame.release(), instance.GetSopInstanceUid(), frameNumber);
          return;  // Success
        }
        catch (Orthanc::OrthancException&)
        {
          /**
           * This happens if the cached DICOM file uses a transfer
           * syntax that is not supported by DCMTK (such as
           * JPEG2k). Fallback to "/rendered" in order to re-download
           * the DICOM file using server-side transcoding. This
           * happens on WRIX dataset.
           **/
        }
      }

      bool isMonochrome1 = (instance.GetImageInformation().GetPhotometricInterpretation() ==
                            Orthanc::PhotometricInterpretation_Monochrome1);

      const std::string uri = ("studies/" + frames_->GetStudyInstanceUid() +
                               "/series/" + frames_->GetSeriesInstanceUid() +
                               "/instances/" + instance.GetSopInstanceUid() +
                               "/frames/" + boost::lexical_cast<std::string>(frameNumber + 1) + "/rendered");

      std::map<std::string, std::string> headers, arguments;
      // arguments["quality"] = "10";   // Low-level quality for test purpose
      arguments["window"] = (
        boost::lexical_cast<std::string>(windowingCenter_) + ","  +
        boost::lexical_cast<std::string>(windowingWidth_) + ",linear");

      std::unique_ptr<OrthancStone::IOracleCommand> command(
        source_.CreateDicomWebCommand(
          uri, arguments, headers, new SetLowQualityFrame(
            GetSharedObserver(), instance.GetSopInstanceUid(), frameNumber,
            windowingCenter_, windowingWidth_, isMonochrome1, isPrefetch)));

      {
        std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(context_.Lock());
        lock->Schedule(GetSharedObserver(), priority, command.release());
      }
    }
  }

  void UpdateCurrentTextureParameters()
  {
    std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());

    if (lock->GetController().GetScene().HasLayer(LAYER_TEXTURE))
    {
      if (lock->GetController().GetScene().GetLayer(LAYER_TEXTURE).GetType() ==
          OrthancStone::ISceneLayer::Type_FloatTexture)
      {
        dynamic_cast<OrthancStone::FloatTextureSceneLayer&>(
          lock->GetController().GetScene().GetLayer(LAYER_TEXTURE)).
          SetCustomWindowing(windowingCenter_, windowingWidth_);
      }

      {
        OrthancStone::TextureBaseSceneLayer& layer = 
          dynamic_cast<OrthancStone::TextureBaseSceneLayer&>(
            lock->GetController().GetScene().GetLayer(LAYER_TEXTURE));

        layer.SetFlipX(flipX_);
        layer.SetFlipY(flipY_);
      }
        
      lock->Invalidate();
    }
  }
  

  ViewerViewport(OrthancStone::WebAssemblyLoadersContext& context,
                 const OrthancStone::DicomSource& source,
                 const std::string& canvas,
                 boost::shared_ptr<FramesCache> cache,
                 bool softwareRendering) :
    context_(context),
    source_(source),
    framesCache_(cache),
    fitNextContent_(true),
    flipX_(false),
    flipY_(false),
    hasFocusOnInstance_(false),
    focusFrameNumber_(0),
    synchronizationOffset_(OrthancStone::LinearAlgebra::CreateVector(0, 0, 0)),
    synchronizationEnabled_(false),
    centralPhysicalWidth_(1),
    centralPhysicalHeight_(1),
    centralPixelSpacingX_(1),
    centralPixelSpacingY_(1)
  {
    if (!framesCache_)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }

    if (softwareRendering)
    {
      LOG(INFO) << "Creating Cairo viewport in canvas: " << canvas;
      viewport_ = OrthancStone::WebAssemblyCairoViewport::Create(canvas);
    }
    else
    {
      LOG(INFO) << "Creating WebGL viewport in canvas: " << canvas;
      viewport_ = OrthancStone::WebGLViewport::Create(canvas);
    }

    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());
      std::string ttf;
      Orthanc::EmbeddedResources::GetFileResource(ttf, Orthanc::EmbeddedResources::UBUNTU_FONT);
      lock->GetCompositor().SetFont(0, ttf, 16 /* font size */, Orthanc::Encoding_Latin1);
    }
    
    emscripten_set_wheel_callback(viewport_->GetCanvasCssSelector().c_str(), this, true, OnWheel);

    SetWindowingPreset();

    stoneAnnotations_.reset(new OrthancStone::AnnotationsSceneLayer(LAYER_ANNOTATIONS_STONE));
  }


  void Handle(const OrthancStone::ViewportController::GrayscaleWindowingChanged& message)
  {
    // This event is triggered by the windowing mouse action, from class "GrayscaleWindowingSceneTracker"
    windowingCenter_ = message.GetWindowingCenter();
    windowingWidth_ = message.GetWindowingWidth();

    if (observer_.get() != NULL)
    {
      observer_->SignalWindowingUpdated(*this, message.GetWindowingCenter(), message.GetWindowingWidth());
    }
  }

  
  static EM_BOOL OnWheel(int eventType,
                         const EmscriptenWheelEvent *wheelEvent,
                         void *userData)
  {
    ViewerViewport& that = *reinterpret_cast<ViewerViewport*>(userData);

    if (that.frames_.get() != NULL &&
        that.cursor_.get() != NULL)
    {
      const bool isCtrl = wheelEvent->mouse.ctrlKey;
      const bool isShift = wheelEvent->mouse.shiftKey;
      
      const size_t previousCursorIndex = that.cursor_->GetCurrentIndex();
      
      if (wheelEvent->deltaY < 0)
      {
        that.ChangeFrame(isCtrl ? SeriesCursor::Action_FastMinus :
                         SeriesCursor::Action_Minus, false /* not circular */);
      }
      else if (wheelEvent->deltaY > 0)
      {
        that.ChangeFrame(isCtrl ? SeriesCursor::Action_FastPlus :
                         SeriesCursor::Action_Plus, false /* not circular */);
      }

      if (that.synchronizationEnabled_)
      {
        const size_t currentCursorIndex = that.cursor_->GetCurrentIndex();

        const OrthancStone::CoordinateSystem3D current =
          that.frames_->GetFrameGeometry(currentCursorIndex);
      
        if (isShift &&
            previousCursorIndex != currentCursorIndex)
        {
          const OrthancStone::CoordinateSystem3D previous =
            that.frames_->GetFrameGeometry(previousCursorIndex);
          that.synchronizationOffset_ += previous.GetOrigin() - current.GetOrigin();
        }

        that.observer_->SignalSynchronizedBrowsing(
          that, current.GetOrigin() + that.synchronizationOffset_, current.GetNormal());
      }
    }
    
    return true;
  }

  void Handle(const OrthancStone::DicomResourcesLoader::SuccessMessage& message)
  {
    dynamic_cast<const ICommand&>(message.GetUserPayload()).Handle(message);
  }

  void Handle(const OrthancStone::HttpCommand::SuccessMessage& message)
  {
    dynamic_cast<const ICommand&>(message.GetOrigin().GetPayload()).Handle(message);
  }

  void Handle(const OrthancStone::ParseDicomSuccessMessage& message)
  {
    dynamic_cast<const ICommand&>(message.GetOrigin().GetPayload()).Handle(message);
  }


  void RefreshAnnotations(bool save)
  {
    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());
      stoneAnnotations_->Render(lock->GetController().GetScene());
      lock->Invalidate();
    }

    if (save)
    {
      if (cursor_.get() != NULL &&
          frames_.get() != NULL)
      {
        const size_t cursorIndex = cursor_->GetCurrentIndex();
        const OrthancStone::DicomInstanceParameters& instance = frames_->GetInstanceOfFrame(cursorIndex);
        const size_t frameNumber = frames_->GetFrameNumberInInstance(cursorIndex);

        StoneAnnotationsRegistry::GetInstance().Save(instance.GetSopInstanceUid(), frameNumber, *stoneAnnotations_);

        if (observer_.get() != NULL)
        {
          observer_->SignalStoneAnnotationsChanged(*this, instance.GetSopInstanceUid(), frameNumber);
        }
      }
    }
  }
  
  void Handle(const OrthancStone::ViewportController::SceneTransformChanged& message)
  {
    RefreshAnnotations(false /* don't save */);
  }

  void Handle(const OrthancStone::AnnotationsSceneLayer::AnnotationChangedMessage& message)
  {
    RefreshAnnotations(true /* save */);
  }

  void Handle(const OrthancStone::AnnotationsSceneLayer::AnnotationAddedMessage& message)
  {
    RefreshAnnotations(true /* save */);

    if (observer_.get() != NULL)
    {
      observer_->SignalStoneAnnotationAdded(*this);
    }
  }

  void Handle(const OrthancStone::AnnotationsSceneLayer::AnnotationRemovedMessage& message)
  {
    RefreshAnnotations(true /* save */);

    if (observer_.get() != NULL)
    {
      observer_->SignalStoneAnnotationRemoved(*this);
    }
  }

public:
  virtual ~ViewerViewport()
  {
    // Unregister the callbacks to avoid any call with a "void*" that
    // has been destroyed. "WebAssemblyViewport::CreateObjectCookie()"
    // provides a more advanced alternative.
    emscripten_set_wheel_callback(viewport_->GetCanvasCssSelector().c_str(), this, true, NULL);
  }

  static boost::shared_ptr<ViewerViewport> Create(OrthancStone::WebAssemblyLoadersContext& context,
                                                  const OrthancStone::DicomSource& source,
                                                  const std::string& canvas,
                                                  boost::shared_ptr<FramesCache> cache,
                                                  bool softwareRendering)
  {
    boost::shared_ptr<ViewerViewport> viewport(
      new ViewerViewport(context, source, canvas, cache, softwareRendering));

    {
      std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(context.Lock());
    
      viewport->loader_ = OrthancStone::DicomResourcesLoader::Create(*lock);
      viewport->Register<OrthancStone::DicomResourcesLoader::SuccessMessage>(
        *viewport->loader_, &ViewerViewport::Handle);

      viewport->Register<OrthancStone::HttpCommand::SuccessMessage>(
        lock->GetOracleObservable(), &ViewerViewport::Handle);

      viewport->Register<OrthancStone::ParseDicomSuccessMessage>(
        lock->GetOracleObservable(), &ViewerViewport::Handle);

      viewport->Register<OrthancStone::AnnotationsSceneLayer::AnnotationChangedMessage>(
        *viewport->stoneAnnotations_, &ViewerViewport::Handle);

      viewport->Register<OrthancStone::AnnotationsSceneLayer::AnnotationAddedMessage>(
        *viewport->stoneAnnotations_, &ViewerViewport::Handle);

      viewport->Register<OrthancStone::AnnotationsSceneLayer::AnnotationRemovedMessage>(
        *viewport->stoneAnnotations_, &ViewerViewport::Handle);
    }

    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport->viewport_->Lock());
      viewport->Register<OrthancStone::ViewportController::GrayscaleWindowingChanged>(lock->GetController(), &ViewerViewport::Handle);
      viewport->Register<OrthancStone::ViewportController::SceneTransformChanged>(lock->GetController(), &ViewerViewport::Handle);
    }

    return viewport;    
  }

  void SetFrames(OrthancStone::SortedFrames* frames)
  {
    if (frames == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
    }

    flipX_ = false;
    flipY_ = false;
    fitNextContent_ = true;
    cineRate_ = DEFAULT_CINE_RATE;
    inverted_ = false;
    serverSideTranscoding_ = false;
    OrthancStone::LinearAlgebra::AssignVector(synchronizationOffset_, 0, 0, 0);

    frames_.reset(frames);
    cursor_.reset(new SeriesCursor(frames_->GetFramesCount()));

    LOG(INFO) << "Number of frames in series: " << frames_->GetFramesCount();

    SetWindowingPreset();
    ClearViewport();
    prefetchQueue_.clear();

    if (observer_.get() != NULL)
    {
      observer_->SignalFrameUpdated(*this, cursor_->GetCurrentIndex(),
                                    frames_->GetFramesCount(), DisplayedFrameQuality_None, 0);
    }

    centralPhysicalWidth_ = 1;
    centralPhysicalHeight_ = 1;

    if (frames_->GetFramesCount() != 0)
    {
      const OrthancStone::DicomInstanceParameters& centralInstance = frames_->GetInstanceOfFrame(cursor_->GetCurrentIndex());

      /**
       * Avoid loading metadata if we know that this cannot be a
       * "true" image with pixel data. Retrieving instance metadata on
       * RTSTRUCT can lead to very large JSON whose parsing will
       * freeze the browser for several seconds.
       **/
      const OrthancStone::SopClassUid uid = centralInstance.GetSopClassUid();
      if (uid != OrthancStone::SopClassUid_EncapsulatedPdf &&
          uid != OrthancStone::SopClassUid_RTDose &&
          uid != OrthancStone::SopClassUid_RTPlan &&
          uid != OrthancStone::SopClassUid_RTStruct &&
          GetSeriesThumbnailType(uid) != OrthancStone::SeriesThumbnailType_Video)
      {
        // Fetch the details of the series from the central instance
        const std::string uri = ("studies/" + frames_->GetStudyInstanceUid() +
                                 "/series/" + frames_->GetSeriesInstanceUid() +
                                 "/instances/" + centralInstance.GetSopInstanceUid() + "/metadata");
        
        loader_->ScheduleGetDicomWeb(
          boost::make_shared<OrthancStone::LoadedDicomResources>(Orthanc::DICOM_TAG_SOP_INSTANCE_UID),
          0, source_, uri, new LoadSeriesDetailsFromInstance(GetSharedObserver()));
      }
    }

    ApplyScheduledFocus();
  }


  // This method is used when the layout of the HTML page changes,
  // which does not trigger the "emscripten_set_resize_callback()"
  void UpdateSize(bool fitContent)
  {
    std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());
    lock->RefreshCanvasSize();

    if (fitContent)
    {
      lock->GetCompositor().FitContent(lock->GetController().GetScene());
    }

    stoneAnnotations_->ClearHover();
    stoneAnnotations_->Render(lock->GetController().GetScene());
    
    lock->Invalidate();
  }

  void AcquireObserver(IObserver* observer)
  {  
    observer_.reset(observer);
  }

  const std::string& GetCanvasId() const
  {
    assert(viewport_);
    return viewport_->GetCanvasId();
  }


  void Redraw()
  {
    if (cursor_.get() != NULL &&
        frames_.get() != NULL)
    {
      const size_t cursorIndex = cursor_->GetCurrentIndex();

      const OrthancStone::DicomInstanceParameters& instance = frames_->GetInstanceOfFrame(cursorIndex);
      const size_t frameNumber = frames_->GetFrameNumberInInstance(cursorIndex);

      FramesCache::Accessor accessor(*framesCache_, instance.GetSopInstanceUid(), frameNumber);
      if (accessor.IsValid())
      {
        RenderCurrentScene(accessor.GetImage(), instance, frameNumber, frames_->GetFrameGeometry(cursorIndex));

        DisplayedFrameQuality quality;
        
        if (accessor.GetQuality() < QUALITY_FULL)
        {
          // This frame is only available in low-res: Download the full DICOM
          ScheduleLoadFullDicomFrame(cursorIndex, PRIORITY_HIGH, false /* not a prefetch */);
          quality = DisplayedFrameQuality_Low;
        }
        else
        {
          quality = DisplayedFrameQuality_High;
        }

        SetupPrefetchAfterRendering(accessor.GetImage(), quality);
      }
      else
      {
        // This frame is not cached yet: Load it
        if (source_.HasDicomWebRendered())
        {
          ScheduleLoadRenderedFrame(cursorIndex, PRIORITY_HIGH, false /* not a prefetch */);
        }
        else
        {
          ScheduleLoadFullDicomFrame(cursorIndex, PRIORITY_HIGH, false /* not a prefetch */);
        }
      }
    }
  }


  // Returns "true" iff the frame has indeed changed
  bool ChangeFrame(SeriesCursor::Action action,
                   bool isCircular)
  {
    if (cursor_.get() != NULL)
    {
      size_t previous = cursor_->GetCurrentIndex();
      
      cursor_->Apply(action, isCircular);
      
      size_t current = cursor_->GetCurrentIndex();
      if (previous != current)
      {
        Redraw();
        return true;
      }
    }

    return false;
  }


  void GoToFirstFrame()
  {
    if (cursor_.get() != NULL &&
        cursor_->GetCurrentIndex() != 0)
    {
      cursor_->SetCurrentIndex(0);
      Redraw();
    }
  }


  void GoToLastFrame()
  {
    if (cursor_.get() != NULL)
    {
      size_t last = cursor_->GetFramesCount() - 1;
      if (cursor_->GetCurrentIndex() != last)
      {
        cursor_->SetCurrentIndex(last);
        Redraw();
      }
    }
  }
  

  bool GetCurrentFrameOfReferenceUid(std::string& frameOfReferenceUid) const
  {
    if (cursor_.get() != NULL &&
        frames_.get() != NULL)
    {
      frameOfReferenceUid = frames_->GetInstanceOfFrame(cursor_->GetCurrentIndex()).GetFrameOfReferenceUid();
      return true;
    }
    else
    {
      return false;
    }
  }

  bool GetCurrentPlane(OrthancStone::CoordinateSystem3D& plane) const
  {
    if (cursor_.get() != NULL &&
        frames_.get() != NULL)
    {
      plane = frames_->GetFrameGeometry(cursor_->GetCurrentIndex());      
      return true;
    }
    else
    {
      return false;
    }
  }

  void UpdateReferenceLines(const std::list<const ViewerViewport*>& viewports)
  {
    std::unique_ptr<OrthancStone::PolylineSceneLayer> layer(new OrthancStone::PolylineSceneLayer);
    
    if (cursor_.get() != NULL &&
        frames_.get() != NULL)
    {
      const size_t cursorIndex = cursor_->GetCurrentIndex();
      const OrthancStone::DicomInstanceParameters& instance = frames_->GetInstanceOfFrame(cursorIndex);
      const unsigned int frame = frames_->GetFrameNumberInInstance(cursorIndex);

      for (std::list<const ViewerViewport*>::const_iterator
             it = viewports.begin(); it != viewports.end(); ++it)
      {
        assert(*it != NULL);

        OrthancStone::CoordinateSystem3D otherPlane;
        std::string otherFrameOfReferenceUid;
        if ((*it)->GetCurrentPlane(otherPlane) &&
            (*it)->GetCurrentFrameOfReferenceUid(otherFrameOfReferenceUid) &&
            otherFrameOfReferenceUid == instance.GetFrameOfReferenceUid())
        {
          double x1, y1, x2, y2;
          if (GetReferenceLineCoordinates(x1, y1, x2, y2, instance, frame, otherPlane))
          {
            OrthancStone::PolylineSceneLayer::Chain chain;
            chain.push_back(OrthancStone::ScenePoint2D(x1, y1));
            chain.push_back(OrthancStone::ScenePoint2D(x2, y2));
            layer->AddChain(chain, false, 0, 255, 0);
          }
        }
      }
    }

    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());

      if (layer->GetChainsCount() == 0)
      {
        lock->GetController().GetScene().DeleteLayer(LAYER_REFERENCE_LINES);
      }
      else
      {
        lock->GetController().GetScene().SetLayer(LAYER_REFERENCE_LINES, layer.release());
      }
      
      //lock->GetCompositor().Refresh(lock->GetController().GetScene());
      lock->Invalidate();
    }
  }


  void ClearReferenceLines()
  {
    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());
      lock->GetController().GetScene().DeleteLayer(LAYER_REFERENCE_LINES);
      lock->Invalidate();
    }
  }


  void SetWindowingPreset()
  {
    assert(windowingPresetCenters_.size() == windowingPresetWidths_.size());
    
    if (windowingPresetCenters_.empty())
    {
      SetWindowing(128, 256);
    }
    else
    {
      SetWindowing(windowingPresetCenters_[0], windowingPresetWidths_[0]);
    }
  }

  void SetWindowing(float windowingCenter,
                    float windowingWidth)
  {
    windowingCenter_ = windowingCenter;
    windowingWidth_ = windowingWidth;
    UpdateCurrentTextureParameters();

    if (observer_.get() != NULL)
    {
      observer_->SignalWindowingUpdated(*this, windowingCenter, windowingWidth);
    }
  }

  void FlipX()
  {
    flipX_ = !flipX_;
    UpdateCurrentTextureParameters();
  }

  void FlipY()
  {
    flipY_ = !flipY_;
    UpdateCurrentTextureParameters();
  }

  void Invert()
  {
    inverted_ = !inverted_;
    
    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());

      if (lock->GetController().GetScene().HasLayer(LAYER_TEXTURE) &&
          lock->GetController().GetScene().GetLayer(LAYER_TEXTURE).GetType() ==
          OrthancStone::ISceneLayer::Type_FloatTexture)
      {
        OrthancStone::FloatTextureSceneLayer& layer = 
          dynamic_cast<OrthancStone::FloatTextureSceneLayer&>(
            lock->GetController().GetScene().GetLayer(LAYER_TEXTURE));

        // NB: Using "IsInverted()" instead of "inverted_" is for
        // compatibility with MONOCHROME1 images
        layer.SetInverted(!layer.IsInverted());
        lock->Invalidate();
      }
    }
  }



  class Interactor : public OrthancStone::DefaultViewportInteractor
  {
  private:
    ViewerViewport&  viewer_;
    WebViewerAction  leftAction_;
    WebViewerAction  middleAction_;
    WebViewerAction  rightAction_;

    bool IsAction(const OrthancStone::PointerEvent& event,
                  WebViewerAction action)
    {
      switch (event.GetMouseButton())
      {
        case OrthancStone::MouseButton_Left:
          return (leftAction_ == action);

        case OrthancStone::MouseButton_Middle:
          return (middleAction_ == action);
      
        case OrthancStone::MouseButton_Right:
          return (rightAction_ == action);

        default:
          return false;
      }
    }
    
  public:
    Interactor(ViewerViewport& viewer,
               WebViewerAction leftAction,
               WebViewerAction middleAction,
               WebViewerAction rightAction) :
      viewer_(viewer),
      leftAction_(leftAction),
      middleAction_(middleAction),
      rightAction_(rightAction)
    {
      SetLeftButtonAction(ConvertWebViewerAction(leftAction));
      SetMiddleButtonAction(ConvertWebViewerAction(middleAction));
      SetRightButtonAction(ConvertWebViewerAction(rightAction));
    }

    virtual OrthancStone::IFlexiblePointerTracker* CreateTracker(
      boost::weak_ptr<OrthancStone::IViewport> viewport,
      const OrthancStone::PointerEvent& event,
      unsigned int viewportWidth,
      unsigned int viewportHeight) ORTHANC_OVERRIDE
    {
      boost::shared_ptr<OrthancStone::IViewport> lock1(viewport.lock());
      
      if (lock1 &&
          IsAction(event, WebViewerAction_Crosshair))
      {
        OrthancStone::CoordinateSystem3D plane;
        if (viewer_.GetCurrentPlane(plane))
        {
          std::unique_ptr<OrthancStone::IViewport::ILock> lock2(lock1->Lock());

          const OrthancStone::ScenePoint2D p = event.GetMainPosition();
          double x = p.GetX();
          double y = p.GetY();
          lock2->GetController().GetCanvasToSceneTransform().Apply(x, y);
          
          OrthancStone::Vector click = plane.MapSliceToWorldCoordinates(x, y);
          if (viewer_.observer_.get() != NULL)
          {
            viewer_.observer_->SignalCrosshair(viewer_, click);
          }
        }
        
        return NULL;  // No need for a tracker, this is just a click
      }
      else
      {
        // Only the left mouse button can be used to edit/create/remove annotations
        if (event.GetMouseButton() == OrthancStone::MouseButton_Left)
        {
          switch (leftAction_)
          {
            case WebViewerAction_CreateAngle:
              viewer_.stoneAnnotations_->SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Angle);
              break;
              
            case WebViewerAction_CreateCircle:
              viewer_.stoneAnnotations_->SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Circle);
              break;
              
            case WebViewerAction_CreateSegment:
              viewer_.stoneAnnotations_->SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Segment);
              break;

            case WebViewerAction_RemoveMeasure:
              viewer_.stoneAnnotations_->SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Remove);
              break;

            default:
              viewer_.stoneAnnotations_->SetActiveTool(OrthancStone::AnnotationsSceneLayer::Tool_Edit);
              break;
          }

          {
            std::unique_ptr<OrthancStone::IViewport::ILock> lock2(lock1->Lock());

            std::unique_ptr<OrthancStone::IFlexiblePointerTracker> t;
            t.reset(viewer_.stoneAnnotations_->CreateTracker(event.GetMainPosition(), lock2->GetController().GetScene()));

            if (t.get() != NULL)
            {
              return t.release();
            }
          }
        }

        return DefaultViewportInteractor::CreateTracker(
          viewport, event, viewportWidth, viewportHeight);
      }
    }

    virtual bool HasMouseHover() const ORTHANC_OVERRIDE
    {
      return true;
    }

    virtual void HandleMouseHover(OrthancStone::IViewport& viewport,
                                  const OrthancStone::PointerEvent& event) ORTHANC_OVERRIDE
    {
      std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport.Lock());

      if (viewer_.stoneAnnotations_->SetMouseHover(event.GetMainPosition(), lock->GetController().GetScene()))
      {
        viewer_.stoneAnnotations_->Render(lock->GetController().GetScene());
        lock->Invalidate();
      }
    }
  };
  

  void SetMouseButtonActions(WebViewerAction leftAction,
                             WebViewerAction middleAction,
                             WebViewerAction rightAction)
  {
    assert(viewport_ != NULL);
    viewport_->AcquireInteractor(new Interactor(*this, leftAction, middleAction, rightAction));
  }

  void FitForPrint()
  {
    viewport_->FitForPrint();
  }

  void SetOsiriXAnnotations(boost::shared_ptr<OrthancStone::OsiriX::CollectionOfAnnotations> annotations)
  {
    osiriXAnnotations_ = annotations;
  }

  void ScheduleFrameFocus(const std::string& sopInstanceUid,
                          unsigned int frameNumber)
  {
    hasFocusOnInstance_ = true;
    focusSopInstanceUid_ = sopInstanceUid;
    focusFrameNumber_ = frameNumber;
    
    ApplyScheduledFocus();
  }

  void ApplyScheduledFocus()
  {
    size_t cursorIndex;
    
    if (hasFocusOnInstance_ &&
        cursor_.get() != NULL &&
        frames_.get() != NULL &&
        frames_->LookupFrame(cursorIndex, focusSopInstanceUid_, focusFrameNumber_))
    {
      size_t current = cursor_->GetCurrentIndex();

      if (current != cursorIndex)
      {
        cursor_->SetCurrentIndex(cursorIndex);
        Redraw();
      }
      
      hasFocusOnInstance_ = false;
    }
  }

  void FocusOnPoint(const OrthancStone::Vector& p)
  {
    //static const double MAX_DISTANCE = 0.5;   // 0.5 cm => TODO parameter?
    static const double MAX_DISTANCE = std::numeric_limits<double>::infinity();

    size_t cursorIndex;
    if (cursor_.get() != NULL &&
        frames_.get() != NULL &&
        frames_->FindClosestFrame(cursorIndex, p, MAX_DISTANCE))
    {
      cursor_->SetCurrentIndex(cursorIndex);
      Redraw();
    }
  }

  unsigned int GetCineRate() const
  {
    return cineRate_;
  }

  void FormatWindowingPresets(Json::Value& target) const
  {
    assert(windowingPresetCenters_.size() == windowingPresetWidths_.size());

    target = Json::arrayValue;

    for (size_t i = 0; i < windowingPresetCenters_.size(); i++)
    {
      const float c = windowingPresetCenters_[i];
      const float w = windowingPresetWidths_[i];
      
      std::string name = "Preset";
      if (windowingPresetCenters_.size() > 1)
      {
        name += " " + boost::lexical_cast<std::string>(i + 1);
      }

      Json::Value preset = Json::objectValue;
      preset["name"] = name;
      preset["center"] = c;
      preset["width"] = w;
      preset["info"] =
        ("C " + boost::lexical_cast<std::string>(static_cast<int>(boost::math::iround<double>(c))) +
         ", W " + boost::lexical_cast<std::string>(static_cast<int>(boost::math::iround<double>(w))));
      
      target.append(preset);
    }
  }


  void SetSynchronizedBrowsingEnabled(int enabled)
  {
    OrthancStone::LinearAlgebra::AssignVector(synchronizationOffset_, 0, 0, 0);
    synchronizationEnabled_ = enabled;
  }


  void SignalStoneAnnotationsChanged(const std::string& sopInstanceUid,
                                     size_t frame)
  {
    if (cursor_.get() != NULL &&
        frames_.get() != NULL)
    {
      const size_t cursorIndex = cursor_->GetCurrentIndex();
      const OrthancStone::DicomInstanceParameters& instance = frames_->GetInstanceOfFrame(cursorIndex);
      const size_t frameNumber = frames_->GetFrameNumberInInstance(cursorIndex);

      if (instance.GetSopInstanceUid() == sopInstanceUid &&
          frameNumber == frame)
      {
        StoneAnnotationsRegistry::GetInstance().Load(*stoneAnnotations_, instance.GetSopInstanceUid(), frame);

        {
          std::unique_ptr<OrthancStone::IViewport::ILock> lock(viewport_->Lock());
          stoneAnnotations_->Render(lock->GetController().GetScene());
          lock->Invalidate();
        }
      }
    }    
  }
};





typedef std::map<std::string, boost::shared_ptr<ViewerViewport> >  Viewports;

static Viewports allViewports_;
static bool showReferenceLines_ = true;
static boost::shared_ptr<OrthancStone::OsiriX::CollectionOfAnnotations>  osiriXAnnotations_;


static void UpdateReferenceLines()
{
  if (showReferenceLines_)
  {
    std::list<const ViewerViewport*> viewports;
    
    for (Viewports::const_iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
    {
      assert(it->second != NULL);
      viewports.push_back(it->second.get());
    }

    for (Viewports::iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
    {
      assert(it->second != NULL);
      it->second->UpdateReferenceLines(viewports);
    }
  }
  else
  {
    for (Viewports::iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
    {
      assert(it->second != NULL);
      it->second->ClearReferenceLines();
    }
  }
}


class WebAssemblyObserver : public ResourcesLoader::IObserver,
                            public ViewerViewport::IObserver
{
public:
  virtual void SignalResourcesLoaded() ORTHANC_OVERRIDE
  {
    DISPATCH_JAVASCRIPT_EVENT("ResourcesLoaded");
  }

  virtual void SignalSeriesThumbnailLoaded(const std::string& studyInstanceUid,
                                           const std::string& seriesInstanceUid) ORTHANC_OVERRIDE
  {
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("ThumbnailLoaded", false, false,
                                    { "studyInstanceUid" : UTF8ToString($0),
                                        "seriesInstanceUid" : UTF8ToString($1) });
        window.dispatchEvent(customEvent);
      },
      studyInstanceUid.c_str(),
      seriesInstanceUid.c_str());
  }

  virtual void SignalSeriesMetadataLoaded(const std::string& studyInstanceUid,
                                          const std::string& seriesInstanceUid) ORTHANC_OVERRIDE
  {
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("MetadataLoaded", false, false,
                                    { "studyInstanceUid" : UTF8ToString($0),
                                        "seriesInstanceUid" : UTF8ToString($1) });
        window.dispatchEvent(customEvent);
      },
      studyInstanceUid.c_str(),
      seriesInstanceUid.c_str());

    for (Viewports::const_iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
    {
      assert(it->second != NULL);
      it->second->ApplyScheduledFocus();
    }
  }

  virtual void SignalSeriesDetailsReady(const ViewerViewport& viewport) ORTHANC_OVERRIDE
  {
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("SeriesDetailsReady", false, false,
                                    { "canvasId" : UTF8ToString($0) });
        window.dispatchEvent(customEvent);
      },
      viewport.GetCanvasId().c_str()
      );
  }

  virtual void SignalFrameUpdated(const ViewerViewport& viewport,
                                  size_t currentFrame,
                                  size_t countFrames,
                                  DisplayedFrameQuality quality,
                                  unsigned int instanceNumber) ORTHANC_OVERRIDE
  {
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("FrameUpdated", false, false,
                                    { "canvasId" : UTF8ToString($0),
                                        "currentFrame" : $1,
                                        "numberOfFrames" : $2,
                                        "quality" : $3,
                                        "instanceNumber" : $4 });
        window.dispatchEvent(customEvent);
      },
      viewport.GetCanvasId().c_str(),
      static_cast<int>(currentFrame),
      static_cast<int>(countFrames),
      quality, instanceNumber);

    UpdateReferenceLines();
  }

  virtual void SignalCrosshair(const ViewerViewport& viewport,
                               const OrthancStone::Vector& click) ORTHANC_OVERRIDE
  {
    if (click.size() != 3u)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
    
    for (Viewports::const_iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
    {
      // TODO - One could check the "Frame Of Reference UID" here
      assert(it->second.get() != NULL);
      if (it->second.get() != &viewport)
      {
        it->second->FocusOnPoint(click);
      }
    }
  }

  virtual void SignalSynchronizedBrowsing(const ViewerViewport& viewport,
                                          const OrthancStone::Vector& click,
                                          const OrthancStone::Vector& normal) ORTHANC_OVERRIDE
  {
    for (Viewports::const_iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
    {
      assert(it->second.get() != NULL);

      OrthancStone::CoordinateSystem3D plane;
      bool isOpposite;
      
      if (it->second.get() != &viewport &&
          it->second->GetCurrentPlane(plane) &&
          OrthancStone::GeometryToolbox::IsParallelOrOpposite(isOpposite, plane.GetNormal(), normal))
      {
        it->second->FocusOnPoint(click);
      }
    }
  }

  virtual void SignalSeriesPdfLoaded(const std::string& studyInstanceUid,
                                     const std::string& seriesInstanceUid,
                                     const std::string& pdf) ORTHANC_OVERRIDE
  {
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("PdfLoaded", false, false,
                                    { "studyInstanceUid" : UTF8ToString($0),
                                        "seriesInstanceUid" : UTF8ToString($1),
                                        "pdfPointer" : $2,
                                        "pdfSize": $3});
        window.dispatchEvent(customEvent);
      },
      studyInstanceUid.c_str(),
      seriesInstanceUid.c_str(),
      pdf.empty() ? 0 : reinterpret_cast<intptr_t>(pdf.c_str()),  // Explicit conversion to an integer
      pdf.size());
  }


  virtual void SignalVirtualSeriesThumbnailLoaded(const std::string& virtualSeriesId,
                                                  const std::string& jpeg) ORTHANC_OVERRIDE
  {
    std::string dataUriScheme;
    Orthanc::Toolbox::EncodeDataUriScheme(dataUriScheme, "image/jpeg", jpeg);    
    
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("VirtualSeriesThumbnailLoaded", false, false,
                                    { "virtualSeriesId" : UTF8ToString($0),
                                        "thumbnail" : UTF8ToString($1) });
        window.dispatchEvent(customEvent);
      },
      virtualSeriesId.c_str(),
      dataUriScheme.c_str());
  }

  virtual void SignalWindowingUpdated(const ViewerViewport& viewport,
                                      double windowingCenter,
                                      double windowingWidth) ORTHANC_OVERRIDE
  {
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("WindowingUpdated", false, false,
                                    { "canvasId" : UTF8ToString($0),
                                        "windowingCenter" : $1,
                                        "windowingWidth" : $2 });
        window.dispatchEvent(customEvent);
      },
      viewport.GetCanvasId().c_str(),
      static_cast<int>(boost::math::iround<double>(windowingCenter)),
      static_cast<int>(boost::math::iround<double>(windowingWidth)));

    UpdateReferenceLines();
  }

  virtual void SignalStoneAnnotationsChanged(const ViewerViewport& viewport,
                                             const std::string& sopInstanceUid,
                                             size_t frame) ORTHANC_OVERRIDE
  {
    for (Viewports::const_iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
    {
      assert(it->second.get() != NULL);

      if (it->second.get() != &viewport)
      {
        it->second->SignalStoneAnnotationsChanged(sopInstanceUid, frame);
      }
    }
  }
  
  virtual void SignalStoneAnnotationAdded(const ViewerViewport& viewport) ORTHANC_OVERRIDE
  {
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("StoneAnnotationAdded", false, false,
                                    { "canvasId" : UTF8ToString($0) });
        window.dispatchEvent(customEvent);
      },
      viewport.GetCanvasId().c_str());
  }

  virtual void SignalStoneAnnotationRemoved(const ViewerViewport& viewport) ORTHANC_OVERRIDE
  {
    EM_ASM({
        const customEvent = document.createEvent("CustomEvent");
        customEvent.initCustomEvent("StoneAnnotationRemoved", false, false,
                                    { "canvasId" : UTF8ToString($0) });
        window.dispatchEvent(customEvent);
      },
      viewport.GetCanvasId().c_str());
  }
};



static OrthancStone::DicomSource source_;
static boost::shared_ptr<FramesCache> framesCache_;
static boost::shared_ptr<OrthancStone::WebAssemblyLoadersContext> context_;
static std::string stringBuffer_;
static bool softwareRendering_ = false;
static WebViewerAction leftButtonAction_ = WebViewerAction_Windowing;
static WebViewerAction middleButtonAction_ = WebViewerAction_Pan;
static WebViewerAction rightButtonAction_ = WebViewerAction_Zoom;


static void FormatTags(std::string& target,
                       const Orthanc::DicomMap& tags)
{
  Orthanc::DicomArray arr(tags);
  Json::Value v = Json::objectValue;

  for (size_t i = 0; i < arr.GetSize(); i++)
  {
    const Orthanc::DicomElement& element = arr.GetElement(i);
    if (!element.GetValue().IsBinary() &&
        !element.GetValue().IsNull())
    {
      v[element.GetTag().Format()] = element.GetValue().GetContent();
    }
  }

  target = v.toStyledString();
}


static ResourcesLoader& GetResourcesLoader()
{
  static boost::shared_ptr<ResourcesLoader>  resourcesLoader_;

  if (!resourcesLoader_)
  {
    std::unique_ptr<OrthancStone::ILoadersContext::ILock> lock(context_->Lock());
    resourcesLoader_ = ResourcesLoader::Create(*lock, source_);
    resourcesLoader_->AcquireObserver(new WebAssemblyObserver);
  }

  return *resourcesLoader_;
}


static boost::shared_ptr<ViewerViewport> GetViewport(const std::string& canvas)
{
  Viewports::iterator found = allViewports_.find(canvas);
  if (found == allViewports_.end())
  {
    boost::shared_ptr<ViewerViewport> viewport(
      ViewerViewport::Create(*context_, source_, canvas, framesCache_, softwareRendering_));
    viewport->SetMouseButtonActions(leftButtonAction_, middleButtonAction_, rightButtonAction_);
    viewport->AcquireObserver(new WebAssemblyObserver);
    viewport->SetOsiriXAnnotations(osiriXAnnotations_);
    allViewports_[canvas] = viewport;
    return viewport;
  }
  else
  {
    return found->second;
  }
}


extern "C"
{
  int main(int argc, char const *argv[]) 
  {
    printf("Initializing Stone\n");
    Orthanc::InitializeFramework("", true);
    Orthanc::Logging::EnableInfoLevel(true);
    //Orthanc::Logging::EnableTraceLevel(true);

    context_.reset(new OrthancStone::WebAssemblyLoadersContext(1, 4, 1));
    context_->SetDicomCacheSize(128 * 1024 * 1024);  // 128MB
    
    framesCache_.reset(new FramesCache);
    osiriXAnnotations_.reset(new OrthancStone::OsiriX::CollectionOfAnnotations);

    DISPATCH_JAVASCRIPT_EVENT("StoneInitialized");
  }


  EMSCRIPTEN_KEEPALIVE
  void SetDicomWebRoot(const char* uri,
                       int useRendered)
  {
    try
    {
      source_.SetDicomWebSource(uri);
      source_.SetDicomWebRendered(useRendered != 0);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }
  

  EMSCRIPTEN_KEEPALIVE
  void SetDicomWebThroughOrthanc(const char* orthancRoot,
                                 const char* serverName,
                                 int hasRendered)
  {
    try
    {
      context_->SetLocalOrthanc(orthancRoot);
      source_.SetDicomWebThroughOrthancSource(serverName);
      source_.SetDicomWebRendered(hasRendered != 0);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  void AddHttpHeader(const char* header,
                     const char* value)
  {
    try
    {
      source_.AddHttpHeader(header, value);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }
  

  EMSCRIPTEN_KEEPALIVE
  void SetDicomCacheSize(int sizeMB)
  {
    try
    {
      if (sizeMB == 0)
      {
        LOG(WARNING) << "The DICOM cache is disabled";
      }
      else
      {
        LOG(INFO) << "The DICOM cache size is set to " << sizeMB << "MB";
      }

      if (sizeMB >= 0)
      {
        context_->SetDicomCacheSize(sizeMB * 1024 * 1024);
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }
  

  EMSCRIPTEN_KEEPALIVE
  void FetchAllStudies()
  {
    try
    {
      GetResourcesLoader().FetchAllStudies();
    }
    EXTERN_CATCH_EXCEPTIONS;
  }

  EMSCRIPTEN_KEEPALIVE
  void FetchPatient(const char* patientId)
  {
    try
    {
      GetResourcesLoader().FetchPatient(patientId);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }

  EMSCRIPTEN_KEEPALIVE
  void FetchStudy(const char* studyInstanceUid)
  {
    try
    {
      GetResourcesLoader().FetchStudy(studyInstanceUid);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }

  EMSCRIPTEN_KEEPALIVE
  void FetchSeries(const char* studyInstanceUid,
                   const char* seriesInstanceUid)
  {
    try
    {
      GetResourcesLoader().FetchSeries(studyInstanceUid, seriesInstanceUid);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }
  
  EMSCRIPTEN_KEEPALIVE
  int GetStudiesCount()
  {
    try
    {
      return GetResourcesLoader().GetStudiesCount();
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;  // on exception
  }
  
  EMSCRIPTEN_KEEPALIVE
  int GetSeriesCount()
  {
    try
    {
      return GetResourcesLoader().GetSeriesCount();
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;  // on exception
  }


  EMSCRIPTEN_KEEPALIVE
  const char* GetStringBuffer()
  {
    return stringBuffer_.c_str();
  }
  

  EMSCRIPTEN_KEEPALIVE
  const char* GetStoneWebViewerVersion()
  {
    return STONE_WEB_VIEWER_VERSION;
  }
  

  EMSCRIPTEN_KEEPALIVE
  const char* GetEmscriptenVersion()
  {
    // WARNING - "static" is important, otherwise the string would be
    // freed when returning to JavaScript
    static const std::string EMSCRIPTEN_VERSION = (
      boost::lexical_cast<std::string>(__EMSCRIPTEN_major__) + "." +
      boost::lexical_cast<std::string>(__EMSCRIPTEN_minor__) + "." +
      boost::lexical_cast<std::string>(__EMSCRIPTEN_tiny__));
    return EMSCRIPTEN_VERSION.c_str();
  }
  

  EMSCRIPTEN_KEEPALIVE
  void LoadStudyTags(int i)
  {
    try
    {
      if (i < 0)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
      
      Orthanc::DicomMap dicom;
      GetResourcesLoader().GetStudy(dicom, i);
      FormatTags(stringBuffer_, dicom);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }
  

  EMSCRIPTEN_KEEPALIVE
  void LoadSeriesTags(int i)
  {
    try
    {
      if (i < 0)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
      
      Orthanc::DicomMap dicom;
      GetResourcesLoader().GetSeries(dicom, i);
      FormatTags(stringBuffer_, dicom);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }
  

  EMSCRIPTEN_KEEPALIVE
  int LoadSeriesThumbnail(const char* seriesInstanceUid)
  {
    try
    {
      std::string image, mime;
      switch (GetResourcesLoader().GetSeriesThumbnail(image, mime, seriesInstanceUid))
      {
        case OrthancStone::SeriesThumbnailType_Image:
          Orthanc::Toolbox::EncodeDataUriScheme(stringBuffer_, mime, image);
          return ThumbnailType_Image;
          
        case OrthancStone::SeriesThumbnailType_Pdf:
          return ThumbnailType_Pdf;
          
        case OrthancStone::SeriesThumbnailType_Video:
          return ThumbnailType_Video;
          
        case OrthancStone::SeriesThumbnailType_NotLoaded:
          return ThumbnailType_Loading;
          
        case OrthancStone::SeriesThumbnailType_Unsupported:
          return ThumbnailType_NoPreview;

        default:
          return ThumbnailType_Unknown;
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
    return ThumbnailType_Unknown;
  }


  EMSCRIPTEN_KEEPALIVE
  void SpeedUpFetchSeriesMetadata(const char* studyInstanceUid,
                                  const char* seriesInstanceUid)
  {
    try
    {
      GetResourcesLoader().FetchSeriesMetadata(PRIORITY_HIGH, studyInstanceUid, seriesInstanceUid);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  int IsSeriesComplete(const char* seriesInstanceUid)
  {
    try
    {
      return GetResourcesLoader().IsSeriesComplete(seriesInstanceUid) ? 1 : 0;
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;
  }

  EMSCRIPTEN_KEEPALIVE
  int LoadSeriesInViewport(const char* canvas,
                           const char* seriesInstanceUid)
  {
    try
    {
      std::unique_ptr<OrthancStone::SortedFrames> frames(new OrthancStone::SortedFrames);
      
      if (GetResourcesLoader().SortSeriesFrames(*frames, seriesInstanceUid))
      {
        GetViewport(canvas)->SetFrames(frames.release());
        return 1;
      }
      else
      {
        return 0;
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;
  }


  EMSCRIPTEN_KEEPALIVE
  int LoadVirtualSeriesInViewport(const char* canvas,
                                  const char* virtualSeriesId)
  {
    try
    {
      std::unique_ptr<OrthancStone::SortedFrames> frames(new OrthancStone::SortedFrames);

      if (GetResourcesLoader().SortVirtualSeriesFrames(*frames, virtualSeriesId))
      {
        GetViewport(canvas)->SetFrames(frames.release());
        return 1;
      }
      else
      {
        return 0;
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;
  }


  EMSCRIPTEN_KEEPALIVE
  void AllViewportsUpdateSize(int fitContent)
  {
    try
    {
      for (Viewports::iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
      {
        assert(it->second != NULL);
        it->second->UpdateSize(fitContent != 0);
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  int DecrementFrame(const char* canvas,
                     int isCircular)
  {
    try
    {
      return GetViewport(canvas)->ChangeFrame(SeriesCursor::Action_Minus, isCircular) ? 1 : 0;
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;
  }


  EMSCRIPTEN_KEEPALIVE
  int IncrementFrame(const char* canvas,
                     int isCircular)
  {
    try
    {
      return GetViewport(canvas)->ChangeFrame(SeriesCursor::Action_Plus, isCircular) ? 1 : 0;
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;
  }  


  EMSCRIPTEN_KEEPALIVE
  void GoToFirstFrame(const char* canvas)
  {
    try
    {
      GetViewport(canvas)->GoToFirstFrame();
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  void GoToLastFrame(const char* canvas)
  {
    try
    {
      GetViewport(canvas)->GoToLastFrame();
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  void ShowReferenceLines(int show)
  {
    try
    {
      showReferenceLines_ = (show != 0);
      UpdateReferenceLines();
    }
    EXTERN_CATCH_EXCEPTIONS;
  }  


  EMSCRIPTEN_KEEPALIVE
  void SetWindowing(const char* canvas,
                    int center,
                    int width)
  {
    try
    {
      GetViewport(canvas)->SetWindowing(center, width);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }  


  EMSCRIPTEN_KEEPALIVE
  void InvertContrast(const char* canvas)
  {
    try
    {
      GetViewport(canvas)->Invert();
    }
    EXTERN_CATCH_EXCEPTIONS;
  }  


  EMSCRIPTEN_KEEPALIVE
  void FlipX(const char* canvas)
  {
    try
    {
      GetViewport(canvas)->FlipX();
    }
    EXTERN_CATCH_EXCEPTIONS;
  }  


  EMSCRIPTEN_KEEPALIVE
  void FlipY(const char* canvas)
  {
    try
    {
      GetViewport(canvas)->FlipY();
    }
    EXTERN_CATCH_EXCEPTIONS;
  }  
  

  EMSCRIPTEN_KEEPALIVE
  void SetSoftwareRendering(int softwareRendering)
  {
    softwareRendering_ = softwareRendering;
  }  


  EMSCRIPTEN_KEEPALIVE
  int IsSoftwareRendering()
  {
    return softwareRendering_;
  }  


  EMSCRIPTEN_KEEPALIVE
  void SetMouseButtonActions(int leftAction,
                             int middleAction,
                             int rightAction)
  {
    try
    {
      leftButtonAction_ = static_cast<WebViewerAction>(leftAction);
      middleButtonAction_ = static_cast<WebViewerAction>(middleAction);
      rightButtonAction_ = static_cast<WebViewerAction>(rightAction);
      
      for (Viewports::iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
      {
        assert(it->second != NULL);
        it->second->SetMouseButtonActions(leftButtonAction_, middleButtonAction_, rightButtonAction_);
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  int GetLeftMouseButtonAction()
  {
    return static_cast<int>(leftButtonAction_);
  }
  

  EMSCRIPTEN_KEEPALIVE
  int GetMiddleMouseButtonAction()
  {
    return static_cast<int>(middleButtonAction_);
  }
  

  EMSCRIPTEN_KEEPALIVE
  int GetRightMouseButtonAction()
  {
    return static_cast<int>(rightButtonAction_);
  }
  

  EMSCRIPTEN_KEEPALIVE
  void FitForPrint()
  {
    try
    {
      for (Viewports::iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
      {
        assert(it->second != NULL);
        it->second->FitForPrint();
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  // Side-effect: "GetStringBuffer()" is filled with the "Series
  // Instance UID" of the first loaded annotation
  EMSCRIPTEN_KEEPALIVE
  int LoadOsiriXAnnotations(const char* xml,
                            int clearPreviousAnnotations)
  {
    try
    {
      if (clearPreviousAnnotations)
      {
        osiriXAnnotations_->Clear();
      }
      
      osiriXAnnotations_->LoadXml(xml);
      
      // Force redraw, as the annotations might have changed
      for (Viewports::iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
      {
        assert(it->second != NULL);
        it->second->Redraw();
      }

      if (osiriXAnnotations_->GetSize() == 0)
      {
        stringBuffer_.clear();
      }
      else
      {
        stringBuffer_ = osiriXAnnotations_->GetAnnotation(0).GetSeriesInstanceUid();
      }

      LOG(WARNING) << "Loaded " << osiriXAnnotations_->GetSize() << " annotations from OsiriX";
      return 1;
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;
  }


  EMSCRIPTEN_KEEPALIVE
  void FocusFirstOsiriXAnnotation(const char* canvas)
  {
    try
    {
      if (osiriXAnnotations_->GetSize() != 0)
      {
        const OrthancStone::OsiriX::Annotation& annotation = osiriXAnnotations_->GetAnnotation(0);
        
        boost::shared_ptr<ViewerViewport> viewport = GetViewport(canvas);
        viewport->ScheduleFrameFocus(annotation.GetSopInstanceUid(), 0 /* focus on first frame */);

        // Force redraw, as the annotations might already have changed
        viewport->Redraw();
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  void FetchPdf(const char* studyInstanceUid,
                const char* seriesInstanceUid)
  {
    try
    {
      LOG(INFO) << "Fetching PDF series: " << seriesInstanceUid;
      GetResourcesLoader().FetchPdf(studyInstanceUid, seriesInstanceUid);
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  unsigned int GetCineRate(const char* canvas)
  {
    try
    {
      return GetViewport(canvas)->GetCineRate();
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;
  }


  EMSCRIPTEN_KEEPALIVE
  unsigned int GetSeriesNumberOfFrames(const char* seriesInstanceUid)
  {
    try
    {
      return GetResourcesLoader().GetSeriesNumberOfFrames(seriesInstanceUid);
    }
    EXTERN_CATCH_EXCEPTIONS;
    return 0;
  }


  EMSCRIPTEN_KEEPALIVE
  void LoadWindowingPresets(const char* canvas)
  {
    try
    {
      Json::Value v;
      GetViewport(canvas)->FormatWindowingPresets(v);
      stringBuffer_ = v.toStyledString();
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  void SetSynchronizedBrowsingEnabled(int enabled)
  {
    try
    {
      for (Viewports::iterator it = allViewports_.begin(); it != allViewports_.end(); ++it)
      {
        assert(it->second != NULL);
        it->second->SetSynchronizedBrowsingEnabled(enabled);
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
  }


  EMSCRIPTEN_KEEPALIVE
  int LookupVirtualSeries(const char* seriesInstanceUid)
  {
    try
    {
      typedef std::map<std::string, unsigned int>  VirtualSeries;

      VirtualSeries virtualSeries;
      if (GetResourcesLoader().LookupVirtualSeries(virtualSeries, seriesInstanceUid))
      {
        Json::Value json = Json::arrayValue;
        for (VirtualSeries::const_iterator it = virtualSeries.begin(); it != virtualSeries.end(); ++it)
        {
          Json::Value item = Json::objectValue;
          item["ID"] = it->first;
          item["NumberOfFrames"] = it->second;
          json.append(item);
        }

        stringBuffer_ = json.toStyledString();
        return true;
      }
      else
      {
        return false;
      }
    }
    EXTERN_CATCH_EXCEPTIONS;
    return false;
  }
}
