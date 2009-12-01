/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: genericcontroller.cxx,v $
 * $Revision: 1.94.24.2 $
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
#ifndef DBAUI_GENERICCONTROLLER_HXX
#include "genericcontroller.hxx"
#endif
#ifndef _COMPHELPER_UNO3_HXX_
#include <comphelper/uno3.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXWINDOW_HXX_
#include <toolkit/awt/vclxwindow.hxx>
#endif
#ifndef DBACCESS_UI_BROWSER_ID_HXX
#include "browserids.hxx"
#endif
#ifndef _SV_SVAPP_HXX //autogen
#include <vcl/svapp.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif
#ifndef DBAUI_DATAVIEW_HXX
#include "dataview.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef TOOLS_DIAGNOSE_EX_H
#include <tools/diagnose_ex.h>
#endif
#ifndef DBACCESS_SHARED_DBUSTRINGS_HRC
#include "dbustrings.hrc"
#endif
#ifndef _VCL_STDTEXT_HXX
#include <vcl/stdtext.hxx>
#endif
#ifndef _CPPUHELPER_TYPEPROVIDER_HXX_
#include <cppuhelper/typeprovider.hxx>
#endif
#include <framework/titlehelper.hxx>
#ifndef _COMPHELPER_SEQUENCE_HXX_
#include <comphelper/sequence.hxx>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _COM_SUN_STAR_SDBC_XDATASOURCE_HPP_
#include <com/sun/star/sdbc/XDataSource.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_SQLCONTEXT_HPP_
#include <com/sun/star/sdb/SQLContext.hpp>
#endif
#ifndef _COM_SUN_STAR_SDB_XCOMPLETEDCONNECTION_HPP_
#include <com/sun/star/sdb/XCompletedConnection.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYSET_HPP_
#include <com/sun/star/beans/XPropertySet.hpp>
#endif
#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
#ifndef DBAUI_TOOLS_HXX
#include "UITools.hxx"
#endif
#ifndef _DBAUI_COMMON_TYPES_HXX_
#include "commontypes.hxx"
#endif

#ifndef _SV_WAITOBJ_HXX
#include <vcl/waitobj.hxx>
#endif
#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef SVTOOLS_URIHELPER_HXX
#include <svtools/urihelper.hxx>
#endif
#ifndef _DBAUI_DATASOURCECONNECTOR_HXX_
#include "datasourceconnector.hxx"
#endif
#ifndef INCLUDED_SVTOOLS_MODULEOPTIONS_HXX
#include <svtools/moduleoptions.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_FRAMESEARCHFLAG_HPP_
#include <com/sun/star/frame/FrameSearchFlag.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_STATUS_VISIBILITY_HPP_
#include <com/sun/star/frame/status/Visibility.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _RTL_LOGFILE_HXX_
#include <rtl/logfile.hxx>
#endif
#include <algorithm>
#include <hash_map>
#include <cppuhelper/implbase1.hxx>
#include <limits>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::frame::status;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::task;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star;
using namespace ::dbtools;
using namespace ::comphelper;

// -------------------------------------------------------------------------
#define ALL_FEATURES	            -1
#define FIRST_USER_DEFINED_FEATURE  ( ::std::numeric_limits< sal_uInt16 >::max() - 1000 )
#define LAST_USER_DEFINED_FEATURE   ( ::std::numeric_limits< sal_uInt16 >::max()        )

// -------------------------------------------------------------------------
typedef ::std::hash_map< sal_Int16, sal_Int16 > CommandHashMap;
typedef ::std::list< DispatchInformation > DispatchInfoList;


// -------------------------------------------------------------------------
const ::rtl::OUString& getConfirmDeletionURL()
{
    static const ::rtl::OUString sConfirmDeletionURL( RTL_CONSTASCII_USTRINGPARAM( ".uno:FormSlots/ConfirmDeletion" ) );
    return sConfirmDeletionURL;
}

namespace dbaui
{

//==========================================================================
//= UserDefinedFeatures
//==========================================================================
class UserDefinedFeatures
{
public:
    UserDefinedFeatures( const Reference< XController >& _rxController );

    FeatureState    getState( const URL& _rFeatureURL );
    void            execute( const URL& _rFeatureURL, const Sequence< PropertyValue>& _rArgs );

private:
    ::com::sun::star::uno::WeakReference< XController > m_aController;
};

//--------------------------------------------------------------------------
UserDefinedFeatures::UserDefinedFeatures( const Reference< XController >& _rxController )
    :m_aController( _rxController )
{
}

//--------------------------------------------------------------------------
FeatureState UserDefinedFeatures::getState( const URL& /*_rFeatureURL*/ )
{
    // for now, enable all the time
    // TODO: we should ask the dispatcher. However, this is laborious, since you cannot ask a dispatcher
    // directly, but need to add a status listener.
    FeatureState aState;
    aState.bEnabled = sal_True;
    return aState;
}

//--------------------------------------------------------------------------
void UserDefinedFeatures::execute( const URL& _rFeatureURL, const Sequence< PropertyValue>& _rArgs )
{
    try
    {
        Reference< XController > xController( (Reference< XController >)m_aController, UNO_SET_THROW );
        Reference< XDispatchProvider > xDispatchProvider( xController->getFrame(), UNO_QUERY_THROW );
        Reference< XDispatch > xDispatch( xDispatchProvider->queryDispatch(
            _rFeatureURL,
            ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "_self" ) ),
            FrameSearchFlag::AUTO
        ) );

        if ( xDispatch == xController )
        {
            OSL_ENSURE( false, "UserDefinedFeatures::execute: the controller shouldn't be the dispatcher here!" );
            xDispatch.clear();
        }

        if ( xDispatch.is() )
            xDispatch->dispatch( _rFeatureURL, _rArgs );
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

//==========================================================================
//= OGenericUnoController_Data
//==========================================================================
struct OGenericUnoController_Data
{
    ::sfx2::UserInputInterception   m_aUserInputInterception;
    UserDefinedFeatures             m_aUserDefinedFeatures;

