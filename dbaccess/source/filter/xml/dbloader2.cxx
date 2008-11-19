/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbloader2.cxx,v $
 * $Revision: 1.32.28.13 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_dbaccess.hxx"

#include "flt_reghelper.hxx"
#include "xmlstrings.hrc"

/** === begin UNO includes === **/
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XEventListener.hpp>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/document/XFilter.hpp>
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/frame/XFrameLoader.hpp>
#include <com/sun/star/frame/XFramesSupplier.hpp>
#include <com/sun/star/frame/XLoadEventListener.hpp>
#include <com/sun/star/frame/XModel2.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <com/sun/star/registry/XRegistryKey.hpp>
#include <com/sun/star/sdb/XDocumentDataSource.hpp>
#include <com/sun/star/task/XJobExecutor.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <com/sun/star/sdb/application/DatabaseObjectContainer.hpp>
#include <com/sun/star/sdb/application/NamedDatabaseObject.hpp>
#include <com/sun/star/frame/XLoadable.hpp>
/** === end UNO includes === **/

#include <comphelper/componentcontext.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/namedvaluecollection.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/sequenceashashmap.hxx>
#include <comphelper/stl_types.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/types.hxx>
#include <cppuhelper/implbase2.hxx>
#include <osl/file.hxx>
#include <sfx2/docfile.hxx>
#include <svtools/moduleoptions.hxx>
#include <toolkit/awt/vclxwindow.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <tools/diagnose_ex.h>
#include <tools/urlobj.hxx>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <ucbhelper/contentbroker.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>

using namespace ::ucbhelper;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::ucb;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::embed;
namespace css = ::com::sun::star;
using namespace ::com::sun::star::ui::dialogs;
using ::com::sun::star::awt::XWindow;
using ::com::sun::star::sdb::application::NamedDatabaseObject;

// -------------------------------------------------------------------------
namespace dbaxml
{

class DBTypeDetection : public ::cppu::WeakImplHelper2< XExtendedFilterDetection, XServiceInfo>
{
    ::comphelper::ComponentContext  m_aContext;

public:
    DBTypeDetection(const Reference< XMultiServiceFactory >&);

    // XServiceInfo
    ::rtl::OUString					SAL_CALL getImplementationName() throw(  );
    sal_Bool 						SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(  );
    Sequence< ::rtl::OUString > 	SAL_CALL getSupportedServiceNames(void) throw(  );

    // static methods
    static ::rtl::OUString 			getImplementationName_Static() throw(  )
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbflt.DBTypeDetection");
    }
    static Sequence< ::rtl::OUString> getSupportedServiceNames_Static(void) throw(  );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    virtual ::rtl::OUString SAL_CALL detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (::com::sun::star::uno::RuntimeException);
};
// -------------------------------------------------------------------------
DBTypeDetection::DBTypeDetection(const Reference< XMultiServiceFactory >& _rxFactory)
    :m_aContext( _rxFactory )
{
}
// -------------------------------------------------------------------------
::rtl::OUString SAL_CALL DBTypeDetection::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& Descriptor ) throw (::com::sun::star::uno::RuntimeException)
{
    try
    {
        ::comphelper::NamedValueCollection aMedia( Descriptor );
        sal_Bool bStreamFromDescr = sal_False;
        ::rtl::OUString sURL = aMedia.getOrDefault( "URL", ::rtl::OUString() );

        Reference< XInputStream > xInStream( aMedia.getOrDefault( "InputStream",  Reference< XInputStream >() ) );
        Reference< XPropertySet > xStorageProperties;
        if ( xInStream.is() )
        {
            bStreamFromDescr = sal_True;
            xStorageProperties.set( ::comphelper::OStorageHelper::GetStorageFromInputStream(
                xInStream, m_aContext.getLegacyServiceFactory() ), UNO_QUERY );
        }
        else
        {
            ::rtl::OUString sSalvagedURL( aMedia.getOrDefault( "SalvagedFile", ::rtl::OUString() ) );

            ::rtl::OUString sFileLocation( sSalvagedURL.getLength() ? sSalvagedURL : sURL );
            if ( sFileLocation.getLength() )
            {
                xStorageProperties.set( ::comphelper::OStorageHelper::GetStorageFromURL(
                    sFileLocation, ElementModes::READ, m_aContext.getLegacyServiceFactory() ), UNO_QUERY );
            }
        }

        if ( xStorageProperties.is() )
        {
            ::rtl::OUString sMediaType;
            xStorageProperties->getPropertyValue( INFO_MEDIATYPE ) >>= sMediaType;
            if ( sMediaType.equalsAscii(MIMETYPE_OASIS_OPENDOCUMENT_DATABASE_ASCII) || sMediaType.equalsAscii(MIMETYPE_VND_SUN_XML_BASE_ASCII) )
            {
                if ( bStreamFromDescr && sURL.compareTo( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:stream" ) ), 14 ) != COMPARE_EQUAL )
                {
                    // After fixing of the i88522 issue ( use the new file locking for database files ) the stream from the type detection can be used further
                    // for now the file should be reopened to have read/write access
                    aMedia.remove( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "InputStream" ) ) );
                    aMedia >>= Descriptor;
                    try
                    {
                        if ( xInStream.is() )
                            xInStream->closeInput();
                    }
                    catch( Exception& )
                    {
                        DBG_UNHANDLED_EXCEPTION();
                    }
                }

                return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StarBase"));
            }
            ::comphelper::disposeComponent(xStorageProperties);
        }
    } catch(Exception&){}
    return ::rtl::OUString();
}
// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL DBTypeDetection::Create( const Reference< XMultiServiceFactory >  & rSMgr )
{
    return *(new DBTypeDetection(rSMgr));
}
// -------------------------------------------------------------------------
// XServiceInfo
::rtl::OUString SAL_CALL DBTypeDetection::getImplementationName() throw(  )
{
    return getImplementationName_Static();
}
// -------------------------------------------------------------------------

