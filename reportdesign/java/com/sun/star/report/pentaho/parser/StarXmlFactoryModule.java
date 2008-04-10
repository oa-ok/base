/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: StarXmlFactoryModule.java,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/


package com.sun.star.report.pentaho.parser;

import org.jfree.xmlns.parser.XmlDocumentInfo;
import org.jfree.xmlns.parser.XmlFactoryModule;
import org.jfree.xmlns.parser.XmlReadHandler;
import com.sun.star.report.pentaho.parser.office.DocumentContentReadHandler;
import com.sun.star.report.pentaho.OfficeNamespaces;

public class StarXmlFactoryModule  implements XmlFactoryModule
{
  public StarXmlFactoryModule ()
  {
  }

  public XmlReadHandler createReadHandler (final XmlDocumentInfo documentInfo)
  {
    return new DocumentContentReadHandler();
  }

  public int getDocumentSupport (final XmlDocumentInfo documentInfo)
  {
    final String rootNamespace = documentInfo.getRootElementNameSpace();
    if (OfficeNamespaces.OFFICE_NS.equals(rootNamespace) == false)
    {
      return XmlFactoryModule.NOT_RECOGNIZED;
    }

    if ("document-content".equals(documentInfo.getRootElement()))
    {
      return XmlFactoryModule.RECOGNIZED_BY_NAMESPACE;
    }
    if ("document".equals(documentInfo.getRootElement()))
    {
      return XmlFactoryModule.RECOGNIZED_BY_NAMESPACE;
    }
    return XmlFactoryModule.NOT_RECOGNIZED;
  }

  public String getDefaultNamespace(final XmlDocumentInfo documentInfo)
  {
    return null;
  }
}
