/* Begin CVS Header
$Source: /fs/turing/cvs/copasi_dev/copasi/utilities/utility.cpp,v $
$Revision: 1.37.2.7 $
$Name: Build-33 $
$Author: shoops $
$Date: 2011/03/31 20:03:36 $
End CVS Header */

// Copyright (C) 2008 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc., EML Research, gGmbH, University of Heidelberg,
// and The University of Manchester.
// All rights reserved.

// Copyright (C) 2001 - 2007 by Pedro Mendes, Virginia Tech Intellectual
// Properties, Inc. and EML Research, gGmbH.
// All rights reserved.

#include "mathematics.h"
#include <stdio.h>
#include <time.h>

#include <limits>
#include <string>
#include <stdlib.h>
// for strcmp
#include <string.h>

#include <sstream>

#include "copasi.h"

#include "utility.h"

std::string ISODateTime(tm * pTime)
{
  char str[20];

  if (pTime)
    {
      sprintf(str, "%d-%.02d-%.02d %.02d:%.02d:%.02d",
              pTime->tm_year + 1900,
              pTime->tm_mon + 1,
              pTime->tm_mday,
              pTime->tm_hour,
              pTime->tm_min,
              pTime->tm_sec);
    }
  else
    {
      sprintf(str, "0000-00-00 00:00:00");
    }

  return (std::string) str;
}

std::string LocalTimeStamp()
{
  time_t Time;
  time(&Time);

  tm *sTime = NULL;
  sTime = localtime(&Time);

  return ISODateTime(sTime);
}

std::string UTCTimeStamp()
{
  time_t Time;
  time(&Time);

  tm *sTime = NULL;
  sTime = gmtime(&Time);

  return ISODateTime(sTime);
}

bool isNumber(const std::string & str)
{
  if (str.find_first_of("+-.0123456789")) return false;

  const char * Tail;
  strToDouble(str.c_str(), & Tail);

  if (*Tail) return false;

  return true;
}

std::string StringPrint(const char * format, ...)
{
  C_INT32 TextSize = INITIALTEXTSIZE;
  C_INT32 Printed = 0;

  char *Text = NULL;

  Text = new char[TextSize + 1];

  va_list Arguments; // = NULL;
  va_start(Arguments, format);
  Printed = vsnprintf(Text, TextSize + 1, format, Arguments);
  va_end(Arguments);

  while (Printed < 0 || TextSize < Printed)
    {
      delete [] Text;

      (Printed < 0) ? TextSize *= 2 : TextSize = Printed;
      Text = new char[TextSize + 1];

      va_list Arguments; // = NULL;
      va_start(Arguments, format);
      Printed = vsnprintf(Text, TextSize + 1, format, Arguments);
      va_end(Arguments);
    }

  std::string Result = Text;

  delete [] Text;
  return Result;
}

std::string unQuote(const std::string & name)
{
  std::string Name = name;
  std::string::size_type len = Name.length();

  if (len > 1 && Name[0] == '"' && Name[len - 1] == '"')
    {
      // Remove surrounding double quotes.
      Name = Name.substr(1, len - 2);

      // Remove escape sequences.
      std::string::size_type pos = Name.find("\\");

      while (pos != std::string::npos)
        {
          Name.erase(pos, 1);
          pos++;
          pos = Name.find("\\", pos);
        }
    }

  return Name;
}

std::string quote(const std::string & name,
                  const std::string & additionalEscapes)
{
  if (name.find_first_of(" \"" + additionalEscapes) == std::string::npos)
    return name;

#define toBeEscaped "\\\""
  std::string Escaped(name);
  std::string::size_type pos = Escaped.find_first_of(toBeEscaped);

  while (pos != std::string::npos)
    {
      Escaped.insert(pos, "\\");
      pos += 2;
      pos = Escaped.find_first_of(toBeEscaped, pos);
    }

  return "\"" + Escaped + "\"";
#undef toBeEscaped
}

/*
 * Fixes a string to be a SName element from SBML
 * (this is a destructive function, some changes are irreversible)
 *
 */

