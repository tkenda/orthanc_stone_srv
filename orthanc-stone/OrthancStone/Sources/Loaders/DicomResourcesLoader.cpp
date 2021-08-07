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


#include "DicomResourcesLoader.h"

#if !defined(ORTHANC_ENABLE_DCMTK)
#  error The macro ORTHANC_ENABLE_DCMTK must be defined
#endif

#if ORTHANC_ENABLE_DCMTK == 1
#  include "../Oracle/ParseDicomFromFileCommand.h"
#  include <DicomParsing/ParsedDicomDir.h>
#  include <DicomParsing/ParsedDicomFile.h>
#endif

#include <Toolbox.h>

#include <boost/filesystem/path.hpp>

namespace OrthancStone
{
  static std::string GetUri(Orthanc::ResourceType level)
  {
    switch (level)
    {
      case Orthanc::ResourceType_Patient:
        return "patients";
        
      case Orthanc::ResourceType_Study:
        return "studies";
        
      case Orthanc::ResourceType_Series:
        return "series";
        
      case Orthanc::ResourceType_Instance:
        return "instances";

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }
  }


  class DicomResourcesLoader::Handler : public Orthanc::IDynamicObject
  {
  private:
    boost::shared_ptr<DicomResourcesLoader>     loader_;
    boost::shared_ptr<LoadedDicomResources>     target_;
    int                                         priority_;
    DicomSource                                 source_;
    boost::shared_ptr<Orthanc::IDynamicObject>  userPayload_;

  public:
    Handler(boost::shared_ptr<DicomResourcesLoader> loader,
            boost::shared_ptr<LoadedDicomResources> target,
            int priority,
            const DicomSource& source,
            boost::shared_ptr<Orthanc::IDynamicObject> userPayload) :
      loader_(loader),
      target_(target),
      priority_(priority),
      source_(source),
      userPayload_(userPayload)
    {
      if (!loader ||
          !target)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
    }
      
    virtual ~Handler()
    {
    }

    void BroadcastSuccess()
    {
      SuccessMessage message(*loader_, target_, priority_, source_, userPayload_.get());
      loader_->BroadcastMessage(message);
    }

    boost::shared_ptr<DicomResourcesLoader> GetLoader()
    {
      assert(loader_);
      return loader_;
    }

    boost::shared_ptr<LoadedDicomResources> GetTarget()
    {
      assert(target_);
      return target_;
    }

    int GetPriority() const
    {
      return priority_;
    }

    const DicomSource& GetSource() const
    {
      return source_;
    }

    const boost::shared_ptr<Orthanc::IDynamicObject> GetUserPayload() const
    {
      return userPayload_;
    }
  };


  class DicomResourcesLoader::StringHandler : public DicomResourcesLoader::Handler
  {
  public:
    StringHandler(boost::shared_ptr<DicomResourcesLoader> loader,
                  boost::shared_ptr<LoadedDicomResources> target,
                  int priority,
                  const DicomSource& source,
                  boost::shared_ptr<Orthanc::IDynamicObject> userPayload) :
      Handler(loader, target, priority, source, userPayload)
    {
    }

    virtual void HandleJson(const Json::Value& body) = 0;
      
    virtual void HandleString(const std::string& body)
    {
      Json::Value value;
      if (Orthanc::Toolbox::ReadJson(value, body))
      {
        HandleJson(value);
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
      }
    }
  };


  class DicomResourcesLoader::DicomWebHandler : public StringHandler
  {
  public:
    DicomWebHandler(boost::shared_ptr<DicomResourcesLoader> loader,
                    boost::shared_ptr<LoadedDicomResources> target,
                    int priority,
                    const DicomSource& source,
                    boost::shared_ptr<Orthanc::IDynamicObject> userPayload) :
      StringHandler(loader, target, priority, source, userPayload)
    {
    }

