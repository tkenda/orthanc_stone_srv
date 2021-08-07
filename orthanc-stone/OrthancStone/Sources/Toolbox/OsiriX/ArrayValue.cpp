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


#include "ArrayValue.h"

#include <OrthancException.h>

#include <cassert>


namespace OrthancStone
{
  namespace OsiriX
  {
    ArrayValue::~ArrayValue()
    {
      for (size_t i = 0; i < content_.size(); i++)
      {
        assert(content_[i] != NULL);
        delete content_[i];
      }
    }


    void ArrayValue::Append(IValue* item)  // Takes ownership
    {
      if (item == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
      else
      {
        content_.push_back(item);
      }
    }


    const IValue& ArrayValue::GetValue(size_t i) const
    {
      if (i >= content_.size())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
      else
      {
        assert(content_[i] != NULL);
        return *content_[i];
      }
    }
  }
}
