/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: RowSet.java,v $
 * $Revision: 1.12 $
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
package complex.dbaccess;

import com.sun.star.beans.PropertyVetoException;
import com.sun.star.beans.UnknownPropertyException;
import com.sun.star.beans.XPropertySet;
import com.sun.star.container.XIndexAccess;
import com.sun.star.lang.WrappedTargetException;
import com.sun.star.lang.XComponent;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.sdb.CommandType;
import com.sun.star.sdb.XParametersSupplier;
import com.sun.star.sdb.XResultSetAccess;
import com.sun.star.sdb.XRowSetApproveBroadcaster;
import com.sun.star.sdbc.SQLException;
import com.sun.star.sdbc.XConnection;
import com.sun.star.sdbc.XParameters;
import com.sun.star.sdbc.XPreparedStatement;
import com.sun.star.sdbc.XResultSet;
import com.sun.star.sdbc.XResultSetUpdate;
import com.sun.star.sdbc.XRow;
import com.sun.star.sdbc.XRowSet;
import com.sun.star.sdbc.XRowUpdate;
import com.sun.star.sdbcx.XColumnsSupplier;
import com.sun.star.sdbcx.XDeleteRows;
import com.sun.star.sdbcx.XRowLocate;
import com.sun.star.sdbcx.XTablesSupplier;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.util.XRefreshable;
import complexlib.ComplexTestCase;
import connectivity.tools.DataSource;
import connectivity.tools.HsqlDatabase;
import java.lang.reflect.Method;
import java.util.Random;

public class RowSet extends ComplexTestCase
{

    static final int MAX_TABLE_ROWS = 100;
    static final int MAX_FETCH_ROWS = 10;
    private static final String NEXT = "next";
    private static final String TEST21 = "Test21";
    HsqlDatabase m_database;
    DataSource m_dataSource;
    XRowSet m_rowSet;
    XResultSet m_resultSet;
    XResultSetUpdate m_resultSetUpdate;
    XRow m_row;
    XRowLocate m_rowLocate;
    XPropertySet m_rowSetProperties;
    XParametersSupplier m_paramsSupplier;

    // --------------------------------------------------------------------------------------------------------
    class ResultSetMovementStress implements Runnable
    {

        XResultSet m_resultSet;
        XRow m_row;
        int m_id;

        public ResultSetMovementStress(XResultSet _resultSet, int _id) throws java.lang.Exception
        {
            m_resultSet = _resultSet;
            m_row = (XRow) UnoRuntime.queryInterface(XRow.class, m_resultSet);
            m_id = _id;
        }

        public void run()
        {
            try
            {
                m_resultSet.beforeFirst();
                for (int i = 0; m_resultSet.next(); ++i)
                {
                    int pos = m_resultSet.getRow();
                    // final int val = m_row.getInt(1);
//                    log.println("Clone Move(" + m_id +")  before i: " + (i+1) + " Pos: " + pos + " Val: " + val);
                    testPosition(m_resultSet, m_row, i + 1, "clone move(" + m_id + ")");
//                    val = m_row.getInt(1);
//                    log.println("Clone Move(" + m_id +") after i: " + (i+1) + " Pos: " + pos + " Val: " + val);
                    int pos2 = m_resultSet.getRow();
                    assure("ResultSetMovementStress wrong position: " + i + " Pos1: " + pos + " Pos2: " + pos2, pos == pos2);
                }
            }
            catch (Exception e)
            {
                assure("ResultSetMovementStress(" + m_id + ") failed: " + e, false);
            }
        }
    }
    // --------------------------------------------------------------------------------------------------------

    public String[] getTestMethodNames()
    {
        return new String[]
                {
                    "testRowSet",
                    "testRowSetEvents",
                    "testDeleteBehavior",
                    "testCloneMovesPlusDeletions",
                    "testCloneMovesPlusInsertions",
                    "testParameters"
                };
    }

    // --------------------------------------------------------------------------------------------------------
    public String getTestObjectName()
    {
        return "RowSet";
    }

