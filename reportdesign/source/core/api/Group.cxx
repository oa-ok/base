/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Group.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:14 $
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
#ifndef REPORTDESIGN_API_GROUP_HXX
#include "Group.hxx"
#endif
#ifndef REPORTDESIGN_API_SECTION_HXX
#include "Section.hxx"
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_GROUPON_HPP_
#include <com/sun/star/report/GroupOn.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_KEEPTOGETHER_HPP_
#include <com/sun/star/report/KeepTogether.hpp>
#endif
#ifndef REPORTDESIGN_SHARED_CORESTRINGS_HRC
#include "corestrings.hrc"
#endif
#ifndef REPORTDESIGN_CORE_RESOURCE_HRC_
#include "core_resource.hrc"
#endif
#ifndef REPORTDESIGN_CORE_RESOURCE_HXX_
#include "core_resource.hxx"
#endif
#ifndef REPORTDESIGN_TOOLS_HXX
#include "Tools.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_PROPERTY_HXX_
#include <comphelper/property.hxx>
#endif
#ifndef REPORTDESIGN_API_FUNCTIONS_HXX
#include "Functions.hxx"
#endif

// =============================================================================
namespace reportdesign
{
// =============================================================================
    using namespace com::sun::star;
    using namespace comphelper;
DBG_NAME( rpt_OGroup )
// -----------------------------------------------------------------------------
OGroup::OGroup(const uno::Reference< report::XGroups >& _xParent
               ,const uno::Reference< uno::XComponentContext >& _xContext)
:GroupBase(m_aMutex)
,GroupPropertySet(_xContext,static_cast< GroupPropertySet::Implements >(IMPLEMENTS_PROPERTY_SET),uno::Sequence< ::rtl::OUString >())
,m_xContext(_xContext)
,m_xParent(_xParent)
{
    DBG_CTOR( rpt_OGroup,NULL);
    osl_incrementInterlockedCount(&m_refCount);
    {
        m_xFunctions = new OFunctions(this,m_xContext);
    }
    osl_decrementInterlockedCount( &m_refCount );
}
//--------------------------------------------------------------------------
OGroup::~OGroup()
{
    DBG_DTOR( rpt_OGroup,NULL);
}
//--------------------------------------------------------------------------
void OGroup::copyGroup(const uno::Reference< report::XGroup >& _xSource)
{
    ::comphelper::copyProperties(_xSource.get(),static_cast<GroupPropertySet*>(this));

    if ( _xSource->getHeaderOn() )
    {
        setHeaderOn(sal_True);
        OSection::lcl_copySection(_xSource->getHeader(),m_xHeader);
    } // if ( _xSource->getHeaderOn() )

    if ( _xSource->getFooterOn() )
    {
        setFooterOn(sal_True);
        OSection::lcl_copySection(_xSource->getFooter(),m_xFooter);
    } // if ( _xSource->getFooterOn() )
}
//--------------------------------------------------------------------------
IMPLEMENT_FORWARD_XINTERFACE2(OGroup,GroupBase,GroupPropertySet)
//--------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGroup::getImplementationName(  ) throw(uno::RuntimeException)
{
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.report.Group"));
}
//------------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString> OGroup::getSupportedServiceNames_Static(void) throw( uno::RuntimeException )
{
    uno::Sequence< ::rtl::OUString> aSupported(1);
    aSupported.getArray()[0] = SERVICE_GROUP;
    return aSupported;
}
//-------------------------------------------------------------------------
uno::Sequence< ::rtl::OUString> SAL_CALL OGroup::getSupportedServiceNames() throw(uno::RuntimeException)
{
    return getSupportedServiceNames_Static();
}
// -----------------------------------------------------------------------------
sal_Bool SAL_CALL OGroup::supportsService( const ::rtl::OUString& _rServiceName ) throw(uno::RuntimeException)
{
    return ::comphelper::existsValue(_rServiceName,getSupportedServiceNames_Static());
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::dispose() throw(uno::RuntimeException) 
{
    GroupPropertySet::dispose();
    cppu::WeakComponentImplHelperBase::dispose(); 
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::disposing()
{
    ::comphelper::disposeComponent(m_xHeader);
    ::comphelper::disposeComponent(m_xFooter);
    ::comphelper::disposeComponent(m_xFunctions);
    m_xContext.clear();
}
// -----------------------------------------------------------------------------
// XGroup
::sal_Bool SAL_CALL OGroup::getSortAscending() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_eSortAscending;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setSortAscending( ::sal_Bool _sortascending ) throw (uno::RuntimeException)
{
    set(PROPERTY_SORTASCENDING,_sortascending,m_aProps.m_eSortAscending);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroup::getHeaderOn() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xHeader.is();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setHeaderOn( ::sal_Bool _headeron ) throw (uno::RuntimeException)
{
    if ( _headeron != m_xHeader.is() )
    {
        ::rtl::OUString sName(RPT_RESSTRING(RID_STR_GROUP_HEADER,m_xContext->getServiceManager()));
        /*const ::rtl::OUString sPlaceHolder(RTL_CONSTASCII_USTRINGPARAM("%1"));
        sName = sName.replaceAt(sName.indexOf(sPlaceHolder),sPlaceHolder.getLength(),m_aProps.m_sExpression);*/
        setSection(PROPERTY_HEADERON,_headeron,sName,m_xHeader);
    }
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroup::getFooterOn() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_xFooter.is();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setFooterOn( ::sal_Bool _footeron ) throw (uno::RuntimeException)
{
    if ( _footeron != m_xFooter.is() )
    {
        ::rtl::OUString sName(RPT_RESSTRING(RID_STR_GROUP_FOOTER,m_xContext->getServiceManager()));
        /*const ::rtl::OUString sPlaceHolder(RTL_CONSTASCII_USTRINGPARAM("%1"));
        sName = sName.replaceAt(sName.indexOf(sPlaceHolder),sPlaceHolder.getLength(),m_aProps.m_sExpression);*/
        setSection(PROPERTY_FOOTERON,_footeron,sName,m_xFooter);
    }
}
// -----------------------------------------------------------------------------
uno::Reference< report::XSection > SAL_CALL OGroup::getHeader() throw (container::NoSuchElementException, uno::RuntimeException)
{
    uno::Reference< report::XSection > xRet;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        xRet = m_xHeader;
    }
    
    if ( !xRet.is() )
        throw container::NoSuchElementException();
    return xRet;
}
// -----------------------------------------------------------------------------
uno::Reference< report::XSection > SAL_CALL OGroup::getFooter() throw (container::NoSuchElementException, uno::RuntimeException)
{
    uno::Reference< report::XSection > xRet;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        xRet = m_xFooter;
    }
    
    if ( !xRet.is() )
        throw container::NoSuchElementException();
    return xRet;
}
// -----------------------------------------------------------------------------
::sal_Int16 SAL_CALL OGroup::getGroupOn() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nGroupOn;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setGroupOn( ::sal_Int16 _groupon ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( _groupon < report::GroupOn::DEFAULT || _groupon > report::GroupOn::INTERVAL )
        throwIllegallArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com::sun::star::report::GroupOn"))
                        ,*this
                        ,1
                        ,m_xContext);
    set(PROPERTY_GROUPON,_groupon,m_aProps.m_nGroupOn);
}
// -----------------------------------------------------------------------------
::sal_Int32 SAL_CALL OGroup::getGroupInterval() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nGroupInterval;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setGroupInterval( ::sal_Int32 _groupinterval ) throw (uno::RuntimeException)
{
    set(PROPERTY_GROUPINTERVAL,_groupinterval,m_aProps.m_nGroupInterval);
}
// -----------------------------------------------------------------------------
::sal_Int16 SAL_CALL OGroup::getKeepTogether() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_nKeepTogether;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setKeepTogether( ::sal_Int16 _keeptogether ) throw (lang::IllegalArgumentException, uno::RuntimeException)
{
    if ( _keeptogether < report::KeepTogether::NO || _keeptogether > report::KeepTogether::WITH_FIRST_DETAIL )
        throwIllegallArgumentException(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com::sun::star::report::KeepTogether"))
                        ,*this
                        ,1
                        ,m_xContext);
    set(PROPERTY_KEEPTOGETHER,_keeptogether,m_aProps.m_nKeepTogether);
}
// -----------------------------------------------------------------------------
uno::Reference< report::XGroups > SAL_CALL OGroup::getGroups() throw (uno::RuntimeException)
{
    return m_xParent;
}
// -----------------------------------------------------------------------------
::rtl::OUString SAL_CALL OGroup::getExpression() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_sExpression;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setExpression( const ::rtl::OUString& _expression ) throw (uno::RuntimeException)
{
    set(PROPERTY_EXPRESSION,_expression,m_aProps.m_sExpression);
}
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroup::getStartNewColumn() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_bStartNewColumn;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setStartNewColumn( ::sal_Bool _startnewcolumn ) throw (uno::RuntimeException)
{
    set(PROPERTY_STARTNEWCOLUMN,_startnewcolumn,m_aProps.m_bStartNewColumn);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
::sal_Bool SAL_CALL OGroup::getResetPageNumber() throw (uno::RuntimeException)
{
    ::osl::MutexGuard aGuard(m_aMutex);
    return m_aProps.m_bResetPageNumber;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setResetPageNumber( ::sal_Bool _resetpagenumber ) throw (uno::RuntimeException)
{
    set(PROPERTY_RESETPAGENUMBER,_resetpagenumber,m_aProps.m_bResetPageNumber);
}
// -----------------------------------------------------------------------------
// XChild
uno::Reference< uno::XInterface > SAL_CALL OGroup::getParent(  ) throw (uno::RuntimeException)
{
    return m_xParent;
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setParent( const uno::Reference< uno::XInterface >& /*Parent*/ ) throw (lang::NoSupportException, uno::RuntimeException)
{
    throw lang::NoSupportException();
}
// -----------------------------------------------------------------------------
uno::Reference< beans::XPropertySetInfo > SAL_CALL OGroup::getPropertySetInfo(  ) throw(uno::RuntimeException)
{
    //	return uno::Reference< beans::XPropertySetInfo >();
    return GroupPropertySet::getPropertySetInfo();
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::setPropertyValue( const ::rtl::OUString& aPropertyName, const uno::Any& aValue ) throw (beans::UnknownPropertyException, beans::PropertyVetoException, lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::setPropertyValue( aPropertyName, aValue );
}
// -----------------------------------------------------------------------------
uno::Any SAL_CALL OGroup::getPropertyValue( const ::rtl::OUString& PropertyName ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    return GroupPropertySet::getPropertyValue( PropertyName);
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& xListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::addPropertyChangeListener( aPropertyName, xListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const uno::Reference< beans::XPropertyChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::removePropertyChangeListener( aPropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::addVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::addVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void SAL_CALL OGroup::removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const uno::Reference< beans::XVetoableChangeListener >& aListener ) throw (beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    GroupPropertySet::removeVetoableChangeListener( PropertyName, aListener );
}
// -----------------------------------------------------------------------------
void OGroup::setSection(	 const ::rtl::OUString& _sProperty
                            ,const sal_Bool& _bOn
                            ,const ::rtl::OUString& _sName
                            ,uno::Reference< report::XSection>& _member)
{
    BoundListeners l;
    {
        ::osl::MutexGuard aGuard(m_aMutex);
        prepareSet(_sProperty, uno::makeAny(_member), uno::makeAny(_bOn), &l);
        lcl_createSectionIfNeeded(_bOn ,this,_member);
        if ( _member.is() )
            _member->setName(_sName);
    }
    l.notify();
}
// -----------------------------------------------------------------------------
uno::Reference< report::XFunctions > SAL_CALL OGroup::getFunctions() throw (uno::RuntimeException)
{
    return m_xFunctions;
}
// =============================================================================
} // namespace reportdesign
// =============================================================================

