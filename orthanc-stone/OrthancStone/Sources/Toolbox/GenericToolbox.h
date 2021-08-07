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

#include <Compatibility.h>
#include <OrthancException.h>
#include <Logging.h>

#include "LinearAlgebra.h"

#include <boost/shared_ptr.hpp>

#include <string>
#include <stdint.h>
#include <math.h>

#include <memory>
#include <vector>

namespace OrthancStone
{
  namespace GenericToolbox
  {
    /**
    Fast floating point string validation.
    No trimming applied, so the input must match regex
    /^[-]?[0-9]*\.?[0-9]*([eE][-+]?[0-9]+)?$/
    The following are allowed as edge cases: "" and "-"

    The parsing always stops if encountering either 0 or the stopChar

    */
    inline bool LegitDoubleString(const char* text, char stopChar = 0)
    {
      const char* p = text;
      if (*p == '-')
        p++;
      size_t period = 0;
      while ((*p != 0) && (*p != stopChar) && (*p != ' ') && (*p != '\t'))
      {
        if (*p >= '0' && *p <= '9')
          ++p;
        else if (*p == '.')
        {
          if (period > 0)
            return false;
          else
            period++;
          ++p;
        }
        else if (*p == 'e' || *p == 'E')
        {
          ++p;
          if (*p == '-' || *p == '+')
            ++p;
          // "e+"/"E+" "e-"/"E-" or "e"/"E" must be followed by a number
          if (!(*p >= '0' && *p <= '9'))
            return false;

          // these must be the last in the string
          while (*p >= '0' && *p <= '9')
            ++p;

          // after that, there can only be spaces
          while ((*p != 0) && (*p != stopChar))
          {
            if ((*p != ' ') && (*p != '\t'))
              return false;
            ++p;
          }

          return ((*p == 0) || (*p == stopChar));
        }
        else
        {
          return false;
        }
      }

      // we only accept trailing whitespace
      while ((*p != 0) && (*p != stopChar))
      {
        if( (*p != ' ') && (*p != '\t'))
          return false;
        ++p;
      }
      return true;
    }



    /**
    Fast integer string validation.
    No trimming applied, so the input must match regex /^-?[0-9]*$/
    The following are allowed as edge cases: "" and "-"

    The parsing always stops if encountering either 0 or the stopChar

    */
    inline bool LegitIntegerString(const char* text, char stopChar = 0)
    {
      const char* p = text;
      if (*p == '-')
        p++;
      while ((*p != 0) && (*p != stopChar))
      {
        if (*p >= '0' && *p <= '9')
          ++p;
        else
          return false;
      }
      return true;
    }


    static const double FRAC_FACTORS[] =
    {
      1.0,
      0.1,
      0.01,
      0.001,
      0.0001,
      0.00001,
      0.000001,
      0.0000001,
      0.00000001,
      0.000000001,
      0.0000000001,
      0.00000000001,
      0.000000000001,
      0.0000000000001,
      0.00000000000001,
      0.000000000000001,
      0.0000000000000001,
      0.00000000000000001,
      0.000000000000000001,
      0.0000000000000000001
    };
    static const size_t FRAC_FACTORS_LEN = sizeof(FRAC_FACTORS) / sizeof(double);

    /**
      Technical version of StringToDouble, meant for parsing bigger strings in-place.

      Only works for dot decimal numbers without digit separation

      The parsing stops when encountering EITHER \x00 or stopChar.

      Instead of filling r and returning true if number is legit, it fills r then 
      returns the number of parsed characters (NOT including the end character (which 
      can be zero, since an empty string is an allowed edge case) BUT including the trailing
      spaces), or -1 if a parsing error occurred.

      Please note that if stopChar is a number, a minus sign, the decimal separator
      or the letters e and E, the behavior is UNDEFINED!!!

      In order to allow the containing string not to be space-stripped:
      - Spaces and tabs are ignored if they occur before the scientific notation e or E letter
      - Spaces and tabs are ignored between the end of the number and the \x00 or stopChar
      - Spaces and tabs cause errors anywhere else

      It is up to the caller to detect whether a successful parsing has reached the
      terminator (\x00) or stopChar.

      In case of an error returned, in a parsing scenario where multiple numbers 
      are to be read in a bigger surrounding string, it is up to the caller to 
      recover from the error by advancing the read pointer to the next character,
      if desirable.
           
      Example: 
      ------
      const char* s = "0.0/.123/3/12.5//-43.1"; 

      int size;
      double r;
      const char* p = s;
      
      size = StringToDoubleEx(r, p, '/');
      // -->
      // r = 0 and size = 3
      
      p += size + 1; // gobble the separator
      size = StringToDoubleEx(r, p, '/');
      // --> 
      // r = 0.123 and size = 4

      p += size + 1; // gobble the separator
      size = StringToDoubleEx(r, p, '/');
      // -->
      // r = 3.0 and size = 1

      p += size + 1; // gobble the separator
      size = StringToDoubleEx(r, p, '/');
      // -->
      // r = 12.5 and size = 3

      p += size + 1; // gobble the separator
      size = StringToDoubleEx(r, p, '/');
      // -->
      // r = 0 and size = 0

      p += size + 1; // gobble the separator
      size = StringToDoubleEx(r, p, '/');
      // -->
      // r = 0 and size = 0

      p += size
      if(p == 0)
        ...stop parsing!
    */

