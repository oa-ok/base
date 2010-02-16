/*************************************************************************
* DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
* 
* Copyright 2009 by Sun Microsystems, Inc.
*
* OpenOffice.org - a multi-platform office productivity suite
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
************************************************************************/

#include "precompiled_dbaccess.hxx"

#include "subcomponentrecovery.hxx"

#include "sdbcoretools.hxx"
#include "storagexmlstream.hxx"
#include "subcomponentloader.hxx"
#include "settingsimport.hxx"

/** === begin UNO includes === **/
#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/frame/XModuleManager.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/ucb/XCommandProcessor.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/sdb/XFormDocumentsSupplier.hpp>
#include <com/sun/star/sdb/XReportDocumentsSupplier.hpp>
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
/** === end UNO includes === **/

#include <comphelper/namedvaluecollection.hxx>
#include <connectivity/dbtools.hxx>
#include <tools/diagnose_ex.h>
#include <xmloff/XMLSettingsExportContext.hxx>
#include <xmloff/SettingsExportHelper.hxx>

//........................................................................
namespace dbaccess
{
//........................................................................

    /** === begin UNO using === **/
    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::XInterface;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::RuntimeException;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::lang::XMultiServiceFactory;
    using ::com::sun::star::embed::XStorage;
    using ::com::sun::star::sdb::application::XDatabaseDocumentUI;
    using ::com::sun::star::beans::Pair;
    using ::com::sun::star::frame::XModuleManager;
    using ::com::sun::star::lang::XComponent;
    using ::com::sun::star::frame::XModel;
    using ::com::sun::star::frame::XController;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::document::XStorageBasedDocument;
    using ::com::sun::star::ucb::XCommandProcessor;
    using ::com::sun::star::container::XHierarchicalNameAccess;
    using ::com::sun::star::sdb::XFormDocumentsSupplier;
    using ::com::sun::star::sdb::XReportDocumentsSupplier;
    using ::com::sun::star::xml::sax::SAXException;
    using ::com::sun::star::xml::sax::XLocator;
    using ::com::sun::star::xml::sax::XDocumentHandler;
    using ::com::sun::star::xml::sax::XAttributeList;
    /** === end UNO using === **/

    namespace ElementModes = ::com::sun::star::embed::ElementModes;


    //====================================================================
    //= helper
    //====================================================================
    namespace
    {
        // .........................................................................
        static const ::rtl::OUString& lcl_getComponentStorageBaseName( const SubComponentType i_eType )
        {
            static const ::rtl::OUString s_sFormBaseName( RTL_CONSTASCII_USTRINGPARAM( "form" ) );
            static const ::rtl::OUString s_sReportBaseName( RTL_CONSTASCII_USTRINGPARAM( "report" ) );
            static const ::rtl::OUString s_sTableBaseName( RTL_CONSTASCII_USTRINGPARAM( "table" ) );
            static const ::rtl::OUString s_sQueryBaseName( RTL_CONSTASCII_USTRINGPARAM( "query" ) );

            switch ( i_eType )
            {
            case FORM:
                return s_sFormBaseName;
            case REPORT:
                return s_sReportBaseName;
            case TABLE:
                return s_sTableBaseName;
            case QUERY:
                return s_sQueryBaseName;
            default:
                break;
            }

            OSL_ENSURE( false, "lcl_getComponentStorageBaseName: unimplemented case!" );
            static const ::rtl::OUString s_sFallback;
            return s_sFallback;
        }

        // .........................................................................
        static SubComponentType lcl_databaseObjectToSubComponentType( const sal_Int32 i_nObjectType )
        {
            switch ( i_nObjectType )
            {
            case DatabaseObject::TABLE: return TABLE;
            case DatabaseObject::QUERY: return QUERY;
            case DatabaseObject::FORM:  return FORM;
            case DatabaseObject::REPORT:return REPORT;
            default:
                break;
            }
            return UNKNOWN;
        }

        // .........................................................................
        static bool lcl_determineReadOnly( const Reference< XComponent >& i_rComponent )
        {
            Reference< XModel > xDocument( i_rComponent, UNO_QUERY );
            if ( !xDocument.is() )
            {
                Reference< XController > xController( i_rComponent, UNO_QUERY_THROW );
                xDocument = xController->getModel();
            }

            if ( !xDocument.is() )
                return false;

            ::comphelper::NamedValueCollection aDocArgs( xDocument->getArgs() );
            return aDocArgs.getOrDefault( "ReadOnly", false );
        }

