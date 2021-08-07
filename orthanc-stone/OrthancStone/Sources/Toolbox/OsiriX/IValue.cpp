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
#include "DictionaryValue.h"
#include "IntegerValue.h"
#include "RealValue.h"
#include "StringValue.h"

#include <OrthancException.h>
#include <Toolbox.h>

#include <boost/lexical_cast.hpp>


namespace OrthancStone
{
  namespace OsiriX
  {
    IValue* IValue::Parse(const pugi::xml_node& node)
    {
      const std::string name(node.name());
  
      if (name == "dict")
      {
        std::unique_ptr<DictionaryValue> dict(new DictionaryValue);

        for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling())
        {
          const std::string name2(child.name());

          if (name2 != "key" ||
              child.text().get() == NULL)
          {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
          }

          const std::string key(child.text().get());

          child = child.next_sibling();
          if (!child)
          {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
          }

          dict->SetValue(key, IValue::Parse(child));
        }

        return dict.release();
      }
      else if (name == "array")
      {
        std::unique_ptr<ArrayValue> array(new ArrayValue);

        for (pugi::xml_node child = node.first_child(); child; child = child.next_sibling())
        {
          array->Append(IValue::Parse(child));
        }
    
        return array.release();
      }
      else if (name == "integer")
      {
        const std::string s = Orthanc::Toolbox::StripSpaces(node.text().get());
      
        try
        {
          int64_t value = boost::lexical_cast<int64_t>(s);
          return new IntegerValue(value);
        }
        catch (boost::bad_lexical_cast&)
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                          "Cannot parse an integer: " + s);
        }
      }    
      else if (name == "real")
      {
        const std::string s = Orthanc::Toolbox::StripSpaces(node.text().get());

        try
        {
          double value = boost::lexical_cast<double>(s);
          return new RealValue(value);
        }
        catch (boost::bad_lexical_cast&)
        {
          throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                          "Cannot parse a real number: " + s);
        }
      }    
      else if (name == "string")
      {
        return new StringValue(node.text().get());
      }
      else
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat,
                                        "Unknown XML element: " + name);
      }
    }
  }
}
