/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgsave.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-06 08:29:20 $
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

#ifndef DBAUI_DLGSAVE_HXX
#define DBAUI_DLGSAVE_HXX

#ifndef _DBASHARED_APITOOLS_HXX_
#include "apitools.hxx"
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _SV_MSGBOX_HXX
#include <vcl/msgbox.hxx>
#endif

namespace com { namespace sun { namespace star { 
    namespace container {
        class XNameAccess;
        class XHierarchicalNameAccess;
    }
    namespace sdbc {
        class XDatabaseMetaData;
        class XConnection;
    }
}}}


#define	SAD_DEFAULT					0x0000
#define SAD_ADDITIONAL_DESCRIPTION	0x0001

#define SAD_TITLE_STORE_AS			0x0000
#define SAD_TITLE_PASTE_AS			0x0100
#define SAD_TITLE_RENAME			0x0200

class Button;
class Edit;
namespace dbaui
{
    class OSaveAsDlgImpl;
    class IObjectNameCheck;
    class OSaveAsDlg : public ModalDialog
    {
    private:
        OSaveAsDlgImpl* m_pImpl;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >    m_xORB;
    public:
        OSaveAsDlg(	Window * pParent,const sal_Int32& _rType,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,
                    const String& rDefault,
                    const IObjectNameCheck& _rObjectNameCheck,
                    sal_Int32 _nFlags = SAD_DEFAULT | SAD_TITLE_STORE_AS);

        OSaveAsDlg(	Window* _pParent,
                    const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rxORB,
                    const String& _rDefault,
                    const String& _sLabel,
                    const IObjectNameCheck& _rObjectNameCheck,
                    sal_Int32 _nFlags = SAD_DEFAULT | SAD_TITLE_STORE_AS);
        virtual ~OSaveAsDlg();

        String getName() const;
        String getCatalog() const;
        String getSchema() const;
    private:
        DECL_LINK(ButtonClickHdl, Button *);
        DECL_LINK(EditModifyHdl,  Edit * );

        void implInitOnlyTitle(const String& _rLabel);
        void implInit();
    };
}

#endif // DBAUI_DLGSAVE_HXX



