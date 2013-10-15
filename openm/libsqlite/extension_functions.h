/*
SQLite extension functions library, see extension-functions.c for details
*/

#ifndef SQLITE_EXTENSIONS_FUNCTIONS_H
#define SQLITE_EXTENSIONS_FUNCTIONS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifdef COMPILE_SQLITE_EXTENSIONS_AS_LOADABLE_MODULE
    #include "sqlite3ext.h"
    SQLITE_EXTENSION_INIT1
    int sqlite3_extension_init(sqlite3 *db, char **pzErrMsg, const sqlite3_api_routines *pApi);
#else
    #include "sqlite3.h"
    int sqlite3_extension_functions_init(sqlite3 *db);
#endif

#ifdef __cplusplus
}
#endif

#endif  /* SQLITE_EXTENSIONS_FUNCTIONS_H */
