// OpenM++ data library: task_log table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // task_log table implementation
    class TaskLogTable : public ITaskLogTable
    {
    public:
        ~TaskLogTable() throw();

        // select table rows by specified filter, sorted by primary key: task id
        static vector<TaskLogRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        TaskLogTable(const TaskLogTable & i_table) = delete;
        TaskLogTable & operator=(const TaskLogTable & i_table) = delete;
    };

    // Columns type for task_log row
    static const type_info * typeTaskLogRow[] = { 
        &typeid(decltype(TaskLogRow::taskLogId)), 
        &typeid(decltype(TaskLogRow::taskId)),
        &typeid(decltype(TaskLogRow::subCount)),
        &typeid(decltype(TaskLogRow::createDateTime)),
        &typeid(decltype(TaskLogRow::status)),
        &typeid(decltype(TaskLogRow::updateDateTime))
    };

    // Size (number of columns) for task_log row
    static const int sizeTaskLogRow = sizeof(typeTaskLogRow) / sizeof(const type_info *);

    // Row adapter to select task_log rows
    class TaskLogRowAdapter : public IRowAdapter
    {
    public:
        TaskLogRowAdapter() { }

        IRowBase * createRow(void) const { return new TaskLogRow(); }
        int size(void) const { return sizeTaskLogRow; }
        const type_info ** columnTypes(void) const { return typeTaskLogRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TaskLogRow *>(i_row)->taskLogId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TaskLogRow *>(i_row)->taskId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TaskLogRow *>(i_row)->subCount = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TaskLogRow *>(i_row)->createDateTime = ((const char *)i_value);
                break;
            case 4:
                dynamic_cast<TaskLogRow *>(i_row)->status = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TaskLogRow *>(i_row)->updateDateTime = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITaskLogTable::~ITaskLogTable(void) throw() { }

// Table never unloaded
TaskLogTable::~TaskLogTable(void) throw() { }

// select table rows
vector<TaskLogRow> ITaskLogTable::select(IDbExec * i_dbExec, int i_taskId)
{
    string sWhere = (i_taskId > 0) ? " WHERE task_id = " + to_string(i_taskId) : "";
    return 
        TaskLogTable::select(i_dbExec, sWhere);
}

// select table rows by task log id
vector<TaskLogRow> ITaskLogTable::byKey(IDbExec * i_dbExec, int i_taskLogId)
{
    string sWhere = " WHERE task_log_id = " + to_string(i_taskLogId);
    return 
        TaskLogTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<TaskLogRow> TaskLogTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = TaskLogRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowList(
        "SELECT task_log_id, task_id, sub_count, create_dt, status, update_dt" \
        " FROM task_log " + i_where + " ORDER BY 1", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), TaskLogRow::keyLess);

    return IMetaTable<TaskLogRow>::rows(vec);
}
