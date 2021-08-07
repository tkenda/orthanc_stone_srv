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

#include "Toolbox/LinearAlgebra.h"

#include <OrthancException.h>

#include <boost/lexical_cast.hpp>

#include <iostream>

namespace OrthancStone
{
  enum ErrorCode
  {
    ErrorCode_Success,
    ErrorCode_OrthancError, // this StoneException is actually an OrthancException with an Orthanc error code
    ErrorCode_ApplicationException, // this StoneException is specific to an application (and should have its own internal error code)
    ErrorCode_NotImplemented, // case not implemented

    ErrorCode_CanOnlyAddOneLayerAtATime,
    ErrorCode_CommandJsonInvalidFormat,
    ErrorCode_WebGLContextLost,
    ErrorCode_Last
  };



  class StoneException
  {
  protected:
    ErrorCode     errorCode_;

  public:
    explicit StoneException(ErrorCode errorCode) :
      errorCode_(errorCode)
    {
    }

    virtual ~StoneException() {}

    ErrorCode GetErrorCode() const
    {
      return errorCode_;
    }

    virtual const char* What() const
    {
      switch (errorCode_)
      {
      case ErrorCode_Success:
        return "Success";
        break;
      case ErrorCode_OrthancError:
        return "OrthancError";
        break;
      case ErrorCode_ApplicationException:
        return "ApplicationException";
        break;
      case ErrorCode_NotImplemented:
        return "NotImplemented";
        break;
      case ErrorCode_CanOnlyAddOneLayerAtATime:
        return "CanOnlyAddOneLayerAtATime";
        break;
      case ErrorCode_CommandJsonInvalidFormat:
        return "CommandJsonInvalidFormat";
        break;
      case ErrorCode_WebGLContextLost:
        return "WebGLContextLost";
        break;
      case ErrorCode_Last:
        return "Last";
        break;
      default:
        return "Unknown exception code!";
      }
    }
  };
}

// See https://isocpp.org/wiki/faq/misc-technical-issues#macros-with-multi-stmts
// (or google "Multiple lines macro C++ faq lite" if link is dead)
#define ORTHANC_ASSERT2(cond,streamChainMessage) \
    if (!(cond)) { \
      std::stringstream sst; \
      sst << "Assertion failed. Condition = \"" #cond "\" Message = \"" << streamChainMessage << "\""; \
      std::string sstr = sst.str(); \
      throw ::Orthanc::OrthancException(::Orthanc::ErrorCode_InternalError,sstr.c_str()); \
    } else (void)0

#define ORTHANC_ASSERT1(cond) \
    if (!(cond)) { \
      std::stringstream sst; \
      sst << "Assertion failed. Condition = \"" #cond "\""; \
      std::string sstr = sst.str(); \
      throw ::Orthanc::OrthancException(::Orthanc::ErrorCode_InternalError,sstr.c_str()); \
    } else (void)0

# define ORTHANC_EXPAND( x ) x 
# define GET_ORTHANC_ASSERT(_1,_2,NAME,...) NAME
# define ORTHANC_ASSERT(...) ORTHANC_EXPAND(GET_ORTHANC_ASSERT(__VA_ARGS__, ORTHANC_ASSERT2, ORTHANC_ASSERT1, UNUSED)(__VA_ARGS__))





/*
Explanation:

ORTHANC_ASSERT(a)
ORTHANC_EXPAND(GET_ORTHANC_ASSERT(a, ORTHANC_ASSERT2, ORTHANC_ASSERT1, UNUSED)(a))
ORTHANC_EXPAND(ORTHANC_ASSERT1(a))
ORTHANC_ASSERT1(a)

ORTHANC_ASSERT(a,b)
ORTHANC_EXPAND(GET_ORTHANC_ASSERT(a, b, ORTHANC_ASSERT2, ORTHANC_ASSERT1, UNUSED)(a,b))
ORTHANC_EXPAND(ORTHANC_ASSERT2(a,b))
ORTHANC_ASSERT2(a,b)

Note: ORTHANC_EXPAND is required for some older compilers (MS v100 cl.exe )
*/






