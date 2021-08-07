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

#include <vector>

namespace OrthancStone
{
  namespace OsiriX
  {
    class ArrayValue : public IValue
    {
    private:
      std::vector<IValue*>  content_;
  
    public:
      virtual ~ArrayValue();

      virtual Type GetType() const ORTHANC_OVERRIDE
      {
        return Type_Array;
      }

      void Append(IValue* item);  // Takes ownership

      void Reserve(size_t n)
      {
        content_.reserve(n);
      }
  
      size_t GetSize() const
      {
        return content_.size();
      }

      const IValue& GetValue(size_t i) const;
    };
  }
}
