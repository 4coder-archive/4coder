/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 25.02.2016
 *
 * File editing view for 4coder
 *
 */

// TOP

#if !defined(OUT_CONTEXT_4CODER)
#define OUT_CONTEXT_4CODER

typedef struct Out_Context{
    char out_directory_space[256];
    String out_directory;
    FILE *file;
    String *str;
} Out_Context;

static void
set_context_directory(Out_Context *context, char *dst_directory){
    context->out_directory = make_fixed_width_string(context->out_directory_space);
    copy_sc(&context->out_directory, dst_directory);
}

static int32_t
begin_file_out(Out_Context *out_context, char *filename, String *out){
    char str_space[512];
    String name = make_fixed_width_string(str_space);
    if (out_context->out_directory.size > 0){
        append_ss(&name, out_context->out_directory);
        append_sc(&name, "\\");
    }
    append_sc(&name, filename);
    terminate_with_null(&name);
    
    int32_t r = 0;
    out_context->file = fopen(name.str, "wb");
    out_context->str = out;
    out->size = 0;
    if (out_context->file){
        r = 1;
    }
    
    return(r);
}

static void
dump_file_out(Out_Context out_context){
    fwrite(out_context.str->str, 1, out_context.str->size, out_context.file);
    out_context.str->size = 0;
}

static void
end_file_out(Out_Context out_context){
    dump_file_out(out_context);
    fclose(out_context.file);
}

static String
make_out_string(int32_t x){
    String str;
    str.size = 0;
    str.memory_size = x;
    str.str = (char*)malloc(x);
    return(str);
}

#endif

// BOTTOM