    virtual void HandleJson(const Json::Value& body) ORTHANC_OVERRIDE
    {
      GetTarget()->AddFromDicomWeb(body);
      BroadcastSuccess();
    }
  };


  class DicomResourcesLoader::OrthancHandler : public StringHandler
  {
  private:
    boost::shared_ptr<unsigned int>  remainingCommands_;

  protected:
    void CloseCommand()
    {
      assert(remainingCommands_);
        
      if (*remainingCommands_ == 0)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }

      (*remainingCommands_) --;

      if (*remainingCommands_ == 0)
      {
        BroadcastSuccess();
      }
    }
      
  public:
    OrthancHandler(boost::shared_ptr<DicomResourcesLoader> loader,
                   boost::shared_ptr<LoadedDicomResources> target,
                   int priority,
                   const DicomSource& source,
                   boost::shared_ptr<unsigned int> remainingCommands,
                   boost::shared_ptr<Orthanc::IDynamicObject> userPayload) :
      StringHandler(loader, target, priority, source, userPayload),
      remainingCommands_(remainingCommands)
    {
      if (!remainingCommands)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }

      (*remainingCommands) ++;
    }

    boost::shared_ptr<unsigned int> GetRemainingCommands()
    {
      assert(remainingCommands_);
      return remainingCommands_;
    }
  };

    
  class DicomResourcesLoader::OrthancInstanceTagsHandler : public OrthancHandler
  {
  public:
    OrthancInstanceTagsHandler(boost::shared_ptr<DicomResourcesLoader> loader,
                               boost::shared_ptr<LoadedDicomResources> target,
                               int priority,
                               const DicomSource& source,
                               boost::shared_ptr<unsigned int> remainingCommands,
                               boost::shared_ptr<Orthanc::IDynamicObject> userPayload) :
      OrthancHandler(loader, target, priority, source, remainingCommands, userPayload)
    {
    }

    virtual void HandleJson(const Json::Value& body) ORTHANC_OVERRIDE
    {
      GetTarget()->AddFromOrthanc(body);
      CloseCommand();
    }
  };

    
  class DicomResourcesLoader::OrthancOneChildInstanceHandler : public OrthancHandler
  {
  public:
    OrthancOneChildInstanceHandler(boost::shared_ptr<DicomResourcesLoader> loader,
                                   boost::shared_ptr<LoadedDicomResources> target,
                                   int  priority,
                                   const DicomSource& source,
                                   boost::shared_ptr<unsigned int> remainingCommands,
                                   boost::shared_ptr<Orthanc::IDynamicObject> userPayload) :
      OrthancHandler(loader, target, priority, source, remainingCommands, userPayload)
    {
    }

    virtual void HandleJson(const Json::Value& body) ORTHANC_OVERRIDE
    {
      static const char* const ID = "ID";
      
      if (body.type() == Json::arrayValue)
      {
        if (body.size() > 0)
        {
          if (body[0].type() == Json::objectValue &&
              body[0].isMember(ID) &&
              body[0][ID].type() == Json::stringValue)
          {
            GetLoader()->ScheduleLoadOrthancInstanceTags
              (GetTarget(), GetPriority(), GetSource(), body[0][ID].asString(), GetRemainingCommands(), GetUserPayload());
            CloseCommand();
          }
          else
          {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
          }
        }
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
      }
    }
  };


  class DicomResourcesLoader::OrthancAllChildrenInstancesHandler : public OrthancHandler
  {
  private:
    Orthanc::ResourceType  bottomLevel_;

  public:
    OrthancAllChildrenInstancesHandler(boost::shared_ptr<DicomResourcesLoader> loader,
                                       boost::shared_ptr<LoadedDicomResources> target,
                                       int  priority,
                                       const DicomSource& source,
                                       boost::shared_ptr<unsigned int> remainingCommands,
                                       Orthanc::ResourceType bottomLevel,
                                       boost::shared_ptr<Orthanc::IDynamicObject> userPayload) :
      OrthancHandler(loader, target, priority, source, remainingCommands, userPayload),
      bottomLevel_(bottomLevel)
    {
    }

    virtual void HandleJson(const Json::Value& body) ORTHANC_OVERRIDE
    {
      static const char* const ID = "ID";
      static const char* const INSTANCES = "Instances";

      if (body.type() == Json::arrayValue)
      {
        for (Json::Value::ArrayIndex i = 0; i < body.size(); i++)
        {
          switch (bottomLevel_)
          {
            case Orthanc::ResourceType_Patient:
            case Orthanc::ResourceType_Study:
              if (body[i].type() == Json::objectValue &&
                  body[i].isMember(ID) &&
                  body[i][ID].type() == Json::stringValue)
              {
                GetLoader()->ScheduleLoadOrthancOneChildInstance
                  (GetTarget(), GetPriority(), GetSource(), bottomLevel_,
                   body[i][ID].asString(), GetRemainingCommands(), GetUserPayload());
              }
              else
              {
                throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
              }
              
              break;
            
            case Orthanc::ResourceType_Series:
              // At the series level, avoid a call to
              // "/series/.../instances", as we already have this
              // information in the JSON
              if (body[i].type() == Json::objectValue &&
                  body[i].isMember(INSTANCES) &&
                  body[i][INSTANCES].type() == Json::arrayValue)
              {
                if (body[i][INSTANCES].size() > 0)
                {
                  if (body[i][INSTANCES][0].type() == Json::stringValue)
                  {
                    GetLoader()->ScheduleLoadOrthancInstanceTags
                      (GetTarget(), GetPriority(), GetSource(),
                       body[i][INSTANCES][0].asString(), GetRemainingCommands(), GetUserPayload());
                  }
                  else
                  {
                    throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
                  }
                }
              }

              break;

            case Orthanc::ResourceType_Instance:
              if (body[i].type() == Json::objectValue &&
                  body[i].isMember(ID) &&
                  body[i][ID].type() == Json::stringValue)
              {
                GetLoader()->ScheduleLoadOrthancInstanceTags
                  (GetTarget(), GetPriority(), GetSource(),
                   body[i][ID].asString(), GetRemainingCommands(), GetUserPayload());
              }
              else
              {
                throw Orthanc::OrthancException(Orthanc::ErrorCode_NetworkProtocol);
              }

              break;

            default:
              throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
          }
        }
      }
      
      CloseCommand();
    }
  };


