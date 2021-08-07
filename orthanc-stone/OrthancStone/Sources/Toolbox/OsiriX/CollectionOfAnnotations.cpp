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


#include "CollectionOfAnnotations.h"

#include "ArrayValue.h"
#include "IntegerValue.h"

#include <OrthancException.h>

#include <cassert>

namespace OrthancStone
{
  namespace OsiriX
  {
    static void GetAttributes(std::map<std::string, std::string>& target,
                              const pugi::xml_node& node)
    {
      for (pugi::xml_attribute attr = node.first_attribute(); attr; attr = attr.next_attribute())
      {
        target[attr.name()] = attr.value();
      }
    }

      
    void CollectionOfAnnotations::Clear()
    {
      for (size_t i = 0; i < annotations_.size(); i++)
      {
        assert(annotations_[i] != NULL);
        delete annotations_[i];
      }

      annotations_.clear();
      index_.clear();
    }


    const Annotation& CollectionOfAnnotations::GetAnnotation(size_t i) const
    {
      if (i >= annotations_.size())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_ParameterOutOfRange);
      }
      else
      {
        assert(annotations_[i] != NULL);
        return *annotations_[i];
      }
    }
    

    void CollectionOfAnnotations::AddAnnotation(Annotation* annotation)
    {
      if (annotation == NULL)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_NullPointer);
      }
      else
      {
        size_t pos = annotations_.size();
        annotations_.push_back(annotation);

        SopInstanceUidIndex::iterator found = index_.find(annotation->GetSopInstanceUid());
        if (found == index_.end())
        {
          std::set<size_t> s;
          s.insert(pos);
          index_[annotation->GetSopInstanceUid()] = s;
        }
        else
        {
          found->second.insert(pos);
        }
      }
    }


    void CollectionOfAnnotations::LookupSopInstanceUid(std::set<size_t>& target,
                                                       const std::string& sopInstanceUid) const
    {
      SopInstanceUidIndex::const_iterator found = index_.find(sopInstanceUid);
      if (found == index_.end())
      {
        target.clear();
      }
      else
      {
        target = found->second;
      }
    }
    

    void CollectionOfAnnotations::LoadXml(const char* xml,
                                          size_t size)
    {
      pugi::xml_document doc;
      pugi::xml_parse_result result = doc.load_buffer(xml, size);
      if (!result)
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }
        
      const pugi::xml_node& root = doc.document_element();
      if (std::string(root.name()) != "plist" ||
          !root.first_child() ||
          root.first_child() != root.last_child())
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      std::map<std::string, std::string> attributes;
      GetAttributes(attributes, root);

      std::map<std::string, std::string>::const_iterator version = attributes.find("version");
      if (version == attributes.end() ||
          version->second != "1.0")
      {
        throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
      }

      std::unique_ptr<IValue> value(IValue::Parse(root.first_child()));

      const DictionaryValue& dict = dynamic_cast<const DictionaryValue&>(*value);

      std::set<std::string> annotations;
      dict.GetMembers(annotations);

      for (std::set<std::string>::const_iterator
             it = annotations.begin(); it != annotations.end(); ++it)
      {
        const ArrayValue& images = dynamic_cast<const ArrayValue&>(dict.GetValue(*it));

        for (size_t i = 0; i < images.GetSize(); i++)
        {
          const DictionaryValue& image = dynamic_cast<const DictionaryValue&>(images.GetValue(i));
          const IntegerValue& number = dynamic_cast<const IntegerValue&>(image.GetValue("NumberOfROIs"));
          const ArrayValue& rois = dynamic_cast<const ArrayValue&>(image.GetValue("ROIs"));
          
          if (static_cast<int64_t>(rois.GetSize()) != number.GetValue())
          {
            throw Orthanc::OrthancException(Orthanc::ErrorCode_BadFileFormat);
          }

          for (size_t j = 0; j < rois.GetSize(); j++)
          {
            const DictionaryValue& roi = dynamic_cast<const DictionaryValue&>(rois.GetValue(i));

            std::unique_ptr<Annotation> annotation(Annotation::Create(roi));
            if (annotation.get() != NULL)
            {
              AddAnnotation(annotation.release());
            }
          }
        }
      }
    }
  }
}
