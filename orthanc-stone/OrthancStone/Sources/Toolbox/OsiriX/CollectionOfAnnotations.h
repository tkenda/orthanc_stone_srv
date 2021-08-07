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

#include "Annotation.h"

#include <set>
#include <vector>

namespace OrthancStone
{
  namespace OsiriX
  {
    class CollectionOfAnnotations : public boost::noncopyable
    {
    private:
      typedef std::map<std::string, std::set<size_t> >  SopInstanceUidIndex;
      
      std::vector<Annotation*>  annotations_;
      SopInstanceUidIndex       index_;

    public:
      ~CollectionOfAnnotations()
      {
        Clear();
      }

      void Clear();
      
      size_t GetSize() const
      {
        return annotations_.size();
      }

      const Annotation& GetAnnotation(size_t i) const;

      void AddAnnotation(Annotation* annotation);  // takes ownership

      void LookupSopInstanceUid(std::set<size_t>& target,
                                const std::string& sopInstanceUid) const;

      // Load an XML from OsiriX
      void LoadXml(const char* xml,
                    size_t size);

      void LoadXml(const std::string& xml)
      {
        LoadXml(xml.empty() ? NULL : xml.c_str(), xml.size());
      }
    };
  }
}
