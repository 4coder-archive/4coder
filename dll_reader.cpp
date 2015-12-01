/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 12.12.2014
 *
 * Application layer for project codename "4ed"
 *
 */

// TOP

#include "4ed_meta.h"
#include "4ed_dll_reader.h"
#include "4ed_dll_reader.cpp"

i32
compare(char *a, char *b, i32 len){
    i32 result;
    char *e;

    result = 0;
    e = a + len;
    for (;a < e && *a == *b; ++a, ++b);
    if (a < e){
        if (*a < *b) result = -1;
        else result = 1;
    }

    return(result);
}

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

Data
load_file(char *filename){
    Data result;
    FILE * file;

    result = {};
    file = fopen(filename, "rb");
    if (!file){
        printf("file %s not found\n", filename);
    }
    else{
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        result.data = (byte*)malloc(result.size);
        fread(result.data, 1, result.size, file);
        fclose(file);
    }
    
    return(result);
}

void
show_reloc_block(Data file, DLL_Data *dll, PE_Section_Definition *reloc_section){
    byte *base;
    Relocation_Block_Header *header;
    Relocation_Block_Entry *entry;
    u32 cursor;
    u32 bytes_in_table;
    u32 block_end;

    base = file.data + reloc_section->disk_location;
    if (dll->is_64bit) bytes_in_table = dll->opt_header_64->data_directory[image_dir_base_reloc_table].size;
    else bytes_in_table = dll->opt_header_32->data_directory[image_dir_base_reloc_table].size;

    for (cursor = 0; cursor < bytes_in_table;){
        header = (Relocation_Block_Header*)(base + cursor);
        block_end = cursor + header->block_size;
        cursor += sizeof(Relocation_Block_Header);
        
        printf("block-size: %d\n", header->block_size);
        printf("offset-base: %d\n", header->page_base_offset);
        
        for (;cursor < block_end;){
            entry = (Relocation_Block_Entry*)(base + cursor);
            cursor += sizeof(Relocation_Block_Entry);
            printf("reloc:  type %d  offset %d\n",
                   (i32)(entry->entry & reloc_entry_type_mask) >> reloc_entry_type_shift,
                   (i32)(entry->entry & reloc_entry_offset_mask));
        }
    }
}

typedef int (Function)(int a, int b);

#include <Windows.h>

#define UseWinDll 0

int
main(int argc, char **argv){
    Function *func;
    i32 x;
    
#if UseWinDll
    HMODULE module;
    
    if (argc < 2){
        printf("usage: dll_reader <dll-file>\n");
        exit(1);
    }

    module = LoadLibraryA(argv[1]);

    if (!module){
        printf("failed to load file %s\n", argv[1]);
        exit(1);
    }
    
    func = (Function*)GetProcAddress(module, "test_func");
    
#else
    Data file, img;
    DLL_Data dll;
    DLL_Loaded dll_loaded;
    PE_Section_Definition *section_def;
    i32 error;
    i32 i;
    
    if (argc < 2){
        printf("usage: dll_reader <dll-file>\n");
        exit(1);
    }

    file = load_file(argv[1]);

    if (!file.data){
        printf("failed to load file %s\n", argv[1]);
        exit(1);
    }
    
    if (!dll_parse_headers(file, &dll, &error)){
        printf("header error %d\n", error);
        exit(1);
    }
    
    printf("this appears to be a dll\n");

    printf("symbol-count: %d  symbol-addr: %d\n",
           dll.coff_header->number_of_symbols,
           dll.coff_header->pointer_to_symbol_table);
    
    if (dll.is_64bit) printf("64bit\n");
    else printf("32bit\n");
    
    printf("built for machine: %s\n", dll_machine_type_str(dll.coff_header->machine, 0));

    if (dll.is_64bit){
        printf("number of directories: %d\n", dll.opt_header_64->number_of_rva_and_sizes);
    }
    else{
        printf("number of directories: %d\n", dll.opt_header_32->number_of_rva_and_sizes);
    }
    
    printf("\nbeginning section decode now\n");

    section_def = dll.section_defs;
    for (i = 0; i < dll.coff_header->number_of_sections; ++i, ++section_def){
        if (section_def->name[7] == 0){
            printf("name: %s\n", section_def->name);
        }
        else{
            printf("name: %.*s\n", 8, section_def->name);
        }
        printf("img-size: %d  img-loc: %d\ndisk-size: %d  disk-loc: %d\n",
               section_def->loaded_size, section_def->loaded_location,
               section_def->disk_size, section_def->disk_location);

        if (compare(section_def->name, ".reloc", 6) == 0){
            show_reloc_block(file, &dll, section_def);
        }
    }

    img.size = dll_total_loaded_size(&dll);
    printf("image size: %d\n", img.size);
    
    img.data = (byte*)
        VirtualAlloc((LPVOID)Tbytes(3), img.size,
                     MEM_COMMIT | MEM_RESERVE,
                     PAGE_READWRITE);
    dll_load(img, &dll_loaded, file, &dll);

    DWORD _extra;
    VirtualProtect(img.data + dll_loaded.text_start,
                   dll_loaded.text_size,
                   PAGE_EXECUTE_READ,
                   &_extra);
    
    func = (Function*)dll_load_function(&dll_loaded, "test_func", 9);
#endif
    
    x = func(10, 20);
    printf("%d\n", x);
    
    x = func(1, 2);
    printf("%d\n", x);
    
    return(0);
}

// BOTTOM