#if ORTHANC_ENABLE_DCMTK == 1
  static void ExploreDicomDir(LoadedDicomResources& instances,
                              const Orthanc::ParsedDicomDir& dicomDir,
                              Orthanc::ResourceType level,
                              size_t index,
                              const Orthanc::DicomMap& parent)
  {
    std::string expectedType;

    switch (level)
    {
      case Orthanc::ResourceType_Patient:
        expectedType = "PATIENT";
        break;

      case Orthanc::ResourceType_Study:
        expectedType = "STUDY";
        break;

      case Orthanc::ResourceType_Series:
        expectedType = "SERIES";
        break;

      case Orthanc::ResourceType_Instance:
        expectedType = "IMAGE";
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    for (;;)
    {
      std::unique_ptr<Orthanc::DicomMap> current(dicomDir.GetItem(index).Clone());
      current->RemoveBinaryTags();
      current->Merge(parent);

      std::string type;
      if (!current->LookupStringValue(type, Orthanc::DICOM_TAG_DIRECTORY_RECORD_TYPE, false))
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      if (type == expectedType)
      {
        if (level == Orthanc::ResourceType_Instance)
        {
          instances.AddResource(*current);
        }
        else
        {
          size_t lower;
          if (dicomDir.LookupLower(lower, index))
          {
            ExploreDicomDir(instances, dicomDir, Orthanc::GetChildResourceType(level), lower, *current);
          }
        }
      }

      size_t next;
      if (dicomDir.LookupNext(next, index))
      {
        index = next;
      }
      else
      {
        return;
      }
    }
  }
#endif


#if ORTHANC_ENABLE_DCMTK == 1
  void DicomResourcesLoader::GetDicomDirInstances(LoadedDicomResources& target,
                                                  const Orthanc::ParsedDicomDir& dicomDir)
  {
    Orthanc::DicomMap parent;
    ExploreDicomDir(target, dicomDir, Orthanc::ResourceType_Patient, 0, parent);
  }
#endif


#if ORTHANC_ENABLE_DCMTK == 1
  class DicomResourcesLoader::DicomDirHandler : public StringHandler
  {
  public:
    DicomDirHandler(boost::shared_ptr<DicomResourcesLoader> loader,
                    boost::shared_ptr<LoadedDicomResources> target,
                    int priority,
                    const DicomSource& source,
                    boost::shared_ptr<Orthanc::IDynamicObject> userPayload) :
      StringHandler(loader, target, priority, source, userPayload)
    {
    }

    virtual void HandleJson(const Json::Value& body) ORTHANC_OVERRIDE
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError);
    }
      
    virtual void HandleString(const std::string& body) ORTHANC_OVERRIDE
    {
      Orthanc::ParsedDicomDir dicomDir(body);
      GetDicomDirInstances(*GetTarget(), dicomDir);
      BroadcastSuccess();
    }
  };