    // --------------------------------------------------------------------------------------------------------
    private void createTestCase(boolean _defaultRowSet)
    {
        if (m_database == null)
        {
            try
            {
                final CRMDatabase database = new CRMDatabase(getFactory());
                m_database = database.getDatabase();
                m_dataSource = m_database.getDataSource();
            }
            catch (Exception e)
            {
                assure("could not create the embedded HSQL database: " + e.getMessage(), false);
            }
        }

        try
        {
            createStruture();
        }
        catch (SQLException e)
        {
            assure("could not connect to the database/table structure, error message:\n" + e.getMessage(), false);
        }

        if (_defaultRowSet)
        {
            createRowSet("TEST1", CommandType.TABLE, true, true);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private XMultiServiceFactory getFactory()
    {
        return (XMultiServiceFactory) param.getMSF();
    }

    // --------------------------------------------------------------------------------------------------------
    /** creates a com.sun.star.sdb.RowSet to use during the test
     *  @param command
     *      the command to use for the RowSet
     *  @param commandType
     *      the command type to use for the RowSet
     *  @param execute
     *      determines whether the RowSet should be executed
     */
    private void createRowSet(String command, int commandType, boolean execute)
    {
        createRowSet(command, commandType, execute, false);
    }

    // --------------------------------------------------------------------------------------------------------
    /** creates a com.sun.star.sdb.RowSet to use during the test
     *  @param command
     *      the command to use for the RowSet
     *  @param commandType
     *      the command type to use for the RowSet
     *  @param limitFetchSize
     *      determines whether the fetch size of the RowSet should be limited to MAX_FETCH_ROWS
     *  @param execute
     *      determines whether the RowSet should be executed
     */
    private void createRowSet(String command, int commandType, boolean execute, boolean limitFetchSize)
    {
        try
        {
            m_rowSet = (XRowSet) UnoRuntime.queryInterface(XRowSet.class,
                    getFactory().createInstance("com.sun.star.sdb.RowSet"));
            final XPropertySet rowSetProperties = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, m_rowSet);
            rowSetProperties.setPropertyValue("Command", command);
            rowSetProperties.setPropertyValue("CommandType", Integer.valueOf(commandType));
            rowSetProperties.setPropertyValue("ActiveConnection", m_database.defaultConnection());
            if (limitFetchSize)
            {
                rowSetProperties.setPropertyValue("FetchSize", Integer.valueOf(MAX_FETCH_ROWS));
            }

            m_resultSet = (XResultSet) UnoRuntime.queryInterface(XResultSet.class, m_rowSet);
            m_resultSetUpdate = (XResultSetUpdate) UnoRuntime.queryInterface(XResultSetUpdate.class, m_rowSet);
            m_row = (XRow) UnoRuntime.queryInterface(XRow.class, m_rowSet);
            m_rowLocate = (XRowLocate) UnoRuntime.queryInterface(XRowLocate.class, m_resultSet);
            m_rowSetProperties = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, m_rowSet);
            m_paramsSupplier = (XParametersSupplier) UnoRuntime.queryInterface(XParametersSupplier.class, m_rowSet);

            if (execute)
            {
                m_rowSet.execute();
            }
        }
        catch (Exception e)
        {
            assure("caught an exception while creating the RowSet. Type:\n" + e.getClass().toString() + "\nMessage:\n" + e.getMessage(), false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    public void testRowSet() throws java.lang.Exception
    {

        log.println("testing testRowSet");
        createTestCase(true);

        // sequential postioning
        m_resultSet.beforeFirst();
        testSequentialPositining(m_resultSet, m_row);

        // absolute positioning
        testAbsolutePositioning(m_resultSet, m_row);

        // 3rd test
        test3(createClone(), m_resultSet);
        // 4th test
        test4(m_resultSet);

        // concurrent (multi threaded) access to the row set and its clones
        testConcurrentAccess(m_resultSet);
    }

    // --------------------------------------------------------------------------------------------------------
    XResultSet createClone() throws SQLException
    {
        final XResultSetAccess rowAcc = (XResultSetAccess) UnoRuntime.queryInterface(XResultSetAccess.class, m_rowSet);
        return rowAcc.createResultSet();
    }

    // --------------------------------------------------------------------------------------------------------
    void createStruture() throws SQLException
    {
        m_database.executeSQL("DROP TABLE \"TEST1\" IF EXISTS");
        m_database.executeSQL("CREATE TABLE \"TEST1\" (\"ID\" integer not null primary key, \"col2\" varchar(50) )");

        final XConnection connection = m_database.defaultConnection();
        final XPreparedStatement prep = connection.prepareStatement("INSERT INTO \"TEST1\" values (?,?)");
        final XParameters para = (XParameters) UnoRuntime.queryInterface(XParameters.class, prep);
        for (int i = 1; i <= MAX_TABLE_ROWS; ++i)
        {
            para.setInt(1, i);
            para.setString(2, "Test" + i);
            prep.executeUpdate();
        }

        final XTablesSupplier suppTables = (XTablesSupplier) UnoRuntime.queryInterface(XTablesSupplier.class, connection);
        final XRefreshable refresh = (XRefreshable) UnoRuntime.queryInterface(XRefreshable.class, suppTables.getTables());
        refresh.refresh();
    }

    // --------------------------------------------------------------------------------------------------------
    void testPosition(XResultSet m_resultSet, XRow m_row, int expectedValue, String location) throws SQLException
    {
        final int val = m_row.getInt(1);
        final int pos = m_resultSet.getRow();
        assure(location + ": value/position do not match: " + pos + " (pos) != " + val + " (val)", val == pos);
        assure(location + ": value/position are not as expected: " + val + " (val) != " + expectedValue + " (expected)", val == expectedValue);
    }

    // --------------------------------------------------------------------------------------------------------
    void testSequentialPositining(XResultSet _resultSet, XRow _row)
    {
        try
        {
            // 1st test
            int i = 1;
            while (_resultSet.next())
            {
                testPosition(_resultSet, _row, i, "testSequentialPositining");
                ++i;
            }
        }
        catch (Exception e)
        {
            assure("testSequentialPositining failed: " + e, false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    void testAbsolutePositioning(XResultSet _resultSet, XRow _row)
    {
        try
        {
            for (int i = 1; i <= MAX_FETCH_ROWS; ++i)
            {
                final int calcPos = (MAX_TABLE_ROWS % i) + 1;
                assure("testAbsolutePositioning failed", _resultSet.absolute(calcPos));
                testPosition(_resultSet, _row, calcPos, "testAbsolutePositioning");
            }
        }
        catch (Exception e)
        {
            assure("testAbsolutePositioning failed: " + e, false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    void test3(XResultSet clone, XResultSet _resultSet)
    {
        try
        {
            final XRow _row = (XRow) UnoRuntime.queryInterface(XRow.class, _resultSet);
            final XRow cloneRow = (XRow) UnoRuntime.queryInterface(XRow.class, clone);
            for (int i = 1; i <= MAX_FETCH_ROWS; ++i)
            {
                final int calcPos = (MAX_TABLE_ROWS % i) + 1;
                if (clone.absolute(calcPos))
                {
                    testPosition(clone, cloneRow, calcPos, "test3");
                    testAbsolutePositioning(_resultSet, _row);
                    testAbsolutePositioning(clone, cloneRow);
                }
            }
        }
        catch (Exception e)
        {
            assure("test3 failed: " + e, false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    void test4(XResultSet _resultSet)
    {
        try
        {
            final XRow _row = (XRow) UnoRuntime.queryInterface(XRow.class, _resultSet);
            _resultSet.beforeFirst();

            for (int i = 1; i <= MAX_TABLE_ROWS; ++i)
            {
                _resultSet.next();
                final XResultSet clone = createClone();
                final XRow cloneRow = (XRow) UnoRuntime.queryInterface(XRow.class, clone);
                final int calcPos = MAX_TABLE_ROWS - 1;
                if (calcPos != 0 && clone.absolute(calcPos))
                {
                    testPosition(clone, cloneRow, calcPos, "test4: clone");
                    testPosition(_resultSet, _row, i, "test4: rowset");
                }
            }
        }
        catch (Exception e)
        {
            assure("test4 failed: " + e, false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    void testConcurrentAccess(XResultSet _resultSet)
    {
        log.println("testing Thread");
        try
        {
            // final XRow _row = (XRow)UnoRuntime.queryInterface(XRow.class,_resultSet);
            _resultSet.beforeFirst();

            final int numberOfThreads = 10;

            final Thread threads[] = new Thread[numberOfThreads];
            for (int i = 0; i < numberOfThreads; ++i)
            {
                threads[i] = new Thread(new ResultSetMovementStress(createClone(), i));
                log.println("starting thread " + (i + 1) + " of " + (numberOfThreads));
                threads[i].start();
            }

            for (int i = 0; i < numberOfThreads; ++i)
            {
                threads[i].join();
            }
        }
        catch (Exception e)
        {
            assure("testConcurrentAccess failed: " + e, false);
        }
    }
    // --------------------------------------------------------------------------------------------------------

    public void testRowSetEvents() throws java.lang.Exception
    {
        log.println("testing RowSet Events");
        createTestCase(true);

        // first we create our RowSet object
        final RowSetEventListener pRow = new RowSetEventListener();

        final XColumnsSupplier colSup = (XColumnsSupplier) UnoRuntime.queryInterface(XColumnsSupplier.class, m_rowSet);
        final XPropertySet col = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, colSup.getColumns().getByName("ID"));
        col.addPropertyChangeListener("Value", pRow);
        m_rowSetProperties.addPropertyChangeListener("IsModified", pRow);
        m_rowSetProperties.addPropertyChangeListener("IsNew", pRow);
        m_rowSetProperties.addPropertyChangeListener("IsRowCountFinal", pRow);
        m_rowSetProperties.addPropertyChangeListener("RowCount", pRow);

        final XRowSetApproveBroadcaster xApBroad = (XRowSetApproveBroadcaster) UnoRuntime.queryInterface(XRowSetApproveBroadcaster.class, m_resultSet);
        xApBroad.addRowSetApproveListener(pRow);
        m_rowSet.addRowSetListener(pRow);

        // do some movements to check if we got all notifications
        final Class cResSet = Class.forName("com.sun.star.sdbc.XResultSet");
        final boolean moves[] = new boolean[9];
        for (int i = 0; i < moves.length; ++i)
        {
            moves[i] = false;
        }
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = true;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.CURSOR_MOVED] = true;
        moves[RowSetEventListener.IS_ROW_COUNT_FINAL] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;

        testCursorMove(m_resultSet, cResSet.getMethod("afterLast", (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_ROW_COUNT_FINAL] = false;
        moves[RowSetEventListener.ROW_COUNT] = false;
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod("last", (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod("first", (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod("previous", (Class[]) null), pRow, moves, null);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);
        moves[RowSetEventListener.IS_MODIFIED] = true;
        final XRowUpdate updRow = (XRowUpdate) UnoRuntime.queryInterface(XRowUpdate.class, m_resultSet);
        updRow.updateString(2, TEST21);
        testCursorMove(m_resultSet, cResSet.getMethod(NEXT, (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_MODIFIED] = false;
        final Class cupd = Class.forName("com.sun.star.sdbc.XResultSetUpdate");
        final XResultSetUpdate upd = (XResultSetUpdate) UnoRuntime.queryInterface(XResultSetUpdate.class, m_resultSet);
        testCursorMove(upd, cupd.getMethod("moveToInsertRow", (Class[]) null), pRow, moves, null);

        updRow.updateInt(1, MAX_TABLE_ROWS + 2);
        updRow.updateString(2, "HHHH");
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = false;
        moves[RowSetEventListener.CURSOR_MOVED] = false;
        moves[RowSetEventListener.IS_MODIFIED] = true;
        moves[RowSetEventListener.IS_NEW] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        moves[RowSetEventListener.APPROVE_ROW_CHANGE] = true;
        moves[RowSetEventListener.ROW_CHANGED] = true;
        testCursorMove(upd, cupd.getMethod("insertRow", (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_NEW] = false;
        moves[RowSetEventListener.ROW_COUNT] = false;
        m_resultSet.first();
        updRow.updateInt(1, MAX_TABLE_ROWS + 3);
        updRow.updateString(2, "__");
        testCursorMove(upd, cupd.getMethod("updateRow", (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_NEW] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        m_resultSet.first();
        testCursorMove(upd, cupd.getMethod("deleteRow", (Class[]) null), pRow, moves, null);

        moves[RowSetEventListener.IS_NEW] = false;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.ROW_COUNT] = false;
        m_resultSet.first();
        updRow.updateString(2, TEST21);
        testCursorMove(m_resultSet, cResSet.getMethod("refreshRow", (Class[]) null), pRow, moves, null);

        m_resultSet.first();
        updRow.updateString(2, TEST21);
        testCursorMove(upd, cupd.getMethod("cancelRowUpdates", (Class[]) null), pRow, moves, null);

        for (int i = 0; i < moves.length; ++i)
        {
            moves[i] = false;
        }
        moves[RowSetEventListener.APPROVE_CURSOR_MOVE] = true;
        moves[RowSetEventListener.COLUMN_VALUE] = true;
        moves[RowSetEventListener.CURSOR_MOVED] = true;

        final Class cloc = Class.forName("com.sun.star.sdbcx.XRowLocate");
        m_resultSet.first();
        final Object bookmark = m_rowLocate.getBookmark();
        m_resultSet.next();
        final Object temp[] = new Object[1];
        temp[0] = bookmark;
        Class ctemp[] = new Class[1];
        ctemp[0] = Object.class;
        testCursorMove(m_rowLocate, cloc.getMethod("moveToBookmark", ctemp), pRow, moves, temp);

        final Object temp2[] = new Object[2];
        temp2[0] = bookmark;
        temp2[1] = Integer.valueOf(1);
        final Class ctemp2[] = new Class[2];
        ctemp2[0] = Object.class;
        ctemp2[1] = int.class;
        testCursorMove(m_rowLocate, cloc.getMethod("moveRelativeToBookmark", ctemp2), pRow, moves, temp2);

        for (int i = 0; i < moves.length; ++i)
        {
            moves[i] = false;
        }
        moves[RowSetEventListener.APPROVE_ROW_CHANGE] = true;
        moves[RowSetEventListener.ROW_CHANGED] = true;
        moves[RowSetEventListener.ROW_COUNT] = true;
        final Class cdelRows = Class.forName("com.sun.star.sdbcx.XDeleteRows");
        ctemp[0] = Object[].class;
        final XDeleteRows delRows = (XDeleteRows) UnoRuntime.queryInterface(XDeleteRows.class, m_resultSet);
        final Object bookmarks[] = new Object[5];
        m_resultSet.first();
        for (int i = 0; i < bookmarks.length; ++i)
        {
            m_resultSet.next();
            bookmarks[i] = m_rowLocate.getBookmark();
        }

        temp[0] = bookmarks;
        testCursorMove(delRows, cdelRows.getMethod("deleteRows", ctemp), pRow, moves, temp);

        // now destroy the RowSet
        final XComponent xComp = (XComponent) UnoRuntime.queryInterface(XComponent.class, m_resultSet);
        xComp.dispose();
    }

    // --------------------------------------------------------------------------------------------------------
    private void testCursorMove(Object res, Method _method, RowSetEventListener _evt, boolean _must[], Object args[]) throws java.lang.Exception
    {
        _evt.clearCalling();
        _method.invoke(res, args);

        log.println("testing events for " + _method.getName());
        final int calling[] = _evt.getCalling();
        int pos = 1;
        assure("Callings are not in the correct order for APPROVE_CURSOR_MOVE ",
                (!_must[RowSetEventListener.APPROVE_CURSOR_MOVE] || calling[RowSetEventListener.APPROVE_CURSOR_MOVE] == -1) || calling[RowSetEventListener.APPROVE_CURSOR_MOVE] == pos++);
        assure("Callings are not in the correct order for APPROVE_ROW_CHANGE",
                (!_must[RowSetEventListener.APPROVE_ROW_CHANGE] || calling[RowSetEventListener.APPROVE_ROW_CHANGE] == -1) || calling[RowSetEventListener.APPROVE_ROW_CHANGE] == pos++);
        assure("Callings are not in the correct order for COLUMN_VALUE",
                (!_must[RowSetEventListener.COLUMN_VALUE] || calling[RowSetEventListener.COLUMN_VALUE] == -1) || calling[RowSetEventListener.COLUMN_VALUE] == pos++);
        assure("Callings are not in the correct order for CURSOR_MOVED",
                (!_must[RowSetEventListener.CURSOR_MOVED] || calling[RowSetEventListener.CURSOR_MOVED] == -1) || calling[RowSetEventListener.CURSOR_MOVED] == pos++);
        assure("Callings are not in the correct order for ROW_CHANGED",
                (!_must[RowSetEventListener.ROW_CHANGED] || calling[RowSetEventListener.ROW_CHANGED] == -1) || calling[RowSetEventListener.ROW_CHANGED] == pos++);
        assure("Callings are not in the correct order for IS_MODIFIED",
                (!_must[RowSetEventListener.IS_MODIFIED] || calling[RowSetEventListener.IS_MODIFIED] == -1) || calling[RowSetEventListener.IS_MODIFIED] == pos++);
        assure("Callings are not in the correct order for IS_NEW",
                (!_must[RowSetEventListener.IS_NEW] || calling[RowSetEventListener.IS_NEW] == -1) || calling[RowSetEventListener.IS_NEW] == pos++);
        assure("Callings are not in the correct order for ROW_COUNT",
                (!_must[RowSetEventListener.ROW_COUNT] || calling[RowSetEventListener.ROW_COUNT] == -1) || calling[RowSetEventListener.ROW_COUNT] == pos++);
        assure("Callings are not in the correct order for IS_ROW_COUNT_FINAL",
                (!_must[RowSetEventListener.IS_ROW_COUNT_FINAL] || calling[RowSetEventListener.IS_ROW_COUNT_FINAL] == -1) || calling[RowSetEventListener.IS_ROW_COUNT_FINAL] == pos);

        _evt.clearCalling();
    }

    // --------------------------------------------------------------------------------------------------------
    /** returns the current row count of the RowSet
     */
    private int currentRowCount() throws UnknownPropertyException, WrappedTargetException
    {
        final Integer rowCount = (Integer) m_rowSetProperties.getPropertyValue("RowCount");
        return rowCount.intValue();
    }

    // --------------------------------------------------------------------------------------------------------
    /** positions the row set at an arbitrary position between 2 and (current row count - 1)
     */
    private int positionRandom() throws SQLException, UnknownPropertyException, WrappedTargetException
    {
        final int position = (new Random()).nextInt(currentRowCount() - 2) + 2;
        assure("sub task failed: could not position to row no. " + (Integer.valueOf(position)).toString(),
                m_resultSet.absolute(position));
        return m_resultSet.getRow();
    }

    // --------------------------------------------------------------------------------------------------------
    /** moves the result set to a random record between 2 and (current row count - 1), and deletes this record
     *
     *  After returning from this method, the row set is still positioned at the deleted record
     *  @return
     *      the number/position of the record which has been deleted
     */
    private int deleteRandom() throws SQLException, UnknownPropertyException, WrappedTargetException
    {
        // check if the current position and the row count in the result set is changed by a deletion (it should not)
        final int positionBefore = positionRandom();
        final int rowCountBefore = currentRowCount();

        m_resultSetUpdate.deleteRow();

        final int positionAfter = m_resultSet.getRow();
        final int rowCountAfter = currentRowCount();
        assure("position changed during |deleteRow| (it should not)", positionAfter == positionBefore);
        assure("row count changed with a |deleteRow| (it should not)", rowCountBefore == rowCountAfter);
        assure("RowSet does not report the current row as deleted after |deleteRow|", m_resultSet.rowDeleted());

        return positionBefore;
    }

    // --------------------------------------------------------------------------------------------------------
    public void testDeleteBehavior() throws Exception
    {
        createTestCase(true);

        // ensure that all records are known
        m_resultSet.last();
        final int initialRowCount = currentRowCount();

        // delete a random row
        int deletedRow = deleteRandom();

        // .....................................................................................................
        // asking for the bookmark of a deleted row should fail
        boolean caughtException = false;
        try
        {
            m_rowLocate.getBookmark();
        }
        catch (SQLException e)
        {
            caughtException = true;
        }
        assure("asking for the bookmark of a deleted row should throw an exception", caughtException);

        // .....................................................................................................
        // isXXX methods should return |false| on a deleted row
        assure("one of the isFoo failed after |deleteRow|", !m_resultSet.isBeforeFirst() && !m_resultSet.isAfterLast() && !m_resultSet.isFirst() && !m_resultSet.isLast());
        // note that we can assume that isFirst / isLast also return |false|, since deleteRandom did
        // not position on the first or last record, but inbetween

        // .....................................................................................................
        // check if moving away from this row in either direction yields the expected results
        assure("|previous| after |deleteRow| failed", m_resultSet.previous());
        final int positionPrevious = m_resultSet.getRow();
        assure("position after |previous| after |deleteRow| is not as expected", positionPrevious == deletedRow - 1);

        deletedRow = deleteRandom();
        assure("|next| after |deleteRow| failed", m_resultSet.next());
        final int positionAfter = m_resultSet.getRow();
        assure("position after |next| after |deleteRow| is not as expected", positionAfter == deletedRow);
        // since the deleted record "vanishs" as soon as the cursor is moved away from it, the absolute position does
        // not change with a |next| call here

        // .....................................................................................................
        // check if the deleted rows really vanished after moving away from them
        assure("row count did not change as expected after two deletions", initialRowCount - 2 == currentRowCount());

        // .....................................................................................................
        // check if the deleted row vanishes after moving to the insertion row
        final int rowCountBefore = currentRowCount();
        final int deletedPos = deleteRandom();
        m_resultSetUpdate.moveToInsertRow();
        assure("moving to the insertion row immediately after |deleteRow| does not adjust the row count", rowCountBefore == currentRowCount() + 1);

        m_resultSetUpdate.moveToCurrentRow();
        assure("|moveToCurrentRow| after |deleteRow| + |moveToInsertRow| results in unexpected position",
                (m_resultSet.getRow() == deletedPos) && !m_resultSet.rowDeleted());

        // the same, but this time with deleting the first row (which is not covered by deleteRandom)
        m_resultSet.last();
        m_resultSetUpdate.deleteRow();
        m_resultSetUpdate.moveToInsertRow();
        m_resultSetUpdate.moveToCurrentRow();
        assure("|last| + |deleteRow| + |moveToInsertRow| + |moveToCurrentRow| results in wrong state", m_resultSet.isAfterLast());

        // .....................................................................................................
        // check if deleting a deleted row fails as expected
        deleteRandom();
        caughtException = false;
        try
        {
            m_resultSetUpdate.deleteRow();
        }
        catch (SQLException e)
        {
            caughtException = true;
        }
        assure("deleting a deleted row succeeded - it shouldn't", caughtException);

        // .....................................................................................................
        // check if deleteRows fails if it contains the bookmark of a previously-deleted row
        m_resultSet.first();
        final Object firstBookmark = m_rowLocate.getBookmark();
        positionRandom();
        final Object deleteBookmark = m_rowLocate.getBookmark();
        m_resultSetUpdate.deleteRow();
        final XDeleteRows multiDelete = (XDeleteRows) UnoRuntime.queryInterface(XDeleteRows.class, m_resultSet);
        final int[] deleteSuccess = multiDelete.deleteRows(new Object[]
                {
                    firstBookmark, deleteBookmark
                });
        assure("XDeleteRows::deleteRows with the bookmark of an already-deleted row failed",
                (deleteSuccess.length == 2) && (deleteSuccess[0] != 0) && (deleteSuccess[1] == 0));

        // .....................................................................................................
        // check if refreshing a deleted row fails as expected
        deleteRandom();
        caughtException = false;
        try
        {
            m_resultSet.refreshRow();
        }
        catch (SQLException e)
        {
            caughtException = true;
        }
        assure("refreshing a deleted row succeeded - it shouldn't", caughtException);

        // .....................................................................................................
        // rowUpdated/rowDeleted
        deleteRandom();
        assure("rowDeleted and/or rowUpdated are wrong on a deleted row", !m_resultSet.rowUpdated() && !m_resultSet.rowInserted());

        // .....................................................................................................
        // updating values in a deleted row should fail
        deleteRandom();
        final XRowUpdate rowUpdated = (XRowUpdate) UnoRuntime.queryInterface(XRowUpdate.class, m_resultSet);
        caughtException = false;
        try
        {
            rowUpdated.updateString(2, TEST21);
        }
        catch (SQLException e)
        {
            caughtException = true;
        }
        assure("updating values in a deleted row should not succeed", caughtException);
    }

    // --------------------------------------------------------------------------------------------------------
    /** checks whether deletions on the main RowSet properly interfere (or don't interfere) with the movement
     *  on a clone of the RowSet
     */
    public void testCloneMovesPlusDeletions() throws SQLException, UnknownPropertyException, WrappedTargetException
    {
        createTestCase(true);
        // ensure that all records are known
        m_resultSet.last();

        final XResultSet clone = createClone();
        final XRowLocate cloneRowLocate = (XRowLocate) UnoRuntime.queryInterface(XRowLocate.class, clone);

        positionRandom();

        // .....................................................................................................
        // move the clone to the same record as the RowSet, and delete this record
        cloneRowLocate.moveToBookmark(m_rowLocate.getBookmark());
        final int clonePosition = clone.getRow();
        m_resultSetUpdate.deleteRow();

        assure("clone doesn't know that its current row has been deleted via the RowSet", clone.rowDeleted());
        assure("clone's position changed somehow during deletion", clonePosition == clone.getRow());

        // .....................................................................................................
        // move the row set away from the deleted record. This should still not touch the state of the clone
        m_resultSet.previous();

        assure("clone doesn't know (anymore) that its current row has been deleted via the RowSet", clone.rowDeleted());
        assure("clone's position changed somehow during deletion and RowSet-movement", clonePosition == clone.getRow());

        // .....................................................................................................
        // move the clone away from the deleted record
        clone.next();
        assure("clone still assumes that its row is deleted - but we already moved it", !clone.rowDeleted());

        // .....................................................................................................
        // check whether deleting the extremes (first / last) work
        m_resultSet.first();
        cloneRowLocate.moveToBookmark(m_rowLocate.getBookmark());
        m_resultSetUpdate.deleteRow();
        clone.previous();
        assure("deleting the first record left the clone in a strange state (after |previous|)", clone.isBeforeFirst());
        clone.next();
        assure("deleting the first record left the clone in a strange state (after |previous| + |next|)", clone.isFirst());

        m_resultSet.last();
        cloneRowLocate.moveToBookmark(m_rowLocate.getBookmark());
        m_resultSetUpdate.deleteRow();
        clone.next();
        assure("deleting the last record left the clone in a strange state (after |next|)", clone.isAfterLast());
        clone.previous();
        assure("deleting the first record left the clone in a strange state (after |next| + |previous|)", clone.isLast());

        // .....................................................................................................
        // check whether movements of the clone interfere with movements of the RowSet, if the latter is on a deleted row
        final int positionBefore = positionRandom();
        m_resultSetUpdate.deleteRow();
        assure("|deleteRow|, but no |rowDeleted| (this should have been found much earlier!)", m_resultSet.rowDeleted());
        clone.beforeFirst();
        while (clone.next());
        assure("row set forgot that the current row is deleted", m_resultSet.rowDeleted());

        assure("moving to the next record after |deleteRow| and clone moves failed", m_resultSet.next());
        assure("wrong position after |deleteRow| and clone movement", !m_resultSet.isAfterLast() && !m_resultSet.isBeforeFirst());
        assure("wrong absolute position after |deleteRow| and clone movement", m_resultSet.getRow() == positionBefore);
    }

    // --------------------------------------------------------------------------------------------------------
    /** checks whether insertions on the main RowSet properly interfere (or don't interfere) with the movement
     *  on a clone of the RowSet
     */
    public void testCloneMovesPlusInsertions() throws SQLException, UnknownPropertyException, WrappedTargetException, PropertyVetoException, com.sun.star.lang.IllegalArgumentException
    {
        createTestCase(true);
        // ensure that all records are known
        m_rowSetProperties.setPropertyValue("FetchSize", Integer.valueOf(10));

        final XResultSet clone = createClone();
        final XRow cloneRow = (XRow) UnoRuntime.queryInterface(XRow.class, clone);

        // .....................................................................................................
        // first check the basic scenario without the |moveToInsertRow| |moveToCurrentRow|, to ensure that
        // really those are broken, if at all
        m_resultSet.last();
        clone.first();
        clone.absolute(11);
        clone.first();

        final int rowValue1 = m_row.getInt(1);
        final int rowPos = m_resultSet.getRow();
        final int rowValue2 = m_row.getInt(1);
        assure("repeated query for the same column value delivers different values (" + rowValue1 + " and " + rowValue2 + ") on row: " + rowPos,
                rowValue1 == rowValue2);

        testPosition(clone, cloneRow, 1, "mixed clone/rowset move: clone check");
        testPosition(m_resultSet, m_row, MAX_TABLE_ROWS, "mixed clone/rowset move: rowset check");

        // .....................................................................................................
        // now the complete scenario
        m_resultSet.last();
        m_resultSetUpdate.moveToInsertRow();
        clone.first();
        clone.absolute(11);
        clone.first();
        m_resultSetUpdate.moveToCurrentRow();

        testPosition(clone, cloneRow, 1, "mixed clone/rowset move/insertion: clone check");
        testPosition(m_resultSet, m_row, 100, "mixed clone/rowset move/insertion: rowset check");
    }

    // --------------------------------------------------------------------------------------------------------
    private void testTableParameters()
    {
        // for a row set simply based on a table, there should be not parameters at all
        createRowSet("products", CommandType.TABLE, false);
        try
        {
            verifyParameters(new String[]
                    {
                    }, "testTableParameters");
        }
        catch (Exception e)
        {
            assure("testing the parameters of a table failed" + e.getMessage(), false);
        }
    }
    // --------------------------------------------------------------------------------------------------------

    private void testParametersAfterNormalExecute()
    {
        try
        {
            createRowSet("SELECT * FROM \"customers\"", CommandType.COMMAND, true);
            m_rowSetProperties.setPropertyValue("Command", "SELECT * FROM \"customers\" WHERE \"City\" = :city");
            final XParameters rowsetParams = (XParameters) UnoRuntime.queryInterface(XParameters.class,
                    m_rowSet);
            rowsetParams.setString(1, "London");
            m_rowSet.execute();
        }
        catch (Exception e)
        {
            assure("testing the parameters of a table failed" + e.getMessage(), false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void verifyParameters(String[] _paramNames, String _context) throws com.sun.star.uno.Exception
    {
        final XIndexAccess params = m_paramsSupplier.getParameters();
        final int expected = _paramNames.length;
        final int found = params != null ? params.getCount() : 0;

        assure("wrong number of parameters (expected: " + expected + ", found: " + found + ") in " + _context,
                found == expected);

        if (found == 0)
        {
            return;
        }

        for (int i = 0; i < expected; ++i)
        {
            final XPropertySet parameter = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class,
                    params.getByIndex(i));

            final String expectedName = _paramNames[i];
            final String foundName = (String) parameter.getPropertyValue("Name");
            assure("wrong parameter name (expected: " + expectedName + ", found: " + foundName + ") in" + _context,
                    expectedName.equals(foundName));
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void testParametrizedQuery()
    {
        try
        {
            // for a row set based on a parametrized query, those parameters should be properly
            // recognized
            m_dataSource.createQuery("products like", "SELECT * FROM \"products\" WHERE \"Name\" LIKE :product_name");
            createRowSet("products like", CommandType.QUERY, false);
            verifyParameters(new String[]
                    {
                        "product_name"
                    }, "testParametrizedQuery");
        }
        catch (Exception e)
        {
            assure("testing the parameters of a parametrized query failed" + e.getMessage(), false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void testParametersInteraction()
    {
        try
        {
            createRowSet("products like", CommandType.QUERY, false);

            // let's fill in a parameter value via XParameters, and see whether it is respected by the parameters container
            final XParameters rowsetParams = (XParameters) UnoRuntime.queryInterface(XParameters.class,
                    m_rowSet);
            rowsetParams.setString(1, "Apples");

            XIndexAccess params = m_paramsSupplier.getParameters();
            XPropertySet firstParam = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, params.getByIndex(0));
            Object firstParamValue = firstParam.getPropertyValue("Value");

            assure("XParameters and the parameters container do not properly interact",
                    "Apples".equals(firstParamValue));

            // let's see whether this also survices an execute of the row set
            rowsetParams.setString(1, "Oranges");
            m_rowSet.execute();
            {
                // TODO: the following would not be necessary if the parameters container would *survive*
                // the execution of the row set. It currently doesn't (though the values it represents do).
                // It would be nice, but not strictly necessary, if it would.
                params = m_paramsSupplier.getParameters();
                firstParam = (XPropertySet) UnoRuntime.queryInterface(XPropertySet.class, params.getByIndex(0));
            }
            firstParamValue = firstParam.getPropertyValue("Value");
            assure("XParameters and the parameters container do not properly interact, after the row set has been executed",
                    "Oranges".equals(firstParamValue));
        }
        catch (Exception e)
        {
            assure("could not text the relationship between XParameters and XParametersSupplier" + e.getMessage(), false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    private void testParametersInFilter()
    {
        try
        {
            createRowSet("SELECT * FROM \"customers\"", CommandType.COMMAND, false);
            m_rowSetProperties.setPropertyValue("Filter", "\"City\" = :city");

            m_rowSetProperties.setPropertyValue("ApplyFilter", Boolean.TRUE);
            verifyParameters(new String[]
                    {
                        "city"
                    }, "testParametersInFilter");

            m_rowSetProperties.setPropertyValue("ApplyFilter", Boolean.FALSE);
            verifyParameters(new String[]
                    {
                    }, "testParametersInFilter");
        }
        catch (Exception e)
        {
            assure("testing the parameters within a WHERE clause failed" + e.getMessage(), false);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    /** checks the XParametersSupplier functionality of a RowSet
     */
    public void testParameters()
    {
        createTestCase(false);
        // use an own RowSet instance, not the one which is also used for the other cases

        testTableParameters();
        testParametrizedQuery();
        testParametersInFilter();

        testParametersAfterNormalExecute();

        testParametersInteraction();
    }
}

