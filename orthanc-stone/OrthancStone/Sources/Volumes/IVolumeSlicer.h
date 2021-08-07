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

#include "../Scene2D/ILayerStyleConfigurator.h"
#include "../Toolbox/CoordinateSystem3D.h"

namespace OrthancStone
{
  /**
  This interface is implemented by objects representing 3D volume data and 
  that are able to return an object that:
  - represent a slice of their data 
  - are able to create the corresponding slice visual representation.
  */
  class IVolumeSlicer : public boost::noncopyable
  {
  public:
    /**
    This interface is implemented by objects representing a slice of 
    volume data and that are able to create a 2D layer to display a this 
    slice.

    The CreateSceneLayer factory method is called with an optional
    configurator that possibly impacts the ISceneLayer subclass that is 
    created (for instance, if a LUT must be applied on the texture when
    displaying it)
    */
    class IExtractedSlice : public boost::noncopyable
    {
    public:
      virtual ~IExtractedSlice()
      {
      }

      /**
      Invalid slices are created when the data is not ready yet or if the
      cut is outside of the available geometry.
      */
      virtual bool IsValid() = 0;

      /**
      This retrieves the *revision* that gets incremented every time the 
      underlying object undergoes a mutable operation (that it, changes its 
      state).
      This **must** be a cheap call.
      */
      virtual uint64_t GetRevision() = 0;

      /** Creates the slice visual representation */
      virtual ISceneLayer* CreateSceneLayer(
        const ILayerStyleConfigurator* configurator,  // possibly absent
        const CoordinateSystem3D& cuttingPlane) = 0;
    };

    /**
    See IExtractedSlice.IsValid()
    */
    class InvalidSlice : public IExtractedSlice
    {
    public:
      virtual bool IsValid() ORTHANC_OVERRIDE
      {
        return false;
      }

      virtual uint64_t GetRevision() ORTHANC_OVERRIDE;

      virtual ISceneLayer* CreateSceneLayer(const ILayerStyleConfigurator* configurator,
                                            const CoordinateSystem3D& cuttingPlane) ORTHANC_OVERRIDE;
    };


    virtual ~IVolumeSlicer()
    {
    }

    /**
    This method is implemented by the objects representing volumetric data
    and must returns an IExtractedSlice subclass that contains all the data
    needed to, later on, create its visual representation through
    CreateSceneLayer.
    Subclasses a.o.: 
    - InvalidSlice, 
    - DicomVolumeImageMPRSlicer::Slice, 
    - DicomVolumeImageReslicer::Slice
    - DicomStructureSetLoader::Slice 
    */
    virtual IExtractedSlice* ExtractSlice(const CoordinateSystem3D& cuttingPlane) = 0;
  };
}
