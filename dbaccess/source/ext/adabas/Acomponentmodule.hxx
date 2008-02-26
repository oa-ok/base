/*************************************************************************
 *
 *  $RCSfile: Acomponentmodule.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2008-02-26 15:53:38 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_COMPONENT_MODULE_HXX_
#define _EXTENSIONS_COMPONENT_MODULE_HXX_

/** you may find this file helpfull if you implement a component (in it's own library) which can't use
    the usual infrastructure.<br/>
    More precise, you find helper classes to ease the use of resources and the registration of services.
    <p>
    You need to define a preprocessor variable COMPMOD_NAMESPACE in order to use this file. Set it to a string
    which should be used as namespace for the classes defined herein.</p>
*/

#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _TOOLS_RESID_HXX
#include <tools/resid.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSINGLESERVICEFACTORY_HPP_
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_REGISTRY_XREGISTRYKEY_HPP_
#include <com/sun/star/registry/XRegistryKey.hpp>
#endif
#ifndef _CPPUHELPER_FACTORY_HXX_ 
#include <cppuhelper/factory.hxx>
#endif
#ifndef _RTL_STRING_HXX_
#include <rtl/string.hxx>
#endif

class ResMgr;

//.........................................................................
namespace COMPMOD_NAMESPACE
{
//.........................................................................

typedef ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory > (SAL_CALL *FactoryInstantiation)
        (
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rServiceManager,
            const ::rtl::OUString & _rComponentName,
            ::cppu::ComponentInstantiation _pCreateFunction,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString > & _rServiceNames,
            rtl_ModuleCount* _pModuleCounter
        );

    //=========================================================================
    //= OModule
    //=========================================================================
    class OModuleImpl;
    class OModule
    {
        friend class OModuleResourceClient;

    private:
        OModule();
            // not implemented. OModule is a static class

    protected:
        // resource administration
        static ::osl::Mutex		s_aMutex;		/// access safety
        static sal_Int32		s_nClients;		/// number of registered clients
        static OModuleImpl*		s_pImpl;		/// impl class. lives as long as at least one client for the module is registered
        static ::rtl::OString	s_sResPrefix;

        // auto registration administration
        static	::com::sun::star::uno::Sequence< ::rtl::OUString >*
            s_pImplementationNames;
        static	::com::sun::star::uno::Sequence< ::com::sun::star::uno::Sequence< ::rtl::OUString > >*
            s_pSupportedServices;
        static	::com::sun::star::uno::Sequence< sal_Int64 >*
            s_pCreationFunctionPointers;
        static	::com::sun::star::uno::Sequence< sal_Int64 >*
            s_pFactoryFunctionPointers;

    public:
        // cna be set as long as no resource has been accessed ...
        static void		setResourceFilePrefix(const ::rtl::OString& _rPrefix);

        /// get the vcl res manager of the module
        static ResMgr*	getResManager();

        /** register a component implementing a service with the given data.
            @param	_rImplementationName
                        the implementation name of the component
            @param	_rServiceNames
                        the services the component supports
            @param	_pCreateFunction
                        a function for creating an instance of the component
            @param	_pFactoryFunction
                        a function for creating a factory for that component
            @see revokeComponent
        */
        static void registerComponent(
            const ::rtl::OUString& _rImplementationName,
            const ::com::sun::star::uno::Sequence< ::rtl::OUString >& _rServiceNames,
            ::cppu::ComponentInstantiation _pCreateFunction,
            FactoryInstantiation _pFactoryFunction);

        /** revoke the registration for the specified component
            @param	_rImplementationName
                the implementation name of the component
        */
        static void revokeComponent(
            const ::rtl::OUString& _rImplementationName);

        /** write the registration information of all known components
            <p>writes the registration information of all components which are currently registered into the
            specified registry.<p/>
            <p>Usually used from within component_writeInfo.<p/>
            @param	_rxServiceManager
                        the service manager
            @param	_rRootKey
                        the registry key under which the information will be stored
            @return
                        sal_True if the registration of all implementations was successfull, sal_False otherwise
        */
        static sal_Bool writeComponentInfos(
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager,
            const ::com::sun::star::uno::Reference< ::com::sun::star::registry::XRegistryKey >& _rRootKey);

