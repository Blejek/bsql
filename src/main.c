#include "bsql.h"
#include <stdlib.h>

#define DB_PATH "C:/dev/c/bsql/test"

int main(int argc, char** argv){
    printf_s("Hello World\n");

    BsqlTable* t = bsqlNewTable(DB_PATH "/db1.bsqldb", true);
    t->name_size = 5;
    t->name = "elo";

    bsqlTableAddColumn(t, 2);



    //bsqlOpenTable(t);


    uint8_t* data = NULL;


    bsqlTableAddColumn(t, 4);

    if (bsqlTableAddRows(t, 2, &data) == BSQL_SUCCESS) {
        *data = '1';
        *(data + t->row_size - 1) = '1';
        *(data + t->row_size) = '2';
        *(data + 2 * t->row_size - 1) = '2';
    }

    if (bsqlTableInsertRows(t, 1, 1, &data) == BSQL_SUCCESS) {
        *data = '3';
        *(data + t->row_size-1) = '3';
    }

    bsqlTableInsertColumn(t, 1, 8);
    bsqlPrintTableData(t);

    bsqlWriteTable(t);

    bsqlCloseTable(t);


    return 0;
}