    OGenericUnoController_Data( OGenericUnoController& _rController, ::osl::Mutex& _rMutex )
        :m_aUserInputInterception( _rController, _rMutex )
        ,m_aUserDefinedFeatures( _rController.getXController() )
    {
    }
};

//==========================================================================
//= OGenericUnoController
//==========================================================================
DBG_NAME(OGenericUnoController)
// -------------------------------------------------------------------------
OGenericUnoController::OGenericUnoController(const Reference< XMultiServiceFactory >& _rM)
    :OGenericUnoController_Base( getMutex() )
#ifdef DBG_UTIL
    ,m_bDescribingSupportedFeatures( false )
#endif
    ,m_aAsyncInvalidateAll(LINK(this, OGenericUnoController, OnAsyncInvalidateAll))
    ,m_aAsyncCloseTask(LINK(this, OGenericUnoController, OnAsyncCloseTask))
    ,m_xServiceFactory(_rM)
    ,m_aCurrentFrame( *this )
    ,m_pView(NULL)
    ,m_bPreview(sal_False)
    ,m_bReadOnly(sal_False)
    ,m_bCurrentlyModified(sal_False)
    ,m_bExternalTitle(sal_False)
{
    osl_incrementInterlockedCount( &m_refCount );
    {
        m_pData.reset( new OGenericUnoController_Data( *this, getMutex() ) );
    }
    osl_decrementInterlockedCount( &m_refCount );

    DBG_CTOR(OGenericUnoController,NULL);

    try
    {
        m_xUrlTransformer = Reference< XURLTransformer > (_rM->createInstance(::rtl::OUString::createFromAscii("com.sun.star.util.URLTransformer")), UNO_QUERY);
    }
    catch(Exception&)
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}

#ifdef WNT
// -----------------------------------------------------------------------------
OGenericUnoController::OGenericUnoController()
    :OGenericUnoController_Base( getMutex() )
#ifdef DBG_UTIL
    ,m_bDescribingSupportedFeatures( false )
#endif
    ,m_aAsyncInvalidateAll(LINK(this, OGenericUnoController, OnAsyncInvalidateAll))
    ,m_aAsyncCloseTask(LINK(this, OGenericUnoController, OnAsyncCloseTask))
    ,m_aCurrentFrame( *this )
    ,m_pView(NULL)
    ,m_bPreview(sal_False)
    ,m_bReadOnly(sal_False)
    ,m_bCurrentlyModified(sal_False)
{
    OSL_ENSURE( false, "OGenericUnoController::OGenericUnoController: illegal call!" );
    // This ctor only exists because the MSVC compiler complained about an unresolved external
    // symbol. It should not be used at all. Since using it yields strange runtime problems,
    // we simply abort here.
    abort();
}
#endif

// -----------------------------------------------------------------------------
OGenericUnoController::~OGenericUnoController()
{

    DBG_DTOR(OGenericUnoController,NULL);
}

// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::Construct(Window* /*pParent*/)
{
    OSL_ENSURE( getView(), "the view is NULL!" );

    if ( getView() )
    {
        getView()->Construct();
        getView()->Show();
    }

    m_aSupportedFeatures.clear();
    fillSupportedFeatures();

    // create the database context
    DBG_ASSERT(getORB().is(), "OGenericUnoController::Construct need a service factory!");
    try
    {
        m_xDatabaseContext = Reference< XNameAccess >(getORB()->createInstance(SERVICE_SDB_DATABASECONTEXT), UNO_QUERY);
    }
    catch(Exception&)
    {
        DBG_ERROR("OGenericUnoController::Construct: could not create (or start listening at) the database context!");
    }

    if (!m_xDatabaseContext.is())
    {		// at least notify the user. Though the whole component does not make any sense without the database context ...
        ShowServiceNotAvailableError(getView(), String(SERVICE_SDB_DATABASECONTEXT), sal_True);
    }
    return sal_True;
}
//------------------------------------------------------------------------------
IMPL_LINK(OGenericUnoController, OnAsyncInvalidateAll, void*, EMPTYARG)
{
    if ( !OGenericUnoController_Base::rBHelper.bInDispose && !OGenericUnoController_Base::rBHelper.bDisposed )
        InvalidateFeature_Impl();
    return 0L;
}
// -----------------------------------------------------------------------------
void OGenericUnoController::impl_initialize()
{
}
// -------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::initialize( const Sequence< Any >& aArguments ) throw(Exception, RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( getMutex() );

    Reference< XWindow >		xParent;
    Reference< XFrame > xFrame;

    PropertyValue aValue;
    const Any* pIter	= aArguments.getConstArray();
    const Any* pEnd 	= pIter + aArguments.getLength();

    for ( ; pIter != pEnd; ++pIter )
    {
        if ( ( *pIter >>= aValue ) && ( 0 == aValue.Name.compareToAscii( "Frame" ) ) )
        {
            xFrame.set(aValue.Value,UNO_QUERY_THROW);
        }
        /* #i42316#
        else if ( ( *pIter >>= aValue ) && ( 0 == aValue.Name.compareToAscii( "ReadOnly" ) ) )
        {
            aValue.Value >>= m_bReadOnly;
        }
        */
        else if ( ( *pIter >>= aValue ) && ( 0 == aValue.Name.compareToAscii( "Preview" ) ) )
        {
            aValue.Value >>= m_bPreview;
            m_bReadOnly = sal_True;
        }
    }
    try
    {
        if ( !xFrame.is() )
            throw IllegalArgumentException( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "need a frame" ) ), *this, 1 );

        xParent = xFrame->getContainerWindow();
        VCLXWindow* pParentComponent = VCLXWindow::GetImplementation(xParent);
        Window* pParentWin = pParentComponent ? pParentComponent->GetWindow() : NULL;
        if (!pParentWin)
        {
            throw IllegalArgumentException( ::rtl::OUString::createFromAscii( "Parent window is null" ), *this, 1 );
        }

        m_aInitParameters.assign( aArguments );
        Construct( pParentWin );

        ODataView* pView = getView();
        if ( !pView )
            throw RuntimeException( ::rtl::OUString::createFromAscii( "unable to create a view" ), *this );

        if ( m_bReadOnly || m_bPreview )
            pView->EnableInput( FALSE );

        impl_initialize();
    }
    catch(Exception& e)
    {
        // no one clears my view if I won't
        ::std::auto_ptr<Window> aTemp(m_pView);
        m_pView = NULL;
        throw e;
    }
}

//------------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::acquire(  ) throw ()
{
    OGenericUnoController_Base::acquire();
}

//------------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::release(  ) throw ()
{
    OGenericUnoController_Base::release();
}

// -------------------------------------------------------------------------
void OGenericUnoController::startFrameListening( const Reference< XFrame >& _rxFrame )
{
    if ( _rxFrame.is() )
        _rxFrame->addFrameActionListener( this );
}

// -------------------------------------------------------------------------
void OGenericUnoController::stopFrameListening( const Reference< XFrame >& _rxFrame )
{
    if ( _rxFrame.is() )
        _rxFrame->removeFrameActionListener( this );
}

// -------------------------------------------------------------------------
void OGenericUnoController::disposing(const EventObject& Source) throw( RuntimeException )
{
    // our frame ?
    if ( Source.Source == getFrame() )
        stopFrameListening( getFrame() );
}
//------------------------------------------------------------------------
void OGenericUnoController::modified(const EventObject& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );	
    if ( !isDataSourceReadOnly() )
    {
        Reference<XModifiable> xModi(aEvent.Source,UNO_QUERY);
        if ( xModi.is() )
            m_bCurrentlyModified = xModi->isModified(); // can only be reset by save
        else
            m_bCurrentlyModified = sal_True;
    }
    InvalidateFeature(ID_BROWSER_SAVEDOC);
    InvalidateFeature(ID_BROWSER_UNDO);
}
// -----------------------------------------------------------------------
Reference< XWindow > SAL_CALL OGenericUnoController::getComponentWindow() throw (RuntimeException)
{
    return VCLUnoHelper::GetInterface( getView() );
}

// -----------------------------------------------------------------------
::rtl::OUString SAL_CALL OGenericUnoController::getViewControllerName() throw (::com::sun::star::uno::RuntimeException)
{
    return ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Default" ) );
}

// -----------------------------------------------------------------------
void OGenericUnoController::attachFrame( const Reference< XFrame >& _rxFrame ) throw( RuntimeException )
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( getMutex() );	

    stopFrameListening( m_aCurrentFrame.getFrame() );
    Reference< XFrame > xFrame = m_aCurrentFrame.attachFrame( _rxFrame );
    startFrameListening( xFrame );

    loadMenu( xFrame );

    if ( getView() )
        getView()->attachFrame( xFrame );
}