#endif
  
    
  void DicomResourcesLoader::Handle(const HttpCommand::SuccessMessage& message)
  {
    if (message.GetOrigin().HasPayload())
    {
      dynamic_cast<StringHandler&>(message.GetOrigin().GetPayload()).HandleString(message.GetAnswer());
    }
  }


  void DicomResourcesLoader::Handle(const OrthancRestApiCommand::SuccessMessage& message)
  {
    if (message.GetOrigin().HasPayload())
    {
      dynamic_cast<StringHandler&>(message.GetOrigin().GetPayload()).HandleString(message.GetAnswer());
    }
  }


  void DicomResourcesLoader::Handle(const ReadFileCommand::SuccessMessage& message)
  {
    if (message.GetOrigin().HasPayload())
    {
      dynamic_cast<StringHandler&>(message.GetOrigin().GetPayload()).HandleString(message.GetContent());
    }
  }


#if ORTHANC_ENABLE_DCMTK == 1
  void DicomResourcesLoader::Handle(const ParseDicomSuccessMessage& message)
  {
    if (message.GetOrigin().HasPayload())
    {
      Handler& handler = dynamic_cast<Handler&>(message.GetOrigin().GetPayload());

      std::set<Orthanc::DicomTag> ignoreTagLength;
      ignoreTagLength.insert(Orthanc::DICOM_TAG_GRID_FRAME_OFFSET_VECTOR);  // Needed for RT-DOSE

      Orthanc::DicomMap summary;

#if ORTHANC_FRAMEWORK_VERSION_IS_ABOVE(1, 7, 3)
      message.GetDicom().ExtractDicomSummary(summary, ORTHANC_STONE_MAX_TAG_LENGTH, ignoreTagLength);
#else
      message.GetDicom().ExtractDicomSummary(summary, ignoreTagLength);
#endif
      
      handler.GetTarget()->AddResource(summary);

      handler.BroadcastSuccess();
    }
  }
