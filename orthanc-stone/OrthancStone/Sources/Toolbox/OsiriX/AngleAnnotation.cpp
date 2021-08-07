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


#include "AngleAnnotation.h"

#include "ArrayValue.h"
#include "IntegerValue.h"
#include "StringValue.h"

#include <OrthancException.h>


namespace OrthancStone
{
  namespace OsiriX
  {
    AngleAnnotation::AngleAnnotation(const DictionaryValue& dict)
    {
      SetupCommon(dict);
    
      const IntegerValue& number = dynamic_cast<const IntegerValue&>(dict.GetValue("NumberOfPoints"));
      const ArrayValue& points = dynamic_cast<const ArrayValue&>(dict.GetValue("Point_mm"));

      if (number.GetValue() != 3 ||
          points.GetSize() != 3)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      dynamic_cast<const StringValue&>(points.GetValue(0)).ParseVector(a_);
      dynamic_cast<const StringValue&>(points.GetValue(1)).ParseVector(center_);
      dynamic_cast<const StringValue&>(points.GetValue(2)).ParseVector(b_);

      if (a_.size() != 3u ||
          center_.size() != 3u ||
          b_.size() != 3u)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }
    }
  }
}
