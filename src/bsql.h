#ifndef BSQL
#define BSQL

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef enum BsqlResult {
	BSQL_SUCCESS = 0,
	BSQL_ERROR = 1,

	// Table errors
	BSQL_TABLE_FILE_OPEN_ERROR = 1000001,
	BSQL_TABLE_FILE_READ_ERROR = 1000002,

	BSQL_TABLE_ADD_COLUMN_ERROR = 1000101,
	BSQL_TABLE_INSERT_COLUMN_ERROR = 1000102,
	BSQL_TABLE_POP_COLUMN_ERROR = 1000103,

	BSQL_TABLE_ADD_ROW_ERROR = 1000201,
	BSQL_TABLE_INSERT_ROW_ERROR = 1000202,
	BSQL_TABLE_POP_ROW_ERROR = 1000203,

} BsqlResult;

#define BSQL_TABLE_MAGIC 0xB05771
#define BSQL_TABLE_VERSION 1

typedef struct BsqlTable {
	int i;
	uint32_t name_size;
	const char* name;
	FILE* file;

	uint32_t column_count;
	uint32_t* columns;

	uint32_t row_count;
	uint32_t row_size;
	uint8_t* row_data;
} BsqlTable;

typedef struct BsqlTableHeader {
	uint32_t magic;
	uint32_t version;
	uint32_t name_size;
	uint32_t column_count;
	uint32_t row_count;
} BsqlTableHeader;

BsqlTable* bsqlNewTable(const char* path, bool overrideOld);
BsqlResult bsqlWriteTable(BsqlTable* t);
BsqlResult bsqlOpenTable(BsqlTable* t);
BsqlResult bsqlCloseTable(BsqlTable* t);

BsqlResult bsqlPrintTableData(BsqlTable* t);

BsqlResult bsqlTableAddColumn(BsqlTable* t, uint32_t size);
BsqlResult bsqlTableInsertColumn(BsqlTable* t, uint32_t index, uint32_t size);
BsqlResult bsqlTablePopColumn(BsqlTable* t, uint32_t index);

BsqlResult bsqlTableAddRows(BsqlTable* t, uint32_t count, uint8_t** data);
BsqlResult bsqlTableInsertRows(BsqlTable* t, uint32_t index, uint32_t count, uint8_t** data);
BsqlResult bsqlTablePopRows(BsqlTable* t, uint32_t index, uint32_t count);

#endif