#endif


  void DicomResourcesLoader::Handle(const OracleCommandExceptionMessage& message)
  {
    // TODO
    LOG(ERROR) << "Exception: " << message.GetException().What();
  }
    

  void DicomResourcesLoader::ScheduleLoadOrthancInstanceTags(boost::shared_ptr<LoadedDicomResources> target,
                                                             int priority,
                                                             const DicomSource& source,
                                                             const std::string& instanceId,
                                                             boost::shared_ptr<unsigned int> remainingCommands,
                                                             boost::shared_ptr<Orthanc::IDynamicObject> userPayload)
  {
    std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
    command->SetUri("/instances/" + instanceId + "/tags");
    command->AcquirePayload(new OrthancInstanceTagsHandler(shared_from_this(), target, priority,
                                                           source, remainingCommands, userPayload));

    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
      lock->Schedule(GetSharedObserver(), priority, command.release());
    }
  }


  void DicomResourcesLoader::ScheduleLoadOrthancOneChildInstance(boost::shared_ptr<LoadedDicomResources> target,
                                                                 int priority,
                                                                 const DicomSource& source,
                                                                 Orthanc::ResourceType level,
                                                                 const std::string& id,
                                                                 boost::shared_ptr<unsigned int> remainingCommands,
                                                                 boost::shared_ptr<Orthanc::IDynamicObject> userPayload)
  {
    std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
    command->SetUri("/" + GetUri(level) + "/" + id + "/instances");
    command->AcquirePayload(new OrthancOneChildInstanceHandler(shared_from_this(), target, priority,
                                                               source, remainingCommands, userPayload));

    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
      lock->Schedule(GetSharedObserver(), priority, command.release());
    }
  }
    
    

  const Orthanc::IDynamicObject& DicomResourcesLoader::SuccessMessage::GetUserPayload() const
  {
    if (userPayload_ == NULL)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
    }
    else
    {
      return *userPayload_;
    }
  }


  boost::shared_ptr<DicomResourcesLoader> DicomResourcesLoader::Create(const ILoadersContext::ILock& stone)
  {
    boost::shared_ptr<DicomResourcesLoader> result(new DicomResourcesLoader(stone.GetContext()));
    result->Register<HttpCommand::SuccessMessage>(stone.GetOracleObservable(), &DicomResourcesLoader::Handle);
    result->Register<OracleCommandExceptionMessage>(stone.GetOracleObservable(), &DicomResourcesLoader::Handle);
    result->Register<OrthancRestApiCommand::SuccessMessage>(stone.GetOracleObservable(), &DicomResourcesLoader::Handle);
    result->Register<ReadFileCommand::SuccessMessage>(stone.GetOracleObservable(), &DicomResourcesLoader::Handle);

#if ORTHANC_ENABLE_DCMTK == 1
    result->Register<ParseDicomSuccessMessage>(stone.GetOracleObservable(), &DicomResourcesLoader::Handle);
#endif
    
    return result;
  }


  static void SetIncludeTags(std::map<std::string, std::string>& arguments,
                             const std::set<Orthanc::DicomTag>& includeTags)
  {
    if (!includeTags.empty())
    {
      std::string s;
      bool first = true;

      for (std::set<Orthanc::DicomTag>::const_iterator
             it = includeTags.begin(); it != includeTags.end(); ++it)
      {
        if (first)
        {
          first = false;
        }
        else
        {
          s += ",";
        }

        char buf[16];
        sprintf(buf, "%04X%04X", it->GetGroup(), it->GetElement());
        s += std::string(buf);
      }

      arguments["includefield"] = s;
    }    
  }
  
  
  void DicomResourcesLoader::ScheduleGetDicomWeb(boost::shared_ptr<LoadedDicomResources> target,
                                                 int priority,
                                                 const DicomSource& source,
                                                 const std::string& uri,
                                                 const std::set<Orthanc::DicomTag>& includeTags,
                                                 Orthanc::IDynamicObject* userPayload)
  {
    boost::shared_ptr<Orthanc::IDynamicObject> protection(userPayload);
    
    if (!source.IsDicomWeb())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls, "Not a DICOMweb source");
    }

    std::map<std::string, std::string> arguments, headers;
    SetIncludeTags(arguments, includeTags);
  
    std::unique_ptr<IOracleCommand> command(
      source.CreateDicomWebCommand(uri, arguments, headers, 
                                   new DicomWebHandler(shared_from_this(), target, priority, source, protection)));
      
    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
      lock->Schedule(GetSharedObserver(), priority, command.release());
    }
  }
  

  void DicomResourcesLoader::ScheduleQido(boost::shared_ptr<LoadedDicomResources> target,
                                          int priority,
                                          const DicomSource& source,
                                          Orthanc::ResourceType level,
                                          const Orthanc::DicomMap& filter,
                                          const std::set<Orthanc::DicomTag>& includeTags,
                                          Orthanc::IDynamicObject* userPayload)
  {
    boost::shared_ptr<Orthanc::IDynamicObject> protection(userPayload);
    
    if (!source.IsDicomWeb())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls, "Not a DICOMweb source");
    }

    std::string uri;
    switch (level)
    {
      case Orthanc::ResourceType_Study:
        uri = "/studies";
        break;

      case Orthanc::ResourceType_Series:
        uri = "/series";
        break;

      case Orthanc::ResourceType_Instance:
        uri = "/instances";
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    std::set<Orthanc::DicomTag> tags;
    filter.GetTags(tags);

    std::map<std::string, std::string> arguments, headers;

    for (std::set<Orthanc::DicomTag>::const_iterator it = tags.begin(); it != tags.end(); ++it)
    {
      std::string s;
      if (filter.LookupStringValue(s, *it, false /* no binary */))
      {
        char buf[16];
        sprintf(buf, "%04X%04X", it->GetGroup(), it->GetElement());
        arguments[buf] = s;
      }
    }

    SetIncludeTags(arguments, includeTags);

    std::unique_ptr<IOracleCommand> command(
      source.CreateDicomWebCommand(uri, arguments, headers, 
                                   new DicomWebHandler(shared_from_this(), target, priority, source, protection)));


    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
      lock->Schedule(GetSharedObserver(), priority, command.release());
    }
  }

    
  void DicomResourcesLoader::ScheduleLoadOrthancResources(boost::shared_ptr<LoadedDicomResources> target,
                                                          int priority,
                                                          const DicomSource& source,
                                                          Orthanc::ResourceType topLevel,
                                                          const std::string& topId,
                                                          Orthanc::ResourceType bottomLevel,
                                                          Orthanc::IDynamicObject* userPayload)
  {
    boost::shared_ptr<Orthanc::IDynamicObject> protection(userPayload);
    
    if (!source.IsOrthanc())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls, "Not an Orthanc source");
    }

    bool ok = false;

    switch (topLevel)
    {
      case Orthanc::ResourceType_Patient:
        ok = (bottomLevel == Orthanc::ResourceType_Patient ||
              bottomLevel == Orthanc::ResourceType_Study ||
              bottomLevel == Orthanc::ResourceType_Series ||
              bottomLevel == Orthanc::ResourceType_Instance);
        break;
              
      case Orthanc::ResourceType_Study:
        ok = (bottomLevel == Orthanc::ResourceType_Study ||
              bottomLevel == Orthanc::ResourceType_Series ||
              bottomLevel == Orthanc::ResourceType_Instance);
        break;
              
      case Orthanc::ResourceType_Series:
        ok = (bottomLevel == Orthanc::ResourceType_Series ||
              bottomLevel == Orthanc::ResourceType_Instance);
        break;
              
      case Orthanc::ResourceType_Instance:
        ok = (bottomLevel == Orthanc::ResourceType_Instance);
        break;

      default:
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    if (!ok)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
    }

    boost::shared_ptr<unsigned int> remainingCommands(new unsigned int(0));

    if (topLevel == Orthanc::ResourceType_Instance)
    {
      ScheduleLoadOrthancInstanceTags(target, priority, source, topId, remainingCommands, protection);
    }
    else if (topLevel == bottomLevel)
    {
      ScheduleLoadOrthancOneChildInstance(target, priority, source, topLevel, topId, remainingCommands, protection);
    }
    else 
    {
      std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
      command->SetUri("/" + GetUri(topLevel) + "/" + topId + "/" + GetUri(bottomLevel));
      command->AcquirePayload(new OrthancAllChildrenInstancesHandler
                              (shared_from_this(), target, priority, source,
                               remainingCommands, bottomLevel, protection));

      {
        std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());

        // GetSharedObserver() means "this" (for use as an IObserver), as a 
        // shared_ptr
        // The oracle will thus call "this" 
        lock->Schedule(GetSharedObserver(), priority, command.release());
      }
    }
  }


  void DicomResourcesLoader::ScheduleLoadDicomDir(boost::shared_ptr<LoadedDicomResources> target,
                                                  int priority,
                                                  const DicomSource& source,
                                                  const std::string& path,
                                                  Orthanc::IDynamicObject* userPayload)
  {
    boost::shared_ptr<Orthanc::IDynamicObject> protection(userPayload);
    
    if (!source.IsDicomDir())
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls, "Not a DICOMDIR source");
    }

    if (target->GetIndexedTag() == Orthanc::DICOM_TAG_SOP_INSTANCE_UID)
    {
      LOG(WARNING) << "If loading DICOMDIR, it is advised to index tag "
                   << "ReferencedSopInstanceUidInFile (0004,1511)";
    }

