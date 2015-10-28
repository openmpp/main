// OpenM++ data library: task_run table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // task_run table implementation
    class TaskRunTable : public ITaskRunTable
    {
    public:
        ~TaskRunTable() throw();

        // select table rows by specified filter, sorted by primary key: task id, set id
        static vector<TaskRunRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        TaskRunTable(const TaskRunTable & i_table) = delete;
        TaskRunTable & operator=(const TaskRunTable & i_table) = delete;
    };

    // Columns type for task_run row
    static const type_info * typeTaskRunRow[] = { 
        &typeid(decltype(TaskRunRow::taskId)),
        &typeid(decltype(TaskRunRow::setId)),
        &typeid(decltype(TaskRunRow::runId))
    };

    // Size (number of columns) for task_run row
    static const int sizeTaskRunRow = sizeof(typeTaskRunRow) / sizeof(const type_info *);

    // Row adapter to select task_run rows
    class TaskRunRowAdapter : public IRowAdapter
    {
    public:
        TaskRunRowAdapter() { }

        IRowBase * createRow(void) const { return new TaskRunRow(); }
        int size(void) const { return sizeTaskRunRow; }
        const type_info ** columnTypes(void) const { return typeTaskRunRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TaskRunRow *>(i_row)->taskId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TaskRunRow *>(i_row)->setId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TaskRunRow *>(i_row)->runId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITaskRunTable::~ITaskRunTable(void) throw() { }

// Table never unloaded
TaskRunTable::~TaskRunTable(void) throw() { }

// select table rows
vector<TaskRunRow> ITaskRunTable::select(IDbExec * i_dbExec, int i_taskId)
{
    string sWhere = (i_taskId > 0) ? " WHERE task_id = " + to_string(i_taskId) : "";
    return 
        TaskRunTable::select(i_dbExec, sWhere);
}

// select table rows by task id and set id
vector<TaskRunRow> ITaskRunTable::byKey(IDbExec * i_dbExec, int i_taskId, int i_setId)
{
    string sWhere = " WHERE task_id = " + to_string(i_taskId) + " AND set_id = " + to_string(i_setId);
    return 
        TaskRunTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<TaskRunRow> TaskRunTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = TaskRunRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowList(
        "SELECT task_id, set_id, run_id FROM task_run " + i_where + " ORDER BY 1, 2", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), TaskRunRow::keyLess);

    return IMetaTable<TaskRunRow>::rows(vec);
}