// XServiceInfo
sal_Bool SAL_CALL DBTypeDetection::supportsService(const ::rtl::OUString& ServiceName) throw(  )
{
    Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString * pBegin	= aSNL.getConstArray();
    const ::rtl::OUString * pEnd	= pBegin + aSNL.getLength();
    for( ; pBegin != pEnd; ++pBegin)
        if( *pBegin == ServiceName )
            return sal_True;
    return sal_False;
}
// -------------------------------------------------------------------------
// XServiceInfo
Sequence< ::rtl::OUString > SAL_CALL DBTypeDetection::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
// ORegistryServiceManager_Static
Sequence< ::rtl::OUString > DBTypeDetection::getSupportedServiceNames_Static(void) throw(  )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.document.ExtendedTypeDetection");
    return aSNS;
}
// -------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_DBTypeDetection()
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::DBTypeDetection > aAutoRegistration;
}
// -----------------------------------------------------------------------------

class DBContentLoader : public ::cppu::WeakImplHelper2< XFrameLoader, XServiceInfo>
{
private:
    ::comphelper::ComponentContext      m_aContext;
    Reference< XFrameLoader >			m_xMySelf;
    ::rtl::OUString						m_sCurrentURL;
    ULONG								m_nStartWizard;

    DECL_LINK( OnStartTableWizard, void* );
public:
    DBContentLoader(const Reference< XMultiServiceFactory >&);
    ~DBContentLoader();

    // XServiceInfo
    ::rtl::OUString					SAL_CALL getImplementationName() throw(  );
    sal_Bool 						SAL_CALL supportsService(const ::rtl::OUString& ServiceName) throw(  );
    Sequence< ::rtl::OUString > 	SAL_CALL getSupportedServiceNames(void) throw(  );

    // static methods
    static ::rtl::OUString 			getImplementationName_Static() throw(  )
    {
        return ::rtl::OUString::createFromAscii("org.openoffice.comp.dbflt.DBContentLoader2");
    }
    static Sequence< ::rtl::OUString> getSupportedServiceNames_Static(void) throw(  );
    static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
            SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

    // XLoader
    virtual void SAL_CALL load(	const Reference< XFrame > & _rFrame, const ::rtl::OUString& _rURL,
                                const Sequence< PropertyValue >& _rArgs,
                                const Reference< XLoadEventListener > & _rListener) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL cancel(void) throw();

private:
    sal_Bool impl_executeNewDatabaseWizard( Reference< XModel >& _rxModel, sal_Bool& _bShouldStartTableWizard );
};
DBG_NAME(DBContentLoader)

DBContentLoader::DBContentLoader(const Reference< XMultiServiceFactory >& _rxFactory)
    :m_aContext( _rxFactory )
    ,m_nStartWizard(0)
{
    DBG_CTOR(DBContentLoader,NULL);

}
// -------------------------------------------------------------------------

DBContentLoader::~DBContentLoader()
{

    DBG_DTOR(DBContentLoader,NULL);
}
// -------------------------------------------------------------------------

