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


#include "DictionaryValue.h"

#include <OrthancException.h>

#include <cassert>


namespace OrthancStone
{
  namespace OsiriX
  {
    DictionaryValue::~DictionaryValue()
    {
      for (Content::iterator it = content_.begin(); it != content_.end(); ++it)
      {
        assert(it->second != NULL);
        delete it->second;
      }
    }


    void DictionaryValue::SetValue(const std::string& key,
                                   IValue* value /* takes ownership */)
    {
      if (value == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
      else
      {
        std::unique_ptr<IValue> protection(value);

        Content::iterator found = content_.find(key);
        if (found == content_.end())
        {
          content_[key] = protection.release();
        }
        else
        {
          assert(found->second != NULL);
          delete found->second;
          found->second = protection.release();
        }
      }
    }
      

    const IValue* DictionaryValue::LookupValue(const std::string& key) const
    {
      Content::const_iterator found = content_.find(key);

      if (found == content_.end())
      {
        return NULL;
      }
      else
      {
        assert(found->second != NULL);
        return found->second;
      }
    }


    const IValue& DictionaryValue::GetValue(const std::string& key) const
    {
      const IValue* value = LookupValue(key);
      if (value == NULL)
      {
        // "HasValue()" should have been called
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadSequenceOfCalls);
      }
      else
      {
        return *value;
      }
    }

      
    void DictionaryValue::GetMembers(std::set<std::string>& target) const
    {
      target.clear();

      for (Content::const_iterator it = content_.begin(); it != content_.end(); ++it)
      {
        target.insert(it->first);
      }
    }
  }
}