        // .........................................................................
        static Reference< XCommandProcessor > lcl_getSubComponentDef_nothrow( const Reference< XDatabaseDocumentUI >& i_rAppUI,
            const SubComponentType i_eType, const ::rtl::OUString& i_rName )
        {
            Reference< XController > xController( i_rAppUI, UNO_QUERY_THROW );
            ENSURE_OR_RETURN( ( i_eType == FORM ) || ( i_eType == REPORT ), "lcl_getSubComponentDef_nothrow: illegal controller", NULL );

            Reference< XCommandProcessor > xCommandProcessor;
            try
            {
                Reference< XHierarchicalNameAccess > xDefinitionContainer;
                if ( i_eType == FORM )
                {
                    Reference< XFormDocumentsSupplier > xSuppForms( xController->getModel(), UNO_QUERY_THROW );
                    xDefinitionContainer.set( xSuppForms->getFormDocuments(), UNO_QUERY_THROW );
                }
                else
                {
                    Reference< XReportDocumentsSupplier > xSuppReports( xController->getModel(), UNO_QUERY_THROW );
                    xDefinitionContainer.set( xSuppReports->getReportDocuments(), UNO_QUERY_THROW );
                }
                xCommandProcessor.set( xDefinitionContainer->getByHierarchicalName( i_rName ), UNO_QUERY_THROW );
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return xCommandProcessor;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getSettingsStreamName()
        {
            static const ::rtl::OUString s_sStatementStreamName( RTL_CONSTASCII_USTRINGPARAM( "settings.xml" ) );
            return s_sStatementStreamName;
        }

        // .........................................................................
        static const ::rtl::OUString& lcl_getCurrentQueryDesignName()
        {
            static const ::rtl::OUString s_sQuerySettingsName( RTL_CONSTASCII_USTRINGPARAM( "ooo:current-query-design" ) );
            return s_sQuerySettingsName;
        }
    }

    //====================================================================
    //= SettingsExportContext
    //====================================================================
    class DBACCESS_DLLPRIVATE SettingsExportContext : public ::xmloff::XMLSettingsExportContext
    {
    public:
        SettingsExportContext( const ::comphelper::ComponentContext& i_rContext, const StorageXMLOutputStream& i_rDelegator )
            :m_rContext( i_rContext )
            ,m_rDelegator( i_rDelegator )
            ,m_aNamespace( ::xmloff::token::GetXMLToken( ::xmloff::token::XML_NP_CONFIG ) )
        {
        }

        virtual ~SettingsExportContext()
        {
        }

    public:
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, const ::rtl::OUString& i_rValue );
        virtual void    AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, enum ::xmloff::token::XMLTokenEnum i_eValue );
        virtual void    StartElement( enum ::xmloff::token::XMLTokenEnum i_eName, const sal_Bool i_bIgnoreWhitespace );
        virtual void    EndElement  ( const sal_Bool i_bIgnoreWhitespace );
        virtual void    Characters( const ::rtl::OUString& i_rCharacters );

        virtual ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >
                        GetServiceFactory() const;

    private:
        ::rtl::OUString impl_prefix( const ::xmloff::token::XMLTokenEnum i_eToken )
        {
            ::rtl::OUStringBuffer aQualifiedName( m_aNamespace );
            aQualifiedName.append( sal_Unicode( ':' ) );
            aQualifiedName.append( ::xmloff::token::GetXMLToken( i_eToken ) );
            return aQualifiedName.makeStringAndClear();
        }

    private:
        const ::comphelper::ComponentContext&   m_rContext;
        const StorageXMLOutputStream&           m_rDelegator;
        const ::rtl::OUStringBuffer             m_aNamespace;
    };

