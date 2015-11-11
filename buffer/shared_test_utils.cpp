/* 
 * Mr. 4th Dimention - Allen Webster
 *  Four Tech
 *
 * public domain -- no warranty is offered or implied; use this code at your own risk
 * 
 * 11.11.2015
 * 
 * Code shared between history_to_replay.cpp and 4coder_test_main.cpp
 * 
 */

// TOP

typedef struct File_Data{
    char *data;
    int size;
} File_Data;

File_Data get_file(const char *filename){
    FILE *file;
    File_Data result;

    memzero_4tech(result);
    
    file = fopen(filename, "rb");
    if (!file){
        printf("error: could not find file %s\n", filename);
    }
    else{
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);

        if (result.size == 0){        
            printf("error: file %s was empty\n", filename);
        }
        else{
            result.data = (char*)malloc(result.size);
            fread(result.data, result.size, 1, file);
        }

        fclose(file);
    }
    
    return(result);
}

void save_file(const char *fname, File_Data file){
    FILE *f = fopen(fname, "wb");
    if (f){
        fwrite(file.data, 1, file.size, f);
        fclose(f);
    }
    else{
        printf("error: could not open %s\n", fname);
    }
}

void free_file(File_Data file){
    free(file.data);
}

typedef struct Edit_Step{
    int type;
    union{
        struct{
            int can_merge;
            Buffer_Edit edit;
            int pre_pos;
            int post_pos;
            int next_block, prev_block;
        };
        struct{
            int first_child;
            int inverse_first_child;
            int inverse_child_count;
            int special_type;
        };
    };
    int child_count;
} Edit_Step;

typedef struct Edit_Stack{
    char *strings;
    int size;
    
    Edit_Step *edits;
    int count;
} Edit_Stack;

typedef struct Small_Edit_Stack{
    char *strings;
    int size;
    
    Buffer_Edit *edits;
    int count;
} Small_Edit_Stack;

typedef struct History{
    Edit_Stack undo;
    Edit_Stack redo;
    Edit_Stack history;
    Small_Edit_Stack children;
} History;

typedef struct Replay{
    Edit_Stack replay;
    Small_Edit_Stack children;

    int str_max;
    int ed_max;
} Replay;

int read_int(char **curs){
    int result;
    result = *(int*)(*curs);
    *curs += 4;
    return(result);
}

char* write_int(char *curs, int x){
    *(int*)(curs) = x;
    curs += 4;
    return(curs);
}

void prepare_replay(File_Data file, Replay *replay){
    char *curs = file.data;
    
    replay->replay.count = read_int(&curs);
    replay->children.count = read_int(&curs);
    replay->replay.size = read_int(&curs);
    replay->children.size = read_int(&curs);
    
    replay->replay.edits = (Edit_Step*)curs;
    curs += sizeof(Edit_Step)*replay->replay.count;
    
    replay->children.edits = (Buffer_Edit*)curs;
    curs += sizeof(Buffer_Edit)*replay->children.count;
    
    replay->replay.strings = curs;
    curs += replay->replay.size;
    
    replay->children.strings = curs;
    curs += replay->children.size;

    assert_4tech((int)(curs - file.data) == file.size);
}

// BOTTOM



