/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: Condition.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:29 $
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

#ifndef RPTUI_CONDITION_HXX
#define RPTUI_CONDITION_HXX

#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _SV_BUTTON_HXX 
#include <vcl/button.hxx>
#endif
#ifndef _SV_TOOLBOX_HXX 
#include <vcl/toolbox.hxx>
#endif
#ifndef DBAUI_TOOLBOXHELPER_HXX
#include <dbaccess/ToolBoxHelper.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XFORMATCONDITION_HPP_
#include <com/sun/star/report/XFormatCondition.hpp>
#endif
#ifndef _SVX_FNTCTRL_HXX
#include <svx/fntctrl.hxx>
#endif
#ifndef _VALUESET_HXX
#include <svtools/valueset.hxx>
#endif

#include <boost/shared_ptr.hpp>

#include <memory>
#include <map>

namespace svx { class ToolboxButtonColorUpdater; }

namespace rptui
{
    class ConditionalFormattingDialog;
    class OColorPopup;
    class OReportController;
    class IConditionalFormatAction;

    //========================================================================
    //= ConditionType
    //========================================================================
    enum ConditionType
    {
        eFieldValueComparison   = 0,
        eExpression             = 1
    };

    //========================================================================
    //= ComparisonOperation
    //========================================================================
    enum ComparisonOperation
    {
        eBetween        = 0,
        eNotBetween     = 1,
        eEqualTo        = 2,
        eNotEqualTo     = 3,
        eGreaterThan    = 4,
        eLessThan       = 5,
        eGreaterOrEqual = 6,
        eLessOrEqual    = 7
    };

    class IExpressionFactory;
    typedef ::boost::shared_ptr< IExpressionFactory >               PExpressionFactory;
    typedef ::std::map< ComparisonOperation, PExpressionFactory >   ExpressionFactories;
    //========================================================================
    //= Condition
    //========================================================================
    class Condition :public Control
                    ,public dbaui::OToolBoxHelper
    {
        ::rptui::OReportController& m_rController;
        IConditionalFormatAction&   m_rAction;
        FixedLine                   m_aHeader;
        ListBox                     m_aConditionType;
        ListBox                     m_aOperationList;
        Edit                        m_aCondLHS;
        FixedText                   m_aOperandGlue;
        Edit                        m_aCondRHS;
        ToolBox                     m_aActions;
        SvxFontPrevWindow           m_aPreview;
        ImageButton                 m_aMoveUp;
        ImageButton                 m_aMoveDown;
        PushButton                  m_aAddCondition;
        PushButton                  m_aRemoveCondition;
        OColorPopup*                m_pColorFloat;

        ::svx::ToolboxButtonColorUpdater*   m_pBtnUpdaterFontColor; // updates the color below the toolbar icon
        ::svx::ToolboxButtonColorUpdater*   m_pBtnUpdaterBackgroundColor;
        
        
        size_t                          m_nCondIndex;
        long                            m_nLastKnownWindowWidth;
        bool                            m_bInDestruction;

        ExpressionFactories             m_aFieldExprFactories;

        DECL_LINK( OnFormatAction,      ToolBox* );
        DECL_LINK( DropdownClick,       ToolBox* );
        DECL_LINK( OnConditionAction,   Button* );

    public:
        Condition( Window* _pParent, IConditionalFormatAction& _rAction, ::rptui::OReportController& _rController );
        virtual ~Condition();

        /** will be called when the id of the image list is needed.
            @param  _eBitmapSet
                <svtools/imgdef.hxx>
            @param  _bHiContast
                <TRUE/> when in high contrast mode.
        */
        virtual ImageList getImageList(sal_Int16 _eBitmapSet,sal_Bool _bHiContast) const;

        /** will be called when the controls need to be resized.
        */
        virtual void resizeControls(const Size& _rDiff);

        /** sets the props at the control
            @param  _xCondition the source
        */
        void setCondition(const com::sun::star::uno::Reference< com::sun::star::report::XFormatCondition >& _xCondition);

        /** fills from the control
            _xCondition the destination
        */
        void fillFormatCondition(const com::sun::star::uno::Reference< com::sun::star::report::XFormatCondition >& _xCondition);

        /** updates the toolbar
            _xCondition the destination
        */
        void updateToolbar(const ::com::sun::star::uno::Reference< ::com::sun::star::report::XReportControlFormat >& _xCondition);

        /// tells the condition its new index within the dialog's condition array
        void setConditionIndex( size_t _nCondIndex, size_t _nCondCount );

        /// returns the condition's index within the dialog's condition array
        size_t  getConditionIndex() const { return m_nCondIndex; }

        /** determines whether the condition is actually empty
        */
        bool    isEmpty() const;

        /** forward to the parent class
        */
        void    ApplyCommand(USHORT _nCommandId, const ::Color& _aColor );

    protected:
        virtual void StateChanged( StateChangedType nStateChange );
        virtual void DataChanged( const DataChangedEvent& rDCEvt );
        virtual void Paint( const Rectangle& rRect );
        virtual void Resize();
        virtual void GetFocus();

    private:
        void    impl_layoutAll();
        void    impl_layoutOperands();

        /// determines the rectangle to be occupied by the toolbar, including the border drawn around it
        Rectangle   impl_getToolBarBorderRect() const;

        inline  ConditionType
                    impl_getCurrentConditionType() const;

        inline  ComparisonOperation
                    impl_getCurrentComparisonOperation() const;

        void    impl_setCondition( const ::rtl::OUString& _rConditionFormula );

    private:
        DECL_LINK( OnTypeSelected, ListBox* );
        DECL_LINK( OnOperationSelected, ListBox* );
    };

    // -------------------------------------------------------------------------
    inline ConditionType Condition::impl_getCurrentConditionType() const
    {
        return sal::static_int_cast< ConditionType >( m_aConditionType.GetSelectEntryPos() );
    }

    // -------------------------------------------------------------------------
    inline ComparisonOperation Condition::impl_getCurrentComparisonOperation() const
    {
        return sal::static_int_cast< ComparisonOperation >( m_aOperationList.GetSelectEntryPos() );
    }

// =============================================================================
} // namespace rptui
// =============================================================================
#endif // RPTUI_CONDITION_HXX

