
/*
 * For generating the header of the TODO files so I don't have to
 * keep doing that by hand and accidentally forgetting half the time.
 *  -Allen
 * 12.05.2016 (dd.mm.yyyy)
 *
 */

// TOP

#include <stdlib.h>
#include <stdio.h>
#include <time.h>

char *header =
"Distribution Date: %d.%d.%d (dd.mm.yyyy)\n"
"\n"
"Thank you for contributing to the 4coder project!\n"
"\n"
"To submit bug reports or to request particular features email editor@4coder.net.\n"
"\n"
"Watch 4coder.net blog and @AllenWebster4th twitter for news about 4coder progress.\n"
"\n"
;

typedef struct Readme_Variables{
    int day, month, year;
} Readme_Variables;

typedef struct File_Data{
    char *data;
    int size;
    int file_exists;
} File_Data;

File_Data
dump(char *file_name){
    File_Data result = {0};
    FILE *file = fopen(file_name, "rb");
    
    if (file){
        result.file_exists = 1;
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        if (result.size > 0){
            result.data = (char*)malloc(result.size + 1);
            result.data[result.size + 1] = 0;
            fseek(file, 0, SEEK_SET);
            fread(result.data, 1, result.size, file);
        }
        fclose(file);
    }
    
    return(result);
}

void
generate(char *file_name_out, char *file_name_body, Readme_Variables vars){
    File_Data in;
    FILE *out;
    
    in = dump(file_name_body);
    
    if (in.file_exists){
        out = fopen(file_name_out, "wb");
        fprintf(out, header, vars.day, vars.month,vars.year);
        if (in.size > 0){
            fprintf(out, "%s", in.data);
        }
        fclose(out);
    }
}

int
main(){
    time_t ctime;
    struct tm tm;
    Readme_Variables vars;
    
    ctime = time(0);
    localtime_s(&tm, &ctime);
    
    vars.day = tm.tm_mday;
    vars.month = tm.tm_mon + 1;
    vars.year = tm.tm_year + 1900;
    
    generate("README.txt", "README_body.txt", vars);
    
    
    return(0);
}

// BOTTOM