// -------------------------------------------------------------------------
Reference< XInterface > SAL_CALL DBContentLoader::Create( const Reference< XMultiServiceFactory >  & rSMgr )
{
    return *(new DBContentLoader(rSMgr));
}
// -------------------------------------------------------------------------
// XServiceInfo
::rtl::OUString SAL_CALL DBContentLoader::getImplementationName() throw(  )
{
    return getImplementationName_Static();
}
// -------------------------------------------------------------------------

// XServiceInfo
sal_Bool SAL_CALL DBContentLoader::supportsService(const ::rtl::OUString& ServiceName) throw(  )
{
    Sequence< ::rtl::OUString > aSNL = getSupportedServiceNames();
    const ::rtl::OUString * pBegin	= aSNL.getConstArray();
    const ::rtl::OUString * pEnd	= pBegin + aSNL.getLength();
    for( ; pBegin != pEnd; ++pBegin)
        if( *pBegin == ServiceName )
            return sal_True;
    return sal_False;
}
// -------------------------------------------------------------------------
// XServiceInfo
Sequence< ::rtl::OUString > SAL_CALL DBContentLoader::getSupportedServiceNames(void) throw(  )
{
    return getSupportedServiceNames_Static();
}
// -------------------------------------------------------------------------
// ORegistryServiceManager_Static
Sequence< ::rtl::OUString > DBContentLoader::getSupportedServiceNames_Static(void) throw(  )
{
    Sequence< ::rtl::OUString > aSNS( 1 );
    aSNS.getArray()[0] = ::rtl::OUString::createFromAscii("com.sun.star.frame.FrameLoader");
    return aSNS;
}

// -----------------------------------------------------------------------
namespace
{
    // ...................................................................
    sal_Bool lcl_urlAllowsInteraction( const ::comphelper::ComponentContext& _rContext, const ::rtl::OUString& _rURL )
    {
        bool bDoesAllow = sal_False;
        try
        {
            Reference< XURLTransformer > xTransformer;
            if ( _rContext.createComponent( "com.sun.star.util.URLTransformer", xTransformer ) )
            {
                URL aURL;
                aURL.Complete = _rURL;
                xTransformer->parseStrict( aURL );
                bDoesAllow = aURL.Arguments.equalsAscii( "Interactive" );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "lcl_urlAllowsInteraction: caught an exception while analyzing the URL!" );
        }
        return bDoesAllow;
    }

    // ...................................................................
    Reference< XWindow > lcl_getTopMostWindow( const ::comphelper::ComponentContext& _rContext )
    {
        Reference< XWindow > xWindow;
        // get the top most window
        Reference < XFramesSupplier > xDesktop;
        if ( _rContext.createComponent( "com.sun.star.frame.Desktop", xDesktop ) )
        {
            Reference < XFrame > xActiveFrame = xDesktop->getActiveFrame();
            if ( xActiveFrame.is() )
            {
                xWindow = xActiveFrame->getContainerWindow();
                Reference<XFrame> xFrame = xActiveFrame;
                while ( xFrame.is() && !xFrame->isTop() )
                    xFrame.set(xFrame->getCreator(),UNO_QUERY);

                if ( xFrame.is() )
                    xWindow = xFrame->getContainerWindow();
            }
        }
        return xWindow;
    }
}

// -----------------------------------------------------------------------
sal_Bool DBContentLoader::impl_executeNewDatabaseWizard( Reference< XModel >& _rxModel, sal_Bool& _bShouldStartTableWizard )
{
    Sequence< Any > aWizardArgs(2);
    aWizardArgs[0] <<= PropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ParentWindow")),
                    0,
                    makeAny( lcl_getTopMostWindow( m_aContext ) ),
                    PropertyState_DIRECT_VALUE);

    aWizardArgs[1] <<= PropertyValue(
                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InitialSelection")),
                    0,
                    makeAny( _rxModel ),
                    PropertyState_DIRECT_VALUE);

    // create the dialog
    Reference< XExecutableDialog > xAdminDialog;
    OSL_VERIFY( m_aContext.createComponentWithArguments( "com.sun.star.sdb.DatabaseWizardDialog", aWizardArgs, xAdminDialog ) );

    // execute it
    if ( !xAdminDialog.is() || ( RET_OK != xAdminDialog->execute() ) )
        return sal_False;

    Reference<XPropertySet> xProp(xAdminDialog,UNO_QUERY);
    sal_Bool bSuccess = sal_False;
    xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OpenDatabase"))) >>= bSuccess;
    xProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("StartTableWizard"))) >>= _bShouldStartTableWizard;
    return bSuccess;
}

