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


#pragma once

#include "../OrthancStone.h"

#if !defined(ORTHANC_ENABLE_DCMTK)
#  error The macro ORTHANC_ENABLE_DCMTK must be defined
#endif

#include "../Oracle/HttpCommand.h"
#include "../Oracle/OracleCommandExceptionMessage.h"
#include "../Oracle/OrthancRestApiCommand.h"
#include "../Oracle/ReadFileCommand.h"
#include "DicomSource.h"
#include "ILoaderFactory.h"
#include "LoadedDicomResources.h"
#include "OracleScheduler.h"

namespace Orthanc
{
#if ORTHANC_ENABLE_DCMTK == 1
  class ParsedDicomDir;
#endif
}

namespace OrthancStone
{
#if ORTHANC_ENABLE_DCMTK == 1
  class ParseDicomFromFileCommand;
#endif
  
  class DicomResourcesLoader :
    public ObserverBase<DicomResourcesLoader>,
    public IObservable
  {
  private:
    class Handler;
    class StringHandler;
    class DicomWebHandler;
    class OrthancHandler;
    class OrthancInstanceTagsHandler;    
    class OrthancOneChildInstanceHandler;
    class OrthancAllChildrenInstancesHandler;

#if ORTHANC_ENABLE_DCMTK == 1
    class DicomDirHandler;
#endif

    void Handle(const HttpCommand::SuccessMessage& message);

    void Handle(const OrthancRestApiCommand::SuccessMessage& message);

    void Handle(const ReadFileCommand::SuccessMessage& message);

    void Handle(const OracleCommandExceptionMessage& message);

#if ORTHANC_ENABLE_DCMTK == 1
    void Handle(const ParseDicomSuccessMessage& message);
#endif

    void ScheduleLoadOrthancInstanceTags(boost::shared_ptr<LoadedDicomResources> target,
                                         int priority,
                                         const DicomSource& source,
                                         const std::string& instanceId,
                                         boost::shared_ptr<unsigned int> remainingCommands,
                                         boost::shared_ptr<Orthanc::IDynamicObject> userPayload);

    void ScheduleLoadOrthancOneChildInstance(boost::shared_ptr<LoadedDicomResources> target,
                                             int priority,
                                             const DicomSource& source,
                                             Orthanc::ResourceType level,
                                             const std::string& id,
                                             boost::shared_ptr<unsigned int> remainingCommands,
                                             boost::shared_ptr<Orthanc::IDynamicObject> userPayload);
    
    explicit DicomResourcesLoader(ILoadersContext& context) :
      context_(context)
    {
    }

    ILoadersContext&  context_;


  public:
    class SuccessMessage : public OriginMessage<DicomResourcesLoader>
    {
      ORTHANC_STONE_MESSAGE(__FILE__, __LINE__);
      
    private:
      boost::shared_ptr<LoadedDicomResources>  resources_;
      int                                      priority_;
      const DicomSource&                       source_;
      const Orthanc::IDynamicObject*           userPayload_;
      
    public:
      SuccessMessage(const DicomResourcesLoader& origin,
                     boost::shared_ptr<LoadedDicomResources> resources,
                     int priority,
                     const DicomSource& source,
                     const Orthanc::IDynamicObject* userPayload) :
        OriginMessage(origin),
        resources_(resources),
        priority_(priority),
        source_(source),
        userPayload_(userPayload)
      {
      }

      int GetPriority() const
      {
        return priority_;
      }

      const boost::shared_ptr<LoadedDicomResources> GetResources() const
      {
        return resources_;
      }

      const DicomSource& GetDicomSource() const
      {
        return source_;
      }

      bool HasUserPayload() const
      {
        return userPayload_ != NULL;
      }

      const Orthanc::IDynamicObject& GetUserPayload() const;
    };


    class Factory : public ILoaderFactory
    {
    public:
      virtual boost::shared_ptr<IObserver> Create(ILoadersContext::ILock& stone) ORTHANC_OVERRIDE
      {
        return DicomResourcesLoader::Create(stone);
      }
    };


    static boost::shared_ptr<DicomResourcesLoader> Create(const ILoadersContext::ILock& stone);

    void ScheduleGetDicomWeb(boost::shared_ptr<LoadedDicomResources> target,
                             int priority,
                             const DicomSource& source,
                             const std::string& uri,
                             const std::set<Orthanc::DicomTag>& includeTags,
                             Orthanc::IDynamicObject* userPayload);

    void ScheduleGetDicomWeb(boost::shared_ptr<LoadedDicomResources> target,
                             int priority,
                             const DicomSource& source,
                             const std::string& uri,
                             Orthanc::IDynamicObject* userPayload)
    {
      std::set<Orthanc::DicomTag> includeTags;
      ScheduleGetDicomWeb(target, priority, source, uri, includeTags, userPayload);
    }        

    void ScheduleQido(boost::shared_ptr<LoadedDicomResources> target,
                      int priority,
                      const DicomSource& source,
                      Orthanc::ResourceType level,
                      const Orthanc::DicomMap& filter,
                      const std::set<Orthanc::DicomTag>& includeTags,
                      Orthanc::IDynamicObject* userPayload);

    void ScheduleLoadOrthancResources(boost::shared_ptr<LoadedDicomResources> target,
                                      int priority,
                                      const DicomSource& source,
                                      Orthanc::ResourceType topLevel,
                                      const std::string& topId,
                                      Orthanc::ResourceType bottomLevel,
                                      Orthanc::IDynamicObject* userPayload);

    void ScheduleLoadOrthancResource(boost::shared_ptr<LoadedDicomResources> target,
                                     int priority,
                                     const DicomSource& source,
                                     Orthanc::ResourceType level,
                                     const std::string& id,
                                     Orthanc::IDynamicObject* userPayload)
    {
      ScheduleLoadOrthancResources(target, priority, source, level, id, level, userPayload);
    }

#if ORTHANC_ENABLE_DCMTK == 1
    static void GetDicomDirInstances(LoadedDicomResources& target,
                                     const Orthanc::ParsedDicomDir& dicomDir);
#endif

    void ScheduleLoadDicomDir(boost::shared_ptr<LoadedDicomResources> target,
                              int priority,
                              const DicomSource& source,
                              const std::string& path,
                              Orthanc::IDynamicObject* userPayload);
    
    void ScheduleLoadDicomFile(boost::shared_ptr<LoadedDicomResources> target,
                               int priority,
                               const DicomSource& source,
                               const std::string& path,
                               bool includePixelData,
                               Orthanc::IDynamicObject* userPayload);

    bool ScheduleLoadDicomFile(boost::shared_ptr<LoadedDicomResources> target,
                               int priority,
                               const DicomSource& source,
                               const std::string& dicomDirPath,
                               const Orthanc::DicomMap& dicomDirEntry,
                               bool includePixelData,
                               Orthanc::IDynamicObject* userPayload);
  };
}
