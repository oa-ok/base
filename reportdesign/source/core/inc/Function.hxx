#ifndef RPT_FUNCTION_HXX
#define RPT_FUNCTION_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Function.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:15 $
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

#ifndef INCLUDED_CPPUHELPER_PROPERTYSETMIXIN_HXX
#include <cppuhelper/propertysetmixin.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XFunction_HPP_
#include <com/sun/star/report/XFunction.hpp>
#endif
#ifndef _CPPUHELPER_BASEMUTEX_HXX_
#include <cppuhelper/basemutex.hxx>
#endif
#ifndef RPT_REPORTCONTROLMODEL_HXX
#include "ReportControlModel.hxx"
#endif
#ifndef _CPPUHELPER_COMPBASE2_HXX_
#include <cppuhelper/compbase2.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif

namespace reportdesign
{
    typedef ::cppu::PropertySetMixin<		 com::sun::star::report::XFunction	> FunctionPropertySet;
    typedef ::cppu::WeakComponentImplHelper2<    com::sun::star::report::XFunction
                                                ,com::sun::star::lang::XServiceInfo	> FunctionBase;

    /** \class OFunction Defines the implementation of a \interface com:::sun::star::report::XFunction
     * \ingroup reportdesign_api
     *
     */
    class OFunction :	public cppu::BaseMutex,
                            public FunctionBase,
                            public FunctionPropertySet
    {
        com::sun::star::beans::Optional< ::rtl::OUString> m_sInitialFormula;
        ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >  m_xContext;
        ::com::sun::star::uno::WeakReference< ::com::sun::star::report::XFunctions >  m_xParent;
        ::rtl::OUString m_sName;
        ::rtl::OUString m_sFormula;        
        ::sal_Bool      m_bPreEvaluated;
        ::sal_Bool      m_bDeepTraversing;
    private:
        OFunction(const OFunction&);
        OFunction& operator=(const OFunction&);

        template <typename T> void set(	 const ::rtl::OUString& _sProperty
                                        ,const T& _Value
                                        ,T& _member)
        {
            BoundListeners l;
            {
                ::osl::MutexGuard aGuard(m_aMutex);
                prepareSet(_sProperty, ::com::sun::star::uno::makeAny(_member), ::com::sun::star::uno::makeAny(_Value), &l);
                _member = _Value;
            }
            l.notify();
        }
    protected:
        virtual ~OFunction();
    public:
        explicit OFunction(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & _xContext);

        DECLARE_XINTERFACE( )
        // ::com::sun::star::lang::XServiceInfo
        virtual ::sal_Bool SAL_CALL supportsService( const ::rtl::OUString& ServiceName ) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getImplementationName(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames(  ) throw(::com::sun::star::uno::RuntimeException);

        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::rtl::OUString getImplementationName_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
            create(::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext);
        // com::sun::star::beans::XPropertySet
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySetInfo > SAL_CALL getPropertySetInfo(  ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPropertyValue( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Any& aValue ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::beans::PropertyVetoException, ::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual ::com::sun::star::uno::Any SAL_CALL getPropertyValue( const ::rtl::OUString& PropertyName ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addPropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& xListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removePropertyChangeListener( const ::rtl::OUString& aPropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertyChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL removeVetoableChangeListener( const ::rtl::OUString& PropertyName, const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XVetoableChangeListener >& aListener ) throw (::com::sun::star::beans::UnknownPropertyException, ::com::sun::star::lang::WrappedTargetException, ::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::report::XFunction:
        virtual ::sal_Bool SAL_CALL getPreEvaluated() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setPreEvaluated(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual ::sal_Bool SAL_CALL getDeepTraversing() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setDeepTraversing(::sal_Bool the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getName() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setName(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual ::rtl::OUString SAL_CALL getFormula() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setFormula(const ::rtl::OUString & the_value) throw (::com::sun::star::uno::RuntimeException);
        virtual com::sun::star::beans::Optional< ::rtl::OUString> SAL_CALL getInitialFormula() throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setInitialFormula(const com::sun::star::beans::Optional< ::rtl::OUString> & the_value) throw (::com::sun::star::uno::RuntimeException);

        // XComponent
        virtual void SAL_CALL dispose() throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL addEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException) 
        { 
            cppu::WeakComponentImplHelperBase::addEventListener(aListener);
        }
        virtual void SAL_CALL removeEventListener(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener > & aListener) throw(::com::sun::star::uno::RuntimeException)
        { 
            cppu::WeakComponentImplHelperBase::removeEventListener(aListener);
        }

        // XChild
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL getParent(  ) throw (::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL setParent( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& Parent ) throw (::com::sun::star::lang::NoSupportException, ::com::sun::star::uno::RuntimeException);
    };
}
#endif //RPT_FUNCTION_HXX

