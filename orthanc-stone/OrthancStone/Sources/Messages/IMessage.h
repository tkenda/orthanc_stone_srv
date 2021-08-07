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

#include <boost/lexical_cast.hpp>
#include <boost/noncopyable.hpp>

#include <string.h>

namespace OrthancStone 
{
  class MessageIdentifier
  {
  private:
    const char*  file_;
    int          line_;

    bool IsEqual(const MessageIdentifier& other) const
    {
      return (line_ == other.line_ &&
              strcmp(file_, other.file_) == 0);
    }

  public:
    MessageIdentifier(const char* file,
                      int line) :
      file_(file),
      line_(line)
    {
    }

    MessageIdentifier() :
      file_(NULL),
      line_(0)
    {
    }

    std::string AsString() const
    {
      return std::string(file_) + ":" + boost::lexical_cast<std::string>(line_);
    }

    bool operator< (const MessageIdentifier& other) const
    {
      if (file_ == NULL)
      {
        return false;
      }
      else if (line_ != other.line_)
      {
        return line_ < other.line_;
      }
      else
      {
        return strcmp(file_, other.file_) < 0;
      }
    }

    bool operator== (const MessageIdentifier& other) const
    {
      return IsEqual(other);
    }

    bool operator!= (const MessageIdentifier& other) const
    {
      return !IsEqual(other);
    }
  };

    
  /**
   * Base messages that are exchanged between IObservable and
   * IObserver. Messages are distinguished by the "__FILE__" and
   * "__LINE__" macro, as in "Orthanc::SQLite::StatementId".
   **/
  class IMessage : public boost::noncopyable
  {
  public:
    virtual ~IMessage()
    {
    }

    virtual const MessageIdentifier& GetIdentifier() const = 0;
  };


  /**
   * Simple message implementation when no payload is needed but the
   * origin is required. Sample usage:
   * typedef OriginMessage<OrthancSlicesLoader> SliceGeometryErrorMessage;
   **/
  template <typename TOrigin>
  class OriginMessage : public IMessage
  {
  private:
    const TOrigin&  origin_;

  public:
    explicit OriginMessage(const TOrigin& origin) :
      origin_(origin)
    {
    }

    const TOrigin& GetOrigin() const
    {
      return origin_;
    }
  };
}


#define ORTHANC_STONE_MESSAGE(FILE, LINE)                               \
  public:                                                               \
  static const ::OrthancStone::MessageIdentifier& GetStaticIdentifier() \
  {                                                                     \
    static const ::OrthancStone::MessageIdentifier id(FILE, LINE);      \
    return id;                                                          \
  }                                                                     \
                                                                        \
  virtual const ::OrthancStone::MessageIdentifier& GetIdentifier() const ORTHANC_OVERRIDE \
  {                                                                     \
    return GetStaticIdentifier();                                       \
  }


#define ORTHANC_STONE_DEFINE_ORIGIN_MESSAGE(FILE, LINE, NAME, ORIGIN)   \
  class NAME : public ::OrthancStone::OriginMessage<ORIGIN>             \
  {                                                                     \
    ORTHANC_STONE_MESSAGE(FILE, LINE);                                  \
                                                                        \
    explicit NAME(const ORIGIN& origin) :                               \
      OriginMessage(origin)                                             \
    {                                                                   \
    }                                                                   \
  };


#define ORTHANC_STONE_DEFINE_EMPTY_MESSAGE(FILE, LINE, NAME)            \
  class NAME : public ::OrthancStone::IMessage                          \
  {                                                                     \
    ORTHANC_STONE_MESSAGE(FILE, LINE);                                  \
  };