#if ORTHANC_ENABLE_DCMTK == 1
    std::unique_ptr<ReadFileCommand> command(new ReadFileCommand(path));
    command->AcquirePayload(new DicomDirHandler(shared_from_this(), target, priority, source, protection));

    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());      
      lock->Schedule(GetSharedObserver(), priority, command.release());
    }
#else
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                    "DCMTK is disabled, cannot load DICOMDIR");
#endif
  }


  void DicomResourcesLoader::ScheduleLoadDicomFile(boost::shared_ptr<LoadedDicomResources> target,
                                                   int priority,
                                                   const DicomSource& source,
                                                   const std::string& path,
                                                   bool includePixelData,
                                                   Orthanc::IDynamicObject* userPayload)
  {
    boost::shared_ptr<Orthanc::IDynamicObject> protection(userPayload);
    
#if ORTHANC_ENABLE_DCMTK == 1
    std::unique_ptr<ParseDicomFromFileCommand> command(new ParseDicomFromFileCommand(source, path));
    command->SetPixelDataIncluded(includePixelData);
    command->AcquirePayload(new Handler(shared_from_this(), target, priority, source, protection));

    {
      std::unique_ptr<ILoadersContext::ILock> lock(context_.Lock());
      lock->Schedule(GetSharedObserver(), priority, command.release());
    }
#else
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                    "DCMTK is disabled, cannot load DICOM files");
#endif
  }


  bool DicomResourcesLoader::ScheduleLoadDicomFile(boost::shared_ptr<LoadedDicomResources> target,
                                                   int priority,
                                                   const DicomSource& source,
                                                   const std::string& dicomDirPath,
                                                   const Orthanc::DicomMap& dicomDirEntry,
                                                   bool includePixelData,
                                                   Orthanc::IDynamicObject* userPayload)
  {
    std::unique_ptr<Orthanc::IDynamicObject> protection(userPayload);
    
#if ORTHANC_ENABLE_DCMTK == 1
    std::string file;
    if (dicomDirEntry.LookupStringValue(file, Orthanc::DICOM_TAG_REFERENCED_FILE_ID, false))
    {
      ScheduleLoadDicomFile(target, priority, source, ParseDicomFromFileCommand::GetDicomDirPath(dicomDirPath, file),
                            includePixelData, protection.release());
      return true;
    }
    else
    {
      return false;
    }
#else
    throw Orthanc::OrthancException(Orthanc::ErrorCode_InternalError,
                                    "DCMTK is disabled, cannot load DICOM files");
#endif
  }
}
