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

#include "../Toolbox/DicomStructureSet.h"
#include "../Volumes/IVolumeSlicer.h"
#include "../Loaders/ILoadersContext.h"
#include "LoaderStateMachine.h"

#include <vector>

namespace OrthancStone
{
  class DicomStructureSetLoader :
    public LoaderStateMachine,
    public IVolumeSlicer,
    public IObservable
  {
  public:
    ORTHANC_STONE_DEFINE_ORIGIN_MESSAGE(__FILE__, __LINE__, StructuresReady, DicomStructureSetLoader);
    ORTHANC_STONE_DEFINE_ORIGIN_MESSAGE(__FILE__, __LINE__, StructuresUpdated, DicomStructureSetLoader);

    /**
    
    Once the structure set has been loaded (the LoadStructure state), we need to fill it with geometry information
    from the referenced slices (tag (0008,1155) described here:
    https://dicom.innolitics.com/ciods/rt-structure-set/general-reference/00081140/00081155

    This interface allows to customize how this information can be gathered. By default, the RestInstanceLookupHandler
    will perform a REST call to the Orthanc API to retrieve this information.

    Injecting another implementation of this interface is useful when where this information can be supplied in 
    another (faster) way (for instance, if a separate loader for the CT series can be used to supply the slice geometry)
    */
    class IInstanceLookupHandler
    {
    public:
      virtual void RetrieveReferencedSlices(const std::set<std::string>& instances) = 0;
    };

    // predeclaration of the default IInstanceLookupHandler implementation
    class RestInstanceLookupHandler;

    static boost::shared_ptr<DicomStructureSetLoader> Create(
      ILoadersContext& loadersContext);

    void SetInstanceLookupHandler(boost::shared_ptr<IInstanceLookupHandler> instanceLookupHandler)
    {
      instanceLookupHandler_ = instanceLookupHandler;
    }

    DicomStructureSet* GetContent()
    {
      return content_.get();
    }

    void SetStructureDisplayState(size_t structureIndex, bool display);
    
    bool GetStructureDisplayState(size_t structureIndex) const
    {
      return structureVisibility_.at(structureIndex);
    }

    ~DicomStructureSetLoader();
    
    void LoadInstance(const std::string& instanceId, 
                      const std::vector<std::string>& initiallyVisibleStructures = std::vector<std::string>());

    void LoadInstanceFullVisibility(const std::string& instanceId);


    virtual IExtractedSlice* ExtractSlice(const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE;

    void SetStructuresReady();
    void SetStructuresUpdated();

    bool AreStructuresReady() const;

    /**
      Called by the IInstanceLookupHandler when slice referenced instance information is available. 
      When the last referenced slice is received, this method will perform a final check and will warn observers
    */
    void AddReferencedSlice(const Orthanc::DicomMap& dicom);

  private:
    class Slice;

    // Only state of LoaderStateMachine
    class LoadStructure;           // 1st state
    
    ILoadersContext&                    loadersContext_;
    std::unique_ptr<DicomStructureSet>  content_;
    uint64_t                            revision_;
    std::string                         instanceId_;
    unsigned int                        countProcessedInstances_;
    unsigned int                        countReferencedInstances_;  

    // will be set to true once the loading is finished
    bool                                structuresReady_;

    /**
    At load time, these strings are used to initialize the structureVisibility_ 
    vector.

    As a special case, if initiallyVisibleStructures_ contains a single string
    that is '*', ALL structures will be made visible.
    */
    std::vector<std::string> initiallyVisibleStructures_;

    /**
    Contains the "Should this structure be displayed?" flag for all structures.
    Only filled when structures are loaded.

    Changing this value directly affects the rendering
    */
    std::vector<bool>                  structureVisibility_;


    boost::shared_ptr<IInstanceLookupHandler> instanceLookupHandler_;

  private:
    void RetrieveReferencedSlices(const std::set<std::string>& nonEmptyInstances);

  protected:
    explicit DicomStructureSetLoader(ILoadersContext& loadersContext);
  };
}