// -----------------------------------------------------------------------
void SAL_CALL DBContentLoader::load(const Reference< XFrame > & rFrame, const ::rtl::OUString& _rURL,
        const Sequence< PropertyValue >& rArgs,
        const Reference< XLoadEventListener > & rListener) throw(::com::sun::star::uno::RuntimeException)
{
    // first check if preview is true, if so return with out creating a controller. Preview is not supported
    ::comphelper::NamedValueCollection aMediaDesc( rArgs );
    sal_Bool bPreview = aMediaDesc.getOrDefault( "Preview", sal_False );
    if ( bPreview )
    {
        if (rListener.is())
            rListener->loadCancelled(this);
        return;
    }

    Reference< XModel > xModel       = aMediaDesc.getOrDefault( "Model", Reference< XModel >() );
    ::rtl::OUString     sSalvagedURL = aMediaDesc.getOrDefault( "SalvagedFile", _rURL );

    sal_Bool bCreateNew = sal_False;        // does the URL denote the private:factory URL?
    sal_Bool bStartTableWizard = sal_False; // start the table wizard after everything was loaded successfully?

    sal_Bool bSuccess = sal_True;

    // If there's no interaction handler in the media descriptor, put one. 
    // By definition, loading via loadComponentFromURL (and thus via the content loader here)
    // is allowed to raise UI. To not burden every place inside the document with creating
    // a default handler, we simply ensure there is one.
    // If a handler is present in the media descriptor, even if it is NULL, we will
    // not touch it.
    if ( !aMediaDesc.has( "InteractionHandler" ) )
    {
        Reference< XInteractionHandler > xHandler;
        if ( m_aContext.createComponent( "com.sun.star.sdb.InteractionHandler", xHandler ) )
            aMediaDesc.put( "InteractionHandler", xHandler );
    }

    // it's allowed to pass an existing document
    Reference< XOfficeDatabaseDocument > xExistentDBDoc;
    xModel.set( aMediaDesc.getOrDefault( "Model", xExistentDBDoc ), UNO_QUERY );
    aMediaDesc.remove( "Model" );

    // also, it's allowed to specify the type of view which should be created
    ::rtl::OUString sViewName = aMediaDesc.getOrDefault( "ViewName", ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Default" ) ) );
    aMediaDesc.remove( "ViewName" );

    sal_Int32 nInitialSelection = -1;
    if ( !xModel.is() )
    {
        Reference< XSingleServiceFactory > xDatabaseContext;
        if ( !m_aContext.createComponent( (::rtl::OUString)SERVICE_SDB_DATABASECONTEXT, xDatabaseContext ) )
            throw RuntimeException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "css.sdb.DatabaseContext not available" ) ), NULL );

        ::rtl::OUString sFactoryName = SvtModuleOptions().GetFactoryEmptyDocumentURL(SvtModuleOptions::E_DATABASE);
        bCreateNew = sFactoryName.match(_rURL);

        Reference< XDocumentDataSource > xDocumentDataSource;
        bool bNewAndInteractive = false;
        if ( bCreateNew )
        {
            bNewAndInteractive = lcl_urlAllowsInteraction( m_aContext, _rURL );
            xDocumentDataSource.set( xDatabaseContext->createInstance(), UNO_QUERY_THROW );
        }
        else
        {
            ::comphelper::NamedValueCollection aCreationArgs;
            aCreationArgs.put( (::rtl::OUString)INFO_POOLURL, sSalvagedURL );
            xDocumentDataSource.set( xDatabaseContext->createInstanceWithArguments( aCreationArgs.getWrappedNamedValues() ), UNO_QUERY_THROW );
        }

        xModel.set( xDocumentDataSource->getDatabaseDocument(), UNO_QUERY );

        if ( bCreateNew && xModel.is() )
        {
            if ( bNewAndInteractive )
            {   
                bSuccess = impl_executeNewDatabaseWizard( xModel, bStartTableWizard );
            }
            else
            {
                try
                {
                    Reference< XLoadable > xLoad( xModel, UNO_QUERY_THROW );
                    xLoad->initNew();
                    bSuccess = true;
                }
                catch( const Exception& )
                {
                    bSuccess = false;
                }
            }

            // initially select the "Tables" category (will be done below)
            nInitialSelection = ::com::sun::star::sdb::application::DatabaseObjectContainer::TABLES;
        }
    }

    if ( !xModel.is() )
    {
        if ( rListener.is() )
            rListener->loadCancelled(this);
        return;
    }

    if ( !bCreateNew && !xModel->getURL().getLength() )
    {
        try
        {
            aMediaDesc.put( "FileName", _rURL );
            Reference< XLoadable > xLoad( xModel, UNO_QUERY_THROW );

            Sequence< PropertyValue > aResource( aMediaDesc.getPropertyValues() );
            xLoad->load( aResource );
            xModel->attachResource( _rURL, aResource );
        }
        catch(const Exception&)
        {
            DBG_UNHANDLED_EXCEPTION();
            bSuccess = sal_False;
        }
    }

    Reference< XController2 > xController;
    if ( bSuccess )
    {
        try
        {
            Reference< XModel2 > xModel2( xModel, UNO_QUERY_THROW );
            xController = xModel2->createViewController( sViewName, Sequence< PropertyValue >(), rFrame );

            bSuccess = xController.is();
            if ( bSuccess )
            {
                xController->attachModel( xModel );
                rFrame->setComponent( xController->getComponentWindow(), xController.get() );
                xController->attachFrame( rFrame );
                xModel->setCurrentController( xController.get() );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
            bSuccess = sal_False;
        }
    }

    if (bSuccess)
    {
        if ( rListener.is() )
            rListener->loadFinished(this);

        if ( nInitialSelection != -1 )
        {
            Reference< css::view::XSelectionSupplier >  xDocView( xModel->getCurrentController(), UNO_QUERY );
            if ( xDocView.is() )
            {
                NamedDatabaseObject aSelection;
                aSelection.Type = nInitialSelection;
                xDocView->select( makeAny( aSelection ) );
            }
        }

        if ( bStartTableWizard )
        {
            // reset the data of the previous async drop (if any)
            if ( m_nStartWizard )
                Application::RemoveUserEvent(m_nStartWizard);
            m_sCurrentURL = xModel->getURL();
            m_xMySelf = this;
            m_nStartWizard = Application::PostUserEvent(LINK(this, DBContentLoader, OnStartTableWizard));
        }
    }
    else
    {
        if ( rListener.is() )
            rListener->loadCancelled( this );
    }

    if ( !bSuccess )
        ::comphelper::disposeComponent(xModel);
}

