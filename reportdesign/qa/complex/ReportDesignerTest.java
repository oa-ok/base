/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
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
 *
 ************************************************************************/

package complex;

import java.io.File;
import java.util.ArrayList;

import com.sun.star.beans.PropertyValue;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XNameAccess;
import com.sun.star.frame.XComponentLoader;
import com.sun.star.frame.XDesktop;
import com.sun.star.frame.XModel;
import com.sun.star.frame.XStorable;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.XDocumentDataSource;
import com.sun.star.sdb.XOfficeDatabaseDocument;
import com.sun.star.sdb.XReportDocumentsSupplier;
import com.sun.star.sdb.application.XDatabaseDocumentUI;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.util.XCloseable;

// import util.BasicMacroTools;
// import util.DesktopTools;
// import util.dbg;
import complexlib.ComplexTestCase;
import util.utils;
import helper.OfficeProvider;
import helper.URLHelper;
import helper.OfficeWatcher;

import convwatch.DB;

// import java.util.Date;
// import java.text.SimpleDateFormat;
// import java.text.ParsePosition;
// import java.sql.Time;
// 
// import java.io.BufferedReader;
// import java.io.File;
// import java.io.FileReader;
// import java.io.IOException;
// import java.io.FilenameFilter;
// 
// import java.util.Vector;
// 
// import helper.AppProvider;
// import java.text.DecimalFormat;
// import util.DynamicClassLoader;
// import java.util.StringTokenizer;




class PropertySetHelper
{
    XPropertySet m_xPropertySet;
    public PropertySetHelper(Object _aObj)
        {
            m_xPropertySet = (XPropertySet)UnoRuntime.queryInterface(XPropertySet.class, _aObj);
        }
    
    /**
       get a property and don't convert it
       @param _sName the string name of the property
       @return the object value of the property without any conversion
    */
    public Object getPropertyValueAsObject(String _sName)
        {
            Object aObject = null;

            if (m_xPropertySet != null)
            {
                try
                {
                    aObject = m_xPropertySet.getPropertyValue(_sName);
                }
                catch (com.sun.star.beans.UnknownPropertyException e)
                {
                    System.out.println("ERROR: UnknownPropertyException caught. '" + _sName + "'");
                    System.out.println("Message: " + e.getMessage());
                }
                catch (com.sun.star.lang.WrappedTargetException e)
                {
                    System.out.println("ERROR: WrappedTargetException caught.");
                    System.out.println("Message: " + e.getMessage());
                }
            }
            return aObject;
        }
}

class PropertyHelper
{
    /**
       Create a PropertyValue[] from a ArrayList
       @param _aArrayList
       @return a PropertyValue[]
    */
    public static PropertyValue[] createPropertyValueArrayFormArrayList(ArrayList _aPropertyList)
        {
            // copy the whole PropertyValue List to an PropertyValue Array
            PropertyValue[] aSaveProperties = null;

            if (_aPropertyList == null)
            {
                aSaveProperties = new PropertyValue[0];
            }
            else
            {
                if (_aPropertyList.size() > 0)
                {
                    aSaveProperties = new PropertyValue[_aPropertyList.size()];
                    for (int i = 0;i<_aPropertyList.size(); i++)
                    {
                        aSaveProperties[i] = (PropertyValue) _aPropertyList.get(i);
                    }
                }
                else
                {
                    aSaveProperties = new PropertyValue[0];
                }
            }
            return aSaveProperties;
        }
}

public class ReportDesignerTest extends ComplexTestCase {
    
    String mTestDocumentPath;

    public String[] getTestMethodNames() 
        {
            return new String[] {"firsttest"};
        }

