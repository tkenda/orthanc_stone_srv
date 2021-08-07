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

#include "IValue.h"

#include <Compatibility.h>

#include <map>
#include <set>
#include <string>


namespace OrthancStone
{
  namespace OsiriX
  {
    class DictionaryValue : public IValue
    {
    private:
      typedef std::map<std::string, IValue*>  Content;

      Content  content_;

    public:
      virtual ~DictionaryValue();

      virtual Type GetType() const ORTHANC_OVERRIDE
      {
        return Type_Dictionary;
      }

      void SetValue(const std::string& key,
                    IValue* value /* takes ownership */);
      
      // Will return "false" if no such item
      const IValue* LookupValue(const std::string& key) const;
      
      bool HasValue(const std::string& key) const
      {
        return LookupValue(key) != NULL;
      }

      const IValue& GetValue(const std::string& key) const;
      
      void GetMembers(std::set<std::string>& target) const;
    };
  }
}
