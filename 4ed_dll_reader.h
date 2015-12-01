/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.11.2015
 *
 * DLL loader declarations for 4coder
 *
 */

// TOP

struct DOS_Header {
    char signature[2];
    i16 lastsize;
    i16 nblocks;
    i16 nreloc;
    i16 hdrsize;
    i16 minalloc;
    i16 maxalloc;
    i16 ss;
    i16 sp;
    i16 checksum;
    i16 ip;
    i16 cs;
    i16 relocpos;
    i16 noverlay;
    i16 reserved1[4];
    i16 oem_id;
    i16 oem_info;
    i16 reserved2[10];
    i32 e_lfanew;
};

enum Target_Machine_Code{
    intel_i386 = 0x14C,
    intel_i860 = 0x14D,

    mips_r3000 = 0x162,
    mips_little_endian = 0x166,
    mips_r10000 = 0x168,

    old_alpha_axp = 0x183,
    alpha_axp = 0x184,

    hitachi_sh3 = 0x1a2,
    hitachi_sh3_dsp = 0x1a3,
    hitachi_sh4 = 0x1a6,
    hitachi_sh5 = 0x1a8,

    arm_little_endian = 0x1c0,
    thumb = 0x1c2,

    matsushita_am33 = 0x1d3,
    power_pc_little_endian = 0x1f0,
    power_pc_with_floating = 0x1f1,

    intel_ia64 = 0x200,
    mips16 = 0x266,
    motorola_68000_series = 0x268,
    
    alpha_axp_64_bit = 0x284,
    
    mips_with_fpu = 0x366,
    mips16_with_fpu = 0x466,
    eft_byte_code = 0xebc,
    
    amd_amd64 = 0x8664,
    mitsubishi_m32r_little_endian = 0x9041,
    clr_pure_msil = 0xc0ee
};

#define file_is_exe 0x2
#define file_is_non_reloctable 0x200
#define file_is_dll 0x2000

struct COFF_Header{
    u16 machine;
    u16 number_of_sections;
    u32 time_date_stamp;
    u32 pointer_to_symbol_table;
    u32 number_of_symbols;
    u16 size_of_optional_header;
    u16 characteristics;
};

struct Data_Directory{ 
    u32 virtual_address;
    u32 size;
};

// This version is untested
struct PE_Opt_Header_32Bit{
    // Universal Portion
    i16 signature;
    i8 major_linker_version; 
    i8 minor_linker_version;
    i32 size_of_code;
    i32 size_of_initialized_data;
    i32 size_of_uninitialized_data;
    i32 address_of_entry_point;
    i32 base_of_code;
    i32 base_of_data;
    
    // Windows Portion
    i32 image_base;
    i32 section_alignment;
    i32 file_alignment;
    i16 major_OS_version;
    i16 minor_OS_version;
    i16 major_image_version;
    i16 minor_image_version;
    i16 major_subsystem_version;
    i16 minor_subsystem_version;
    i32 reserved;
    i32 size_of_image;
    i32 size_of_headers;
    i32 checksum;
    i16 subsystem;
    i16 DLL_characteristics;
    i32 size_of_stack_reserve;
    i32 size_of_stack_commit;
    i32 size_of_heap_reserve;
    i32 size_of_heap_commit;
    i32 loader_flags;
    i32 number_of_rva_and_sizes;
    Data_Directory data_directory[16];
};

struct PE_Opt_Header_64Bit{
    // Universal Portion
    u16 signature;
    u8 major_linker_version; 
    u8 minor_linker_version;
    u32 size_of_code;
    u32 size_of_initialized_data;
    u32 size_of_uninitialized_data;
    u32 address_of_entry_point;
    u32 base_of_code;
    
    // Windows Portion
    u64 image_base;
    u32 section_alignment;
    u32 file_alignment;
    u16 major_OS_version;
    u16 minor_OS_version;
    u16 major_image_version;
    u16 minor_image_version;
    u16 major_subsystem_version;
    u16 minor_subsystem_version;
    u32 reserved;
    u32 size_of_image;
    u32 size_of_headers;
    u32 checksum;
    u16 subsystem;
    u16 DLL_characteristics;
    u64 size_of_stack_reserve;
    u64 size_of_stack_commit;
    u64 size_of_heap_reserve;
    u64 size_of_heap_commit;
    u32 loader_flags;
    u32 number_of_rva_and_sizes;
    Data_Directory data_directory[16];
};

#define bitsig_32bit 267
#define bitsig_64bit 523

#define image_dir_entry_export 0
#define image_dir_entry_import 1
#define image_dir_entry_resource 2
#define image_dir_base_reloc_table 5
#define image_dir_entry_bound_import 11