// -----------------------------------------------------------------------------
struct CommandCollector : public ::std::unary_function< SupportedFeatures::value_type, void>
{
    sal_uInt16  m_nFeature;
    StringBag&  m_rFeatureCommands;
    CommandCollector( sal_uInt16 _nFeature, StringBag& _rFeatureCommands )
        :m_nFeature        ( _nFeature         )
        ,m_rFeatureCommands( _rFeatureCommands )
    {
    }

    void operator() ( const SupportedFeatures::value_type& lhs )
    {
        if ( lhs.second.nFeatureId == m_nFeature )
            m_rFeatureCommands.insert( lhs.first );
    }
};

// -----------------------------------------------------------------------
namespace
{
    typedef ::std::vector< Any >    States;

    // ...................................................................
    void    lcl_notifyMultipleStates( XStatusListener& _rListener, FeatureStateEvent& _rEvent, const States& _rStates )
    {
        for (   States::const_iterator state = _rStates.begin();
                state != _rStates.end();
                ++state
            )
        {
            _rEvent.State = *state;
            _rListener.statusChanged( _rEvent );
        }
    }

    // ...................................................................
    void    lcl_collectStates( const FeatureState& _rFeatureState, States& _out_rStates )
    {
        // order matters, due to a bug in framework which resets the check state when any non-boolean event
        // arrives
        // #i68215# is the bug to (re-)introduce this "ordered" notification here
        // #i67882# is the bug which was caused by the real fix which we did in framework
        // #i68216# is the bug which requests to fix the code in Draw which relies on
        //          framework's implementation details
        // 2006-08-07 / frank.schoenheit@sun.com
        if ( !!_rFeatureState.sTitle )
            _out_rStates.push_back( makeAny( *_rFeatureState.sTitle ) );
        if ( !!_rFeatureState.bChecked )
            _out_rStates.push_back( makeAny( (sal_Bool)*_rFeatureState.bChecked ) );
        if ( !!_rFeatureState.bInvisible )
            _out_rStates.push_back( makeAny( Visibility( !*_rFeatureState.bInvisible ) ) );
        if ( _rFeatureState.aValue.hasValue() )
            _out_rStates.push_back( _rFeatureState.aValue );
        if ( _out_rStates.empty() )
            _out_rStates.push_back( Any() );
    }
}

// -----------------------------------------------------------------------
void OGenericUnoController::ImplBroadcastFeatureState(const ::rtl::OUString& _rFeature, const Reference< XStatusListener > & xListener, sal_Bool _bIgnoreCache)
{
    sal_uInt16 nFeat = m_aSupportedFeatures[ _rFeature ].nFeatureId;
    FeatureState aFeatState( GetState( nFeat ) );

    FeatureState& rCachedState = m_aStateCache[nFeat];	// creates if neccessary
    if ( !_bIgnoreCache )
    {
        // check if we really need to notify the listeners : this method may be called much more often than needed, so check
        // the cached state of the feature
        sal_Bool bAlreadyCached = ( m_aStateCache.find(nFeat) != m_aStateCache.end() );
        if ( bAlreadyCached )
            if  (   ( rCachedState.bEnabled == aFeatState.bEnabled )
                &&  ( rCachedState.bChecked == aFeatState.bChecked )
                &&  ( rCachedState.bInvisible == aFeatState.bInvisible )
                &&  ( rCachedState.sTitle == aFeatState.sTitle )
                )
            return;
    }
    rCachedState = aFeatState;

    FeatureStateEvent aEvent;
    aEvent.FeatureURL.Complete = _rFeature;
    if (m_xUrlTransformer.is())
        m_xUrlTransformer->parseStrict(aEvent.FeatureURL);
    aEvent.Source		= (XDispatch*)this;
    aEvent.IsEnabled	= aFeatState.bEnabled;

    // collect all states to be notified
    States aStates;
    lcl_collectStates( aFeatState, aStates );

    // a special listener ?
    if ( xListener.is() )
        lcl_notifyMultipleStates( *xListener.get(), aEvent, aStates );
    else
    {	// no -> iterate through all listeners responsible for the URL
        StringBag aFeatureCommands;
        ::std::for_each(
            m_aSupportedFeatures.begin(),
            m_aSupportedFeatures.end(),
            CommandCollector( nFeat, aFeatureCommands )
        );

        // it is possible that listeners are registered or revoked while
        // we are notifying them, so we must use a copy of m_arrStatusListener, not
        // m_arrStatusListener itself
        // #121276# / 2005-05-19 / frank.schoenheit@sun.com
        Dispatch aNotifyLoop( m_arrStatusListener );
        DispatchIterator iterSearch = aNotifyLoop.begin();
        DispatchIterator iterEnd = aNotifyLoop.end();

        while (iterSearch != iterEnd)
        {
            DispatchTarget& rCurrent = *iterSearch;
            if ( aFeatureCommands.find( rCurrent.aURL.Complete ) != aFeatureCommands.end() )
            {
                aEvent.FeatureURL = rCurrent.aURL;
                lcl_notifyMultipleStates( *rCurrent.xListener.get(), aEvent, aStates );
            }
            ++iterSearch;
        }
    }

}

//------------------------------------------------------------------------------
sal_Bool OGenericUnoController::isFeatureSupported( sal_Int32 _nId )
{
    SupportedFeatures::iterator aFeaturePos = ::std::find_if(
        m_aSupportedFeatures.begin(),
        m_aSupportedFeatures.end(),
        ::std::bind2nd( CompareFeatureById(), _nId )
    );

    return ( m_aSupportedFeatures.end() != aFeaturePos && aFeaturePos->first.getLength());
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature(const ::rtl::OUString& _rURLPath, const Reference< XStatusListener > & _xListener, sal_Bool _bForceBroadcast)
{
    ImplInvalidateFeature( m_aSupportedFeatures[ _rURLPath ].nFeatureId, _xListener, _bForceBroadcast );
}

// -----------------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature_Impl()
{
#ifdef DBG_UTIL
    static sal_Int32 s_nRecursions = 0;
    ++s_nRecursions;
#endif

    sal_Bool bEmpty = sal_True;
    FeatureListener aNextFeature;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        bEmpty = m_aFeaturesToInvalidate.empty();
        if (!bEmpty)
            aNextFeature = m_aFeaturesToInvalidate.front();
    }
    while(!bEmpty)
    {
        if ( ALL_FEATURES == aNextFeature.nId )
        {
            InvalidateAll_Impl();
            break;
        }
        else
        {
            SupportedFeatures::iterator aFeaturePos = ::std::find_if(
                m_aSupportedFeatures.begin(),
                m_aSupportedFeatures.end(),
                ::std::bind2nd( CompareFeatureById(), aNextFeature.nId )
            );

#if OSL_DEBUG_LEVEL > 0
            if ( m_aSupportedFeatures.end() == aFeaturePos )
            {
                ::rtl::OString sMessage( "OGenericUnoController::InvalidateFeature_Impl: feature id " );
                sMessage += ::rtl::OString::valueOf( aNextFeature.nId );
                sMessage += ::rtl::OString( " has been invalidated, but is not supported!" );
                OSL_ENSURE( false, sMessage.getStr() );
            }
#endif
            if ( m_aSupportedFeatures.end() != aFeaturePos )
                // we really know this feature
                ImplBroadcastFeatureState( aFeaturePos->first, aNextFeature.xListener, aNextFeature.bForceBroadcast );
        }

        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        m_aFeaturesToInvalidate.pop_front();
        bEmpty = m_aFeaturesToInvalidate.empty();
        if (!bEmpty)
            aNextFeature = m_aFeaturesToInvalidate.front();
    }

#ifdef DBG_UTIL
    --s_nRecursions;
#endif
}

