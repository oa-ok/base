/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ReportEngineJFree.cxx,v $
 * $Revision: 1.8 $
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
#include <com/sun/star/beans/PropertyValue.hpp>
#include "ReportEngineJFree.hxx"
#include <comphelper/enumhelper.hxx>
#include <comphelper/documentconstants.hxx>
#include <comphelper/storagehelper.hxx>
#include <connectivity/dbtools.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <comphelper/sequence.hxx>
#include <com/sun/star/frame/XComponentLoader.hpp>
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/sdb/XCompletedExecution.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryAnalyzer.hpp>
#include <com/sun/star/sdb/XSingleSelectQueryComposer.hpp>
#include <com/sun/star/sdb/CommandType.hpp>

#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/XJob.hpp>
#ifndef REPORTDESIGN_SHARED_CORESTRINGS_HRC
#include "corestrings.hrc"
#endif
#include <tools/debug.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/sharedunocomponent.hxx>
#include <comphelper/mimeconfighelper.hxx>
#include "Tools.hxx"
#include <comphelper/property.hxx>
#include <connectivity/CommonTools.hxx>
#include <rtl/ustrbuf.hxx>
// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;

DBG_NAME( rpt_OReportEngineJFree )
// -----------------------------------------------------------------------------
OReportEngineJFree::OReportEngineJFree( const uno::Reference< uno::XComponentContext >& context)
:ReportEngineBase(m_aMutex)
,ReportEnginePropertySet(context,static_cast< Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< ::rtl::OUString >())
,m_xContext(context)
{
    DBG_CTOR( rpt_OReportEngineJFree,NULL);
}
// -----------------------------------------------------------------------------
// TODO: VirtualFunctionFinder: This is virtual function! 
// 
OReportEngineJFree::~OReportEngineJFree()
{
    DBG_DTOR( rpt_OReportEngineJFree,NULL);
}
//--------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OReportEngineJFree,ReportEngineBase,ReportEnginePropertySet)
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::dispose() throw(uno::RuntimeException) 
{
    ReportEnginePropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose(); 
    m_xActiveConnection.clear();
}
// -----------------------------------------------------------------------------
::rtl::OUString OReportEngineJFree::getImplementationName_Static(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.OReportEngineJFree"));
}

//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OReportEngineJFree::getImplementationName(  ) throw(uno::RuntimeException)
{
    return getImplementationName_Static();
}
//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > OReportEngineJFree::getSupportedServiceNames_Static(  ) throw(uno::RuntimeException)
{
    uno::Sequence< ::rtl::OUString > aServices(1);
    aServices.getArray()[0] = SERVICE_REPORTENGINE;
    
    return aServices;
}
//------------------------------------------------------------------------------
uno::Reference< uno::XInterface > OReportEngineJFree::create(uno::Reference< uno::XComponentContext > const & xContext)
{
    return *(new OReportEngineJFree(xContext));
}