    private void checkIfOfficeExists(String _sOfficePathWithTrash)
        {
            String sOfficePath = "";
            int nIndex = _sOfficePathWithTrash.indexOf("soffice.exe");
            if (nIndex > 0)
            {
                sOfficePath = _sOfficePathWithTrash.substring(0, nIndex + 11);
            }
            else
            {
                nIndex = _sOfficePathWithTrash.indexOf("soffice");
                if (nIndex > 0)
                {
                    sOfficePath = _sOfficePathWithTrash.substring(0, nIndex + 7);
                }
            }
            
            log.println(sOfficePath);
            File sOffice = new File(sOfficePath);
            if (! sOffice.exists())
            {
                log.println("ERROR: There exists no office installation at given path: '" + sOfficePath + "'");
                System.exit(0);
            }
        }
    
    
    private static XDesktop m_xDesktop = null;
    public static XDesktop getXDesktop()
        {

            if (m_xDesktop == null) 
            {
                try 
                {
                    XInterface xInterface = (XInterface) m_xXMultiServiceFactory.createInstance( "com.sun.star.frame.Desktop" );
                    m_xDesktop = (XDesktop) UnoRuntime.queryInterface(XDesktop.class, xInterface);
                }
                catch (com.sun.star.uno.Exception e) 
                {
                    log.println("ERROR: uno.Exception caught");
                    log.println("Message: " + e.getMessage());
                }
            }
            return m_xDesktop;
        }
            
    private void showElements(XNameAccess _xNameAccess)
        {
            if (_xNameAccess != null)
            {
                String[] sElementNames = _xNameAccess.getElementNames();
                for(int i=0;i<sElementNames.length; i++)
                {
                    System.out.println("Value: [" + i + "] := " + sElementNames[i]);
                }
            }
            else
            {
                System.out.println("Warning: Given object is null.");
            }
        }
        
        
    private OfficeProvider m_aProvider = null;
    private static XMultiServiceFactory m_xXMultiServiceFactory = null;
    private void startOffice()
        {
            // int tempTime = param.getInt("SingleTimeOut");
            param.put("TimeOut", new Integer(300000));
            System.out.println("TimeOut: " + param.getInt("TimeOut"));
            System.out.println("ThreadTimeOut: " + param.getInt("ThreadTimeOut"));

            // OfficeProvider aProvider = null;
            m_aProvider = new OfficeProvider();
            m_xXMultiServiceFactory = (XMultiServiceFactory) m_aProvider.getManager(param);
            param.put("ServiceFactory", m_xXMultiServiceFactory);
        }

    private void stopOffice()
        {
            if (m_aProvider != null)
            {
                m_aProvider.closeExistingOffice(param, true);
                m_aProvider = null;
            }
        }
        
    private String m_sMailAddress = null;
    private String m_sUPDMinor;
    private String m_sCWS_WORK_STAMP;

    private static final int WRITER = 1;
    private static final int CALC = 2;

