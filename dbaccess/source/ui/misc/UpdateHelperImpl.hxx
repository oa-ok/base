/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: UpdateHelperImpl.hxx,v $
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
#ifndef DBAUI_UPDATEHELPERIMPL_HXX
#define DBAUI_UPDATEHELPERIMPL_HXX

#include <com/sun/star/sdbc/XResultSetUpdate.hpp>
#include <com/sun/star/sdbc/XRowUpdate.hpp>
#include <com/sun/star/sdbc/XParameters.hpp>
#include <com/sun/star/sdbc/XPreparedStatement.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include "IUpdateHelper.hxx"
#include <rtl/logfile.hxx>

namespace dbaui
{
    class ORowUpdateHelper : public IUpdateHelper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowUpdate >			m_xRowUpdate;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSetUpdate >	m_xResultSetUpdate;	// 
    public:
        ORowUpdateHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet>& _xRowSet)
            :m_xRowUpdate(_xRowSet,::com::sun::star::uno::UNO_QUERY)
            ,m_xResultSetUpdate(_xRowSet,::com::sun::star::uno::UNO_QUERY)
        {
        }
        virtual ~ORowUpdateHelper() {}
        virtual void updateString(sal_Int32 _nPos, const ::rtl::OUString& _sValue)
        {
            m_xRowUpdate->updateString(_nPos, _sValue);
        }
        virtual void updateDouble(sal_Int32 _nPos,const double& _nValue)
        {
            m_xRowUpdate->updateDouble(_nPos, _nValue);
        }
        virtual void updateDate(sal_Int32 _nPos,const ::com::sun::star::util::Date& _nValue)
        {
            m_xRowUpdate->updateDate(_nPos, _nValue);
        }
        virtual void updateTime(sal_Int32 _nPos,const ::com::sun::star::util::Time& _nValue)
        {
            m_xRowUpdate->updateTime(_nPos, _nValue);
        }
        virtual void updateTimestamp(sal_Int32 _nPos,const ::com::sun::star::util::DateTime& _nValue)
        {
            m_xRowUpdate->updateTimestamp(_nPos, _nValue);
        }
        virtual void updateInt(sal_Int32 _nPos,const sal_Int32& _nValue)
        {
            m_xRowUpdate->updateInt(_nPos, _nValue);
        }
        virtual void updateNull(sal_Int32 _nPos, ::sal_Int32)
        {
            m_xRowUpdate->updateNull(_nPos);
        }
        virtual void moveToInsertRow()
        {
            m_xResultSetUpdate->moveToInsertRow();
        }
        virtual void insertRow()
        {
            m_xResultSetUpdate->insertRow();
        }
    };

    class OParameterUpdateHelper : public IUpdateHelper
    {
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement >  m_xPrepared;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XParameters >         m_xParameters;

    public:
        OParameterUpdateHelper(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement >& _xPrepared)
           :m_xPrepared(_xPrepared)
           ,m_xParameters(_xPrepared,::com::sun::star::uno::UNO_QUERY)
        {
        }
        virtual ~OParameterUpdateHelper() {}
        virtual void updateString(sal_Int32 _nPos, const ::rtl::OUString& _sValue)
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OParameterUpdateHelper::updateString" );
            m_xParameters->setString(_nPos, _sValue);
        }
        virtual void updateDouble(sal_Int32 _nPos,const double& _nValue)
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OParameterUpdateHelper::updateDouble" );
            m_xParameters->setDouble(_nPos, _nValue);
        }
        virtual void updateDate(sal_Int32 _nPos,const ::com::sun::star::util::Date& _nValue)
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OParameterUpdateHelper::updateDouble" );
            m_xParameters->setDate(_nPos, _nValue);
        }
        virtual void updateTime(sal_Int32 _nPos,const ::com::sun::star::util::Time& _nValue)
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OParameterUpdateHelper::updateDouble" );
            m_xParameters->setTime(_nPos, _nValue);
        }
        virtual void updateTimestamp(sal_Int32 _nPos,const ::com::sun::star::util::DateTime& _nValue)
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OParameterUpdateHelper::updateDouble" );
            m_xParameters->setTimestamp(_nPos, _nValue);
        }
        virtual void updateInt(sal_Int32 _nPos,const sal_Int32& _nValue)
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OParameterUpdateHelper::updateInt" );
            m_xParameters->setInt(_nPos, _nValue);
        }
        virtual void updateNull(sal_Int32 _nPos, ::sal_Int32 sqlType)
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OParameterUpdateHelper::updateNull" );
            m_xParameters->setNull(_nPos,sqlType);
        }
        virtual void moveToInsertRow()
        {
        }
        virtual void insertRow()
        {
            RTL_LOGFILE_CONTEXT_AUTHOR( aLogger, "misc", "Ocke.Janssen@sun.com", "OParameterUpdateHelper::insertRow" );
            m_xPrepared->executeUpdate();
        }
    };
}

#endif // DBAUI_UPDATEHELPERIMPL_HXX