    //--------------------------------------------------------------------
    void SettingsExportContext::AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, const ::rtl::OUString& i_rValue )
    {
        m_rDelegator.addAttribute( impl_prefix( i_eName ), i_rValue );
    }

    //--------------------------------------------------------------------
    void SettingsExportContext::AddAttribute( enum ::xmloff::token::XMLTokenEnum i_eName, enum ::xmloff::token::XMLTokenEnum i_eValue )
    {
        m_rDelegator.addAttribute( impl_prefix( i_eName ), ::xmloff::token::GetXMLToken( i_eValue ) );
    }

    //--------------------------------------------------------------------
    void SettingsExportContext::StartElement( enum ::xmloff::token::XMLTokenEnum i_eName, const sal_Bool i_bIgnoreWhitespace )
    {
        if ( i_bIgnoreWhitespace )
            m_rDelegator.ignorableWhitespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );

        m_rDelegator.startElement( impl_prefix( i_eName ) );
    }

    //--------------------------------------------------------------------
    void SettingsExportContext::EndElement( const sal_Bool i_bIgnoreWhitespace )
    {
        if ( i_bIgnoreWhitespace )
            m_rDelegator.ignorableWhitespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );
        m_rDelegator.endElement();
    }

    //--------------------------------------------------------------------
    void SettingsExportContext::Characters( const ::rtl::OUString& i_rCharacters )
    {
        m_rDelegator.characters( i_rCharacters );
    }

    //--------------------------------------------------------------------
    Reference< XMultiServiceFactory > SettingsExportContext::GetServiceFactory() const
    {
        return m_rContext.getLegacyServiceFactory();
    }

    //==================================================================================================================
    //= SettingsDocumentHandler
    //==================================================================================================================
    typedef ::cppu::WeakImplHelper1 <   XDocumentHandler
                                    >   SettingsDocumentHandler_Base;
    class DBACCESS_DLLPRIVATE SettingsDocumentHandler : public SettingsDocumentHandler_Base
    {
    public:
        SettingsDocumentHandler()
        {
        }

    protected:
        virtual ~SettingsDocumentHandler()
        {
        }

    public:
        // XDocumentHandler
        virtual void SAL_CALL startDocument(  ) throw (SAXException, RuntimeException);
        virtual void SAL_CALL endDocument(  ) throw (SAXException, RuntimeException);
        virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const Reference< XAttributeList >& xAttribs ) throw (SAXException, RuntimeException);
        virtual void SAL_CALL endElement( const ::rtl::OUString& aName ) throw (SAXException, RuntimeException);
        virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (SAXException, RuntimeException);
        virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (SAXException, RuntimeException);
        virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (SAXException, RuntimeException);
        virtual void SAL_CALL setDocumentLocator( const Reference< XLocator >& xLocator ) throw (SAXException, RuntimeException);

        const ::comphelper::NamedValueCollection&   getSettings() const { return m_aSettings; }

    private:
        ::std::stack< ::rtl::Reference< SettingsImport > >  m_aStates;
        ::comphelper::NamedValueCollection                  m_aSettings;
    };

    //--------------------------------------------------------------------
    void SAL_CALL SettingsDocumentHandler::startDocument(  ) throw (SAXException, RuntimeException)
    {
    }
    
    //--------------------------------------------------------------------
    void SAL_CALL SettingsDocumentHandler::endDocument(  ) throw (SAXException, RuntimeException)
    {
    }
    
    //--------------------------------------------------------------------
    void SAL_CALL SettingsDocumentHandler::startElement( const ::rtl::OUString& i_Name, const Reference< XAttributeList >& i_Attribs ) throw (SAXException, RuntimeException)
    {
        ::rtl::Reference< SettingsImport >  pNewState;

        if ( m_aStates.empty() )
        {
            if ( i_Name.equalsAscii( "office:settings" ) )
            {
                pNewState = new OfficeSettingsImport( m_aSettings );
            }
            else
            {
                OSL_ENSURE( false, "SettingsDocumentHandler::startElement: invalid settings file!" );
                // Yes, that's not correct. Somebody could, in theory, give us a document which starts with "foo:settings",
                // where "foo" is mapped to the proper namespace URL.
                // However, there's no need to bother with this. The "recovery" sub storage we're recovering from is
                // not part of ODF, so we can impose any format restrictions on it ...
            }
        }
        else
        {
            ::rtl::Reference< SettingsImport > pCurrentState( m_aStates.top() );
            pNewState = pCurrentState->nextState( i_Name );
        }

        ENSURE_OR_THROW( pNewState.is(), "no new state - aborting import" );
        pNewState->startElement( i_Attribs );

        m_aStates.push( pNewState );
    }
    
    //--------------------------------------------------------------------
    void SAL_CALL SettingsDocumentHandler::endElement( const ::rtl::OUString& i_Name ) throw (SAXException, RuntimeException)
    {
        ENSURE_OR_THROW( !m_aStates.empty(), "no active element" );
        (void)i_Name;

        ::rtl::Reference< SettingsImport > pCurrentState( m_aStates.top() );
        pCurrentState->endElement();
        m_aStates.pop();
    }
    
    //--------------------------------------------------------------------
    void SAL_CALL SettingsDocumentHandler::characters( const ::rtl::OUString& i_Chars ) throw (SAXException, RuntimeException)
    {
        ENSURE_OR_THROW( !m_aStates.empty(), "no active element" );

        ::rtl::Reference< SettingsImport > pCurrentState( m_aStates.top() );
        pCurrentState->characters( i_Chars );
    }
    
    //--------------------------------------------------------------------
    void SAL_CALL SettingsDocumentHandler::ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (SAXException, RuntimeException)
    {
        // ignore them - that's why they're called "ignorable"
        (void)aWhitespaces;
    }
    
    //--------------------------------------------------------------------
    void SAL_CALL SettingsDocumentHandler::processingInstruction( const ::rtl::OUString& i_Target, const ::rtl::OUString& i_Data ) throw (SAXException, RuntimeException)
    {
        OSL_ENSURE( false, "SettingsDocumentHandler::processingInstruction: unexpected ..." );
        (void)i_Target;
        (void)i_Data;
    }
    
    //--------------------------------------------------------------------
    void SAL_CALL SettingsDocumentHandler::setDocumentLocator( const Reference< XLocator >& i_Locator ) throw (SAXException, RuntimeException)
    {
        // TODO: place your code here
        (void)i_Locator;
    }

    //====================================================================
    //= SubComponentRecovery
    //====================================================================
    //--------------------------------------------------------------------
    const ::rtl::OUString SubComponentRecovery::getComponentsStorageName( const SubComponentType i_eType )
    {
        static const ::rtl::OUString s_sFormsStorageName( RTL_CONSTASCII_USTRINGPARAM( "forms" ) );
        static const ::rtl::OUString s_sReportsStorageName( RTL_CONSTASCII_USTRINGPARAM( "reports" ) );
        static const ::rtl::OUString s_sTablesStorageName( RTL_CONSTASCII_USTRINGPARAM( "tables" ) );
        static const ::rtl::OUString s_sQueriesStorageName( RTL_CONSTASCII_USTRINGPARAM( "queries" ) );
        static const ::rtl::OUString s_sRelationsStorageName( RTL_CONSTASCII_USTRINGPARAM( "relations" ) );

        switch ( i_eType )
        {
        case FORM:
            return s_sFormsStorageName;
        case REPORT:
            return s_sReportsStorageName;
        case TABLE:
            return s_sTablesStorageName;
        case QUERY:
            return s_sQueriesStorageName;
        case RELATION_DESIGN:
            return s_sRelationsStorageName;
        default:
            break;
        }

        OSL_ENSURE( false, "SubComponentRecovery::getComponentsStorageName: unimplemented case!" );
        static const ::rtl::OUString s_sFallback;
        return s_sFallback;
    }

    //--------------------------------------------------------------------
    void SubComponentRecovery::saveToRecoveryStorage( const Reference< XStorage >& i_rRecoveryStorage,
        MapCompTypeToCompDescs& io_mapCompDescs )
    {
        if ( m_eType == UNKNOWN )
            // quite fatal, but has already been reported (as assertion) before
            return;

        // open the sub storage for the given kind of components
        const ::rtl::OUString& rStorageName( getComponentsStorageName( m_eType ) );
        const Reference< XStorage > xComponentsStorage( i_rRecoveryStorage->openStorageElement(
            rStorageName, ElementModes::READWRITE ), UNO_QUERY_THROW );

        // find a free sub storage name, and create Yet Another Sub Storage
        const ::rtl::OUString& rBaseName( lcl_getComponentStorageBaseName( m_eType ) );
        const ::rtl::OUString sStorName = ::dbtools::createUniqueName( xComponentsStorage.get(), rBaseName, true );
        const Reference< XStorage > xObjectStor( xComponentsStorage->openStorageElement(
            sStorName, ElementModes::READWRITE ), UNO_QUERY_THROW );

        switch ( m_eType )
        {
        case FORM:
        case REPORT:
            impl_saveSubDocument_throw( xObjectStor );
            break;

        case QUERY:
            impl_saveQueryDesign_throw( xObjectStor );
            break;

        default:
            // TODO
            OSL_ENSURE( false, "SubComponentRecoverys::saveToRecoveryStorage: unimplemented case!" );
            break;
        }

        // commit the storage(s)
        tools::stor::commitStorageIfWriteable( xObjectStor );
        tools::stor::commitStorageIfWriteable( xComponentsStorage );

        // remember the relationship from the component name to the storage name
        MapStringToCompDesc& rMapCompDescs = io_mapCompDescs[ m_eType ];
        OSL_ENSURE( rMapCompDescs.find( sStorName ) == rMapCompDescs.end(),
            "SubComponentRecoverys::saveToRecoveryStorage: object name already used!" );
        rMapCompDescs[ sStorName ] = m_aCompDesc;
    }

    //--------------------------------------------------------------------
    void SubComponentRecovery::impl_identifyComponent_throw()
    {
        // ask the controller
        Pair< sal_Int32, ::rtl::OUString > aComponentIdentity = m_xDocumentUI->identifySubComponent( m_xComponent );
        m_eType = lcl_databaseObjectToSubComponentType( aComponentIdentity.First );
        m_aCompDesc.sName = aComponentIdentity.Second;

        // what the controller didn't give us is the information whether this is in edit mode or not ...
        Reference< XModuleManager > xModuleManager( m_rContext.createComponent( "com.sun.star.frame.ModuleManager" ), UNO_QUERY_THROW );
        const ::rtl::OUString sModuleIdentifier = xModuleManager->identify( m_xComponent );

        switch ( m_eType )
        {
        case TABLE:
            m_aCompDesc.bForEditing = sModuleIdentifier.equalsAscii( "com.sun.star.sdb.TableDesign" );
            break;

        case QUERY:
            m_aCompDesc.bForEditing = sModuleIdentifier.equalsAscii( "com.sun.star.sdb.QueryDesign" );
            break;

        case REPORT:
            if ( sModuleIdentifier.equalsAscii( "com.sun.star.report.ReportDefinition" ) )
            {
                // it's an SRB report desginer
                m_aCompDesc.bForEditing = true;
                break;
            }
            // fall through

        case FORM:
            m_aCompDesc.bForEditing = !lcl_determineReadOnly( m_xComponent );
            break;

        default:
            if ( sModuleIdentifier.equalsAscii( "com.sun.star.sdb.RelationDesign" ) )
            {
                m_eType = RELATION_DESIGN;
                m_aCompDesc.bForEditing = true;
            }
            else
            {
                OSL_ENSURE( false, "SubComponentRecovery::impl_identifyComponent_throw: couldn't classify the given sub component!" );
            }
            break;
        }

        OSL_POSTCOND( m_eType != UNKNOWN,
            "SubComponentRecovery::impl_identifyComponent_throw: couldn't classify the component!" );
    }

    //--------------------------------------------------------------------
    void SubComponentRecovery::impl_saveQueryDesign_throw( const Reference< XStorage >& i_rObjectStorage )
    {
        ENSURE_OR_THROW( m_eType == QUERY, "illegal sub component type" );
        ENSURE_OR_THROW( i_rObjectStorage.is(), "illegal storage" );

        // retrieve the current query design (which might differ from what we can retrieve as ActiveCommand property, since
        // the latter is updated only upon successful save of the design)
        Reference< XPropertySet > xDesignerProps( m_xComponent, UNO_QUERY_THROW );
        Sequence< PropertyValue > aCurrentQueryDesign;
        OSL_VERIFY( xDesignerProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "CurrentQueryDesign" ) ) ) >>= aCurrentQueryDesign );

        // write the query design
        StorageXMLOutputStream aDesignOutput( m_rContext, i_rObjectStorage, lcl_getSettingsStreamName() );
        SettingsExportContext aSettingsExportContext( m_rContext, aDesignOutput );

        const ::rtl::OUString sWhitespace( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( " " ) ) );

        aDesignOutput.startElement( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "office:settings" ) ) );
        aDesignOutput.ignorableWhitespace( sWhitespace );

        XMLSettingsExportHelper aSettingsExporter( aSettingsExportContext );
        aSettingsExporter.exportAllSettings( aCurrentQueryDesign, lcl_getCurrentQueryDesignName() );

        aDesignOutput.ignorableWhitespace( sWhitespace );
        aDesignOutput.endElement();
        aDesignOutput.close();
    }

    //--------------------------------------------------------------------
    void SubComponentRecovery::impl_saveSubDocument_throw( const Reference< XStorage >& i_rObjectStorage )
    {
        ENSURE_OR_THROW( ( m_eType == FORM ) || ( m_eType == REPORT ), "illegal sub component type" );
        ENSURE_OR_THROW( i_rObjectStorage.is(), "illegal storage" );

        // store the document into the storage
        Reference< XStorageBasedDocument > xStorageDocument( m_xComponent, UNO_QUERY_THROW );
        xStorageDocument->storeToStorage( i_rObjectStorage, Sequence< PropertyValue >() );
    }

    //--------------------------------------------------------------------
    Reference< XComponent > SubComponentRecovery::impl_recoverSubDocument_throw( const Reference< XStorage >& i_rRecoveryStorage,
            const ::rtl::OUString& i_rComponentName, const bool i_bForEditing )
    {
        Reference< XComponent > xSubComponent;
        Reference< XCommandProcessor > xDocDefinition;

        ::comphelper::NamedValueCollection aLoadArgs;
        aLoadArgs.put( "RecoveryStorage", i_rRecoveryStorage );

        // load/create the sub component hidden. We'll show it when the main app window is shown.
        aLoadArgs.put( "Hidden", true );

        if ( i_rComponentName.getLength() )
        {
            xDocDefinition = lcl_getSubComponentDef_nothrow( m_xDocumentUI, m_eType, i_rComponentName );
            xSubComponent.set( m_xDocumentUI->loadComponentWithArguments(
                    m_eType,
                    i_rComponentName,
                    i_bForEditing,
                    aLoadArgs.getPropertyValues()
                ),
                UNO_SET_THROW
            );
        }
        else
        {
            Reference< XComponent > xDocDefComponent;
            xSubComponent.set( m_xDocumentUI->createComponentWithArguments(
                    m_eType,
                    aLoadArgs.getPropertyValues(),
                    xDocDefComponent
                ),
                UNO_SET_THROW
            );

            xDocDefinition.set( xDocDefComponent, UNO_QUERY );
            OSL_ENSURE( xDocDefinition.is(), "DatabaseDocumentRecovery::recoverSubDocuments: loaded a form/report, but don't have a document definition?!" );
        }

        if ( xDocDefinition.is() )
        {
            Reference< XController > xController( m_xDocumentUI, UNO_QUERY_THROW );
            Reference< XInterface > xLoader( *new SubComponentLoader( xController, xDocDefinition ) );
            (void)xLoader;
        }

        return xSubComponent;
    }

    //--------------------------------------------------------------------
    Reference< XComponent > SubComponentRecovery::impl_recoverQueryDesign_throw( const Reference< XStorage >& i_rRecoveryStorage,
        const ::rtl::OUString& i_rComponentName,  const bool i_bForEditing )
    {
        Reference< XComponent > xSubComponent;

        // first read the settings query design settings from the storage
        StorageXMLInputStream aDesignInput( m_rContext, i_rRecoveryStorage, lcl_getSettingsStreamName() );

        ::rtl::Reference< SettingsDocumentHandler > pDocHandler( new SettingsDocumentHandler );
        aDesignInput.import( pDocHandler.get() );

        const ::comphelper::NamedValueCollection& rSettings( pDocHandler->getSettings() );

        // TODO
        (void)i_rComponentName;
        (void)i_bForEditing;

        return xSubComponent;
    }

    //--------------------------------------------------------------------
    Reference< XComponent > SubComponentRecovery::recoverFromStorage( const Reference< XStorage >& i_rRecoveryStorage,
            const ::rtl::OUString& i_rComponentName, const bool i_bForEditing )
    {
        Reference< XComponent > xSubComponent;
        switch ( m_eType )
        {
        case FORM:
        case REPORT:
            xSubComponent = impl_recoverSubDocument_throw( i_rRecoveryStorage, i_rComponentName, i_bForEditing );
            break;
        case QUERY:
            xSubComponent = impl_recoverQueryDesign_throw( i_rRecoveryStorage, i_rComponentName, i_bForEditing );
            break;
        default:
            OSL_ENSURE( false, "SubComponentRecovery::recoverFromStorage: unimplemented case!" );
            break;
        }
        return xSubComponent;
    }

//........................................................................
} // namespace dbaccess
//........................................................................