    public void firsttest() 
        {
            convwatch.GlobalLogWriter.set(log);
            try
            {
                    
                // -------------------- preconditions, try to find an office --------------------
                
                String sAppExecutionCommand = (String) param.get("AppExecutionCommand");

                String sUser = System.getProperty("user.name");
                log.println("user.name='" + sUser + "'");
                    
                String sVCSID = System.getProperty("VCSID");
                log.println("VCSID='" + sVCSID + "'");
                m_sMailAddress = sVCSID + "@openoffice.org";
                log.println("Assumed mail address: " + m_sMailAddress);

                m_sUPDMinor = System.getProperty("UPDMINOR");
                m_sCWS_WORK_STAMP = System.getProperty("CWS_WORK_STAMP");
                // createDBEntry();
                log.println("Current CWS: " + m_sCWS_WORK_STAMP);
                log.println("Current MWS: " + m_sUPDMinor);

                // System.exit(1);

                sAppExecutionCommand = sAppExecutionCommand.replaceAll( "\\$\\{USERNAME\\}", sUser);
                log.println("sAppExecutionCommand='" + sAppExecutionCommand + "'");
                    
                // an other way to replace strings
                // sAppExecutionCommand = utils.replaceAll13(sAppExecutionCommand, "${USERNAME}", sUser);
                    
                checkIfOfficeExists(sAppExecutionCommand);
                param.put("AppExecutionCommand", new String(sAppExecutionCommand));
                    
                // --------------------------- Start the given Office ---------------------------
                    
                startOffice();
                
                // ------------------------------ Start a test run ------------------------------
                    
                String sCurrentDirectory = System.getProperty("user.dir");
                log.println("Current Dir: " + sCurrentDirectory);
                    
                String sWriterDocument =  sCurrentDirectory + "/" + "RPTWriterTests.odb";
                startTestForFile(sWriterDocument, WRITER);

                String sCalcDocument =  sCurrentDirectory + "/" + "RPTCalcTests.odb";
                startTestForFile(sCalcDocument, CALC);
            }
            catch (AssureException e)
            {
                stopOffice();
                throw new AssureException(e.getMessage());
            }
                
            // ------------------------------ Office shutdown ------------------------------
            stopOffice();
        }
        
// -----------------------------------------------------------------------------
    private void startTestForFile(String _sDocument, int _nType)
        {
            File aFile = new File(_sDocument);
            assure("Test File doesn't '" + _sDocument + "'exist.", aFile.exists());
            
            String sFileURL = URLHelper.getFileURLFromSystemPath(_sDocument);
            log.println("File URL: " + sFileURL);
                    
            XComponent xDocComponent = loadComponent(sFileURL, getXDesktop(), null);
            log.println("Load done");
// 	context = createUnoService("com.sun.star.sdb.DatabaseContext")
//     oDataBase = context.getByName("hh")
//     oDBDoc = oDataBase.DatabaseDocument
// 
// 	dim args(1) as new com.sun.star.beans.PropertyValue
// 	args(0).Name = "ActiveConnection"
// 	args(0).Value = oDBDoc.getCurrentController().getPropertyValue("ActiveConnection")
// 	reportContainer = oDBDoc.getReportDocuments()
//     report = reportContainer.loadComponentFromURL("Report40","",0,args)
                    
            try
            {
                XInterface x = (XInterface)m_xXMultiServiceFactory.createInstance("com.sun.star.sdb.DatabaseContext");
                assure("can't create instance of com.sun.star.sdb.DatabaseContext", x != null);
                log.println("createInstance com.sun.star.sdb.DatabaseContext done");
                
                XNameAccess xNameAccess = (XNameAccess) UnoRuntime.queryInterface(XNameAccess.class, x);
                showElements(xNameAccess);
                Object aObj = xNameAccess.getByName(sFileURL);
//                    log.println("1");
                
                    // PropertySetHelper aHelper = new PropertySetHelper(aObj);
                XDocumentDataSource xDataSource = (XDocumentDataSource)UnoRuntime.queryInterface(XDocumentDataSource.class, aObj);
//                    Object aDatabaseDocmuent = aHelper.getPropertyValueAsObject("DatabaseDocument");
                XOfficeDatabaseDocument xOfficeDBDoc = xDataSource.getDatabaseDocument();
                
                // XOfficeDatabaseDocument xOfficeDBDoc = (XOfficeDatabaseDocument)UnoRuntime.queryInterface(XOfficeDatabaseDocument.class, aDatabaseDocument);
                assure("can't access DatabaseDocument", xOfficeDBDoc != null);
//                    log.println("2");
                
                XModel xDBSource = (XModel)UnoRuntime.queryInterface(XModel.class, xOfficeDBDoc);
                Object aController = xDBSource.getCurrentController();
                assure("Controller of xOfficeDatabaseDocument is empty!", aController != null);
//                     log.println("3");
                
                XDatabaseDocumentUI aDBDocUI = (XDatabaseDocumentUI)UnoRuntime.queryInterface(XDatabaseDocumentUI.class, aController);
                boolean isConnect = aDBDocUI.connect();
//                     if (isConnect)
//                     {
//                         System.out.println("true");
//                     }
//                     else
//                     {
//                         System.out.println("false");
//                     }
//                     log.println("4");
                
                // aHelper = new PropertySetHelper(aController);
                
                // Object aActiveConnectionObj = aHelper.getPropertyValueAsObject("ActiveConnection");
                Object aActiveConnectionObj = aDBDocUI.getActiveConnection(); 
                assure("ActiveConnection is empty", aActiveConnectionObj != null);
//                     log.println("5");
                
                XReportDocumentsSupplier xSupplier = (XReportDocumentsSupplier)UnoRuntime.queryInterface(XReportDocumentsSupplier.class, xOfficeDBDoc);
                xNameAccess = xSupplier.getReportDocuments();
                assure("xOfficeDatabaseDocument returns no Report Document", xNameAccess != null);
//                     log.println("5");
                
                showElements(xNameAccess);
                
                ArrayList aPropertyList = new ArrayList();
                
                PropertyValue aActiveConnection = new PropertyValue();
                aActiveConnection.Name = "ActiveConnection";
                aActiveConnection.Value = aActiveConnectionObj;
                aPropertyList.add(aActiveConnection);
                
                loadAndStoreReports(xNameAccess, aPropertyList, _nType);
                createDBEntry(_nType);
            }
            catch(com.sun.star.uno.Exception e)
            {
                log.println("ERROR: Exception caught");
            }
            
                // String mTestDocumentPath = (String) param.get("TestDocumentPath");
                // System.out.println("mTestDocumentPath: '" + mTestDocumentPath + "'");
                // // workaround for issue using deprecated "DOCPTH" prop
                // System.setProperty("DOCPTH", mTestDocumentPath);
                    
                // Close the document
                closeComponent(xDocComponent);
        }