// -----------------------------------------------------------------------
void OGenericUnoController::ImplInvalidateFeature( sal_Int32 _nId, const Reference< XStatusListener >& _xListener, sal_Bool _bForceBroadcast )
{
    FeatureListener aListener;
    aListener.nId               = _nId;
    aListener.xListener         = _xListener;
    aListener.bForceBroadcast   = _bForceBroadcast;

    sal_Bool bWasEmpty;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex );
        bWasEmpty = m_aFeaturesToInvalidate.empty();
        m_aFeaturesToInvalidate.push_back( aListener );
    }

    if ( bWasEmpty )
        m_aAsyncInvalidateAll.Call();
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateFeature(sal_uInt16 _nId, const Reference< XStatusListener > & _xListener, sal_Bool _bForceBroadcast)
{
    ImplInvalidateFeature( _nId, _xListener, _bForceBroadcast );
}

// -----------------------------------------------------------------------
void OGenericUnoController::InvalidateAll()
{
    ImplInvalidateFeature( ALL_FEATURES, NULL, sal_True );
}

// -----------------------------------------------------------------------------
void OGenericUnoController::InvalidateAll_Impl()
{
    // ---------------------------------
    // invalidate all aupported features
    
    for (   SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
            aIter != m_aSupportedFeatures.end();
            ++aIter
        )
        ImplBroadcastFeatureState( aIter->first, NULL, sal_True );

    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        DBG_ASSERT(m_aFeaturesToInvalidate.size(), "OGenericUnoController::InvalidateAll_Impl: to be called from within InvalidateFeature_Impl only!");
        m_aFeaturesToInvalidate.pop_front();
        if(!m_aFeaturesToInvalidate.empty())
            m_aAsyncInvalidateAll.Call();
    }
}

// -----------------------------------------------------------------------
Reference< XDispatch >	OGenericUnoController::queryDispatch(const URL& aURL, const ::rtl::OUString& aTargetFrameName, sal_Int32 nSearchFlags) throw( RuntimeException )
{
    Reference< XDispatch > xReturn;

    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::queryDispatch: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    // URL's we can handle ourself?
    if  (   aURL.Complete.equals( getConfirmDeletionURL() )
        ||  (   ( m_aSupportedFeatures.find( aURL.Complete ) != m_aSupportedFeatures.end() )
            &&  !isUserDefinedFeature( aURL.Complete )
            )
        )
    {
        xReturn = this;
    }
    // no? -> ask the slave dispatcher
    else if ( m_xSlaveDispatcher.is() )
    {
        xReturn = m_xSlaveDispatcher->queryDispatch(aURL, aTargetFrameName, nSearchFlags);
    }

    // outta here
    return xReturn;
}

// -----------------------------------------------------------------------
Sequence< Reference< XDispatch > > OGenericUnoController::queryDispatches(const Sequence< DispatchDescriptor >& aDescripts) throw( RuntimeException )
{
    Sequence< Reference< XDispatch > > aReturn;
    sal_Int32 nLen = aDescripts.getLength();
    if ( nLen )
    {
        aReturn.realloc( nLen );
                Reference< XDispatch >* pReturn 	= aReturn.getArray();
        const	Reference< XDispatch >* pReturnEnd	= aReturn.getArray() + nLen;
        const	DispatchDescriptor* 	pDescripts	= aDescripts.getConstArray();

        for ( ; pReturn != pReturnEnd; ++ pReturn, ++pDescripts )
        {
            *pReturn = queryDispatch( pDescripts->FeatureURL, pDescripts->FrameName, pDescripts->SearchFlags );
        }
    }

    return aReturn;
}

// -----------------------------------------------------------------------
Reference< XDispatchProvider >	OGenericUnoController::getSlaveDispatchProvider(void) throw( RuntimeException )
{
    return m_xSlaveDispatcher;
}

// -----------------------------------------------------------------------
void OGenericUnoController::setSlaveDispatchProvider(const Reference< XDispatchProvider > & _xNewProvider) throw( RuntimeException )
{
    m_xSlaveDispatcher = _xNewProvider;
}

// -----------------------------------------------------------------------
Reference< XDispatchProvider >	OGenericUnoController::getMasterDispatchProvider(void) throw( RuntimeException )
{
    return m_xMasterDispatcher;
}

// -----------------------------------------------------------------------
void OGenericUnoController::setMasterDispatchProvider(const Reference< XDispatchProvider > & _xNewProvider) throw( RuntimeException )
{
    m_xMasterDispatcher = _xNewProvider;
}

// -----------------------------------------------------------------------
void OGenericUnoController::dispatch(const URL& _aURL, const Sequence< PropertyValue >& aArgs) throw(RuntimeException)
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    // Since the fix for #123967#, the SolarMutex is not locked anymore when the framework calls into
    // here. So, lock it ourself. The real solution would be to lock it only in the places
    // where it's needed, but a) this might turn out difficult, since we then also need to care
    // for locking in the proper order (SolarMutex and m_aMutex), and b) this would be too many places
    // for the time frame of the fix.
    // #i52602# / frank.schoenheit@sun.com / 2005-07-29

#ifdef TIMELOG
    ::rtl::OString sLog( "OGenericUnoController::dispatch( '" );
    sLog += ::rtl::OString( _aURL.Main.getStr(), _aURL.Main.getLength(), osl_getThreadTextEncoding() );
    sLog += ::rtl::OString( "' )" );
    RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "dbaccess", "frank.schoenheit@sun.com", sLog.getStr() );
#endif

    executeChecked(_aURL,aArgs);
}

// -----------------------------------------------------------------------
void OGenericUnoController::addStatusListener(const Reference< XStatusListener > & aListener, const URL& _rURL) throw(RuntimeException)
{
    // parse the ULR now and here, this saves later parsing in each notification round
    URL aParsedURL( _rURL );
    if ( m_xUrlTransformer.is() )
        m_xUrlTransformer->parseStrict( aParsedURL );

    // remeber the listener together with the URL
    m_arrStatusListener.insert( m_arrStatusListener.end(), DispatchTarget( aParsedURL, aListener ) );

    // initially broadcast the state
    ImplBroadcastFeatureState( aParsedURL.Complete, aListener, sal_True );
        // force the new state to be broadcasted to the new listener
}

