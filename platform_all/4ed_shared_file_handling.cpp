/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 18.07.2017
 *
 * Cross platform logic for work queues.
 *
 */

// TOP

internal
Sys_Directory_CD_Sig(system_directory_cd){
    String directory = make_string_cap(dir, *len, cap);
    b32 result = false;
    
    if (rel_path[0] != 0){
        if (rel_path[0] == '.' && rel_path[1] == 0){
            result = true;
        }
        else if (rel_path[0] == '.' && rel_path[1] == '.' && rel_path[2] == 0){
            result = remove_last_folder(&directory);
            terminate_with_null(&directory);
        }
        else{
            if (directory.size + rel_len + 1 > directory.memory_size){
                i32 old_size = directory.size;
                append_partial_sc(&directory, rel_path);
                append_s_char(&directory, SLASH);
                terminate_with_null(&directory);
                
                if (system_directory_exists(directory.str)){
                    result = true;
                }
                else{
                    directory.size = old_size;
                }
            }
        }
    }
    
    *len = directory.size;
    LOGF("%.*s: %d\n", directory.size, directory.str, result);
    
    return(result);
}

// BOTTOM

