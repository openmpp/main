// OpenM++ data library: task_set table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // task_set table implementation
    class TaskSetTable : public ITaskSetTable
    {
    public:
        ~TaskSetTable() noexcept;

        // select table rows by specified filter, sorted by primary key: task id, set id
        static vector<TaskSetRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        TaskSetTable(const TaskSetTable & i_table) = delete;
        TaskSetTable & operator=(const TaskSetTable & i_table) = delete;
    };

    // Columns type for task_set row
    static const type_info * typeTaskSetRow[] = { 
        &typeid(decltype(TaskSetRow::taskId)),
        &typeid(decltype(TaskSetRow::setId))
    };

    // Size (number of columns) for task_set row
    static const int sizeTaskSetRow = sizeof(typeTaskSetRow) / sizeof(const type_info *);

    // Row adapter to select task_set rows
    class TaskSetRowAdapter : public IRowAdapter
    {
    public:
        TaskSetRowAdapter() { }

        IRowBase * createRow(void) const { return new TaskSetRow(); }
        int size(void) const { return sizeTaskSetRow; }
        const type_info * const * columnTypes(void) const { return typeTaskSetRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TaskSetRow *>(i_row)->taskId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TaskSetRow *>(i_row)->setId = (*(int *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITaskSetTable::~ITaskSetTable(void) noexcept { }

// Table never unloaded
TaskSetTable::~TaskSetTable(void) noexcept { }

// select table rows
vector<TaskSetRow> ITaskSetTable::select(IDbExec * i_dbExec, int i_taskId)
{
    string sWhere = (i_taskId > 0) ? " WHERE task_id = " + to_string(i_taskId) : "";
    return 
        TaskSetTable::select(i_dbExec, sWhere);
}

// select table rows by task id and set id
vector<TaskSetRow> ITaskSetTable::byKey(IDbExec * i_dbExec, int i_taskId, int i_setId)
{
    string sWhere = " WHERE task_id = " + to_string(i_taskId) + " AND set_id = " + to_string(i_setId);
    return 
        TaskSetTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<TaskSetRow> TaskSetTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = TaskSetRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowVector(
        "SELECT task_id, set_id FROM task_set " + i_where + " ORDER BY 1, 2", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), TaskSetRow::keyLess);

    return IMetaTable<TaskSetRow>::rows(vec);
}