    private String getDocumentPoolName(int _nType)
        {
            return getFileFormat(_nType);
        }
    
// -----------------------------------------------------------------------------
    private void createDBEntry(int _nType)
        {
            // try to connect the database
            String sDBConnection = (String)param.get( convwatch.PropertyName.DB_CONNECTION_STRING );
            log.println("DBConnection: " + sDBConnection);
            DB.init(sDBConnection);
            String sDestinationVersion = m_sCWS_WORK_STAMP;
            if (sDestinationVersion.length() == 0)
            {
                sDestinationVersion = m_sUPDMinor;
            }
            String sDestinationName = "";
            String sDestinationCreatorType = "";
            String sDocumentPoolDir = getOutputPath(_nType);
            String sDocumentPoolName = getDocumentPoolName(_nType);
            String sSpecial = "";

            String sFixRefSubDirectory = "ReportDesign_qa_complex_" + getFileFormat(_nType);
            DB.insertinto_documentcompare(sFixRefSubDirectory, "", "fixref",
                                          sDestinationVersion, sDestinationName, sDestinationCreatorType,
                                          sDocumentPoolDir, sDocumentPoolName, m_sMailAddress,
                                          sSpecial);
            // DB.test();
            // System.exit(1);
        }
    
    private void loadAndStoreReports(XNameAccess _xNameAccess, ArrayList _aPropertyList, int _nType)
        {
            if (_xNameAccess != null)
            {
                String[] sElementNames = _xNameAccess.getElementNames();
                for(int i=0;i<sElementNames.length; i++)
                {
                    String sReportName = sElementNames[i];
                    XComponent xDoc = loadComponent(sReportName, _xNameAccess, _aPropertyList);
                    // print? or store?
                    storeComponent(sReportName, xDoc, _nType);
                    closeComponent(xDoc);
                }
            }
        }

    private String getFormatExtension(int _nType)
        {
            String sExtension;
            switch(_nType)
            {
            case WRITER:
                sExtension = ".odt";
                break;
            case CALC:
                sExtension = ".ods";
                break;
            default:
                sExtension = ".UNKNOWN";
            }
            return sExtension;
        }
    private String getFileFormat(int _nType)
        {
            String sFileType;
            switch(_nType)
            {
            case WRITER:
                sFileType = "writer8";
                break;
            case CALC:
                sFileType = "calc8";
                break;
            default:
                sFileType = "UNKNOWN";
            }
            return sFileType;
        }
    