        /** creates a Factory for the component with the given implementation name.
            <p>Usually used from within component_getFactory.<p/>
            @param	_rxServiceManager
                        a pointer to an XMultiServiceFactory interface as got in component_getFactory
            @param	_pImplementationName
                        the implementation name of the component
            @return
                        the XInterface access to a factory for the component
        */
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > getComponentFactory(
            const ::rtl::OUString& _rImplementationName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxServiceManager
            );

    protected:
        /// register a client for the module
        static void	registerClient();
        /// revoke a client for the module
        static void	revokeClient();

    private:
        /** ensure that the impl class exists
            @precond m_aMutex is guarded when this method gets called
        */
        static void ensureImpl();
    };

    //=========================================================================
    //= OModuleResourceClient
    //=========================================================================
    /** base class for objects which uses any global module-specific ressources 
    */
    class OModuleResourceClient
    {
    public:
        OModuleResourceClient()		{ OModule::registerClient(); }
        ~OModuleResourceClient()	{ OModule::revokeClient(); }
    };

    //=========================================================================
    //= ModuleRes
    //=========================================================================
    /** specialized ResId, using the ressource manager provided by the global module
    */
    class ModuleRes : public ::ResId
    {
    public:
        ModuleRes(USHORT _nId) : ResId(_nId, *OModule::getResManager()) { }
    };

    //==========================================================================
    //= OMultiInstanceAutoRegistration
    //==========================================================================
    template <class TYPE>
    class OMultiInstanceAutoRegistration
    {
    public:
        /** automatically registeres a multi instance component
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static ::rtl::OUString getImplementationName_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <method>OModule::registerComponent</method>.
            <p/>
            The factory creation function used is <code>::cppu::createSingleFactory</code>.
            @see OOneInstanceAutoRegistration
        */
        OMultiInstanceAutoRegistration();
        ~OMultiInstanceAutoRegistration();
    };

    template <class TYPE>
    OMultiInstanceAutoRegistration<TYPE>::OMultiInstanceAutoRegistration()
    {
        OModule::registerComponent(
            TYPE::getImplementationName_Static(),
            TYPE::getSupportedServiceNames_Static(),
            TYPE::Create,
            ::cppu::createSingleFactory
            );
    }

    template <class TYPE>
    OMultiInstanceAutoRegistration<TYPE>::~OMultiInstanceAutoRegistration()
    {
        OModule::revokeComponent(TYPE::getImplementationName_Static());
    }

    //==========================================================================
    //= OOneInstanceAutoRegistration
    //==========================================================================
    template <class TYPE>
    class OOneInstanceAutoRegistration
    {
    public:
        /** automatically registeres a single instance component
            <p>Assumed that the template argument has the three methods
                <ul>
                    <li><code>static ::rtl::OUString getImplementationName_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static()</code><li/>
                    <li><code>static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                        Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&)</code>
                        </li>
                <ul/>
            the instantiation of this object will automatically register the class via <method>OModule::registerComponent</method>.
            <p/>
            The factory creation function used is <code>::cppu::createOneInstanceFactory</code>.
            @see OOneInstanceAutoRegistration
        */
        OOneInstanceAutoRegistration();
        ~OOneInstanceAutoRegistration();
    };

    template <class TYPE>
    OOneInstanceAutoRegistration<TYPE>::OOneInstanceAutoRegistration()
    {
        OModule::registerComponent(
            TYPE::getImplementationName_Static(),
            TYPE::getSupportedServiceNames_Static(),
            TYPE::Create,
            ::cppu::createOneInstanceFactory
            );
    }

    template <class TYPE>
    OOneInstanceAutoRegistration<TYPE>::~OOneInstanceAutoRegistration()
    {
        OModule::revokeComponent(TYPE::getImplementationName_Static());
    }

//.........................................................................
}	// namespace COMPMOD_NAMESPACE
//.........................................................................

#endif // _EXTENSIONS_COMPONENT_MODULE_HXX_

