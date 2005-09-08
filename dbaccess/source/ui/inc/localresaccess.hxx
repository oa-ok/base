/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: localresaccess.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 15:58:19 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
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

#ifndef _DBAUI_LOCALRESACCESS_HXX_
#define _DBAUI_LOCALRESACCESS_HXX_

#ifndef _DBAUI_MODULE_DBU_HXX_
#include "moduledbu.hxx"
#endif
#ifndef _SVTOOLS_LOCALRESACCESS_HXX_ 
#include <svtools/localresaccess.hxx>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................

//=========================================================================
//= OLocalResourceAccess
//=========================================================================
/** helper class for acessing local resources
*/
typedef ::svt::OLocalResourceAccess LRA_Base;
class OLocalResourceAccess : protected LRA_Base
{
public:
    inline OLocalResourceAccess( sal_uInt16 _nId, RESOURCE_TYPE _rType )
        :LRA_Base( ModuleRes( _nId ), _rType )
    {
    }
};

//.........................................................................
}	// namespace dbaui
//.........................................................................

#endif // _DBAUI_LOCALRESACCESS_HXX_