    inline int32_t StringToDoubleEx(double& r, const char* text, char stopChar = 0)
    {
      if (!LegitDoubleString(text,stopChar))
        return -1;

      r = 0.0;
      double neg = 1.0;
      const char* p = text;

      if (*p == '-')
      {
        neg = -1.0;
        ++p;
      }
      // 12345.67890
      while (*p >= '0' && *p <= '9')
      {
        r = (r * 10.0) + (*p - '0'); // 1 12 123 123 12345
        ++p;
      }
      if (*p == '.')
      {
        double f = 0.0;
        size_t n = 1;
        ++p;
        while (*p >= '0' && *p <= '9' && n < FRAC_FACTORS_LEN)
        {
          f += (*p - '0') * FRAC_FACTORS[n];
          ++p;
          ++n;
        }
        r += f;
      }
      r *= neg;

      // skip the remaining numbers until we reach not-a-digit (either the 
      // end of the string OR the scientific notation symbol)
      // spaces are skipped in this phase here
      while ((*p >= '0' && *p <= '9') || *p == ' ' || *p == '\t')
        ++p;

      if ( (*p == 0) || (*p == stopChar))
      {
        return static_cast<int32_t>(p - text);
      }
      else if ((*p == 'e') || (*p == 'E'))
      {
        // process the scientific notation
        double sign; // no init is safe (read below)
        ++p;
        if (*p == '-')
        {
          sign = -1.0;
          // point to first number
          ++p;
        }
        else if (*p == '+')
        {
          sign = 1.0;
          // point to first number
          ++p;
        }
        else if (*p >= '0' && *p <= '9')
        {
          sign = 1.0;
        }
        else
        {
          // only a sign char or a number is allowed
          return -1;
        }
        // now p points to the absolute value of the exponent
        double exp = 0;
        while (*p >= '0' && *p <= '9')
        {
          exp = (exp * 10.0) + static_cast<double>(*p - '0'); // 1 12 123 123 12345
          ++p;
        }
        // now we have our exponent. put a sign on it.
        exp *= sign;
        double scFac = ::pow(10.0, exp);
        r *= scFac;

        // skip the trailing spaces
        while (*p == ' ' || *p == '\t')
          ++p;

        // only allowed symbol here is EOS or stopChar
        if ((*p == 0) || (*p == stopChar))
          return static_cast<int32_t>(p - text);
        else
          return -1;
      }
      else
      {
        // not allowed
        return -1;
      }
    }

    /**
      Fast string --> double conversion.
      Must pass the LegitDoubleString test

      String to doubles with at most 18 digits

      Returns true if okay and false if failed.

      The end-of-substring is character \x00
    */
    inline bool StringToDouble(double& r, const char* text)
    {
      int32_t size = StringToDoubleEx(r, text, 0);
      return (size != -1);
    }

    /**
      See main overload
    */
    inline bool StringToDouble(double& r, const std::string& text)
    {
      return StringToDouble(r, text.c_str());
    }

    /**
    Fast string to integer conversion. Leading zeroes and minus are accepted,
    but a leading + sign is NOT.
    Must pass the LegitIntegerString function test.
    In addition, an empty string (or lone minus sign) yields 0.
    */

    template<typename T>
    inline bool StringToInteger(T& r, const char* text)
    {
      if (!LegitIntegerString(text))
        return false;

      r = 0;
      T neg = 1;
      const char* p = text;

      if (*p == '-')
      {
        neg = -1;
        ++p;
      }
      while (*p >= '0' && *p <= '9')
      {
        r = (r * 10) + static_cast<T>(*p - '0'); // 1 12 123 123 12345
        ++p;
      }
      r *= neg;
      if (*p == 0)
        return true;
      else
        return false;
    }