void FixSName(const std::string &original, std::string &fixed)
{
  size_t i, len;

  // check reserved names
  if (original == "abs") {fixed = "_abs"; return;}

  if (original == "acos") {fixed = "_acos"; return;}

  if (original == "and") {fixed = "_and"; return;}

  if (original == "asin") {fixed = "_asin"; return;}

  if (original == "atan") {fixed = "_atan"; return;}

  if (original == "ceil") {fixed = "_ceil"; return;}

  if (original == "cos") {fixed = "_cos"; return;}

  if (original == "exp") {fixed = "_exp"; return;}

  if (original == "floor") {fixed = "_floor"; return;}

  if (original == "hilli") {fixed = "_hilli"; return;}

  if (original == "hillmmr") {fixed = "_hillmmr"; return;}

  if (original == "hillmr") {fixed = "_hillmr"; return;}

  if (original == "hillr") {fixed = "_hillr"; return;}

  if (original == "isouur") {fixed = "_isouur"; return;}

  if (original == "log") {fixed = "_log"; return;}

  if (original == "log10") {fixed = "_log10"; return;}

  if (original == "massi") {fixed = "_massi"; return;}

  if (original == "massr") {fixed = "_massr"; return;}

  if (original == "not") {fixed = "_not"; return;}

  if (original == "or") {fixed = "_or"; return;}

  if (original == "ordbbr") {fixed = "_ordbbr"; return;}

  if (original == "ordbur") {fixed = "_ordbur"; return;}

  if (original == "ordubr") {fixed = "_ordubr"; return;}

  if (original == "pow") {fixed = "_pow"; return;}

  if (original == "ppbr") {fixed = "_ppbr"; return;}

  if (original == "sin") {fixed = "_sin"; return;}

  if (original == "sqr") {fixed = "_sqr"; return;}

  if (original == "sqrt") {fixed = "_sqrt"; return;}

  if (original == "substance") {fixed = "_substance"; return;}

  if (original == "time") {fixed = "_time"; return;}

  if (original == "tan") {fixed = "_tan"; return;}

  if (original == "umai") {fixed = "_umai"; return;}

  if (original == "umar") {fixed = "_umar"; return;}

  if (original == "uai") {fixed = "_uai"; return;}

  if (original == "ualii") {fixed = "_ualii"; return;}

  if (original == "uar") {fixed = "_uar"; return;}

  if (original == "ucii") {fixed = "_ucii"; return;}

  if (original == "ucir") {fixed = "_ucir"; return;}

  if (original == "ucti") {fixed = "_ucti"; return;}

  if (original == "uctr") {fixed = "_uctr"; return;}

  if (original == "uhmi") {fixed = "_uhmi"; return;}

  if (original == "uhmr") {fixed = "_uhmr"; return;}

  if (original == "umi") {fixed = "_umi"; return;}

  if (original == "unii") {fixed = "_unii"; return;}

  if (original == "unir") {fixed = "_unir"; return;}

  if (original == "uuhr") {fixed = "_uuhr"; return;}

  if (original == "umr") {fixed = "_umr"; return;}

  if (original == "usii") {fixed = "_usii"; return;}

  if (original == "usir") {fixed = "_usir"; return;}

  if (original == "uuci") {fixed = "_uuci"; return;}

  if (original == "uucr") {fixed = "_uucr"; return;}

  if (original == "uui") {fixed = "_uui"; return;}

  if (original == "uur") {fixed = "_uur"; return;}

  if (original == "volume") {fixed = "_volume"; return;}

  if (original == "xor") {fixed = "_xor"; return;}

  len = original.length();

  // check rule for initial characters
  // if first not a letter...
  if (((original[0] < 'A') || (original[0] > 'z')) && (original[0] != '_'))
    {
      if ((original[0] >= '0') && (original[0] <= '9'))
        fixed = "_" + original;
      else
        {fixed = original; fixed [0] = '_';}
    }
  else
    fixed = original;

  len = fixed.length();

  for (i = 1; i < len; i++)
    if ((fixed [i] != '_') && ((fixed [i] < 'A') || (fixed [i] > 'z')) &&
        ((fixed [i] < '0') || (fixed [i] > '9')))
      fixed [i] = '_';
}

