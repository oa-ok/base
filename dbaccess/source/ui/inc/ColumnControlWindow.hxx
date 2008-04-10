/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: ColumnControlWindow.hxx,v $
 * $Revision: 1.5 $
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
#ifndef DBAUI_COLUMNCONTROLWINDOW_HXX
#define DBAUI_COLUMNCONTROLWINDOW_HXX

#ifndef DBAUI_FIELDDESCRIPTIONCONTROL_HXX
#include "FieldDescControl.hxx"
#endif
#ifndef DBAUI_TYPEINFO_HXX
#include "TypeInfo.hxx"
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XNUMBERFORMATTER_HPP_
#include <com/sun/star/util/XNumberFormatter.hpp>
#endif

//.........................................................................
namespace dbaui
{
//.........................................................................
    // =============================================================================================
    // OColumnControlWindow
    // =============================================================================================
    class OColumnControlWindow : public OFieldDescControl
    {
        ::com::sun::star::lang::Locale		m_aLocale;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>	m_xORB;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>			m_xConnection;
        mutable ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > 	m_xFormatter;	// a number formatter working with the connection's NumberFormatsSupplier

        OTypeInfoMap				m_aDestTypeInfo;
        ::std::vector<OTypeInfoMap::iterator> m_aDestTypeInfoIndex;

        mutable TOTypeInfoSP		m_pTypeInfo; // default type
        String						m_sTypeNames;		// these type names are the ones out of the resource file
        ::rtl::OUString				m_sAutoIncrementValue;
        sal_Bool					m_bAutoIncrementEnabled;
    protected:
        virtual void		ActivateAggregate( EControlType eType );
        virtual void		DeactivateAggregate( EControlType eType );

        virtual ::com::sun::star::lang::Locale	GetLocale() const;
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter > GetFormatter() const;
        virtual TOTypeInfoSP		getTypeInfo(sal_Int32 _nPos);
        virtual sal_Bool			isAutoIncrementValueEnabled() const;
        virtual ::rtl::OUString		getAutoIncrementValue() const;
        virtual void				CellModified(long nRow, USHORT nColId );

    public:
        OColumnControlWindow(Window* pParent
                            ,const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory);
        virtual ~OColumnControlWindow();

        void setConnection(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xCon);

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData> getMetaData();
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> getConnection();
        virtual const OTypeInfoMap*	getTypeInfo() const;
        TOTypeInfoSP getDefaultTyp() const;
    };
//.........................................................................
}	// namespace dbaui
//.........................................................................
#endif // DBAUI_COLUMNCONTROLWINDOW_HXX
