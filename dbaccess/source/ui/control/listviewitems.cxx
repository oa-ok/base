/*************************************************************************
 *
 *  $RCSfile: listviewitems.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: fs $ $Date: 2001-01-30 08:29:23 $
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

#ifndef _DBAUI_LISTVIEWITEMS_HXX_
#include "listviewitems.hxx"
#endif

//........................................................................
namespace dbaui
{
//........................................................................

    //========================================================================
    // class OBoldListboxString
    //========================================================================
    //------------------------------------------------------------------------
    void OBoldListboxString::InitViewData( SvLBox* pView,SvLBoxEntry* pEntry, SvViewDataItem* _pViewData)
    {
        SvLBoxString::InitViewData(pView,pEntry, _pViewData);
        if (!_pViewData)
            _pViewData = pView->GetViewDataItem( pEntry, this );

        Font aOldFont( pView->GetFont());
        Font aFont( aOldFont );
        aFont.SetWeight(WEIGHT_BOLD);
        pView->SetFont( aFont );

        _pViewData->aSize = Size(pView->GetTextWidth(GetText()), pView->GetTextHeight());
        pView->SetFont( aOldFont );
    }

    //------------------------------------------------------------------------
    USHORT OBoldListboxString::IsA()
    {
        return SV_ITEM_ID_BOLDLBSTRING;
    }

    //------------------------------------------------------------------------
    void OBoldListboxString::Paint(const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry )
    {
        if (m_bEmphasized)
        {
            Font aOldFont( rDev.GetFont());
            Font aFont( aOldFont );
            aFont.SetWeight(WEIGHT_BOLD);
            rDev.SetFont( aFont );

            Point aPos(rPos);
            rDev.DrawText( aPos, GetText() );
            rDev.SetFont( aOldFont );
        }
        else
            SvLBoxString::Paint(rPos, rDev, nFlags, pEntry);
    }

//........................................................................
}	// namespace dbaui
//........................................................................

/*************************************************************************
 * history:
 *	$Log: not supported by cvs2svn $
 *
 *	Revision 1.0 29.01.01 10:26:53  fs
 ************************************************************************/

