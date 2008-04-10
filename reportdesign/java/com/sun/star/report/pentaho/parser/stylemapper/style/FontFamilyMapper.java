/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: FontFamilyMapper.java,v $
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


package com.sun.star.report.pentaho.parser.stylemapper.style;

import com.sun.star.report.pentaho.parser.StyleMapper;
import org.jfree.layouting.input.style.CSSDeclarationRule;
import org.jfree.layouting.input.style.values.CSSStringValue;
import org.jfree.layouting.input.style.values.CSSStringType;
import org.jfree.layouting.input.style.values.CSSValue;
import org.jfree.layouting.input.style.values.CSSValueList;
import org.jfree.layouting.input.style.keys.font.FontStyleKeys;

public class FontFamilyMapper implements StyleMapper
{
  public FontFamilyMapper ()
  {
  }

  public void updateStyle (String uri, String attrName, String attrValue,
                           CSSDeclarationRule targetRule)
  {
    final CSSStringValue cssVal = new CSSStringValue(CSSStringType.STRING, attrValue);

    final CSSValue value = targetRule.getPropertyCSSValue(FontStyleKeys.FONT_FAMILY);
    if (value instanceof CSSValueList == false)
    {
      targetRule.setPropertyValue(FontStyleKeys.FONT_FAMILY,
            new CSSValueList(new CSSValue[]{ cssVal }));
    }
    else
    {
      CSSValueList list = (CSSValueList) value;
      targetRule.setPropertyValue(FontStyleKeys.FONT_FAMILY,
            CSSValueList.insertFirst(list, value));
    }
  }
}