struct PE_Section_Definition{
    char name[8];
    u32 loaded_size;
    u32 loaded_location;
    u32 disk_size;
    u32 disk_location;
    u32 disk_relocs;
    u32 reserved1;
    u16 number_of_relocs;
    u16 reserved2;
    u32 flags;
};

#define image_scn_type_no_pad                0x00000008
#define image_scn_cnt_code                   0x00000020
#define image_scn_cnt_initialized_data       0x00000040
#define image_scn_cnt_uninitialized_data     0x00000080
#define image_scn_lnk_other                  0x00000100
#define image_scn_lnk_info                   0x00000200
#define image_scn_lnk_remove                 0x00000800
#define image_scn_lnk_comdat                 0x00001000
#define image_scn_no_defer_spec_exc          0x00004000
#define image_scn_gprel                      0x00008000
#define image_scn_mem_fardata                0x00008000
#define image_scn_mem_purgeable              0x00020000
#define image_scn_mem_16BIT                  0x00020000
#define image_scn_mem_locked                 0x00040000
#define image_scn_mem_preload                0x00080000

#define image_scn_align_1bytes               0x00100000
#define image_scn_align_2bytes               0x00200000
#define image_scn_align_4bytes               0x00300000
#define image_scn_align_8bytes               0x00400000
#define image_scn_align_16bytes              0x00500000
#define image_scn_align_32bytes              0x00600000
#define image_scn_align_64bytes              0x00700000
#define image_scn_align_128bytes             0x00800000
#define image_scn_align_256bytes             0x00900000
#define image_scn_align_512bytes             0x00A00000
#define image_scn_align_1024bytes            0x00B00000
#define image_scn_align_2048bytes            0x00C00000
#define image_scn_align_4096bytes            0x00D00000
#define image_scn_align_8192bytes            0x00E00000
#define image_scn_align_mask                 0x00F00000

#define image_scn_lnk_nreloc_ovfl            0x01000000
#define image_scn_mem_discardable            0x02000000
#define image_scn_mem_not_cached             0x04000000
#define image_scn_mem_not_paged              0x08000000
#define image_scn_mem_shared                 0x10000000
#define image_scn_mem_execute                0x20000000
#define image_scn_mem_read                   0x40000000
#define image_scn_mem_write                  0x80000000

#pragma pack(push, 1)
struct COFF_Relocation{
    u32 virtual_address;
    u32 symbol_table_index;
    u16 type;
};
#pragma pack(pop)

enum Image_Rel_Amd64{
    image_rel_amd64_absolute  = 0x00,
    image_rel_amd64_addr64    = 0x01,
    image_rel_amd64_addr32    = 0x02,
    image_rel_amd64_addr32nb  = 0x03,
    image_rel_amd64_rel32     = 0x04,
    image_rel_amd64_rel32_1   = 0x05,
    image_rel_amd64_rel32_2   = 0x06,
    image_rel_amd64_rel32_3   = 0x07,
    image_rel_amd64_rel32_4   = 0x08,
    image_rel_amd64_rel32_5   = 0x09,
    image_rel_amd64_section   = 0x0A,
    image_rel_amd64_secrel    = 0x0B,
    image_rel_amd64_secrel7   = 0x0C,
    image_rel_amd64_token     = 0x0D,
    image_rel_amd64_srel32    = 0x0E,
    image_rel_amd64_pair      = 0x0F,
    image_rel_amd64_sspan32   = 0x10
};

enum Image_Rel_Arm{
    image_rel_arm_absolute    = 0x0,
    image_rel_arm_addr32      = 0x1,
    image_rel_arm_addr32nb    = 0x2,
    image_rel_arm_branch24    = 0x3,
    image_rel_arm_branch11    = 0x4,
    image_rel_arm_token       = 0x5,
    image_rel_arm_blx24       = 0x6,
    image_rel_arm_blx11       = 0x7,
    image_rel_arm_section     = 0x8,
    image_rel_arm_secrel      = 0x9,
    image_rel_arm_mov32a      = 0xA,
    image_rel_arm_mov32t      = 0xB,
    image_rel_arm_branch20t   = 0xC,
    image_rel_arm_branch24t   = 0xD,
    image_rel_arm_blx32t      = 0xE
};

