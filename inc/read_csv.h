#include <stdint.h>
#include <stdbool.h>

void readCSV(const char *filename, int32_t **data, uint32_t *max_row, uint32_t *max_col, bool skip_1st_row);