// OpenM++ data library: task_run_set table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // task_run_set table implementation
    class TaskRunSetTable : public ITaskRunSetTable
    {
    public:
        ~TaskRunSetTable() throw();

        // select table rows by specified filter, sorted by primary key: task run id, run id
        static vector<TaskRunSetRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        TaskRunSetTable(const TaskRunSetTable & i_table) = delete;
        TaskRunSetTable & operator=(const TaskRunSetTable & i_table) = delete;
    };

    // Columns type for task_run_set row
    static const type_info * typeTaskRunSetRow[] = {
        &typeid(decltype(TaskRunSetRow::taskRunId)),
        &typeid(decltype(TaskRunSetRow::runId)),
        &typeid(decltype(TaskRunSetRow::setId)),
        &typeid(decltype(TaskRunSetRow::taskId))
    };

    // Size (number of columns) for task_run_set row
    static const int sizeTaskRunSetRow = sizeof(typeTaskRunSetRow) / sizeof(const type_info *);

    // Row adapter to select task_run_set rows
    class TaskRunSetRowAdapter : public IRowAdapter
    {
    public:
        TaskRunSetRowAdapter() { }

        IRowBase * createRow(void) const { return new TaskRunSetRow(); }
        int size(void) const { return sizeTaskRunSetRow; }
        const type_info ** columnTypes(void) const { return typeTaskRunSetRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TaskRunSetRow *>(i_row)->taskRunId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TaskRunSetRow *>(i_row)->runId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TaskRunSetRow *>(i_row)->setId = (*(int *)i_value);
                break;
            case 3:
                dynamic_cast<TaskRunSetRow *>(i_row)->taskId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITaskRunSetTable::~ITaskRunSetTable(void) throw() { }

// Table never unloaded
TaskRunSetTable::~TaskRunSetTable(void) throw() { }

// select table rows
vector<TaskRunSetRow> ITaskRunSetTable::select(IDbExec * i_dbExec, int i_taskRunId)
{
    string sWhere = (i_taskRunId > 0) ? " WHERE task_run_id = " + to_string(i_taskRunId) : "";
    return
        TaskRunSetTable::select(i_dbExec, sWhere);
}

// select table rows by task run id, run id
vector<TaskRunSetRow> ITaskRunSetTable::byKey(IDbExec * i_dbExec, int i_taskRunId, int i_runId)
{
    string sWhere = " WHERE task_run_id = " + to_string(i_taskRunId) + " AND run_id = " + to_string(i_runId);
    return
        TaskRunSetTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<TaskRunSetRow> TaskRunSetTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = TaskRunSetRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowList(
        "SELECT task_run_id, run_id, set_id, task_id FROM task_run_set " + i_where + " ORDER BY 1, 2",
        adp
        );
    stable_sort(vec.begin(), vec.end(), TaskRunSetRow::keyLess);

    return IMetaTable<TaskRunSetRow>::rows(vec);
}

