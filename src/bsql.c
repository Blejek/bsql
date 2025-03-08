#include "bsql.h"
#include <string.h>
#include <stdlib.h>

BsqlTable* bsqlNewTable(const char* path, bool overrideOld) {
	BsqlTable* t = malloc(sizeof(BsqlTable));
	if (t == NULL) {
		return NULL;
	}

	if (overrideOld) {
		t->file = fopen(path, "w");
		if (t->file == NULL) {
			return BSQL_TABLE_FILE_OPEN_ERROR;
		}
	}
	else {
		t->file = fopen(path, "r+");
		if (t->file == NULL) {
			t->file = fopen(path, "w");
			if (t->file == NULL) {
				return BSQL_TABLE_FILE_OPEN_ERROR;
			}
		}
	}

	fseek(t->file, 0, SEEK_SET);

	t->name_size = 0;
	t->column_count = 0;
	t->columns = NULL;

	t->row_count = 0;
	t->row_size = 0;
	t->row_data = NULL;

	return t;
}

BsqlResult bsqlWriteTable(BsqlTable* t) {
	if (t == NULL) {
		return BSQL_ERROR;
	}

	if (t->file == NULL) {
		return BSQL_ERROR;
	}

	// Write header
	fseek(t->file, 0, SEEK_SET);

	BsqlTableHeader header;
	header.magic = BSQL_TABLE_MAGIC;
	header.version = BSQL_TABLE_VERSION;
	header.name_size = t->name_size;
	header.column_count = t->column_count;
	header.row_count = t->row_count;

	fwrite(&header, sizeof(header), 1, t->file);

	// Write name and columns
	fwrite(t->name, sizeof(char) * t->name_size, 1, t->file);
	fwrite(t->columns, sizeof(uint32_t) * t->column_count, 1, t->file);
	fwrite(t->row_data, sizeof(uint8_t) * t->row_size * t->row_count, 1, t->file);

	return BSQL_SUCCESS;
}

BsqlResult bsqlOpenTable(BsqlTable* t) {
	if (t == NULL) {
		return BSQL_ERROR;
	}

	if (t->file == NULL) {
		return BSQL_ERROR;
	}

	// Read header
	fseek(t->file, 0, SEEK_SET);

	BsqlTableHeader header;
	fread(&header, sizeof(BsqlTableHeader), 1, t->file);

	if (header.magic != BSQL_TABLE_MAGIC) {
		return BSQL_TABLE_FILE_READ_ERROR;
	}

	// Read name and columns
	uint32_t name_size = sizeof(char) * header.name_size;
	char* name = malloc(name_size);
	if (name == NULL) {
		return BSQL_ERROR;
	}
	fread(name, name_size, 1, t->file);
	t->name = name;
	t->name_size = header.name_size;

	uint32_t column_size = sizeof(uint32_t) * header.column_count;
	uint32_t* columns = malloc(column_size);
	if (columns == NULL) {
		return BSQL_ERROR;
	}
	fread(columns, column_size, 1, t->file);
	t->columns = name;
	t->column_count = header.column_count;

	// Read Rows

	uint32_t row_size = 0;
	for (uint32_t i = 0; i < t->column_count; i++) {
		row_size += columns[i];
	}
	t->row_size = row_size;

	uint32_t row_data_size = sizeof(uint8_t) * header.row_count * row_size;
	uint8_t* row_data = malloc(row_data_size);
	if (row_data == NULL) {
		return BSQL_ERROR;
	}
	fread(row_data, row_data_size, 1, t->file);
	t->row_data = name;
	t->row_count = header.row_count;

	return BSQL_SUCCESS;
}

BsqlResult bsqlCloseTable(BsqlTable* t) {
	fclose(t->file);
	free(t);

	return BSQL_SUCCESS;
}