/*
 * Fixes a string to a XHTML valid equivalent
 */
void FixXHTML(const std::string &original, std::string &fixed)
{
  size_t i, p, len;
  std::string Str;
  // find the next illegal character
  Str = original;
  fixed.erase();

  for (i = 0; i != std::string::npos;)
    {
      p = Str.find_first_of("&><\"�����������������������������������������������������������������������������������������������");
      fixed += Str.substr(0, p)
               ;
      len = Str.length();
      i = Str.find_first_of("&><\"�����������������������������������������������������������������������������������������������");

      if (i != std::string::npos)
        {
          switch (Str[i])
            {
              case '&':
                fixed += "&amp; ";
                break;
              case '>':
                fixed += "&gt; ";
                break;
              case '<':
                fixed += "&lt; ";
                break;
              case '"':
                fixed += "&quot; ";
                break;
              case '�':
                fixed += "&#161; ";
                break;
              case '�':
                fixed += "&#162; ";
                break;
              case '�':
                fixed += "&#163; ";
                break;
              case '�':
                fixed += "&#164; ";
                break;
              case '�':
                fixed += "&#165; ";
                break;
              case '�':
                fixed += "&#166; ";
                break;
              case '�':
                fixed += "&#167; ";
                break;
              case '�':
                fixed += "&#168; ";
                break;
              case '�':
                fixed += "&#169; ";
                break;
              case '�':
                fixed += "&#170; ";
                break;
              case '�':
                fixed += "&#171; ";
                break;
              case '�':
                fixed += "&#172; ";
                break;
              case '�':
                fixed += "&#173; ";
                break;
              case '�':
                fixed += "&#174; ";
                break;
              case '�':
                fixed += "&#175; ";
                break;
              case '�':
                fixed += "&#176; ";
                break;
              case '�':
                fixed += "&#177; ";
                break;
              case '�':
                fixed += "&#178; ";
                break;
              case '�':
                fixed += "&#179; ";
                break;
              case '�':
                fixed += "&#180; ";
                break;
              case '�':
                fixed += "&#181; ";
                break;
              case '�':
                fixed += "&#182; ";
                break;
              case '�':
                fixed += "&#183; ";
                break;
              case '�':
                fixed += "&#184; ";
                break;
              case '�':
                fixed += "&#185; ";
                break;
              case '�':
                fixed += "&#186; ";
                break;
              case '�':
                fixed += "&#187; ";
                break;
              case '�':
                fixed += "&#188; ";
                break;
              case '�':
                fixed += "&#189; ";
                break;
              case '�':
                fixed += "&#190; ";
                break;
              case '�':
                fixed += "&#191; ";
                break;
              case '�':
                fixed += "&#192; ";
                break;
              case '�':
                fixed += "&#193; ";
                break;
              case '�':
                fixed += "&#194; ";
                break;
              case '�':
                fixed += "&#195; ";
                break;
              case '�':
                fixed += "&#196; ";
                break;
              case '�':
                fixed += "&#197; ";
                break;
              case '�':
                fixed += "&#198; ";
                break;
              case '�':
                fixed += "&#199; ";
                break;
              case '�':
                fixed += "&#200; ";
                break;
              case '�':
                fixed += "&#201; ";
                break;
              case '�':
                fixed += "&#202; ";
                break;
              case '�':
                fixed += "&#203; ";
                break;
              case '�':
                fixed += "&#204; ";
                break;
              case '�':
                fixed += "&#205; ";
                break;
              case '�':
                fixed += "&#206; ";
                break;
              case '�':
                fixed += "&#207; ";
                break;
              case '�':
                fixed += "&#208; ";
                break;
              case '�':
                fixed += "&#209; ";
                break;
              case '�':
                fixed += "&#210; ";
                break;
              case '�':
                fixed += "&#211; ";
                break;
              case '�':
                fixed += "&#212; ";
                break;
              case '�':
                fixed += "&#213; ";
                break;
              case '�':
                fixed += "&#214; ";
                break;
              case '�':
                fixed += "&#215; ";
                break;
              case '�':
                fixed += "&#216; ";
                break;
              case '�':
                fixed += "&#217; ";
                break;
              case '�':
                fixed += "&#218; ";
                break;
              case '�':
                fixed += "&#219; ";
                break;
              case '�':
                fixed += "&#220; ";
                break;
              case '�':
                fixed += "&#221; ";
                break;
              case '�':
                fixed += "&#222; ";
                break;
              case '�':
                fixed += "&#223; ";
                break;
              case '�':
                fixed += "&#224; ";
                break;
              case '�':
                fixed += "&#225; ";
                break;
              case '�':
                fixed += "&#226; ";
                break;
              case '�':
                fixed += "&#227; ";
                break;
              case '�':
                fixed += "&#228; ";
                break;
              case '�':
                fixed += "&#229; ";
                break;
              case '�':
                fixed += "&#230; ";
                break;
              case '�':
                fixed += "&#231; ";
                break;
              case '�':
                fixed += "&#232; ";
                break;
              case '�':
                fixed += "&#233; ";
                break;
              case '�':
                fixed += "&#234; ";
                break;
              case '�':
                fixed += "&#235; ";
                break;
              case '�':
                fixed += "&#236; ";
                break;
              case '�':
                fixed += "&#237; ";
                break;
              case '�':
                fixed += "&#238; ";
                break;
              case '�':
                fixed += "&#239; ";
                break;
              case '�':
                fixed += "&#240; ";
                break;
              case '�':
                fixed += "&#241; ";
                break;
              case '�':
                fixed += "&#242; ";
                break;
              case '�':
                fixed += "&#243; ";
                break;
              case '�':
                fixed += "&#244; ";
                break;
              case '�':
                fixed += "&#245; ";
                break;
              case '�':
                fixed += "&#246; ";
                break;
              case '�':
                fixed += "&#247; ";
                break;
              case '�':
                fixed += "&#248; ";
                break;
              case '�':
                fixed += "&#249; ";
                break;
              case '�':
                fixed += "&#250; ";
                break;
              case '�':
                fixed += "&#251; ";
                break;
              case '�':
                fixed += "&#252; ";
                break;
              case '�':
                fixed += "&#253; ";
                break;
              case '�':
                fixed += "&#254; ";
                break;
              case '�':
                fixed += "&#255; ";
                break;
            }
        }

      Str = Str.substr(len - i - 1);
    }
}

