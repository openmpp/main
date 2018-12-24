// OpenM++ data library: task_txt table
// Copyright (c) 2013-2015 OpenM++
// This code is licensed under the MIT license (see LICENSE.txt for details)

#include "libopenm/db/dbMetaRow.h"
#include "dbMetaTable.h"
using namespace openm;

namespace openm
{
    // task_txt table implementation
    class TaskTxtTable : public ITaskTxtTable
    {
    public:
        ~TaskTxtTable() noexcept;

        // select table rows by specified filter, sorted by primary key: task id and language id
        static vector<TaskTxtRow> select(IDbExec * i_dbExec, const string & i_fromWhere);

    private:
        TaskTxtTable(const TaskTxtTable & i_table) = delete;
        TaskTxtTable & operator=(const TaskTxtTable & i_table) = delete;
    };

    // Columns type for task_txt row
    static const type_info * typeTaskTxtRow[] = { 
        &typeid(decltype(TaskTxtRow::taskId)), 
        &typeid(decltype(TaskTxtRow::langId)), 
        &typeid(decltype(TaskTxtRow::descr)),
        &typeid(decltype(TaskTxtRow::note))
    };

    // Size (number of columns) for task_txt row
    static const int sizeTaskTxtRow = sizeof(typeTaskTxtRow) / sizeof(const type_info *);

    // Row adapter to select task_txt rows
    class TaskTxtRowAdapter : public IRowAdapter
    {
    public:
        TaskTxtRowAdapter() { }

        IRowBase * createRow(void) const { return new TaskTxtRow(); }
        int size(void) const { return sizeTaskTxtRow; }
        const type_info * const * columnTypes(void) const { return typeTaskTxtRow; }

        void set(IRowBase * i_row, int i_column, const void * i_value) const
        {
            switch (i_column) {
            case 0:
                dynamic_cast<TaskTxtRow *>(i_row)->taskId = (*(int *)i_value);
                break;
            case 1:
                dynamic_cast<TaskTxtRow *>(i_row)->langId = (*(int *)i_value);
                break;
            case 2:
                dynamic_cast<TaskTxtRow *>(i_row)->descr = ((const char *)i_value);
                break;
            case 3:
                dynamic_cast<TaskTxtRow *>(i_row)->note = ((const char *)i_value);
                break;
            default:
                throw DbException("db column number out of range");
            }
        }
    };
}

// Table never unloaded
ITaskTxtTable::~ITaskTxtTable(void) noexcept { }

// Table never unloaded
TaskTxtTable::~TaskTxtTable(void) noexcept { }

// select table rows
vector<TaskTxtRow> ITaskTxtTable::select(IDbExec * i_dbExec, int i_langId)
{
    string sWhere = (i_langId >= 0) ? " WHERE lang_id = " + to_string(i_langId) : "";
    return 
        TaskTxtTable::select(i_dbExec, sWhere);
}

// select table rows by task id and language id
vector<TaskTxtRow> ITaskTxtTable::byKey(IDbExec * i_dbExec, int i_taskId, int i_langId)
{
    string sWhere = " WHERE task_id = " + to_string(i_taskId) + " AND lang_id = " + to_string(i_langId);
    return 
        TaskTxtTable::select(i_dbExec, sWhere);
}

// select table rows by specified filter
vector<TaskTxtRow> TaskTxtTable::select(IDbExec * i_dbExec, const string & i_where)
{
    if (i_dbExec == NULL) throw DbException("invalid (NULL) database connection");

    const IRowAdapter & adp = TaskTxtRowAdapter();
    IRowBaseVec vec = i_dbExec->selectRowVector(
        "SELECT task_id, lang_id, descr, note FROM task_txt" + i_where + " ORDER BY 1, 2",
        adp
        );
    stable_sort(vec.begin(), vec.end(), TaskTxtRow::keyLess);

    return IMetaTable<TaskTxtRow>::rows(vec);
}