enum Image_Rel_Arm64{
    image_rel_arm64_absolute        = 0x0,
    image_rel_arm64_addr32          = 0x1,
    image_rel_arm64_addr32nb        = 0x2,
    image_rel_arm64_branch26        = 0x3,
    image_rel_arm64_pagebase_rel21  = 0x4,
    image_rel_arm64_rel21           = 0x5,
    image_rel_arm64_pageoffset_12a  = 0x6,
    image_rel_arm64_pageoffset_12l  = 0x7,
    image_rel_arm64_secrel          = 0x8,
    image_rel_arm64_secrel_low12a   = 0x9,
    image_rel_arm64_secrel_high12a  = 0xA,
    image_rel_arm64_secrel_low12l   = 0xB,
    image_rel_arm64_token           = 0xC,
    image_rel_arm64_section         = 0xD,
    image_rel_arm64_addr64          = 0xE
};

// NOTE(allen):
//  skipped Hitachi SuperH
//  skiiped IBM PowerPC

enum Image_Rel_i386{
    image_rel_i386_absolute = 0x0,
    image_rel_i386_dir16    = 0x1,
    image_rel_i386_rel16    = 0x2,
    image_rel_i386_dir32    = 0x3,
    image_rel_i386_dir32nb  = 0x4,
    image_rel_i386_seg12    = 0x5,
    image_rel_i386_section  = 0x6,
    image_rel_i386_secrel   = 0x7,
    image_rel_i386_token    = 0x8,
    image_rel_i386_secrel7  = 0x9,
    image_rel_i386_rel32    = 0xA
};

// NOTE(allen):
//  skipped ia64
//  skipped MIPS
//  skiiped Mitsubishi

struct Relocation_Block_Header{
    u32 page_base_offset;
    u32 block_size;
};

#define reloc_entry_type_mask   0xF000
#define reloc_entry_type_shift  12
#define reloc_entry_offset_mask 0x0FFF

struct Relocation_Block_Entry{
    u16 entry;
};

enum DLL_Relocation_Type{
    image_base_absolute,
    // nothing 
    
    image_base_high,
    // add high 16 bits of diff to 16 bits at offset 
    
    image_base_low,
    // add low 16 bits of diff to 16 bits at offset 
    
    image_base_highlow,
    // adds all 32 bits to 32 bits at offset 
    
    image_base_highadj,
    // consumes two slots: high 16 bits at location, low 16 bits at next location
    
    image_base_arm_mov32a,
    // mips: jump instruction; arm: MOVW+MOVT 
    
    image_base_reserved1,
    
    image_base_arm_mov32t,
    // MOVW+MOVT in Thumb mode 
    
    image_base_reserved2,
    
    image_base_mips_jmpaddr16,
    // mips16 jump instruction 
    
    image_base_dir64
    // adds to 64 bits field
};

struct DLL_Data{
    DOS_Header *dos_header;
    COFF_Header *coff_header;
    PE_Opt_Header_32Bit *opt_header_32;
    PE_Opt_Header_64Bit *opt_header_64;
    PE_Section_Definition *section_defs;
    b32 is_64bit;
};

struct DLL_Loaded{
    DOS_Header *dos_header;
    COFF_Header *coff_header;
    PE_Opt_Header_32Bit *opt_header_32;
    PE_Opt_Header_64Bit *opt_header_64;
    PE_Section_Definition *section_defs;
    b32 is_64bit;
    
    Data img;
    u32 export_start;
    u32 text_start;
    u32 text_size;
};

struct DLL_Export_Directory_Table{
    u32 export_flags;
    u32 time_date_stamp;
    u16 major_version;
    u16 minor_version;
    u32 name_offset;
    u32 ordinal_base;
    u32 number_of_addresses;
    u32 number_of_name_pointers;
    u32 address_offset;
    u32 name_pointer_offset;
    u32 ordinal_offset;
};

struct DLL_Export_Address{
    u32 export_offset;
};

struct DLL_Export_Name{
    u32 name_offset;
};

struct DLL_Export_Ordinal{
    u16 ordinal;
};

struct DLL_Debug_Entry{
    u32 characteristics;
    u32 time_date_stamp;
    u16 major_version;
    u16 minor_version;
    u32 type;
    u32 size_of_data;
    u32 offset_of_data;
    u32 disk_offset_of_data;
} thingy;

enum DLL_Debug_Type{
    img_dbg_type_unknown,
    img_dbg_type_coff,
    img_dbg_type_codeview,
    img_dbg_type_fpo,
    img_dbg_type_misc,
    img_dbg_type_exception,
    img_dbg_type_fixup,
    img_dbg_type_omap_to_src,
    img_dbg_type_omap_from_src
};

char DOS_error_message[] = "This program cannot be run in DOS mode.";
i32 DOS_error_offset = 0x4E;
i32 DOS_error_size = sizeof(DOS_error_message) - 1;

char PE_header[] = {'P', 'E', 0, 0};
i32 PE_header_size = 4;

// BOTTOM

