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


#include "DicomStructureSetLoader.h"

#include "../Scene2D/PolylineSceneLayer.h"
#include "../StoneException.h"
#include "../Toolbox/GeometryToolbox.h"

#include <Toolbox.h>

#if STONE_TIME_BLOCKING_OPS
# include <boost/date_time/posix_time/posix_time.hpp>
#endif

#include <algorithm>

namespace OrthancStone
{

#if 0
  void DumpDicomMap(std::ostream& o, const Orthanc::DicomMap& dicomMap)
  {
    using namespace std;
    //ios_base::fmtflags state = o.flags();
    //o.flags(ios::right | ios::hex);
    //o << "(" << setfill('0') << setw(4) << tag.GetGroup()
    //  << "," << setw(4) << tag.GetElement() << ")";
    //o.flags(state);
    Json::Value val;
    dicomMap.Serialize(val);
    o << val;
    //return o;
  }
#endif

  // implementation of IInstanceLookupHandler that uses Orthanc REST API calls to retrive the 
  // geometry of referenced instances
  class DicomStructureSetLoader::RestInstanceLookupHandler : public DicomStructureSetLoader::IInstanceLookupHandler,
    public LoaderStateMachine
  {
  public:
    static boost::shared_ptr<RestInstanceLookupHandler > Create(DicomStructureSetLoader& loader)
    {
      boost::shared_ptr<RestInstanceLookupHandler> obj(new RestInstanceLookupHandler(loader));
      obj->LoaderStateMachine::PostConstructor();
      return obj;
    }

  protected:
    explicit RestInstanceLookupHandler(DicomStructureSetLoader& loader) :
      LoaderStateMachine(loader.loadersContext_),
      loader_(loader)
    {
    }

    virtual void RetrieveReferencedSlices(const std::set<std::string>& nonEmptyInstances) ORTHANC_OVERRIDE;

  private:
    // these subclasses hold the loading state
    class AddReferencedInstance;   // 2nd state
    class LookupInstance;          // 1st state

    DicomStructureSetLoader& loader_;
  };

  class DicomStructureSetLoader::RestInstanceLookupHandler::AddReferencedInstance : public LoaderStateMachine::State
  {
  private:
    std::string instanceId_;
      
  public:
    AddReferencedInstance(DicomStructureSetLoader& that,
                          const std::string& instanceId) :
      State(that),
      instanceId_(instanceId)
    {
    }

    virtual void Handle(const OrthancRestApiCommand::SuccessMessage& message) ORTHANC_OVERRIDE
    {
      Json::Value tags;
      message.ParseJsonBody(tags);
        
      Orthanc::DicomMap dicom;
      dicom.FromDicomAsJson(tags);

      DicomStructureSetLoader& loader = GetLoader<DicomStructureSetLoader>();
    
      loader.AddReferencedSlice(dicom);
    }
  };


  // State that converts a "SOP Instance UID" to an Orthanc identifier
  class DicomStructureSetLoader::RestInstanceLookupHandler::LookupInstance : public LoaderStateMachine::State
  {
  private:
    std::string  sopInstanceUid_;
      
  public:
    LookupInstance(DicomStructureSetLoader& that,
                   const std::string& sopInstanceUid) :
      State(that),
      sopInstanceUid_(sopInstanceUid)
    {
    }

    virtual void Handle(const OrthancRestApiCommand::SuccessMessage& message) ORTHANC_OVERRIDE
    {
      DicomStructureSetLoader& loader = GetLoader<DicomStructureSetLoader>();

      Json::Value lookup;
      message.ParseJsonBody(lookup);

      if (lookup.type() != Json::arrayValue ||
          lookup.size() != 1 ||
          !lookup[0].isMember("Type") ||
          !lookup[0].isMember("Path") ||
          lookup[0]["Type"].type() != Json::stringValue ||
          lookup[0]["ID"].type() != Json::stringValue ||
          lookup[0]["Type"].asString() != "Instance")
      {
        std::stringstream msg;
        msg << "Unknown resource! message.GetAnswer() = " << message.GetAnswer() << " message.GetAnswerHeaders() = ";
        for (OrthancRestApiCommand::HttpHeaders::const_iterator it = message.GetAnswerHeaders().begin();
             it != message.GetAnswerHeaders().end(); ++it)
        {
          msg << "\nkey: \"" << it->first << "\" value: \"" << it->second << "\"\n";
        }
        const std::string msgStr = msg.str();
        LOG(ERROR) << msgStr;
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadJson);
      }

