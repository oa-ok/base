/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DefaultParameterMap.java,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:13 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2007 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *    Copyright 2007 by Pentaho Corporation
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/


package com.sun.star.report.util;

import java.util.HashMap;

import com.sun.star.report.ParameterMap;

public class DefaultParameterMap implements ParameterMap
{
  private HashMap backend;

  public DefaultParameterMap ()
  {
    backend = new HashMap();
  }

  public void clear ()
  {
    backend.clear();
  }

  /**
   * Retrieves the value stored for a key in this properties collection.
   *
   * @param key the property key.
   * @return The stored value, or <code>null</code> if the key does not exist in this
   *         collection.
   */
  public Object get (final String key)
  {
    if (key == null)
    {
      throw new NullPointerException
              ("DefaultParameterMap.get (..): Parameter 'key' must not be null");
    }
    return backend.get(key);
  }

  /**
   * Retrieves the value stored for a key in this properties collection, and returning the
   * default value if the key was not stored in this properties collection.
   *
   * @param key          the property key.
   * @param defaultValue the default value to be returned when the key is not stored in
   *                     this properties collection.
   * @return The stored value, or the default value if the key does not exist in this
   *         collection.
   */
  public Object get (final String key, final Object defaultValue)
  {
    if (key == null)
    {
      throw new NullPointerException
              ("DefaultParameterMap.get (..): Parameter 'key' must not be null");
    }
    final Object o = this.backend.get(key);
    if (o == null)
    {
      return defaultValue;
    }
    return o;
  }

  public String[] keys ()
  {
    return (String[]) this.backend.keySet().toArray(new String[backend.size()]);
  }

  /**
   * Adds a property to this properties collection. If a property with the given name
   * exist, the property will be replaced with the new value. If the value is null, the
   * property will be removed.
   *
   * @param key   the property key.
   * @param value the property value.
   */
  public void put (final String key, final Object value)
  {
    if (key == null)
    {
      throw new NullPointerException
              ("ReportProperties.put (..): Parameter 'key' must not be null");
    }
    if (value == null)
    {
      this.backend.remove(key);
    }
    else
    {
      this.backend.put(key, value);
    }
  }

  public int size ()
  {
    return this.backend.size();
  }
}