// -----------------------------------------------------------------------
void OGenericUnoController::removeStatusListener(const Reference< XStatusListener > & aListener, const URL& _rURL) throw(RuntimeException)
{
    DispatchIterator iterSearch = m_arrStatusListener.begin();

    sal_Bool bRemoveForAll = (_rURL.Complete.getLength() == 0);
    while ( iterSearch != m_arrStatusListener.end() )
    {
        DispatchTarget& rCurrent = *iterSearch;
        if	(	(rCurrent.xListener == aListener)
            &&	(	bRemoveForAll
                ||	(rCurrent.aURL.Complete.equals(_rURL.Complete))
                )
            )
        {
            m_arrStatusListener.erase( iterSearch );
            if (!bRemoveForAll)
                // remove the listener only for the given URL, so we can exit the loop after deletion
                break;
        }
        else
            ++iterSearch;
    }

    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::removeStatusListener: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find(_rURL.Complete);
    if (aIter != m_aSupportedFeatures.end())
    {	// clear the cache for that feature
        StateCacheIterator aCachePos = m_aStateCache.find( aIter->second.nFeatureId );
        if ( aCachePos != m_aStateCache.end() )
            m_aStateCache.erase( aCachePos );
    }

    // now remove the listener from the deque
    ::osl::MutexGuard aGuard( m_aFeatureMutex );
    m_aFeaturesToInvalidate.erase(
        ::std::remove_if(	m_aFeaturesToInvalidate.begin(),
                            m_aFeaturesToInvalidate.end(),
                            ::std::bind2nd(FindFeatureListener(),aListener))
        ,m_aFeaturesToInvalidate.end());
}
// -----------------------------------------------------------------------------
void OGenericUnoController::releaseNumberForComponent()
{
    try
    {
        Reference< XUntitledNumbers > xUntitledProvider(getPrivateModel(), UNO_QUERY      );
        if ( xUntitledProvider.is() )
            xUntitledProvider->releaseNumberForComponent(static_cast<XWeak*>(this));
    }
    catch( const Exception& )
    {
        // NII
    }
}
// -----------------------------------------------------------------------
void OGenericUnoController::disposing()
{
    {
        EventObject aDisposeEvent;
        aDisposeEvent.Source = static_cast<XWeak*>(this);
        Dispatch aStatusListener = m_arrStatusListener;
        Dispatch::iterator aEnd = aStatusListener.end();
        for (Dispatch::iterator aIter = aStatusListener.begin(); aIter != aEnd; ++aIter)
        {		
            aIter->xListener->disposing(aDisposeEvent);
        }
        m_arrStatusListener.clear();
    }

    m_xDatabaseContext = NULL;
    {
        ::osl::MutexGuard aGuard( m_aFeatureMutex);
        m_aAsyncInvalidateAll.CancelCall();
        m_aFeaturesToInvalidate.clear();
    }

    releaseNumberForComponent();

    // check out from all the objects we are listening
    // the frame
    stopFrameListening( m_aCurrentFrame.getFrame() );
    m_aCurrentFrame.attachFrame( NULL );

    m_xMasterDispatcher = NULL;
    m_xSlaveDispatcher = NULL;
    m_xServiceFactory = NULL;
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::addEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    // disambiguate
    OGenericUnoController_Base::WeakComponentImplHelperBase::addEventListener( xListener );
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::removeEventListener( const Reference< XEventListener >& xListener ) throw (RuntimeException)
{
    // disambiguate
    OGenericUnoController_Base::WeakComponentImplHelperBase::removeEventListener( xListener );
}

//------------------------------------------------------------------------------
void OGenericUnoController::frameAction(const FrameActionEvent& aEvent) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( aEvent.Frame == m_aCurrentFrame.getFrame() )
        m_aCurrentFrame.frameAction( aEvent.Action );
}

//------------------------------------------------------------------------------
void OGenericUnoController::implDescribeSupportedFeature( const sal_Char* _pAsciiCommandURL,
        sal_uInt16 _nFeatureId, sal_Int16 _nCommandGroup )
{
#ifdef DBG_UTIL
    DBG_ASSERT( m_bDescribingSupportedFeatures, "OGenericUnoController::implDescribeSupportedFeature: bad timing for this call!" );
#endif
    OSL_PRECOND( _nFeatureId < FIRST_USER_DEFINED_FEATURE, "OGenericUnoController::implDescribeSupportedFeature: invalid feature id!" );

    ControllerFeature aFeature;
    aFeature.Command = ::rtl::OUString::createFromAscii( _pAsciiCommandURL );
    aFeature.nFeatureId = _nFeatureId;
    aFeature.GroupId = _nCommandGroup;

#if OSL_DEBUG_LEVEL > 0
    OSL_ENSURE( m_aSupportedFeatures.find( aFeature.Command ) == m_aSupportedFeatures.end(),
        "OGenericUnoController::implDescribeSupportedFeature: this feature is already there!" );
#endif
    m_aSupportedFeatures[ aFeature.Command ] = aFeature;
}

//------------------------------------------------------------------------------
void OGenericUnoController::describeSupportedFeatures()
{
    // add all supported features
    implDescribeSupportedFeature( ".uno:Copy", ID_BROWSER_COPY, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Cut", ID_BROWSER_CUT, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:Paste", ID_BROWSER_PASTE, CommandGroup::EDIT );
    implDescribeSupportedFeature( ".uno:ClipboardFormatItems", ID_BROWSER_CLIPBOARD_FORMAT_ITEMS );
    implDescribeSupportedFeature( ".uno:DSBEditDoc", ID_BROWSER_EDITDOC, CommandGroup::DOCUMENT );
}

//------------------------------------------------------------------------------
FeatureState OGenericUnoController::GetState( sal_uInt16 _nId ) const
{
    FeatureState aReturn;
        // (disabled automatically)

    switch ( _nId )
    {
        case ID_BROWSER_UNDO:
        case ID_BROWSER_SAVEDOC:
            aReturn.bEnabled = sal_True;
            break;
        default:
            aReturn = m_pData->m_aUserDefinedFeatures.getState( getURLForId( _nId ) );
            break;
    }

    return aReturn;
}

//------------------------------------------------------------------------------
void OGenericUnoController::Execute( sal_uInt16 _nId, const Sequence< PropertyValue>& _rArgs )
{
    OSL_ENSURE( isUserDefinedFeature( _nId ),
        "OGenericUnoController::Execute: responsible for user defined features only!" );

    // user defined features can be handled by dispatch interceptors resp. protocol handlers only.
    // So, we need to do a queryDispatch, and dispatch the URL
    m_pData->m_aUserDefinedFeatures.execute( getURLForId( _nId ), _rArgs );
}

//------------------------------------------------------------------------------
URL OGenericUnoController::getURLForId(sal_Int32 _nId) const
{
    URL aReturn;
    if ( m_xUrlTransformer.is() )
    {
        SupportedFeatures::const_iterator aIter = ::std::find_if(
            m_aSupportedFeatures.begin(),
            m_aSupportedFeatures.end(),
            ::std::bind2nd( CompareFeatureById(), _nId )
        );

        if ( m_aSupportedFeatures.end() != aIter && aIter->first.getLength() )
        {
            aReturn.Complete = aIter->first;
            m_xUrlTransformer->parseStrict( aReturn );
        }
    }
    return aReturn;
}

//-------------------------------------------------------------------------
bool OGenericUnoController::isUserDefinedFeature( const sal_uInt16 _nFeatureId ) const
{
    return ( _nFeatureId >= FIRST_USER_DEFINED_FEATURE ) && ( _nFeatureId < LAST_USER_DEFINED_FEATURE );
}

