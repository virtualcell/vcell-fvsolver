/**
 * @file    XercesNamespaces.h
 * @brief   Extracts XML namespace declarations from Xerces-C++ attributes.
 * @author  Ben Bornstein
 *
 * $Id: XercesNamespaces.h 12780 2011-02-08 04:12:54Z mhucka $
 * $HeadURL: http://sbml.svn.sourceforge.net/svnroot/sbml/trunk/libsbml/src/xml/XercesNamespaces.h $
 *
 * <!--------------------------------------------------------------------------
 * This file is part of libSBML.  Please visit http://sbml.org for more
 * information about SBML, and the latest version of libSBML.
 *
 * Copyright (C) 2009-2011 jointly by the following organizations: 
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. EMBL European Bioinformatics Institute (EBML-EBI), Hinxton, UK
 *  
 * Copyright (C) 2006-2008 by the California Institute of Technology,
 *     Pasadena, CA, USA 
 *  
 * Copyright (C) 2002-2005 jointly by the following organizations: 
 *     1. California Institute of Technology, Pasadena, CA, USA
 *     2. Japan Science and Technology Agency, Japan
 * 
 * This library is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation.  A copy of the license agreement is provided
 * in the file named "LICENSE.txt" included with this software distribution and
 * also available online as http://sbml.org/software/libsbml/license.html
 * ---------------------------------------------------------------------- -->*/

#ifndef XercesNamespaces_h
#define XercesNamespaces_h

#ifdef __cplusplus

#include <string>

#include <xercesc/sax2/Attributes.hpp>
#include <sbml/xml/XMLNamespaces.h>

LIBSBML_CPP_NAMESPACE_BEGIN

/** @cond doxygen-libsbml-internal */

class XercesNamespaces : public XMLNamespaces
{
public:

  /**
   * Creates a new list of XML namespaces declarations from a "raw" Xerces-C++
   * Attributes set.
   */
  XercesNamespaces (const xercesc::Attributes& attrs);


  /**
   * Destroys this list of XML namespace declarations.
   */
  virtual ~XercesNamespaces ();
};

/** @endcond */

LIBSBML_CPP_NAMESPACE_END

#endif  /* __cplusplus */
#endif  /* XercesNamespaces_h */