BsqlResult bsqlPrintTableData(BsqlTable* t) {
	if (t == NULL) {
		return BSQL_ERROR;
	}
	printf("Table - %s\n", t->name);
	printf("Column count - %d\n", t->column_count);
	printf("Row count - %d\n", t->row_count);
	printf("Row size - %d\n", t->row_size);


	if (t->row_count > 0) {
		printf("%s\n", t->row_data);
	}

	return BSQL_SUCCESS;
}

BsqlResult bsqlTableAddColumn(BsqlTable* t, uint32_t size) {
	if (t) {
		uint32_t* old_columns = t->columns;
		uint32_t old_column_count = t->column_count;
		t->column_count++;

		t->columns = malloc(sizeof(uint32_t) * t->column_count);
		if (t->columns == NULL) {
			return BSQL_TABLE_ADD_COLUMN_ERROR;
		}

		uint32_t old_row_size = t->row_size;
		*(t->columns + t->column_count - 1) = size;
		t->row_size += size;

		if (old_columns) {
			memcpy(t->columns, old_columns, old_column_count * sizeof(uint32_t));

			uint8_t* old_data = t->row_data;
			t->row_data = malloc(sizeof(uint8_t) * t->row_count * t->row_size);
			if (t->row_data == NULL) {
				return BSQL_TABLE_ADD_COLUMN_ERROR;
			}
			for (uint32_t r = 0; r < t->row_count; r++) {
				uint8_t* old_row = old_data + r * old_row_size;
				uint8_t* row = t->row_data + r * t->row_size;

				for (uint32_t c = 0; c < old_column_count; c++) {
					memcpy(row + old_columns[c] * c, old_row + old_columns[c] * c, old_columns[c] * sizeof(uint8_t));
				}

			}
			free(old_data);
			free(old_columns);
		}

		return BSQL_SUCCESS;
	}
	return BSQL_TABLE_ADD_COLUMN_ERROR;
}

BsqlResult bsqlTableInsertColumn(BsqlTable* t, uint32_t index, uint32_t size) {
	if (t) {
		uint32_t* old_columns = t->columns;
		uint32_t old_column_count = t->column_count;
		t->column_count++;

		t->columns = malloc(sizeof(uint32_t) * t->column_count);
		if (t->columns == NULL) {
			return BSQL_TABLE_INSERT_COLUMN_ERROR;
		}


		uint32_t old_row_size = t->row_size;
		*(t->columns + index) = size;
		t->row_size += size;

		if (old_columns) {
			memcpy(t->columns, old_columns, index * sizeof(uint32_t));
			memcpy(t->columns + index + 1, old_columns + index, (old_column_count - index) * sizeof(uint32_t));

			uint8_t* old_data = t->row_data;
			t->row_data = malloc(sizeof(uint8_t) * t->row_count * t->row_size);
			if (t->row_data == NULL) {
				return BSQL_TABLE_INSERT_COLUMN_ERROR;
			}
			for (uint32_t r = 0; r < t->row_count; r++) {
				uint8_t* old_row = old_data + r * old_row_size;
				uint8_t* row = t->row_data + r * t->row_size;

				uint32_t c = 0;
				for (uint32_t new_c = 0; new_c < t->column_count; new_c++) {
					if (new_c == index) {
						*row = (uint8_t)-1;
						row += size;
					}
					else {
						memcpy(row + old_columns[c] * c, old_row + old_columns[c] * c, old_columns[c] * sizeof(uint8_t));
						c++;
					}
				}
			}
			free(old_data);
			free(old_columns);
		}


		return BSQL_SUCCESS;
	}
	return BSQL_TABLE_INSERT_COLUMN_ERROR;
}

