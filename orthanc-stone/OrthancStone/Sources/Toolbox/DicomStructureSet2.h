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

#ifdef BGO_ENABLE_DICOMSTRUCTURESETLOADER2

#include "../Scene2D/Color.h"
#include "CoordinateSystem3D.h"
#include "DicomStructure2.h"
#include "Extent2D.h"
#include "OrthancDatasets/FullOrthancDataset.h"

#include <list>

namespace OrthancStone
{
  class DicomStructureSet2 : public boost::noncopyable
  {
  public:
    DicomStructureSet2();
    ~DicomStructureSet2();
   
    void SetContents(const FullOrthancDataset& tags);

    size_t GetStructuresCount() const
    {
      return structures_.size();
    }

    void Clear();

    const DicomStructure2& GetStructure(size_t i) const
    {
      // at() is like []() but with range check
      return structures_.at(i);
    }

    /** Internal use only */
    void FillStructuresFromDataset(const FullOrthancDataset& tags);

    /** Internal use only */
    void ComputeDependentProperties();

    /** Internal use only */
    std::vector<DicomStructure2> structures_;
  };
}

#endif 
// BGO_ENABLE_DICOMSTRUCTURESETLOADER2


