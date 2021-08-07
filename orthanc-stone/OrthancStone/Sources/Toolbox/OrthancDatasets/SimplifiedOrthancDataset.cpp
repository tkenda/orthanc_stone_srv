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


#include "SimplifiedOrthancDataset.h"

#include <OrthancException.h>
#include <DicomParsing/FromDcmtkBridge.h>

namespace OrthancStone
{
  const Json::Value* SimplifiedOrthancDataset::LookupPath(const Orthanc::DicomPath& path) const
  {
    const Json::Value* content = &root_;
                                  
    for (unsigned int depth = 0; depth < path.GetPrefixLength(); depth++)
    {
      const std::string name = Orthanc::FromDcmtkBridge::GetTagName(
        path.GetPrefixTag(depth), "" /* no private creator */);
      if (content->type() != Json::objectValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      if (!content->isMember(name))
      {
        return NULL;
      }

      const Json::Value& sequence = (*content) [name];
      if (sequence.type() != Json::arrayValue)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      size_t index = path.GetPrefixIndex(depth);
      if (index >= sequence.size())
      {
        return NULL;
      }
      else
      {
        content = &sequence[static_cast<Json::Value::ArrayIndex>(index)];
      }
    }

    const std::string name = Orthanc::FromDcmtkBridge::GetTagName(
      path.GetFinalTag(), "" /* no private creator */);

    if (content->type() != Json::objectValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    if (!content->isMember(name))
    {
      return NULL;
    }
    else
    {
      return &((*content) [name]);
    }
  }


  void SimplifiedOrthancDataset::CheckRoot() const
  {
    if (root_.type() != Json::objectValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
  }


  SimplifiedOrthancDataset::SimplifiedOrthancDataset(IOrthancConnection& orthanc,
                                                     const std::string& uri)
  {
    IOrthancConnection::RestApiGet(root_, orthanc, uri);
    CheckRoot();
  }


  SimplifiedOrthancDataset::SimplifiedOrthancDataset(const std::string& content)
  {
    IOrthancConnection::ParseJson(root_, content);
    CheckRoot();
  }


  bool SimplifiedOrthancDataset::GetStringValue(std::string& result,
                                                const Orthanc::DicomPath& path) const
  {
    const Json::Value* value = LookupPath(path);

    if (value == NULL)
    {
      return false;
    }
    else if (value->type() != Json::stringValue)
    {
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    else
    {
      result = value->asString();
      return true;
    }
  }


  bool SimplifiedOrthancDataset::GetSequenceSize(size_t& size,
                                                 const Orthanc::DicomPath& path) const
  {
    const Json::Value* sequence = LookupPath(path);

    if (sequence == NULL)
    {
      // Inexistent path
      return false;
    }
    else if (sequence->type() != Json::arrayValue)
    {
      // Not a sequence
      throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
    }
    else
    {
      size = sequence->size();
      return true;
    }
  }
}