BsqlResult bsqlTablePopColumn(BsqlTable* t, uint32_t index) {
	if (t && index  < t->column_count) {
		uint32_t* old_columns = t->columns;
		uint32_t old_column_count = t->column_count;
		t->column_count--;

		uint32_t old_row_size = t->row_size;
		t->row_size -= t->columns[index];

		t->columns = malloc(t->row_count * sizeof(uint32_t));
		if (t->columns == NULL) {
			return BSQL_TABLE_POP_COLUMN_ERROR;
		}

		if (old_columns) {
			memcpy(t->columns, old_columns, index * sizeof(uint32_t));
			memcpy(t->columns + index, old_columns + (index + 1), old_column_count * sizeof(uint32_t) - (index + 1) * sizeof(uint32_t));
		
			uint8_t* old_data = t->row_data;
			t->row_data = malloc(sizeof(uint8_t) * t->row_count * t->row_size);
			if (t->row_data == NULL) {
				return BSQL_TABLE_POP_COLUMN_ERROR;
			}
			for (uint32_t r = 0; r < t->row_count; r++) {
				uint8_t* old_row = old_data + r * old_row_size;
				uint8_t* row = t->row_data + r * t->row_size;

				uint32_t new_c = 0;
				for (uint32_t c = 0; c < old_column_count; c++) {
					if (c == index) {
						old_row += old_columns[c];
					}
					else {
						memcpy(row + t->columns[new_c] * new_c, old_row + t->columns[new_c] * new_c, t->columns[new_c] * sizeof(uint8_t));
						new_c++;
					}
				}
			}
			free(old_data);
			free(old_columns);
		}

		return BSQL_SUCCESS;
	}
	return BSQL_TABLE_POP_COLUMN_ERROR;
}

BsqlResult bsqlTableAddRows(BsqlTable* t, uint32_t count, uint8_t** data) {
	if (t) {
		uint8_t* old_data = t->row_data;

		uint32_t old_row_count = t->row_count;
		t->row_count += count;

		t->row_data = malloc(sizeof(uint8_t) * t->row_count * t->row_size);
		if (t->row_data == NULL) {
			return BSQL_TABLE_ADD_ROW_ERROR;
		}

		
		if (old_data) {
			memcpy(t->row_data, old_data, old_row_count * t->row_size);
			free(old_data);
		}

		*data = t->row_data + old_row_count * t->row_size;

		return BSQL_SUCCESS;
	}
	return BSQL_TABLE_ADD_ROW_ERROR;
}

BsqlResult bsqlTableInsertRows(BsqlTable* t, uint32_t index, uint32_t count, uint8_t** data) {
	if (t) {
		uint8_t* old_data = t->row_data;

		uint32_t old_row_count = t->row_count;
		t->row_count += count;


		t->row_data = malloc(sizeof(uint8_t) * t->row_count * t->row_size);
		if (t->row_data == NULL) {
			return BSQL_TABLE_INSERT_ROW_ERROR;
		}

		if (old_data) {
			memcpy(t->row_data, old_data, index * t->row_size);
			memcpy(t->row_data + (index + count) * t->row_size, old_data + index * t->row_size, old_row_count * t->row_size - index * t->row_size);
		
			free(old_data);
		}

		*data = t->row_data + index * t->row_size;


		return BSQL_SUCCESS;
	}
	return BSQL_TABLE_INSERT_ROW_ERROR;
}

BsqlResult bsqlTablePopRows(BsqlTable* t, uint32_t index, uint32_t count) {
	if (t && index + count <= t->row_count) {
		uint8_t* old_data = t->row_data;

		uint32_t old_row_count = t->row_count;
		uint32_t old_row_data_size = old_row_count * t->row_size;
		t->row_count -= count;

		t->row_data = malloc(sizeof(uint8_t) * t->row_count * t->row_size);
		if (t->row_data == NULL) {
			return BSQL_TABLE_POP_ROW_ERROR;
		}

		if (old_data) {
			memcpy(t->row_data, old_data, index * t->row_size);
			memcpy(t->row_data + index * t->row_size, old_data + (index + count) * t->row_size, old_row_data_size - (index + count) * t->row_size);
		
			free(old_data);
		}

		return BSQL_SUCCESS;
	}
	return BSQL_TABLE_POP_ROW_ERROR;
}
