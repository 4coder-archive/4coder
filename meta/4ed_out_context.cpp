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

internal void
end_file_out(char *out_file, String *out_data){
    FILE *file = fopen(out_file, "wb");
    if (file != 0){
        fwrite(out_data->str, 1, out_data->size, file);
        fclose(file);
    }
    else{
        fprintf(stdout, "Could not open output file %s\n", out_file);
    }
    out_data->size = 0;
}

#endif

// BOTTOM


