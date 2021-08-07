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


#include "LineAnnotation.h"

#include "ArrayValue.h"
#include "IntegerValue.h"
#include "StringValue.h"

#include <OrthancException.h>


namespace OrthancStone
{
  namespace OsiriX
  {
    LineAnnotation::LineAnnotation(const DictionaryValue& dict,
                                   bool isArrow) :
      isArrow_(isArrow)
    {
      SetupCommon(dict);
    
      const IntegerValue& number = dynamic_cast<const IntegerValue&>(dict.GetValue("NumberOfPoints"));
      const ArrayValue& points = dynamic_cast<const ArrayValue&>(dict.GetValue("Point_mm"));

      if (number.GetValue() != 2 ||
          points.GetSize() != 2)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      dynamic_cast<const StringValue&>(points.GetValue(0)).ParseVector(p1_);
      dynamic_cast<const StringValue&>(points.GetValue(1)).ParseVector(p2_);

      if (p1_.size() != 3u ||
          p2_.size() != 3u)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }
    }
  }
}