double strToDouble(const char * str,
                   char const ** pTail)
{
  double Value = std::numeric_limits< C_FLOAT64 >::quiet_NaN();

  if (pTail != NULL)
    {
      *pTail = str;
    }

  if (str == NULL || *str == 0x0)
    {
      return Value;
    }

  std::istringstream in;

  in.imbue(std::locale::classic());
  in.str(str);

  in >> Value;

  if (pTail != NULL && !isnan(Value))
    {
      *pTail = str + std::min< size_t >(in.tellg(), strlen(str));
    }

  return Value;
}

C_INT32 strToInt(const char * str,
                 char const ** pTail)
{
  C_INT32 Value = std::numeric_limits< C_INT32 >::quiet_NaN();

  if (pTail != NULL)
    {
      *pTail = str;
    }

  if (str == NULL || *str == 0x0)
    {
      return Value;
    }

  std::istringstream in;

  in.imbue(std::locale::classic());
  in.str(str);

  in >> Value;

  if (pTail != NULL && !isnan(Value))
    {
      *pTail = str + std::min< size_t >(in.tellg(), strlen(str));
    }

  return Value;
}

unsigned C_INT32 strToUnsignedInt(const char * str,
                                  char const ** pTail)
{
  unsigned C_INT32 Value = std::numeric_limits< unsigned C_INT32 >::quiet_NaN();

  if (pTail != NULL)
    {
      *pTail = str;
    }

  if (str == NULL || *str == 0x0)
    {
      return Value;
    }

  std::istringstream in;

  in.imbue(std::locale::classic());
  in.str(str);

  in >> Value;

  if (pTail != NULL && !isnan(Value))
    {
      *pTail = str + std::min< size_t >(in.tellg(), strlen(str));
    }

  return Value;
}