//-------------------------------------------------------------------------
bool OGenericUnoController::isUserDefinedFeature( const ::rtl::OUString& _rFeatureURL ) const
{
    SupportedFeatures::const_iterator pos = m_aSupportedFeatures.find( _rFeatureURL );
    OSL_PRECOND( pos != m_aSupportedFeatures.end(),
        "OGenericUnoController::isUserDefinedFeature: this is no supported feature at all!" );

    return ( pos != m_aSupportedFeatures.end() ) ? isUserDefinedFeature( pos->second.nFeatureId ) : false;
}

//-------------------------------------------------------------------------
sal_Bool SAL_CALL OGenericUnoController::supportsService(const ::rtl::OUString& ServiceName) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aSupported(getSupportedServiceNames());

    const ::rtl::OUString* pArray = aSupported.getConstArray();
    const ::rtl::OUString* pArrayEnd = aSupported.getConstArray() + aSupported.getLength();

    for ( ;( pArray != pArrayEnd ) && !pArray->equals( ServiceName ); ++pArray )
        ;
    return pArray != pArrayEnd;
}

// -----------------------------------------------------------------------------
void OGenericUnoController::startConnectionListening(const Reference< XConnection >& _rxConnection)
{
    // we have to remove ourself before dispoing the connection
    Reference< XComponent >  xComponent(_rxConnection, UNO_QUERY);
    if (xComponent.is())
        xComponent->addEventListener(static_cast<XFrameActionListener*>(this));
}

// -----------------------------------------------------------------------------
void OGenericUnoController::stopConnectionListening(const Reference< XConnection >& _rxConnection)
{
    // we have to remove ourself before dispoing the connection
    Reference< XComponent >  xComponent(_rxConnection, UNO_QUERY);
    if (xComponent.is())
        xComponent->removeEventListener(static_cast<XFrameActionListener*>(this));
}
// -----------------------------------------------------------------------------
Reference< XConnection > OGenericUnoController::connect( const Reference< XDataSource>& _xDataSource,
    ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    WaitObject aWaitCursor( getView() );

    ODatasourceConnector aConnector( getORB(), getView(), ::rtl::OUString() );
    Reference< XConnection > xConnection = aConnector.connect( _xDataSource, _pErrorInfo );
    startConnectionListening( xConnection );

    return xConnection;
}
// -----------------------------------------------------------------------------
Reference< XConnection > OGenericUnoController::connect( const ::rtl::OUString& _rDataSourceName,
    const ::rtl::OUString& _rContextInformation, ::dbtools::SQLExceptionInfo* _pErrorInfo )
{
    WaitObject aWaitCursor( getView() );

    ODatasourceConnector aConnector( getORB(), getView(), _rContextInformation );
    Reference<XConnection> xConnection = aConnector.connect( _rDataSourceName, _pErrorInfo );
    startConnectionListening( xConnection );

    return xConnection;
}

// -----------------------------------------------------------------------------
void OGenericUnoController::showError(const SQLExceptionInfo& _rInfo)
{
    ::dbaui::showError(_rInfo,getView(),getORB());
}
// -----------------------------------------------------------------------------
Reference< XLayoutManager > OGenericUnoController::getLayoutManager(const Reference< XFrame >& _xFrame) const
{
    Reference< XPropertySet > xPropSet( _xFrame, UNO_QUERY );
    Reference< XLayoutManager > xLayoutManager;
    if ( xPropSet.is() )
    {
        try
        {
            xLayoutManager.set(xPropSet->getPropertyValue( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "LayoutManager" ))),UNO_QUERY);
        }
        catch ( Exception& )
        {
        }
    }
    return xLayoutManager;
}
// -----------------------------------------------------------------------------
void OGenericUnoController::loadMenu(const Reference< XFrame >& _xFrame)
{
    Reference< XLayoutManager > xLayoutManager = getLayoutManager(_xFrame);
    if ( xLayoutManager.is() )
    {
        xLayoutManager->lock();
        xLayoutManager->createElement( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/menubar/menubar" )));
        xLayoutManager->createElement( rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "private:resource/toolbar/toolbar" )));
        xLayoutManager->unlock();
        xLayoutManager->doLayout();
    }

    onLoadedMenu( xLayoutManager );
}

// -----------------------------------------------------------------------------
void OGenericUnoController::onLoadedMenu(const Reference< XLayoutManager >& /*_xLayoutManager*/)
{
    // not interested in
}

