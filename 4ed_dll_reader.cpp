/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.11.2015
 *
 * DLL loader declarations for 4coder
 *
 */

// TOP

// TODO(allen):
//  Check the relocation table, if it contains anything that
// is platform specific generate an error to avoid calling
// into invalid code.

i32
dll_compare(char *a, char *b, i32 len){
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

enum DLL_Error{
    dll_err_too_small_for_header = 1,
    dll_err_wrong_MZ_signature,
    dll_err_wrong_DOS_error,
    dll_err_wrong_PE_signature,
    dll_err_unrecognized_bit_signature,
};

b32
dll_parse_headers(Data file, DLL_Data *dll, i32 *error){
    b32 result;
    i32 pe_offset;
    i32 read_pos;

    result = 1;
    if (file.size <= sizeof(DOS_Header) + DOS_error_size){
        if (error) *error = dll_err_too_small_for_header;
        result = 0;
        goto dll_parse_end;
    }
    
    dll->dos_header = (DOS_Header*)file.data;
        
    if (dll_compare(dll->dos_header->signature, "MZ", 2) != 0){
        if (error) *error = dll_err_wrong_MZ_signature;
        result = 0;
        goto dll_parse_end;
    }
   
    if (file.size <= DOS_error_offset + DOS_error_size){
        if (error) *error = dll_err_too_small_for_header;
        result = 0;
        goto dll_parse_end;
    }
    
    if (dll_compare((char*)(file.data + DOS_error_offset), DOS_error_message,
                    sizeof(DOS_error_message) - 1) != 0){
        if (error) *error = dll_err_wrong_DOS_error;
        result = 0;
        goto dll_parse_end;
    }
    
    pe_offset = dll->dos_header->e_lfanew;
    read_pos = pe_offset;

    if (file.size <= read_pos + PE_header_size){
        if (error) *error = dll_err_too_small_for_header;
        result = 0;
        goto dll_parse_end;
    }
    
    if (dll_compare((char*)(file.data + read_pos),
                    PE_header, PE_header_size) != 0){
        if (error) *error = dll_err_wrong_PE_signature;
        result = 0;
        goto dll_parse_end;
    }
    
    read_pos += PE_header_size;

    if (file.size <= read_pos + sizeof(COFF_Header)){
        if (error) *error = dll_err_too_small_for_header;
        result = 0;
        goto dll_parse_end;
    }

    dll->coff_header = (COFF_Header*)(file.data + read_pos);
    read_pos += sizeof(COFF_Header);
    
    if (file.size <= read_pos + dll->coff_header->size_of_optional_header){
        if (error) *error = dll_err_too_small_for_header;
        result = 0;
        goto dll_parse_end;
    }
        
    dll->opt_header_32 = (PE_Opt_Header_32Bit*)(file.data + read_pos);
    dll->opt_header_64 = (PE_Opt_Header_64Bit*)(file.data + read_pos);
    read_pos += dll->coff_header->size_of_optional_header;

    if (dll->opt_header_32->signature != bitsig_32bit &&
        dll->opt_header_32->signature != bitsig_64bit){
        if (error) *error = dll_err_unrecognized_bit_signature;
        result = 0;
        goto dll_parse_end;
    }

    if (dll->opt_header_32->signature == bitsig_32bit) dll->is_64bit = 0;
    else dll->is_64bit = 1;

    dll->section_defs = (PE_Section_Definition*)(file.data + read_pos);
    
dll_parse_end:
    return(result);
}

i32
dll_total_loaded_size(DLL_Data *dll){
    COFF_Header *coff_header;
    PE_Section_Definition *section_def;
    i32 result, section_end, i;

    coff_header = dll->coff_header;
    section_def = dll->section_defs;
    result = 0;
    
    for (i = 0; i < coff_header->number_of_sections; ++i, ++section_def){
        section_end = section_def->loaded_location + section_def->loaded_size;
        if (section_end > result){
            result = section_end;
        }
    }
    
    return(result);
}

b32
dll_perform_reloc(DLL_Loaded *loaded){
    Data img;
    byte *base;
    Relocation_Block_Header *header;
    Relocation_Block_Entry *entry;
    Data_Directory *data_directory;
    u32 cursor;
    u32 bytes_in_table;
    u32 block_end;
    u32 type;
    u32 offset;
    b32 result;
    b32 highadj_stage;
    
    u64 dif64;
    
    result = 1;
    img = loaded->img;
    if (loaded->is_64bit){
        data_directory = loaded->opt_header_64->data_directory;
        dif64 = ((u64)img.data - (u64)loaded->opt_header_64->image_base);
    }
    else{
        data_directory = loaded->opt_header_32->data_directory;
        dif64 = ((u64)img.data - (u64)loaded->opt_header_32->image_base);
    }
    data_directory += image_dir_base_reloc_table;
    base = img.data + data_directory->virtual_address;
    bytes_in_table = data_directory->size;

    highadj_stage = 1;
    
    
    for (cursor = 0; cursor < bytes_in_table;){
        header = (Relocation_Block_Header*)(base + cursor);
        block_end = cursor + header->block_size;
        cursor += sizeof(Relocation_Block_Header);
        
        for (;cursor < block_end;){
            entry = (Relocation_Block_Entry*)(base + cursor);
            cursor += sizeof(Relocation_Block_Entry);
            
            type = (u32)(entry->entry & reloc_entry_type_mask) >> reloc_entry_type_shift;
            offset = (u32)(entry->entry & reloc_entry_offset_mask) + header->page_base_offset;

            switch (type){
            case image_base_absolute: break;
                
            case image_base_high:
            case image_base_low:
            case image_base_highlow:
            case image_base_highadj:
            case image_base_arm_mov32a:
            case image_base_arm_mov32t:
            case image_base_mips_jmpaddr16:
                result = 0;
                goto dll_reloc_end;

            case image_base_dir64:
                *(u64*)(img.data + offset) += dif64;
                break;
            }
        }
    }

dll_reloc_end:
    return(result);
}

b32
dll_load_sections(Data img, DLL_Loaded *loaded,
                  Data file, DLL_Data *dll){
    COFF_Header *coff_header;
    PE_Section_Definition *section_def;
    u32 header_size;
    u32 size;
    u32 i;

    coff_header = dll->coff_header;
    section_def = dll->section_defs;

    header_size =
        (u32)((byte*)(section_def + coff_header->number_of_sections) - file.data);
    
    memcpy(img.data, file.data, header_size);
    memset(img.data + header_size, 0, img.size - header_size);
    
    for (i = 0; i < coff_header->number_of_sections; ++i, ++section_def){
        size = section_def->loaded_size;
        if (size > section_def->disk_size)
            size = section_def->disk_size;
        
        memcpy(img.data + section_def->loaded_location,
               file.data + section_def->disk_location,
               size);

        if (dll_compare(section_def->name, ".text", 5) == 0){
            loaded->text_start = section_def->loaded_location;
            loaded->text_size = section_def->loaded_size;
        }
    }
    
    return(1);
}

void
dll_load(Data img, DLL_Loaded *loaded, Data file, DLL_Data *dll){
    Data_Directory *export_dir;
    
    dll_load_sections(img, loaded, file, dll);
    loaded->img = img;

    loaded->dos_header = (DOS_Header*)((byte*)img.data + ((byte*)dll->dos_header - file.data));
    loaded->coff_header = (COFF_Header*)((byte*)img.data + ((byte*)dll->coff_header - file.data));
    
    loaded->opt_header_32 = (PE_Opt_Header_32Bit*)
        ((byte*)img.data + ((byte*)dll->opt_header_32 - file.data));
    loaded->opt_header_64 = (PE_Opt_Header_64Bit*)
        ((byte*)img.data + ((byte*)dll->opt_header_64 - file.data));
    
    loaded->section_defs = (PE_Section_Definition*)
        ((byte*)img.data + ((byte*)dll->section_defs - file.data));
    
    loaded->is_64bit = dll->is_64bit;
    
    if (dll->is_64bit){
        export_dir = dll->opt_header_64->data_directory;
    }
    else{
        export_dir = dll->opt_header_32->data_directory;
    }
    export_dir += image_dir_entry_export;
    loaded->export_start = export_dir->virtual_address;

    dll_perform_reloc(loaded);
}

void*
dll_load_function(DLL_Loaded *dll, char *func_name, i32 size){
    Data img;
    DLL_Export_Directory_Table *export_dir;
    DLL_Export_Address *address_ptr;
    DLL_Export_Name *name_ptr;
    void *result;
    u32 count, i;
    u32 result_offset;
    u32 ordinal;
    
    img = dll->img;
    export_dir = (DLL_Export_Directory_Table*)(img.data + dll->export_start);

    count = export_dir->number_of_name_pointers;
    name_ptr = (DLL_Export_Name*)(img.data + export_dir->name_pointer_offset);

    result = 0;
    for (i = 0; i < count; ++i, ++name_ptr){
        if (dll_compare((char*)img.data + name_ptr->name_offset,
                        func_name, size) == 0){
            ordinal = ((u16*)(img.data + export_dir->ordinal_offset))[i];
#if 0
            // NOTE(allen): The MS docs say to do this, but
            // it appears to just be downright incorrect.
            ordinal -= export_dir->ordinal_base;
#endif
            address_ptr = (DLL_Export_Address*)(img.data + export_dir->address_offset);
            address_ptr += ordinal;
            result_offset = address_ptr->export_offset;
            result = (img.data + result_offset);
            break;
        }
    }

    return(result);
}

#define MachineCase(x) case x: result = #x; *len = sizeof(#x) - 1; break

char*
dll_machine_type_str(u16 machine, i32 *len){
    char *result;
    i32 extra;
    
    if (!len) len = &extra;
    result = 0;
    
    switch (machine){
        MachineCase(intel_i386);
        MachineCase(intel_i860);

        MachineCase(mips_r3000);
        MachineCase(mips_little_endian);
        MachineCase(mips_r10000);

        MachineCase(old_alpha_axp);
        MachineCase(alpha_axp);

        MachineCase(hitachi_sh3);
        MachineCase(hitachi_sh3_dsp);
        MachineCase(hitachi_sh4);
        MachineCase(hitachi_sh5);

        MachineCase(arm_little_endian);
        MachineCase(thumb);

        MachineCase(matsushita_am33);
        MachineCase(power_pc_little_endian);
        MachineCase(power_pc_with_floating);

        MachineCase(intel_ia64);
        MachineCase(mips16);
        MachineCase(motorola_68000_series);
    
        MachineCase(alpha_axp_64_bit);
    
        MachineCase(mips_with_fpu);
        MachineCase(mips16_with_fpu);
        MachineCase(eft_byte_code);
    
        MachineCase(amd_amd64);
        MachineCase(mitsubishi_m32r_little_endian);
        MachineCase(clr_pure_msil);
    }

    return(result);
}

#undef MachineCase

// BOTTOM