//--------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString > SAL_CALL OReportEngineJFree::getSupportedServiceNames(  ) throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
//------------------------------------------------------------------------------
sal_Bool SAL_CALL OReportEngineJFree::supportsService(const ::rtl::OUString& ServiceName) throw( uno::RuntimeException )
{
    return ::comphelper::existsValue(ServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
// XReportEngine
    // Attributes
uno::Reference< report::XReportDefinition > SAL_CALL OReportEngineJFree::getReportDefinition() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xReport;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setReportDefinition( const uno::Reference< report::XReportDefinition >& _report ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( !_report.is() ) 
        throw lang::IllegalArgumentException();
    BoundListeners l;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        if ( m_xReport != _report )
        {
            prepareSet(PROPERTY_REPORTDEFINITION, uno::makeAny(m_xReport), uno::makeAny(_report), &l);
            m_xReport = _report;
        }
    }
    l.notify();
}
// -----------------------------------------------------------------------------
uno::Reference< task::XStatusIndicator > SAL_CALL OReportEngineJFree::getStatusIndicator() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_StatusIndicator;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setStatusIndicator( const uno::Reference< task::XStatusIndicator >& _statusindicator ) throw (uno::RuntimeException)
{
    set(PROPERTY_STATUSINDICATOR,_statusindicator,m_StatusIndicator);
}
// -----------------------------------------------------------------------------
::rtl::OUString OReportEngineJFree::transform()
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("content.xml")); /// TODO has to be changed into the real name for the report transformation
}
// -----------------------------------------------------------------------------
::rtl::OUString OReportEngineJFree::getNewOutputName()
{
    ::rtl::OUString sOutputName;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
        if ( !m_xReport.is() || !m_xActiveConnection.is() )
            throw lang::IllegalArgumentException();

        try
        {
            uno::Reference< embed::XStorage > xTemp = OStorageHelper::GetTemporaryStorage(/*sFileTemp,embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,*/uno::Reference< lang::XMultiServiceFactory >(m_xContext->getServiceManager(),uno::UNO_QUERY));
            utl::DisposableComponent aTemp(xTemp);
            uno::Sequence< beans::PropertyValue > aEmpty;
            uno::Reference< beans::XPropertySet> xStorageProp(xTemp,uno::UNO_QUERY);
            if ( xStorageProp.is() )
            {
                static const ::rtl::OUString sPropName(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
                xStorageProp->setPropertyValue( sPropName, uno::makeAny(m_xReport->getMimeType()));
            }
            m_xReport->storeToStorage(xTemp,aEmpty); // store to temp file because it may contain information which aren't in the database yet.

            uno::Sequence< beans::NamedValue > aConvertedProperties(5/*6*/);
            sal_Int32 nPos = 0;
            aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("InputStorage"));
            aConvertedProperties[nPos++].Value <<= xTemp;
            aConvertedProperties[nPos].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("OutputStorage"));

            const static String s_sExt = String::CreateFromAscii(".rpt");
            String sName = m_xReport->getName();
            ::utl::TempFile aFile(sName,sal_False,&s_sExt);
            uno::Reference< embed::XStorage > xOut = OStorageHelper::GetStorageFromURL(aFile.GetURL(),embed::ElementModes::WRITE | embed::ElementModes::TRUNCATE,uno::Reference< lang::XMultiServiceFactory >(m_xContext->getServiceManager(),uno::UNO_QUERY));
            utl::DisposableComponent aOut(xOut);
            xStorageProp.set(xOut,uno::UNO_QUERY);
            if ( xStorageProp.is() )
            {
                static const ::rtl::OUString sPropName = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("MediaType"));
                xStorageProp->setPropertyValue( sPropName, uno::makeAny(m_xReport->getMimeType()));
            }
            
            aConvertedProperties[nPos++].Value <<= xOut;

            aConvertedProperties[nPos].Name = PROPERTY_REPORTDEFINITION;
            aConvertedProperties[nPos++].Value <<= m_xReport;

            aConvertedProperties[nPos].Name = PROPERTY_ACTIVECONNECTION;
            aConvertedProperties[nPos++].Value <<= m_xActiveConnection;

            // create job factory and initialize
            const ::rtl::OUString sReportEngineServiceName = ::dbtools::getDefaultReportEngineServiceName(uno::Reference< lang::XMultiServiceFactory >(m_xContext->getServiceManager(),uno::UNO_QUERY_THROW));
            uno::Reference<task::XJob> xJob(m_xContext->getServiceManager()->createInstanceWithContext(sReportEngineServiceName,m_xContext),uno::UNO_QUERY_THROW);
            if ( m_xReport->getCommand().getLength() )
            {
                xJob->execute(aConvertedProperties);
                if ( xStorageProp.is() )
                {
                    //xStorageProp->getPropertyValue(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("URL"))) >>= sOutputName;
                    sOutputName = aFile.GetURL();
                }
            }

            uno::Reference<embed::XTransactedObject> xTransact(xOut,uno::UNO_QUERY);
            if ( sOutputName.getLength() && xTransact.is() )
                xTransact->commit();

            if ( !sOutputName.getLength() )
                throw lang::IllegalArgumentException();
        }
        catch(const uno::Exception& e)
        {
            (void)e; // helper to know what e contains
            throw;
        }
    }
    return sOutputName;
}
// -----------------------------------------------------------------------------
// Methods
uno::Reference< frame::XModel > SAL_CALL OReportEngineJFree::createDocumentModel( ) throw (lang::DisposedException, lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel;
    ::rtl::OUString sOutputName = getNewOutputName();
    if ( sOutputName.getLength() ) 
    {
        uno::Reference< lang::XMultiServiceFactory > xFac(m_xContext->getServiceManager(),uno::UNO_QUERY);
        ::comphelper::MimeConfigurationHelper aHelper(xFac);
        ::rtl::OUString sServiceName = aHelper.GetDocServiceNameFromMediaType(m_xReport->getMimeType());
        xModel.set(m_xContext->getServiceManager()->createInstanceWithContext(sServiceName,m_xContext),uno::UNO_QUERY_THROW);
        uno::Sequence< beans::PropertyValue > aArguments;
        xModel->attachResource(sOutputName,aArguments);
    }
    return xModel;
}
// -----------------------------------------------------------------------------
uno::Reference< frame::XModel > SAL_CALL OReportEngineJFree::createDocumentAlive( const uno::Reference< frame::XFrame >& _frame ) throw (lang::DisposedException, lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    uno::Reference< frame::XModel > xModel;
    ::rtl::OUString sOutputName = getNewOutputName(); // starts implicite the report generator
    if ( sOutputName.getLength() ) 
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
        uno::Reference<frame::XComponentLoader> xFrameLoad(_frame,uno::UNO_QUERY);
        if ( !xFrameLoad.is() )
        {
            // if there is no frame given, find the right
            xFrameLoad.set( m_xContext->getServiceManager()->createInstanceWithContext(
                                                    ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.frame.Desktop")) 
                                                    ,m_xContext)
                                                    ,uno::UNO_QUERY);
            ::rtl::OUString sTarget = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_blank"));
            sal_Int32 nFrameSearchFlag = frame::FrameSearchFlag::TASKS | frame::FrameSearchFlag::CREATE;
            uno::Reference< frame::XFrame> xFrame = uno::Reference< frame::XFrame>(xFrameLoad,uno::UNO_QUERY)->findFrame(sTarget,nFrameSearchFlag);
            xFrameLoad.set( xFrame,uno::UNO_QUERY);
        }
        
        if ( xFrameLoad.is() )
        {
            uno::Sequence < beans::PropertyValue > aArgs( 2 );
            sal_Int32 nLen = 0;
            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("AsTemplate"));
            aArgs[nLen++].Value <<= sal_False;

            aArgs[nLen].Name = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("ReadOnly"));
            aArgs[nLen++].Value <<= sal_True;

            uno::Reference< lang::XMultiServiceFactory > xFac(m_xContext->getServiceManager(),uno::UNO_QUERY);
            ::comphelper::MimeConfigurationHelper aHelper(xFac);
            xModel.set( xFrameLoad->loadComponentFromURL(
                sOutputName,
                ::rtl::OUString(), // empty frame name
                0,
                aArgs
                ),uno::UNO_QUERY);
        }
    }
    return xModel;
}
// -----------------------------------------------------------------------------
util::URL SAL_CALL OReportEngineJFree::createDocument( ) throw (lang::DisposedException, lang::IllegalArgumentException, uno::Exception, uno::RuntimeException)
{
    util::URL aRet;
    uno::Reference< frame::XModel > xModel = createDocumentModel();
    if ( xModel.is() )
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
    }
    return aRet;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::interrupt(  ) throw (lang::DisposedException, uno::Exception, uno::RuntimeException)
{
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        ::connectivity::checkDisposed(ReportEngineBase::rBHelper.bDisposed);
    }
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OReportEngineJFree::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    return ReportEnginePropertySet::getPropertySetInfo();
}
// -------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OReportEngineJFree::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return ReportEnginePropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    ReportEnginePropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
uno::Reference< sdbc::XConnection > SAL_CALL OReportEngineJFree::getActiveConnection() throw (uno::RuntimeException)
{
    return m_xActiveConnection;
}
// -----------------------------------------------------------------------------
void SAL_CALL OReportEngineJFree::setActiveConnection( const uno::Reference< sdbc::XConnection >& _activeconnection ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( !_activeconnection.is() ) 
        throw lang::IllegalArgumentException();
    set(PROPERTY_ACTIVECONNECTION,_activeconnection,m_xActiveConnection);
}
// -----------------------------------------------------------------------------
::rtl::OUString OReportEngineJFree::getOrderStatement() const
{
    OSL_ENSURE(m_xActiveConnection.is(),"OReportEngineJFree::getOrderStatement: The ActiveConnection can not be NULL here!");
    OSL_ENSURE(m_xReport.is(),"OReportEngineJFree::getOrderStatement: The ReportDefinition can not be NULL here!");

    // retrieve columns of the object we're bound to
    uno::Reference< lang::XComponent > xHoldAlive;
    ::dbtools::SQLExceptionInfo aErrorInfo;
    uno::Reference< container::XNameAccess> xColumns( ::dbtools::getFieldsByCommandDescriptor(
        m_xActiveConnection, m_xReport->getCommandType(), m_xReport->getCommand(), xHoldAlive, & aErrorInfo ) );
    if ( aErrorInfo.isValid() )
        aErrorInfo.doThrow();
    if ( !xColumns.is() )
    {
        DBG_WARNING( "OReportEngineJFree::getOrderStatement: could not retrieve the columns for the ORDER statement!" );
        return ::rtl::OUString();
    }

    // set order for groups
    ::rtl::OUStringBuffer aOrder;

    const ::rtl::OUString sQuote = m_xActiveConnection->getMetaData()->getIdentifierQuoteString();

    uno::Reference< report::XGroups> xGroups = m_xReport->getGroups();
    const sal_Int32 nCount = xGroups->getCount();
    for (sal_Int32 i = 0; i < nCount; ++i )
    {
        uno::Reference< report::XGroup> xGroup(xGroups->getByIndex(i),uno::UNO_QUERY);
        ::rtl::OUString sExpression = xGroup->getExpression();
        if ( xColumns->hasByName(sExpression) )
            sExpression = ::dbtools::quoteName( sQuote, sExpression );
        sExpression = sExpression.trim(); // Trim away white spaces
        if (sExpression.getLength() > 0)
        {
            aOrder.append( sExpression );
            if (aOrder.getLength() > 0) 
                aOrder.appendAscii( " " );
            if ( !xGroup->getSortAscending() )
                aOrder.appendAscii( "DESC" );
            if ( (i+1) < nCount )
                aOrder.appendAscii( "," );
        }
    }

    return aOrder.makeStringAndClear();
}
// =============================================================================
} // namespace reportdesign
// =============================================================================