    template<typename T>
    inline bool StringToInteger(T& r, const std::string& text)
    {
      return StringToInteger<T>(r, text.c_str());
    }

    /**
    if input is "rgb(12,23,255)"  --> function fills `red`, `green` and `blue` and returns true
    else ("everything else")      --> function returns false and leaves all values untouched
    */
    bool GetRgbValuesFromString(uint8_t& red, uint8_t& green, uint8_t& blue, const char* text);

    /**
    See main overload
    */
    inline bool GetRgbValuesFromString(uint8_t& red, uint8_t& green, uint8_t& blue, const std::string& text)
    {
      return GetRgbValuesFromString(red, green, blue, text.c_str());
    }

    /**
    Same as GetRgbValuesFromString
    */
    bool GetRgbaValuesFromString(uint8_t& red,
      uint8_t& green,
      uint8_t& blue,
      uint8_t& alpha,
      const char* text);

    /**
    Same as GetRgbValuesFromString
    */
    inline bool GetRgbaValuesFromString(uint8_t& red,
      uint8_t& green,
      uint8_t& blue,
      uint8_t& alpha,
      const std::string& text)
    {
      return GetRgbaValuesFromString(red, green, blue, alpha, text.c_str());
    }


    /**
    This method could have been called StripSpacesAndChangeToLower but we might want to
    add some UUID validation to the argument
    */
    void NormalizeUuid(std::string& uuid);


    inline void FastTokenizeString(std::vector<std::string>& result,
      const std::string& value,
      char separator)
    {
      size_t countSeparators = 0;

      for (size_t i = 0; i < value.size(); i++)
      {
        if (value[i] == separator)
        {
          countSeparators++;
        }
      }

      result.clear();
      result.reserve(countSeparators + 1);

      std::string currentItem;

      for (size_t i = 0; i < value.size(); i++)
      {
        if (value[i] == separator)
        {
          result.push_back(currentItem);
          currentItem.clear();
        }
        else
        {
          currentItem.push_back(value[i]);
        }
      }

      result.push_back(currentItem);
    }


    inline std::string FastStripSpaces(const std::string& source)
    {
      size_t first = 0;

      while (first < source.length() &&
        isspace(source[first]))
      {
        first++;
      }

      if (first == source.length())
      {
        // String containing only spaces
        return "";
      }

      size_t last = source.length();
      while (last > first &&
        isspace(source[last - 1]))
      {
        last--;
      }

      assert(first <= last);
      return source.substr(first, last - first);
    }

    /**
    Return the raw numbers of occurrences of `separator` in s (starting at s up to \x00)
    */
    inline size_t GetCharCount(const char* s, const char separator)
    {
      const char* p = s;
      size_t sepCount = 0;

      while (*p != 0)
      {
        if(*p == separator)
          sepCount++;
        ++p;
      }
      return sepCount;
    }

    inline bool FastParseVector(Vector& target, const std::string& value)
    {
      const char* s = value.c_str();
      const char SEP = '\\';

      size_t sepCount = GetCharCount(s, SEP);

      size_t itemCount = sepCount + 1;
      target.resize(itemCount);

      while (*s == ' ' || *s == '\t')
        ++s;

      const char* p = s;

      double r;
      for (size_t i = 0; i < itemCount; i++)
      {
        int32_t numberCharCount = StringToDoubleEx(r, p, SEP);
        if (numberCharCount == -1)
        {
          LOG(ERROR) << "Parsing error for vector \"" << value << "\". Current position (0-based) = " << i;
          return false;
        }
        p += numberCharCount;
        if (*p == 0)
        {
          // if we are at the end of the string, it means we have processed the last character
          // let's check this. this is a small price to pay for a useful check
          if (i != (itemCount - 1))
          {
            LOG(ERROR) << "Parsing error for vector \"" << value << "\". Reached end of the string without consuming the right # of items! Current position (0-based) = " << i;
            return false;
          }
        }
        else
        {
          if (*p != SEP)
          {
            LOG(ERROR) << "Parsing error for vector \"" << value << "\". Character past end of number Reached end of the string without consuming the right # of items! Current position (0-based) = " << i << " and r = " << r;
            return false;
          }
          if (i == (itemCount - 1))
          {
            LOG(ERROR) << "Parsing error for vector \"" << value << "\". Reached end of the vector too soon. Current position (0-based) = " << i << " and r = " << r;
            return false;
          }
          // advance to next number 
          p += 1;
        }
        target[i] = r;
      }
      return true;
    }


  }
}