// -----------------------------------------------------------------------
void DBContentLoader::cancel(void) throw()
{
}
// -----------------------------------------------------------------------------
IMPL_LINK( DBContentLoader, OnStartTableWizard, void*, /*NOTINTERESTEDIN*/ )
{
    m_nStartWizard = 0;
    try
    {
        Sequence< Any > aWizArgs(1);
        PropertyValue aValue;
        aValue.Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DatabaseLocation"));
        aValue.Value <<= m_sCurrentURL;
        aWizArgs[0] <<= aValue;

        ::vos::OGuard aGuard(Application::GetSolarMutex());
        Reference< XJobExecutor > xTableWizard;
        if ( m_aContext.createComponentWithArguments( "com.sun.star.wizards.table.CallTableWizard", aWizArgs, xTableWizard ) )
            xTableWizard->trigger(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("start")));
    }
    catch(const Exception&)
    {
        OSL_ENSURE(sal_False, "caught an exception while starting the table wizard!");
    }
    m_xMySelf = NULL;
    return 0L;
}
}
// -------------------------------------------------------------------------
extern "C" void SAL_CALL createRegistryInfo_DBContentLoader2()
{
    static ::dbaxml::OMultiInstanceAutoRegistration< ::dbaxml::DBContentLoader > aAutoRegistration;
}
// -------------------------------------------------------------------------
extern "C" void SAL_CALL writeDBLoaderInfo2(void* pRegistryKey)
{
    Reference< XRegistryKey> xKey(reinterpret_cast< XRegistryKey*>(pRegistryKey));

    // register content loader for dispatch
    ::rtl::OUString aImpl = ::rtl::OUString::createFromAscii("/");
    aImpl += ::dbaxml::DBContentLoader::getImplementationName_Static();

    ::rtl::OUString aImpltwo = aImpl;
    aImpltwo += ::rtl::OUString::createFromAscii("/UNO/Loader");
    Reference< XRegistryKey> xNewKey = xKey->createKey( aImpltwo );
    aImpltwo = aImpl;
    aImpltwo += ::rtl::OUString::createFromAscii("/Loader");
    Reference< XRegistryKey >  xLoaderKey = xKey->createKey( aImpltwo );
    xNewKey = xLoaderKey->createKey( ::rtl::OUString::createFromAscii("Pattern") );
    xNewKey->setAsciiValue( ::rtl::OUString::createFromAscii("private:factory/sdatabase") );
}
// -----------------------------------------------------------------------------

