/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ModuleHelper.cxx,v $
 * $Revision: 1.4 $
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
#include "ModuleHelper.hxx"
#include <comphelper/configurationhelper.hxx>
#include <comphelper/processfactory.hxx>
#include <osl/thread.h>
#include <com/sun/star/util/XMacroExpander.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <rtl/uri.hxx>
#include <tools/debug.hxx>
#ifndef _SOLAR_HRC
#include <svl/solar.hrc>
#endif

#define EXPAND_PROTOCOL     "vnd.sun.star.expand:"
#define ENTER_MOD_METHOD()	\
    ::osl::MutexGuard aGuard(s_aMutex);	\
    ensureImpl()

//.........................................................................
namespace rptui
{
//.........................................................................
    using namespace ::com::sun::star;
//=========================================================================
//= OModuleImpl
//=========================================================================
/** implementation for <type>OModule</type>. not threadsafe, has to be guarded by it's owner
*/
class OModuleImpl
{
    ResMgr*	m_pRessources;

public:
    /// ctor
    OModuleImpl();
    ~OModuleImpl();

    /// get the manager for the ressources of the module
    ResMgr*	getResManager();
};

DBG_NAME( rpt_OModuleImpl )
//-------------------------------------------------------------------------
OModuleImpl::OModuleImpl()
    :m_pRessources(NULL)
{
    DBG_CTOR( rpt_OModuleImpl,NULL);

}

//-------------------------------------------------------------------------
OModuleImpl::~OModuleImpl()
{
    if (m_pRessources)
        delete m_pRessources;

    DBG_DTOR( rpt_OModuleImpl,NULL);
}

//-------------------------------------------------------------------------
ResMgr*	OModuleImpl::getResManager()
{
    // note that this method is not threadsafe, which counts for the whole class !

    if (!m_pRessources)
    {
        // create a manager with a fixed prefix
        rtl::OString sName = rtl::OString( "rptui" );
        m_pRessources = ResMgr::CreateResMgr(sName);
    }
    return m_pRessources;
}

//=========================================================================
//= OModule
//=========================================================================
::osl::Mutex	OModule::s_aMutex;
sal_Int32		OModule::s_nClients = 0;
OModuleImpl*	OModule::s_pImpl = NULL;
//-------------------------------------------------------------------------
ResMgr*	OModule::getResManager()
{
    ENTER_MOD_METHOD();
    return s_pImpl->getResManager();
}

//-------------------------------------------------------------------------
void OModule::registerClient()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    ++s_nClients;
}

//-------------------------------------------------------------------------
void OModule::revokeClient()
{
    ::osl::MutexGuard aGuard(s_aMutex);
    if (!--s_nClients && s_pImpl)
    {
        delete s_pImpl;
        s_pImpl = NULL;
    }
}

//-------------------------------------------------------------------------
void OModule::ensureImpl()
{
    if (s_pImpl)
        return;
    s_pImpl = new OModuleImpl();
}

//.........................................................................
}	// namespace dbaui
//.........................................................................
