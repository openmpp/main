#!/bin/bash
#
# Xcode modelOne project: copy modelOne.sqlite and *.ini into target dir
#
# set -x
set -e

MODEL_SQLITE=${MODEL_NAME}.sqlite

if [ "$MODEL_SQLITE" = '.sqlite' ]; then
  echo "Invalid (empty) model name. Use .xconfig to set model name."
  exit 1
fi

echo Create: ${MODEL_SQLITE}

if [ ! -d ${OM_SQL_DIR} ] || [ ! -d ${OM_SQLITE_DIR} ]; then
  echo SQL scripts not found: ${OM_SQL_DIR} or ${OM_SQLITE_DIR}
  exit 1
fi

SQLITE_EXE=sqlite3

if ! type ${SQLITE_EXE} >/dev/null 2>&1; then
  echo Not found: ${SQLITE_EXE}. Please install SQLite.
  exit 1
fi

# delete existing model.sqlite
TARGET_MODEL_SQLITE=${TARGET_BUILD_DIR}/${MODEL_SQLITE}

rm -f ${TARGET_MODEL_SQLITE}

# create empty modelOne.sqlite
${SQLITE_EXE} ${TARGET_MODEL_SQLITE} < ${OM_SQL_DIR}/create_db.sql
${SQLITE_EXE} ${TARGET_MODEL_SQLITE} < ${OM_SQL_DIR}/insert_default.sql
${SQLITE_EXE} ${TARGET_MODEL_SQLITE} < ${MODEL_NAME}_create_model.sql
${SQLITE_EXE} ${TARGET_MODEL_SQLITE} < ${MODEL_NAME}_insert_param.sql

# create optional views
${SQLITE_EXE} ${TARGET_MODEL_SQLITE} < ${OM_SQLITE_DIR}/optional_meta_views_sqlite.sql
${SQLITE_EXE} ${TARGET_MODEL_SQLITE} < ${MODEL_NAME}_optional_views.sql

# done