    private String getOutputPath(int _nType)
        {
            String sOutputPath = (String)param.get( convwatch.PropertyName.DOC_COMPARATOR_OUTPUT_PATH );

            if (!sOutputPath.endsWith("/") ||         // construct the output file name
                !sOutputPath.endsWith("\\"))
            {
                sOutputPath += System.getProperty("file.separator");
            }
            sOutputPath += "tmp_123";
            sOutputPath += System.getProperty("file.separator");

            // sOutputPath += getFileFormat(_nType);
            // sOutputPath += System.getProperty("file.separator");

            File aOutputFile = new File(sOutputPath); // create the directory of the given output path
            aOutputFile.mkdirs();

            return sOutputPath;
        }
    
    /*
      store given _xComponent under the given Name in DOC_COMPARATOR_INPUTPATH
     */
    private void storeComponent(String _sName, Object _xComponent, int _nType)
        {
            String sOutputPath = getOutputPath(_nType);

            // add DocumentPoolName
            sOutputPath += getDocumentPoolName(_nType);
            sOutputPath += System.getProperty("file.separator");

            File aOutputFile = new File(sOutputPath); // create the directory of the given output path
            aOutputFile.mkdirs();

            sOutputPath += _sName;
            sOutputPath += getFormatExtension(_nType);
            
            String sOutputURL = URLHelper.getFileURLFromSystemPath(sOutputPath);

            ArrayList aPropertyList = new ArrayList(); // set some properties for storeAsURL

            PropertyValue aFileFormat = new PropertyValue();
            aFileFormat.Name = "FilterName";
            aFileFormat.Value = getFileFormat(_nType);
            aPropertyList.add(aFileFormat);

            PropertyValue aOverwrite = new PropertyValue(); // always overwrite already exist files
            aOverwrite.Name = "Overwrite";
            aOverwrite.Value = Boolean.TRUE;
            aPropertyList.add(aOverwrite);

            // store the document in an other directory
            XStorable aStorable = (XStorable) UnoRuntime.queryInterface( XStorable.class, _xComponent);
            if (aStorable != null)
            {
                log.println("store document as URL: '" + sOutputURL + "'");
                try
                {
                    aStorable.storeAsURL(sOutputURL, PropertyHelper.createPropertyValueArrayFormArrayList(aPropertyList));
                }
                catch (com.sun.star.io.IOException e)
                {
                    log.println("ERROR: Exception caught");
                    log.println("Can't write document URL: '" + sOutputURL + "'");
                    log.println("Message: " + e.getMessage());
                }
            }
        }
    
    private XComponent loadComponent(String _sName, Object _xComponent, ArrayList _aPropertyList)
        {
            XComponent xDocComponent = null;
            XComponentLoader xComponentLoader = (XComponentLoader) UnoRuntime.queryInterface( XComponentLoader.class, _xComponent );

            try
            {
                PropertyValue[] aLoadProperties = PropertyHelper.createPropertyValueArrayFormArrayList(_aPropertyList);
                log.println("Load component: '" + _sName + "'");
                xDocComponent = xComponentLoader.loadComponentFromURL(_sName, "_blank", 0, aLoadProperties);
            }
            catch (com.sun.star.io.IOException e)
            {
                log.println("ERROR: Exception caught");
                log.println("Can't load document '" + _sName + "'");
                log.println("Message: " + e.getMessage());
            }
            catch (com.sun.star.lang.IllegalArgumentException e)
            {
                log.println("ERROR: Exception caught");
                log.println("Illegal Arguments given to loadComponentFromURL.");
                log.println("Message: " + e.getMessage());
            }
            return xDocComponent;
        }

    private void closeComponent(XComponent _xDoc)
        {
            // Close the document
            XCloseable xCloseable = (XCloseable) UnoRuntime.queryInterface(XCloseable.class, _xDoc);
            try
            {
                xCloseable.close(true);
            }
            catch (com.sun.star.util.CloseVetoException e)
            {
                log.println("ERROR: CloseVetoException caught");
                log.println("CloseVetoException occured Can't close document.");
                log.println("Message: " + e.getMessage());
            }
        }
        
}
