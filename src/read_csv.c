#include "read_csv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void readCSV(const char *filename, int32_t **data, uint32_t *max_row, uint32_t *max_col, bool skip_1st_row)
{
    FILE *csvFile;
    char line[1024]; // Assuming a maximum line length of 1024 characters
    int row = 0, col = 0;

    csvFile = fopen(filename, "r");

    if (csvFile == NULL)
    {
        perror("Error opening file");
        return;
    }

    while (fgets(line, sizeof(line), csvFile) != NULL && row < *max_row)
    {
        if (skip_1st_row && row == 0)
        {
            row++;
            continue;
        }

        col = 0;
        char *token = strtok(line, ",");
        while (token != NULL && col < *max_col)
        {
            data[row][col] = atoi(token);
            token = strtok(NULL, ",");
            col++;
        }
        row++;
    }

    //TODO: return error when max_row and max_col is not enough to store al the data
    *max_row = row;
    *max_col = col;
    
    fclose(csvFile);
}

void readCSV2(const char *filename, int32_t **data, uint32_t max_row, uint32_t max_col, bool skip_first_row)
{
    FILE *csvFile;
    char line[1024]; // Assuming a maximum line length of 1024 characters
    int row = 0, col = 0;

    csvFile = fopen(filename, "r");

    if (csvFile == NULL)
    {
        perror("Error opening file");
        return;
    }

    while (fgets(line, sizeof(line), csvFile) != NULL && row < max_row)
    {
        if (skip_first_row && row == 0)
        {
            row++;
            continue;
        }

        col = 0;
        char *token = strtok(line, ",");
        while (token != NULL && col < max_col)
        {
            data[row][col] = atoi(token);
            token = strtok(NULL, ",");
            col++;
        }
        row++;
    }

    fclose(csvFile);
}