// -----------------------------------------------------------------------------
void OGenericUnoController::closeTask()
{
    m_aAsyncCloseTask.Call();
}
// -----------------------------------------------------------------------------
IMPL_LINK(OGenericUnoController, OnAsyncCloseTask, void*, EMPTYARG)
{
    if ( !OGenericUnoController_Base::rBHelper.bInDispose )
    {
        try
        {
            Reference< util::XCloseable > xCloseable( m_aCurrentFrame.getFrame(), UNO_QUERY_THROW );
            xCloseable->close( sal_False ); // false - holds the owner ship for this frame inside this object!
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
    return 0L;
}
// -----------------------------------------------------------------------------
Any SAL_CALL OGenericUnoController::getViewData(void) throw( RuntimeException )
{
    return Any();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::restoreViewData(const Any& /*Data*/) throw( RuntimeException )
{
}

// -----------------------------------------------------------------------------
Reference< XModel > SAL_CALL OGenericUnoController::getModel(void) throw( RuntimeException )
{
    return Reference< XModel >();
}

// -----------------------------------------------------------------------------
Reference< XFrame > SAL_CALL OGenericUnoController::getFrame(void) throw( RuntimeException )
{
    ::osl::MutexGuard aGuard( getMutex() );
    return m_aCurrentFrame.getFrame();
}

// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OGenericUnoController::attachModel(const Reference< XModel > & /*xModel*/) throw( RuntimeException )
{
    OSL_ENSURE( false, "OGenericUnoController::attachModel: not supported!" );
    return sal_False;
}

// -----------------------------------------------------------------------------
void OGenericUnoController::executeUnChecked(sal_uInt16 _nCommandId, const Sequence< PropertyValue >& aArgs)
{
    Execute(_nCommandId, aArgs);
}
// -----------------------------------------------------------------------------
void OGenericUnoController::executeUnChecked(const util::URL& _rCommand, const Sequence< PropertyValue >& aArgs)
{
    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::executeUnChecked: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCommand.Complete );
    if (aIter != m_aSupportedFeatures.end())
        Execute( aIter->second.nFeatureId, aArgs );
}
// -----------------------------------------------------------------------------
void OGenericUnoController::executeChecked(const util::URL& _rCommand, const Sequence< PropertyValue >& aArgs)
{
    OSL_PRECOND( !m_aSupportedFeatures.empty(), "OGenericUnoController::executeChecked: shouldn't this be filled at construction time?" );
    if ( m_aSupportedFeatures.empty() )
        fillSupportedFeatures();

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCommand.Complete );
    if ( aIter != m_aSupportedFeatures.end() )
    {
        sal_uInt16 nFeatureId = aIter->second.nFeatureId;
        if ( GetState( nFeatureId ).bEnabled )
            Execute( nFeatureId, aArgs );
    }
}
// -----------------------------------------------------------------------------
//------------------------------------------------------------------------------
namespace
{
    ::rtl::OUString lcl_getModuleHelpModuleName( const Reference< XFrame >& _rxFrame )
    {
        const sal_Char* pReturn = NULL;

        try
        {
            // get the model of the document in the given frame
            Reference< XController > xController;
            if ( _rxFrame.is() )
                xController = _rxFrame->getController();
            Reference< XModel > xModel;
            if ( xController.is() )
                xModel = xController->getModel();
            Reference< XServiceInfo > xSI( xModel, UNO_QUERY );

            if ( !xSI.is() )
            {	// try to go up the frame hierarchy

                Reference< XFrame > xParentFrame;
                if ( _rxFrame.is() )
                    xParentFrame = xParentFrame.query( _rxFrame->getCreator() );
                // did we find a parent frame? Which is no top-level frame?
                if ( xParentFrame.is() && !_rxFrame->isTop() )
                    // TODO: to prevent framework assertions, re-insert this "isTop" once 98303 is fixed
                    return lcl_getModuleHelpModuleName( xParentFrame );
            }
            else
            {
#if OSL_DEBUG_LEVEL > 0
                Sequence< ::rtl::OUString > sServiceNames = xSI->getSupportedServiceNames();
                const ::rtl::OUString* pLoop = sServiceNames.getConstArray();
                for ( sal_Int32 i=0; i<sServiceNames.getLength(); ++i, ++pLoop )
                {
                    sal_Int32 nDummy = 0;
                    (void)nDummy;
                }
#endif

                // check which service we know ....
                static const sal_Char* pTransTable[] = {
                    "com.sun.star.sdb.OfficeDatabaseDocument","sdatabase",
                    "com.sun.star.report.ReportDefinition","sdatabase",
                    "com.sun.star.text.TextDocument",	"swriter",
                    "com.sun.star.sheet.SpreadsheetDocument", "scalc",
                    "com.sun.star.presentation.PresentationDocument", "simpress",
                    "com.sun.star.drawing.DrawingDocument", "sdraw",
                    "com.sun.star.formula.FormularProperties", "smath",
                    "com.sun.star.chart.ChartDocument", "schart"
                };
                OSL_ENSURE( ( sizeof( pTransTable ) / sizeof( pTransTable[0] ) ) % 2 == 0,
                    "lcl_getModuleHelpModuleName: odd size of translation table!" );

                // loop through the table
                sal_Int32 nTableEntries = ( sizeof( pTransTable ) / sizeof( pTransTable[0] ) ) / 2;
                const sal_Char** pDocumentService = pTransTable;
                const sal_Char** pHelpModuleName = pTransTable + 1;
                for ( sal_Int32 j=0; j<nTableEntries; ++j )
                {
                    if ( xSI->supportsService( ::rtl::OUString::createFromAscii( *pDocumentService ) ) )
                    {	// found a table entry which matches the model's services
                        pReturn = *pHelpModuleName;
                        break;
                    }

                    ++pDocumentService; ++pDocumentService;
                    ++pHelpModuleName; ++pHelpModuleName;
                }
            }

            if ( !pReturn )
            {
                // could not determine the document type we're living in
                // ->fallback
                SvtModuleOptions aModOpt;
                if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SWRITER ) )
                    pReturn = "swriter";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDATABASE ) )
                    pReturn = "sdatabase";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCALC ) )
                    pReturn = "scalc";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SIMPRESS ) )
                    pReturn = "simpress";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SDRAW ) )
                    pReturn = "sdraw";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SMATH ) )
                    pReturn = "smath";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SCHART ) )
                    pReturn = "schart";
                else if ( aModOpt.IsModuleInstalled( SvtModuleOptions::E_SBASIC ) )
                    pReturn = "sbasic";
                else
                {
                    OSL_ENSURE( sal_False, "lcl_getModuleHelpModuleName: no installed module found" );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }

        if ( !pReturn )
            pReturn = "swriter";

        return ::rtl::OUString::createFromAscii( pReturn );
    }
}

// -----------------------------------------------------------------------------

void OGenericUnoController::openHelpAgent(rtl::OUString const& _suHelpStringURL )
{
    rtl::OUString suURL(_suHelpStringURL);
    rtl::OUString sLanguage = rtl::OUString::createFromAscii("Language=");
    if (suURL.indexOf(sLanguage) == -1)
    {
        AppendConfigToken(suURL, sal_False /* sal_False := add '&' */ );
    }
    URL aURL;
    aURL.Complete = suURL;
    
    openHelpAgent( aURL );
}

void OGenericUnoController::openHelpAgent(sal_Int32 _nHelpId)
{
    openHelpAgent( createHelpAgentURL( lcl_getModuleHelpModuleName( getFrame() ), _nHelpId ) );
}

void OGenericUnoController::openHelpAgent( const URL& _rURL )
{
    try
    {
        URL aURL( _rURL );

        if ( m_xUrlTransformer.is() )
            m_xUrlTransformer->parseStrict(aURL);

        Reference< XDispatchProvider > xDispProv( m_aCurrentFrame.getFrame(), UNO_QUERY );
        Reference< XDispatch > xHelpDispatch;
        if ( xDispProv.is() )
            xHelpDispatch = xDispProv->queryDispatch(aURL, ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("_helpagent")), FrameSearchFlag::PARENT | FrameSearchFlag::SELF);
        OSL_ENSURE(xHelpDispatch.is(), "SbaTableQueryBrowser::openHelpAgent: could not get a dispatcher!");
        if (xHelpDispatch.is())
        {
            xHelpDispatch->dispatch(aURL, Sequence< PropertyValue >());
        }
    }
    catch( const Exception& )
    {
        DBG_UNHANDLED_EXCEPTION();
    }
}
// -----------------------------------------------------------------------------
Reference< awt::XWindow> OGenericUnoController::getTopMostContainerWindow() const
{
    Reference< ::com::sun::star::awt::XWindow> xWindow;

    // get the top most window
    Reference< XFrame > xFrame( m_aCurrentFrame.getFrame() );
    if ( xFrame.is() )
    {
        xWindow = xFrame->getContainerWindow();

        while ( xFrame.is() && !xFrame->isTop() )
        {
            xFrame.set( xFrame->getCreator(), UNO_QUERY );
        }
        if ( xFrame.is() )
            xWindow = xFrame->getContainerWindow();
    }
    return xWindow;
}
// -----------------------------------------------------------------------------
Reference< XTitle > OGenericUnoController::impl_getTitleHelper_throw()
{
    ::vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( getMutex() );

    if ( ! m_xTitleHelper.is ())
    {
        Reference< XUntitledNumbers > xUntitledProvider(getPrivateModel(), UNO_QUERY      );
        Reference< XController >      xThis(static_cast< XController* >(this), UNO_QUERY_THROW);
    
        ::framework::TitleHelper* pHelper = new ::framework::TitleHelper(m_xServiceFactory);
        m_xTitleHelper.set( static_cast< ::cppu::OWeakObject* >(pHelper), UNO_QUERY_THROW);
    
        pHelper->setOwner                   (xThis            );
        pHelper->connectWithUntitledNumbers (xUntitledProvider);
    }

    return m_xTitleHelper;
}

//=============================================================================
// XTitle
::rtl::OUString SAL_CALL OGenericUnoController::getTitle()
    throw (RuntimeException)
{
    ::osl::MutexGuard aGuard( getMutex() );
    if ( m_bExternalTitle )
        return impl_getTitleHelper_throw()->getTitle ();
    return getPrivateTitle() + impl_getTitleHelper_throw()->getTitle ();
}