      const std::string instanceId = lookup[0]["ID"].asString();

      {
        std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
        command->SetCallerName("DicomStructureSetLoader::RestInstanceLookupHandler::LookupInstance");
        command->SetHttpHeader("Accept-Encoding", "gzip");
        std::string uri = "/instances/" + instanceId + "/tags";
        command->SetUri(uri);
        command->AcquirePayload(new AddReferencedInstance(loader, instanceId));
        Schedule(command.release());
      }
    }
  };

  void DicomStructureSetLoader::RestInstanceLookupHandler::RetrieveReferencedSlices(
    const std::set<std::string>& nonEmptyInstances)
  {
    for (std::set<std::string>::const_iterator it = nonEmptyInstances.begin(); 
         it != nonEmptyInstances.end(); 
         ++it)
    {
      std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
      command->SetCallerName("DicomStructureSetLoader::RestInstanceLookupHandler");
      command->SetUri("/tools/lookup");
      command->SetMethod(Orthanc::HttpMethod_Post);
      command->SetBody(*it);
      command->AcquirePayload(new LookupInstance(loader_, *it));
      Schedule(command.release());
    }
  }

  class DicomStructureSetLoader::LoadStructure : public LoaderStateMachine::State
  {
  public:
    explicit LoadStructure(DicomStructureSetLoader& that) :
      State(that)
    {
    }
    
    virtual void Handle(const OrthancRestApiCommand::SuccessMessage& message) ORTHANC_OVERRIDE
    {
#if STONE_TIME_BLOCKING_OPS
      boost::posix_time::ptime timerStart = boost::posix_time::microsec_clock::universal_time();
#endif

      DicomStructureSetLoader& loader = GetLoader<DicomStructureSetLoader>();

      // Set the actual structure set content
      {
        FullOrthancDataset dicom(message.GetAnswer());

        loader.content_.reset(new DicomStructureSet(dicom));
      }

      // initialize visibility flags
      SetDefaultStructureVisibility();

      // retrieve the (non-empty) referenced instances (the CT slices containing the corresponding structures)
      // Some (admittedly invalid) Dicom files have empty values in the 
      // 0008,1155 tag. We try our best to cope with this.
      // this is why we use `nonEmptyInstances` and not `instances`
      std::set<std::string> instances;
      std::set<std::string> nonEmptyInstances;

      // this traverses the polygon collection for all structures and retrieve the SOPInstanceUID of 
      // the referenced instances
      loader.content_->GetReferencedInstances(instances);

      for (std::set<std::string>::const_iterator
        it = instances.begin(); it != instances.end(); ++it)
      {
        std::string instance = Orthanc::Toolbox::StripSpaces(*it);
        if(instance != "")
          nonEmptyInstances.insert(instance);
      }

      loader.RetrieveReferencedSlices(nonEmptyInstances);
#if STONE_TIME_BLOCKING_OPS
      boost::posix_time::ptime timerEnd = boost::posix_time::microsec_clock::universal_time();
      boost::posix_time::time_duration duration = timerEnd - timerStart;
      int64_t durationMs = duration.total_milliseconds();
      LOG(WARNING) << "DicomStructureSetLoader::LoadStructure::Handle took " << durationMs << " ms";
#endif

    }

    void SetDefaultStructureVisibility()
    {
      DicomStructureSetLoader& loader = GetLoader<DicomStructureSetLoader>();

      size_t structureCount = loader.content_->GetStructuresCount();

      loader.structureVisibility_.resize(structureCount);
      bool everythingVisible = false;
      if ((loader.initiallyVisibleStructures_.size() == 1)
          && (loader.initiallyVisibleStructures_[0].size() == 1)
          && (loader.initiallyVisibleStructures_[0][0] == '*'))
      {
        everythingVisible = true;
      }

      for (size_t i = 0; i < structureCount; ++i)
      {
        // if a single "*" string is supplied, this means we want everything 
        // to be visible...
        if (everythingVisible)
        {
          loader.structureVisibility_.at(i) = true;
        }
        else
        {
          // otherwise, we only enable visibility for those structures whose 
          // names are mentioned in the initiallyVisibleStructures_ array
          const std::string& structureName = loader.content_->GetStructureName(i);

          std::vector<std::string>::iterator foundIt =
            std::find(
              loader.initiallyVisibleStructures_.begin(),
              loader.initiallyVisibleStructures_.end(),
              structureName);
          std::vector<std::string>::iterator endIt = loader.initiallyVisibleStructures_.end();
          if (foundIt != endIt)
            loader.structureVisibility_.at(i) = true;
          else
            loader.structureVisibility_.at(i) = false;
        }
      }
    }

    private:


  };
    

  class DicomStructureSetLoader::Slice : public IExtractedSlice
  {
  private:
    const DicomStructureSet&  content_;
    uint64_t                  revision_;
    bool                      isValid_;
    std::vector<bool>         visibility_;
      
  public:
    /**
    The visibility vector must either:
    - be empty
    or
    - contain the same number of items as the number of structures in the 
      structure set.
    In the first case (empty vector), all the structures are displayed.
    In the second case, the visibility of each structure is defined by the 
    content of the vector at the corresponding index.
    */
    Slice(const DicomStructureSet& content,
          uint64_t revision,
          const CoordinateSystem3D& cuttingPlane,
          const std::vector<bool>& visibility) :
      content_(content),
      revision_(revision),
      visibility_(visibility)
    {
      ORTHANC_ASSERT((visibility_.size() == content_.GetStructuresCount())
        || (visibility_.size() == 0u));

      bool opposite;

      const Vector normal = content.GetNormal();
      isValid_ = (
        GeometryToolbox::IsParallelOrOpposite(opposite, normal, cuttingPlane.GetNormal()) ||
        GeometryToolbox::IsParallelOrOpposite(opposite, normal, cuttingPlane.GetAxisX()) ||
        GeometryToolbox::IsParallelOrOpposite(opposite, normal, cuttingPlane.GetAxisY()));
    }
      
    virtual bool IsValid() ORTHANC_OVERRIDE
    {
      return isValid_;
    }

    virtual uint64_t GetRevision() ORTHANC_OVERRIDE
    {
      return revision_;
    }

    virtual ISceneLayer* CreateSceneLayer(
      const ILayerStyleConfigurator* configurator,
      const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE
    {
      assert(isValid_);

      std::unique_ptr<PolylineSceneLayer> layer(new PolylineSceneLayer);
      layer->SetThickness(2);

#if 0
      // For testing - This displays a cross at the origin of the 3D
      // cutting plane in the 2D viewport
      {
        PolylineSceneLayer::Chain chain;
        chain.push_back(ScenePoint2D(-100, 0));
        chain.push_back(ScenePoint2D(100, 0));
        layer->AddChain(chain, false, Color(255, 0, 0));
      }

      {
        PolylineSceneLayer::Chain chain;
        chain.push_back(ScenePoint2D(0, -100));
        chain.push_back(ScenePoint2D(0, 100));
        layer->AddChain(chain, false, Color(255, 0, 0));
      }
#endif

      for (size_t i = 0; i < content_.GetStructuresCount(); i++)
      {
        if ((visibility_.size() == 0) || visibility_.at(i))
        {
          const Color& color = content_.GetStructureColor(i);

#if USE_BOOST_UNION_FOR_POLYGONS == 1
          std::vector< std::vector<Point2D> > polygons;
          
          if (content_.ProjectStructure(polygons, i, cuttingPlane))
          {
            for (size_t j = 0; j < polygons.size(); j++)
            {
              PolylineSceneLayer::Chain chain;
              chain.resize(polygons[j].size());
              
              for (size_t k = 0; k < polygons[j].size(); k++)
              {
                chain[k] = ScenePoint2D(polygons[j][k].x, polygons[j][k].y);
              }
              
              layer->AddChain(chain, true /* closed */, color);
            }
          }
#else
          std::vector< std::pair<Point2D, Point2D> > segments;

          if (content_.ProjectStructure(segments, i, cuttingPlane))
          {
            for (size_t j = 0; j < segments.size(); j++)
            {
              PolylineSceneLayer::Chain chain;
              chain.resize(2);

              chain[0] = ScenePoint2D(segments[j].first.x, segments[j].first.y);
              chain[1] = ScenePoint2D(segments[j].second.x, segments[j].second.y);

              layer->AddChain(chain, false /* NOT closed */, color);
            }
          }
#endif        
        }
      }

      return layer.release();
    }
  };
    

  DicomStructureSetLoader::DicomStructureSetLoader(
    ILoadersContext& loadersContext) 
    : LoaderStateMachine(loadersContext)
    , loadersContext_(loadersContext)
    , revision_(0)
    , countProcessedInstances_(0)
    , countReferencedInstances_(0)
    , structuresReady_(false)
  {
    // the default handler to retrieve slice geometry is RestInstanceLookupHandler
    instanceLookupHandler_ = RestInstanceLookupHandler::Create(*this);
  }
    
  boost::shared_ptr<DicomStructureSetLoader> DicomStructureSetLoader::Create(ILoadersContext& loadersContext)
  {
    boost::shared_ptr<DicomStructureSetLoader> obj(
      new DicomStructureSetLoader(
        loadersContext));
    obj->LoaderStateMachine::PostConstructor();
    return obj;
  }

  void DicomStructureSetLoader::AddReferencedSlice(const Orthanc::DicomMap& dicom)
  {
    content_->AddReferencedSlice(dicom);
    countProcessedInstances_ ++;
    assert(countProcessedInstances_ <= countReferencedInstances_);

    revision_++;
    SetStructuresUpdated();

    if (countProcessedInstances_ == countReferencedInstances_)
    {
      // All the referenced instances have been loaded, finalize the RT-STRUCT
      content_->CheckReferencedSlices();
      revision_++;
      SetStructuresReady();
    }
  }

  void DicomStructureSetLoader::RetrieveReferencedSlices(const std::set<std::string>& nonEmptyInstances)
  {
    // we set the number of referenced instances. This allows to know, in the method above, when we're done
    countReferencedInstances_ = static_cast<unsigned int>(nonEmptyInstances.size());
    instanceLookupHandler_->RetrieveReferencedSlices(nonEmptyInstances);
  }

  void DicomStructureSetLoader::SetStructureDisplayState(size_t structureIndex, bool display)
  {
    structureVisibility_.at(structureIndex) = display;
    revision_++;
  }

  DicomStructureSetLoader::~DicomStructureSetLoader()
  {
    LOG(TRACE) << "DicomStructureSetLoader::~DicomStructureSetLoader()";
  }

  void DicomStructureSetLoader::LoadInstance(
    const std::string& instanceId, 
    const std::vector<std::string>& initiallyVisibleStructures)
  {
    Start();
      
    instanceId_ = instanceId;
    initiallyVisibleStructures_ = initiallyVisibleStructures;

    {
      std::unique_ptr<OrthancRestApiCommand> command(new OrthancRestApiCommand);
      command->SetCallerName("DicomStructureSetLoader::LoadInstance");
      command->SetHttpHeader("Accept-Encoding", "gzip");

      std::string uri = "/instances/" + instanceId + "/tags?ignore-length=3006-0050";

      command->SetUri(uri);
      command->AcquirePayload(new LoadStructure(*this));
      Schedule(command.release());
    }
  }

  void DicomStructureSetLoader::LoadInstanceFullVisibility(const std::string& instanceId)
  {
    std::vector<std::string> initiallyVisibleStructures;
    initiallyVisibleStructures.push_back("*"); // wildcard to make all structure sets visible
    LoadInstance(instanceId, initiallyVisibleStructures);
  }

  IVolumeSlicer::IExtractedSlice* DicomStructureSetLoader::ExtractSlice(const CoordinateSystem3D& cuttingPlane)
  {
    if (content_.get() == NULL)
    {
      // Geometry is not available yet
      return new IVolumeSlicer::InvalidSlice;
    }
    else
    {
      return new Slice(*content_, revision_, cuttingPlane, structureVisibility_);
    }
  }

  void DicomStructureSetLoader::SetStructuresUpdated()
  {
    BroadcastMessage(DicomStructureSetLoader::StructuresUpdated(*this));
  }

  void DicomStructureSetLoader::SetStructuresReady()
  {
    ORTHANC_ASSERT(!structuresReady_);
    structuresReady_ = true;
    BroadcastMessage(DicomStructureSetLoader::StructuresReady(*this));
  }

  bool DicomStructureSetLoader::AreStructuresReady() const
  {
    return structuresReady_;
  }

}
