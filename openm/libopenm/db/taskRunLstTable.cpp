// OpenM++ data library: task_run_lst table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // task_run_lst table implementation
    class TaskRunLstTable : public ITaskRunLstTable
    {
    public:
        ~TaskRunLstTable() noexcept;

        // select table rows by specified filter, sorted by primary key: task id
        static vector<TaskRunLstRow> select(IDbExec * i_dbExec, const string & i_where);

    private:
        TaskRunLstTable(const TaskRunLstTable & i_table) = delete;
        TaskRunLstTable & operator=(const TaskRunLstTable & i_table) = delete;
    };

    // Columns type for task_run_lst row
    static const type_info * typeTaskRunLstRow[] = { 
        &typeid(decltype(TaskRunLstRow::taskRunId)), 
        &typeid(decltype(TaskRunLstRow::taskId)),
        &typeid(decltype(TaskRunLstRow::name)),
        &typeid(decltype(TaskRunLstRow::subCount)),
        &typeid(decltype(TaskRunLstRow::createDateTime)),
        &typeid(decltype(TaskRunLstRow::status)),
        &typeid(decltype(TaskRunLstRow::updateDateTime)),
        &typeid(decltype(TaskRunLstRow::runStamp))
    };

    // Size (number of columns) for task_run_lst row
    static const int sizeTaskRunLstRow = sizeof(typeTaskRunLstRow) / sizeof(const type_info *);

    // Row adapter to select task_run_lst rows
    class TaskRunLstRowAdapter : public IRowAdapter
    {
    public:
        TaskRunLstRowAdapter() { }

        IRowBase * createRow(void) const { return new TaskRunLstRow(); }
        int size(void) const { return sizeTaskRunLstRow; }
        const type_info * const * columnTypes(void) const { return typeTaskRunLstRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TaskRunLstRow *>(i_row)->taskRunId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TaskRunLstRow *>(i_row)->taskId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TaskRunLstRow *>(i_row)->name = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<TaskRunLstRow *>(i_row)->subCount = (*(int *)i_value);
                break;
            case 4:
                dynamic_cast<TaskRunLstRow *>(i_row)->createDateTime = ((const char *)i_value);
                break;
            case 5:
                dynamic_cast<TaskRunLstRow *>(i_row)->status = ((const char *)i_value);
                break;
            case 6:
                dynamic_cast<TaskRunLstRow *>(i_row)->updateDateTime = ((const char *)i_value);
                break;
            case 7:
                dynamic_cast<TaskRunLstRow *>(i_row)->runStamp = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITaskRunLstTable::~ITaskRunLstTable(void) noexcept { }

// Table never unloaded
TaskRunLstTable::~TaskRunLstTable(void) noexcept { }

// select table rows
vector<TaskRunLstRow> ITaskRunLstTable::select(IDbExec * i_dbExec, int i_taskId)
{
    string sWhere = (i_taskId > 0) ? " WHERE task_id = " + to_string(i_taskId) : "";
    return 
        TaskRunLstTable::select(i_dbExec, sWhere);
}

// select table rows by task run id
vector<TaskRunLstRow> ITaskRunLstTable::byKey(IDbExec * i_dbExec, int i_TaskRunId)
{
    string sWhere = " WHERE task_run_id = " + to_string(i_TaskRunId);
    return 
        TaskRunLstTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<TaskRunLstRow> TaskRunLstTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = TaskRunLstRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowVector(
        "SELECT task_run_id, task_id, run_name, sub_count, create_dt, status, update_dt, run_stamp" \
        " FROM task_run_lst " + i_where + " ORDER BY 1", 
        adp
        );
    stable_sort(vec.begin(), vec.end(), TaskRunLstRow::keyLess);

    return IMetaTable<TaskRunLstRow>::rows(vec);
}