//=============================================================================
// XTitle
void SAL_CALL OGenericUnoController::setTitle(const ::rtl::OUString& sTitle)
    throw (RuntimeException)
{
    vos::OGuard aSolarGuard( Application::GetSolarMutex() );
    ::osl::MutexGuard aGuard( getMutex() );
    m_bExternalTitle = sal_True;
    impl_getTitleHelper_throw()->setTitle (sTitle);
}
    
//=============================================================================
// XTitleChangeBroadcaster
void SAL_CALL OGenericUnoController::addTitleChangeListener(const Reference< XTitleChangeListener >& xListener)
    throw (RuntimeException)
{
    Reference< XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->addTitleChangeListener (xListener);
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::removeTitleChangeListener(const Reference< XTitleChangeListener >& xListener)
    throw (RuntimeException)
{
    Reference< XTitleChangeBroadcaster > xBroadcaster(impl_getTitleHelper_throw(), UNO_QUERY);
    if (xBroadcaster.is ())
        xBroadcaster->removeTitleChangeListener (xListener);
}

// =============================================================================
// XUserInputInterception
// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::addKeyHandler( const Reference< XKeyHandler >& _rxHandler ) throw (RuntimeException)
{
    if ( _rxHandler.is() )
        m_pData->m_aUserInputInterception.addKeyHandler( _rxHandler );
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::removeKeyHandler( const Reference< XKeyHandler >& _rxHandler ) throw (RuntimeException)
{
    m_pData->m_aUserInputInterception.removeKeyHandler( _rxHandler );
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::addMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler ) throw (RuntimeException)
{
    if ( _rxHandler.is() )
        m_pData->m_aUserInputInterception.addMouseClickHandler( _rxHandler );
}

// -----------------------------------------------------------------------------
void SAL_CALL OGenericUnoController::removeMouseClickHandler( const Reference< XMouseClickHandler >& _rxHandler ) throw (RuntimeException)
{
    m_pData->m_aUserInputInterception.removeMouseClickHandler( _rxHandler );
}

// =============================================================================
// -----------------------------------------------------------------------------
void OGenericUnoController::executeChecked(sal_uInt16 _nCommandId, const Sequence< PropertyValue >& aArgs)
{
    if ( isCommandEnabled(_nCommandId) )
        Execute(_nCommandId, aArgs);
}

// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::isCommandEnabled(sal_uInt16 _nCommandId) const
{
    return GetState( _nCommandId ).bEnabled;
}

// -----------------------------------------------------------------------------
sal_uInt16 OGenericUnoController::registerCommandURL( const ::rtl::OUString& _rCompleteCommandURL )
{
    if ( !_rCompleteCommandURL.getLength() )
        return 0;

    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCompleteCommandURL );
    if ( aIter != m_aSupportedFeatures.end() )
        return aIter->second.nFeatureId;

    // this is a previously unkwnon command
    sal_uInt16 nFeatureId = FIRST_USER_DEFINED_FEATURE;
    while ( isFeatureSupported( nFeatureId ) && ( nFeatureId < LAST_USER_DEFINED_FEATURE ) )
        ++nFeatureId;
    if ( nFeatureId == LAST_USER_DEFINED_FEATURE )
    {
        OSL_ENSURE( false, "OGenericUnoController::registerCommandURL: no more space for user defined features!" );
        return 0L;
    }

    ControllerFeature aFeature;
    aFeature.Command = _rCompleteCommandURL;
    aFeature.nFeatureId = nFeatureId;
    aFeature.GroupId = CommandGroup::INTERNAL;
    m_aSupportedFeatures[ aFeature.Command ] = aFeature;

    return nFeatureId;
}

// -----------------------------------------------------------------------------
void OGenericUnoController::notifyHiContrastChanged()
{
}

// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::isDataSourceReadOnly() const
{
    return sal_False;
}

// -----------------------------------------------------------------------------
Reference< XController > OGenericUnoController::getXController() throw( RuntimeException )
{
    return this;
}

// -----------------------------------------------------------------------------
bool OGenericUnoController::interceptUserInput( const NotifyEvent& _rEvent )
{
    return m_pData->m_aUserInputInterception.handleNotifyEvent( _rEvent );
}

// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::isCommandChecked(sal_uInt16 _nCommandId) const
{
    FeatureState aState = GetState( _nCommandId );
    
    return aState.bChecked && (sal_Bool)*aState.bChecked;
}
// -----------------------------------------------------------------------------
sal_Bool OGenericUnoController::isCommandEnabled( const ::rtl::OUString& _rCompleteCommandURL ) const
{
    OSL_ENSURE( _rCompleteCommandURL.getLength(), "OGenericUnoController::isCommandEnabled: Empty command url!" );
    
    sal_Bool bIsEnabled = sal_False;
    SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.find( _rCompleteCommandURL );
    if ( aIter != m_aSupportedFeatures.end() )
        bIsEnabled = isCommandEnabled( aIter->second.nFeatureId );

    return bIsEnabled;
}

// -----------------------------------------------------------------------------
Sequence< ::sal_Int16 > SAL_CALL OGenericUnoController::getSupportedCommandGroups() throw (RuntimeException)
{
    CommandHashMap aCmdHashMap;
    for (   SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
            aIter != m_aSupportedFeatures.end();
            ++aIter
        )
        if ( aIter->second.GroupId != CommandGroup::INTERNAL )
            aCmdHashMap.insert( CommandHashMap::value_type( aIter->second.GroupId, 0 ));

    Sequence< sal_Int16 > aCommandGroups( aCmdHashMap.size() );
    ::std::transform( aCmdHashMap.begin(),
        aCmdHashMap.end(),
        aCommandGroups.getArray(),
        ::std::select1st< CommandHashMap::value_type >()
    );

    return aCommandGroups;
}

// -----------------------------------------------------------------------------
Sequence< DispatchInformation > SAL_CALL OGenericUnoController::getConfigurableDispatchInformation( ::sal_Int16 CommandGroup ) throw (RuntimeException)
{
    DispatchInfoList    aInformationList;
    DispatchInformation aDispatchInfo;
    for (   SupportedFeatures::const_iterator aIter = m_aSupportedFeatures.begin();
            aIter != m_aSupportedFeatures.end();
            ++aIter
        )
    {
        if ( sal_Int16( aIter->second.GroupId ) == CommandGroup )
        {
            aDispatchInfo = aIter->second;
            aInformationList.push_back( aDispatchInfo );
        }
    }

    Sequence< DispatchInformation > aInformation( aInformationList.size() );
    ::std::transform( aInformationList.begin(),
        aInformationList.end(),
        aInformation.getArray(),
        ::std::identity< DispatchInformation >()
    );

    return aInformation;
}
// -----------------------------------------------------------------------------
void OGenericUnoController::fillSupportedFeatures()
{
#ifdef DBG_UTIL
    m_bDescribingSupportedFeatures = true;
#endif
    describeSupportedFeatures();
// -----------------------------------------------------------------------------
#ifdef DBG_UTIL
    m_bDescribingSupportedFeatures = false;
#endif
}


void SAL_CALL OGenericUnoController::dispose() throw(::com::sun::star::uno::RuntimeException)
{
    ::vos::OGuard aSolarGuard(Application::GetSolarMutex());
    OGenericUnoController_Base::dispose();
}
}   // namespace dbaui

