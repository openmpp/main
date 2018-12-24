// OpenM++ data library: task_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // task_lst table implementation
    class TaskLstTable : public ITaskLstTable
    {
    public:
        ~TaskLstTable() noexcept;

        // select table rows by specified filter, sorted by primary key: task id
        static vector<TaskLstRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        TaskLstTable(const TaskLstTable & i_table) = delete;
        TaskLstTable & operator=(const TaskLstTable & i_table) = delete;
    };

    // Columns type for task_lst row
    static const type_info * typeTaskLstRow[] = { 
        &typeid(decltype(TaskLstRow::taskId)), 
        &typeid(decltype(TaskLstRow::modelId)), 
        &typeid(decltype(TaskLstRow::name)) 
    };

    // Size (number of columns) for task_lst row
    static const int sizeTaskLstRow = sizeof(typeTaskLstRow) / sizeof(const type_info *);

    // Row adapter to select task_lst rows
    class TaskLstRowAdapter : public IRowAdapter
    {
    public:
        TaskLstRowAdapter() { }

        IRowBase * createRow(void) const { return new TaskLstRow(); }
        int size(void) const { return sizeTaskLstRow; }
        const type_info * const * columnTypes(void) const { return typeTaskLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TaskLstRow *>(i_row)->taskId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TaskLstRow *>(i_row)->modelId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TaskLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITaskLstTable::~ITaskLstTable(void) noexcept { }

// Table never unloaded
TaskLstTable::~TaskLstTable(void) noexcept { }

// select table rows
vector<TaskLstRow> ITaskLstTable::select(IDbExec * i_dbExec, int i_modelId)
{
    string sWhere = (i_modelId > 0) ? " WHERE model_id = " + to_string(i_modelId) : "";
    return 
        TaskLstTable::select(i_dbExec, sWhere);
}

// select table rows by task id
vector<TaskLstRow> ITaskLstTable::byKey(IDbExec * i_dbExec, int i_taskId)
{
    string sWhere = " WHERE task_id = " + to_string(i_taskId);
    return 
        TaskLstTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<TaskLstRow> TaskLstTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = TaskLstRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowVector(
        "SELECT task_id, model_id, task_name FROM task_lst " + i_where + " ORDER BY 1", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), TaskLstRow::keyLess);

    return IMetaTable<TaskLstRow>::rows(vec);
}
