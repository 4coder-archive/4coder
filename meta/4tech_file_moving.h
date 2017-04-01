/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 21.01.2017
 *
 * Code for file manipulating scripts.
 *
 */

// TOP

#if !defined(FTECH_FILE_MOVING_H)
#define FTECH_FILE_MOVING_H

#include "../4tech_defines.h"
#include "../4coder_lib/4coder_string.h"

#include <stdio.h>  // include system for windows
#include <stdlib.h> // include system for linux   (YAY!)
#include <string.h>

static char SF_CMD[4096];
static i32 error_state = 0;
static i32 prev_error = 0;

#define systemf(...) do{                                       \
    int32_t n = snprintf(SF_CMD, sizeof(SF_CMD), __VA_ARGS__); \
    AllowLocal(n);                                             \
    Assert(n < sizeof(SF_CMD));                                \
    /** printf("%s\n", SF_CMD); /**/                          \
    prev_error = system(SF_CMD);                               \
    if (prev_error != 0) error_state = 1;                      \
}while(0)

// OS selectors
#if defined(IS_WINDOWS)
#  define ONLY_WINDOWS(x) x
#  define ONLY_LINUX(x) (void)0
#elif defined(IS_LINUX)
#  define ONLY_WINDOWS(x) (void)0
#  define ONLY_LINUX(x) x
#else
#  error File moving is not setup on this OS.
#endif

// String helpers
internal void
require_size(String *str, i32 size){
    if (str->memory_size <= size){
        free(str->str);
        str->memory_size = l_round_up_i32(str->size*2, 64);
        str->str = (char*)malloc(str->memory_size);
    }
}
internal void
require_size_preserve(String *str, i32 size){
    if (str->memory_size <= size){
        str->memory_size = l_round_up_i32(str->size*2, 64);
        char *newstr = (char*)malloc(str->memory_size);
        memcpy(newstr, str->str, str->size);
        free(str->str);
        str->str = newstr;
    }
}

internal void
copy_always(String *dst, String src){
    require_size(dst, src.size+1);
    copy(dst, src);
    terminate_with_null(dst);
}

internal void
append_always(String *dst, String src){
    require_size_preserve(dst, dst->size+src.size+1);
    append(dst, src);
    terminate_with_null(dst);
}

// Instruction format
typedef u16 Instruction_FM;
enum Instruction_FM_{
    InstrFM_BeginTime,
    InstrFM_EndTime,
    InstrFM_Push,
    InstrFM_StackLoad,
    InstrFM_Pop,
    InstrFM_MovCWD,
    InstrFM_Mov,
    InstrFM_Leaf,
    InstrFM_Path,
    InstrFM_Cat,
    InstrFM_SlashFix,
    InstrFM_MkDir,
    InstrFM_ClrDir,
    InstrFM_Del,
    InstrFM_Copy,
    InstrFM_CopyAll,
    InstrFM_Zip,
    InstrFM_CD,
    InstrFM_Command,
    InstrFM_COUNT
};
enum Instruction_Flag_FM_{
    InstrFM_LiteralParam1 = 0x8000,
    InstrFM_LiteralParam2 = 0x4000,
    InstrFM_LiteralParam3 = 0x2000,
    InstrFM_InstrMask     = 0x00FF
};

typedef u16 String_Reg;

// Instruction VM
#define REG_COUNT_FM 16
struct Virtual_Machine_FM{
    umem pc;
    u64 time_start;
    String temp;
    String stage_reg;
    String reg[REG_COUNT_FM];
    
    String *stack_base;
    String *stack_ptr;
};

internal String_Reg
read_register_fm(u8 **ptr){
    String_Reg result = *(String_Reg*)(*ptr);
    Assert(result < REG_COUNT_FM);
    (*ptr) += 2;
    return(result);
}

internal u16
read_u16_fm(u8 **ptr){
    u16 result = *(u16*)(*ptr);
    (*ptr) += 2;
    return(result);
}

internal String
read_parameter_fm(b32 is_literal, u8 **ptr){
    String result = {0};
    if (is_literal){
        result.size = *(u16*)(*ptr);
        result.memory_size = result.size+1;
        (*ptr) += 2;
        result.str = (char*)ptr;
        u16 step_forward = (result.memory_size+1)&(~1);
        ptr += step_forward;
    }
    else{
        String_Reg result = *(String_Reg*)(*ptr);
        Assert(result < REG_COUNT_FM);
        (*ptr) += 2;
    }
    return(result);
}

internal void
stage_params(String *params, u32 count, u16 dest, Virtual_Machine_FM *vm){
    String dest_str = vm->reg[dest];
    u32 i = 0;
    for (; i < count; ++i){
        if (params[i].str == dest_str.str){
            copy_always(&vm->stage_reg, params[i]);
            params[i] = vm->stage_reg;
            break;
        }
    }
    for (++i; i < count; ++i){
        if (params[i].str == dest_str.str){
            params[i] = vm->stage_reg;
        }
    }
}

internal void run_instruction_fm(Virtual_Machine_FM *vm, u16 op, String_Reg dest, u16 n, String *params);

internal void
execute_instructions_fm(void *code, umem length){
    Assert(InstrFM_InstrMask >= InstrFM_COUNT-1);
    
    u8 *base = (u8*)code;
    Virtual_Machine_FM vm = {0};
    
    u32 n = 0;
    String_Reg dest = 0;
    String param[3] = {0};
    for (;vm.pc < length;){
        Instruction_FM ins = *(Instruction_FM*)(base+vm.pc);
        u16 op = (ins)&InstrFM_InstrMask;
        
        u8 *ptr = (u8*)(base+vm.pc+2);
        switch(op){
            case InstrFM_MovCWD:
            {
                dest = read_register_fm(&ptr);
            }break;
            
            case InstrFM_StackLoad:
            {
                dest = read_register_fm(&ptr);
                n = read_u16_fm(&ptr);
            }break;
            
            case InstrFM_Pop:
            {
                n = read_u16_fm(&ptr);
            }break;
            
            case InstrFM_Mov:
            case InstrFM_Leaf:
            case InstrFM_Path:
            case InstrFM_Cat:
            case InstrFM_SlashFix:
            {
                dest = read_register_fm(&ptr);
                param[0] = read_parameter_fm(ins&InstrFM_LiteralParam1, &ptr);
                stage_params(param, 1, dest, &vm);
            }break;
            
            case InstrFM_Push:
            case InstrFM_EndTime:
            case InstrFM_MkDir:
            case InstrFM_ClrDir:
            case InstrFM_Del:
            case InstrFM_CD:
            case InstrFM_Command:
            {
                param[0] = read_parameter_fm(ins&InstrFM_LiteralParam1, &ptr);
            }break;
            
            case InstrFM_Copy:
            case InstrFM_Zip:
            {
                param[0] = read_parameter_fm(ins&InstrFM_LiteralParam1, &ptr);
                param[1] = read_parameter_fm(ins&InstrFM_LiteralParam2, &ptr);
            }break;
            
            case InstrFM_CopyAll:
            {
                param[0] = read_parameter_fm(ins&InstrFM_LiteralParam1, &ptr);
                param[1] = read_parameter_fm(ins&InstrFM_LiteralParam2, &ptr);
                param[2] = read_parameter_fm(ins&InstrFM_LiteralParam3, &ptr);
            }break;
        }
        
        run_instruction_fm(&vm, op, dest, n, param);
        
        vm.pc = (umem)((ptr)-(base+vm.pc));
        Assert(vm.pc <= length);
    }
}

#define LLU_CAST(n) (long long unsigned int)(n)
#define TSCALE 1000000
#define PRINT_TIME_FM(n) \
printf("%-20s: %.2llu.%.6llu\n", (n), LLU_CAST(total/TSCALE), LLU_CAST(total%TSCALE));

#if defined(IS_WINDOWS)

#define DONT_BUG_ME\
FOF_SILENT | FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_NOCONFIRMMKDIR

internal void
run_instruction_fm(Virtual_Machine_FM *vm, u16 op, String_Reg dest, String *params){
    switch(op){
        case InstrFM_BeginTime:
        case InstrFM_EndTime:
        {
            u64 time = 0;
            LARGE_INTEGER lint;
            if (QueryPerformanceCounter(&lint)){
                time = lint.QuadPart;
                time = (time * 1000000) / perf_frequency;
            }
            if (op == InstrFM_BeginTime){
                vm->time_start = time;
            }
            else{
                u64 total = time - vm->time_start;
                PRINT_TIME_FM(params[0].str);
            }
        }break;
        
        case InstrFM_MovCWD:
        {
            String *d = &vm->reg[dest];
            i32 length = GetCurrentDirectoryA(0, 0);
            require_size(d, length+1);
            d->size = GetCurrentDirectoryA(d->memory_size, d->str);
            terminate_with_null(d);
        }break;
        
        case InstrFM_Push:
        {
            copy_always(&vm->stack_ptr[0], params[0]);
            ++vm->stack_ptr;
        }break;
        
        case InstrFM_StackLoad:
        {
            Assert((vm->stack_ptr - vm->stack_base) >= n);
            String *d = &vm->reg[dest];
            String *s = vm->stack_ptr - n;
            copy_always(d, *s);
        }break;
        
        case InstrFM_Pop:
        {
            Assert((vm->stack_ptr - vm->stack_base) >= n);
            stack_ptr = vm->strack_ptr - n;
        }break;
        
        case InstrFM_Mov:
        {
            String *d = &vm->reg[dest];
            copy_always(d, params[0]);
            terminate_with_null(d);
        }break;
        
        case InstrFM_Leaf:
        {
            String *d = &vm->reg[dest];
            copy_always(d, front_of_directory(params[0]));
            terminate_with_null(d);
        }break;
        
        case InstrFM_Path:
        {
            String *d = &vm->reg[dest];
            copy_always(d, path_of_directory(params[0]));
            terminate_with_null(d);
        }break;
        
        case InstrFM_Cat:
        {
            String *d = &vm->reg[dest];
            append_always(d, params[0]);
            terminate_with_null(d);
        }break;
        
        case InstrFM_SlashFix:
        {
            String *d = &vm->reg[dest];
            copy_always(d, params[0]);
            u32 size = d->size;
            char *str = d->str;
            for (u32 i = 0; i < size; ++i){
                if (str[i] == '/'){
                    str[i] == '//';
                }
            }
            terminate_with_null(d);
        }break;
        
        case InstrFM_MkDir:
        {
            String path = param[0];
            char *p = path.str;
            for (; *p; ++p){
                if (*p == '\\'){
                    *p = 0;
                    CreateDirectoryA(path.str, 0);
                    *p = '\\';
                }
            }
            if (!CreateDirectoryA(path.str, 0)){
                // TODO(allen): error.
            }
        }break;
        
        case InstrFM_ClrDir:
        {
            String *d = &vm->temp;
            copy_always(d, params[0].size+4);
            if (d->str[d->size-1] != '\\'){
                d->str[d->size++] = '\\';
            }
            d->str[d->size++] = '*';
            d->str[d->size++] = 0;
            d->str[d->size] = 0;
            
            SHFILEOPSTRUCT fileop = {0};
            fileop.wFunc = FO_DELETE;
            fileop.pFrom = d->str;
            fileop.fFlags = DONT_BUG_ME;
            if (SHFileOperation(&fileop) != 0){
                // TODO(allen): error.
            }
        }break;
        
        case InstrFM_Del:
        {
            if (!DeleteFileA(param[0].str)){
                // TODO(allen): error.
            }
        }break;
        
        case InstrFM_CD:
        {
            if (!SetCurrentDirectoryA(param[0].str)){
                // TODO(allen): error.
            }
        }break;
        
        case InstrFM_Command:
        {
            systemf("%s", param[0].str);
        }break;
        
        case InstrFM_Copy:
        {
            if (!CopyFileA(param[0], param[1], false)){
                // TODO(allen): error.
            }
        }break;
        
        case InstrFM_Zip:
        {
            u32 length = GetCurrentDirectoryA(0, 0);
            String *d = &vm->temp;
            require_size(d, length+11);
            d->size = GetCurrentDirectoryA(d->memory_size, d->str);
            terminate_with_null(d);
            
            i32 original_length = params[0].size;
            remove_last_folder(&params[0]);
            terminate_with_null(&params[0]);
            if (SetCurrentDirectoryA(params[0].str)){
                systemf("zip %s\\4tech_gobble.zip", d->str);
                SetCurrentDirectoryA(d->str);
                append_always(d, make_lit_string("\\gobble.zip"));
                terminate_with_null(d);
                
                if (!MoveFile(d->str, params[1].str)){
                    // TODO(allen): error.
                }
            }
            else{
                // TODO(allen): error.
            }
            params[0].str[params[0].size-1] = '\\';
            params[0].size = original_length;
        }break;
        
        case InstrFM_CopyAll:
        {
            require_size_preserve(&params[2], params[2].size+2);
            params[2].str[params[2].size] = 0;
            params[2].str[params[2].size+1] = 0;
            
            require_size(&vm->temp, params[0].size+params[1].size+3);
            copy_always(&vm->temp, params[0]);
            copy_always(&vm->temp, make_lit_string("\\"));
            copy_always(&vm->temp, params[1]);
            vm->temp.str[vm->temp.size] = 0;
            vm->temp.str[vm->temp.size+1] = 0;
            
            SHFILEOPSTRUCT fileop = {0};
            fileop.wFunc = FO_COPY;
            fileop.pFrom = vm->temp.str;
            fileop.pTo = param[2].str;
            fileop.fFlags = DONT_BUG_ME | FOF_NORECURSION | FOF_FILESONLY;
            if (SHFileOperation(&fileop) != 0){
                // TODO(allen): error.
            }
        }break;
        
        default: InvalidCodePath;
    }
}

#undef DONT_BUG_ME

#elif defined(IS_LINUX)

#include <time.h>
#include <unistd.h>

internal void
run_instruction_fm(Virtual_Machine_FM *vm, u16 op, String_Reg dest, u16 n, String *params){
    switch(op){
        case InstrFM_BeginTime:
        case InstrFM_EndTime:
        {
            struct timespec spec;
            clock_gettime(CLOCK_MONOTONIC, &spec);
            u64 time = (spec.tv_sec * 1000000) + (spec.tv_nsec / 1000);
            if (op == InstrFM_BeginTime){
                vm->time_start = time;
            }
            else{
                u64 total = time - vm->time_start;
                PRINT_TIME_FM(params[0].str);
            }
        }break;
        
        case InstrFM_MovCWD:
        {
            String *d = &vm->reg[dest];
            char *r = getcwd(d->str, d->memory_size);
            if (r == d->str){
                d->size = str_size(d->str);
            }
            terminate_with_null(d);
        }break;
        
        case InstrFM_Push:
        {
            copy_always(&vm->stack_ptr[0], params[0]);
            ++vm->stack_ptr;
        }break;
        
        case InstrFM_StackLoad:
        {
            Assert((vm->stack_ptr - vm->stack_base) >= n);
            String *d = &vm->reg[dest];
            String *s = vm->stack_ptr - n;
            copy_always(d, *s);
        }break;
        
        case InstrFM_Pop:
        {
            Assert((vm->stack_ptr - vm->stack_base) >= n);
            vm->stack_ptr = vm->stack_ptr - n;
        }break;
        
        case InstrFM_Mov:
        {
            String *d = &vm->reg[dest];
            copy_always(d, params[0]);
            terminate_with_null(d);
        }break;
        
        case InstrFM_Leaf:
        {
            String *d = &vm->reg[dest];
            copy_always(d, front_of_directory(params[0]));
            terminate_with_null(d);
        }break;
        
        case InstrFM_Cat:
        {
            String *d = &vm->reg[dest];
            append_always(d, params[0]);
            terminate_with_null(d);
        }break;
        
        case InstrFM_SlashFix:break;
        
        case InstrFM_MkDir:
        {
            systemf("mkdir -p %s", params[0].str);
        }break;
        
        case InstrFM_ClrDir:
        {
            systemf("rm -rf %s*", params[0].str);
        }break;
        
        case InstrFM_Del:
        {
            systemf("rm %s", params[0].str);
        }break;
        
        case InstrFM_CD:
        {
            chdir(params[0].str);
        }break;
        
        case InstrFM_Command:
        {
            systemf("%s", params[0].str);
        }break;
        
        case InstrFM_Copy:
        {
            systemf("cp %s %s", params[0].str, params[1].str);
        }break;
        
        case InstrFM_Zip:
        {
            String *d = &vm->temp;
            char *r = getcwd(d->str, d->memory_size);
            
            if (r == d->str){
                d->size = str_size(d->str);
                
                i32 original_length = params[0].size;
                remove_last_folder(&params[0]);
                terminate_with_null(&params[0]);
                if (chdir(params[0].str)){
                    char *folder = params[0].str + params[0].size + 1;
                    systemf("zip -r %s %s", params[1].str, folder);
                    chdir(d->str);
                    terminate_with_null(d);
                }
                else{
                    // TODO(allen): error.
                }
                params[0].str[params[0].size-1] = '\\';
                params[0].size = original_length;
            }
            else{
                // TODO(allen): error.
            }
        }break;
        
        case InstrFM_CopyAll:
        {
            
        }break;
    }
}
#else
#error No implementation for run_instruction_fm on this OS.
#endif

// Instruction emitter
struct Emitter_FM{
    u8 *buffer;
    umem pos, max;
};
Emitter_FM global_emitter_fm = {0};

internal void
init_fm(void *buffer, umem length){
    global_emitter_fm.buffer = (u8*)buffer;
    global_emitter_fm.pos = 0;
    global_emitter_fm.max = length;
}

internal void
execute_fm(){
    execute_instructions_fm(global_emitter_fm.buffer, global_emitter_fm.pos);
    global_emitter_fm.pos = 0;
}

internal void
emit_u16_fm(u16 x){
    Assert(global_emitter_fm.pos+2 <= global_emitter_fm.max);
    *(u16*)(global_emitter_fm.buffer + global_emitter_fm.pos) = x;
    global_emitter_fm.pos += 2;
}

internal void
emit_strlit_fm(char *str, u16 len){
    u16 full_len = len+1;
    u16 mem_len = (full_len+1)&(~1);
    Assert(global_emitter_fm.pos+mem_len <= global_emitter_fm.max);
    u8 *dst = global_emitter_fm.buffer + global_emitter_fm.pos;
    u16 i = 0;
    for (; i < len; ++i){
        dst[i] = str[i];
    }
    dst[i] = 0;
    global_emitter_fm.pos += mem_len;
}

struct String_Param{
    b32 is_literal;
    String_Reg reg;
    u16 len;
    char *n;
};
internal String_Param
reg_fm(String_Reg reg){
    String_Param result = {0};
    result.reg = reg;
    return(result);
}
internal String_Param
lit_fm(char *n){
    String_Param result = {0};
    result.is_literal = true;
    result.len = str_size(n);
    result.n = n;
    return(result);
}
internal String_Param
litn_fm(char *n, u16 len){
    String_Param result = {0};
    result.is_literal = true;
    result.len = len;
    result.n = n;
    return(result);
}
internal void
emit_param_fm(String_Param n){
    if (n.is_literal){
        emit_strlit_fm(n.n, n.len);
    }
    else{
        emit_u16_fm(n.reg);
    }
}
internal void
param_flag_fm(u16 *ins, String_Param n, u16 param_i){
    local_persist u16 flags[] = {
        InstrFM_LiteralParam1,
        InstrFM_LiteralParam2,
        InstrFM_LiteralParam3,
    };
    if (n.is_literal){
        *ins |= flags[param_i];
    }
}

internal void
begin_time_fm(){
    u16 ins = InstrFM_BeginTime;
    emit_u16_fm(ins);
}

internal void
end_time_fm(String_Param n){
    u16 ins = InstrFM_EndTime;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_param_fm(n);
}

internal void
push_fm(String_Param n){
    u16 ins = InstrFM_Push;
    emit_u16_fm(ins);
    emit_param_fm(n);
}

internal void
stack_load_fm(String_Reg reg, u16 index){
    u16 ins = InstrFM_StackLoad;
    emit_u16_fm(ins);
    emit_u16_fm(reg);
    emit_u16_fm(index);
}

internal void
pop_fm(u16 count){
    u16 ins = InstrFM_Pop;
    emit_u16_fm(ins);
    emit_u16_fm(count);
}

internal void
mov_cwd_fm(String_Reg reg){
    u16 ins = InstrFM_MovCWD;
    emit_u16_fm(ins);
    emit_u16_fm(reg);
}

internal void
mov_fm(String_Reg reg, String_Param n){
    u16 ins = InstrFM_Mov;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_u16_fm(reg);
    emit_param_fm(n);
}

internal void
leaf_fm(String_Reg reg, String_Param n){
    u16 ins = InstrFM_Leaf;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_u16_fm(reg);
    emit_param_fm(n);
}

internal void
path_fm(String_Reg reg, String_Param n){
    u16 ins = InstrFM_Path;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_u16_fm(reg);
    emit_param_fm(n);
}

internal void
cat_fm(String_Reg reg, String_Param n){
    u16 ins = InstrFM_Cat;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_u16_fm(reg);
    emit_param_fm(n);
}

internal void
slash_fix_fm(String_Reg reg, String_Param n){
    u16 ins = InstrFM_SlashFix;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_u16_fm(reg);
    emit_param_fm(n);
}

internal void
mkdir_fm(String_Param n){
    u16 ins = InstrFM_MkDir;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_param_fm(n);
}

internal void
clrdir_fm(String_Param n){
    u16 ins = InstrFM_ClrDir;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_param_fm(n);
}

internal void
del_fm(String_Param n){
    u16 ins = InstrFM_Del;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_param_fm(n);
}

internal void
copy_fm(String_Param s, String_Param d){
    u16 ins = InstrFM_Copy;
    param_flag_fm(&ins, s, 0);
    param_flag_fm(&ins, d, 1);
    emit_u16_fm(ins);
    emit_param_fm(s);
    emit_param_fm(d);
}

internal void
copy_all_fm(String_Param s, String_Param d, String_Param p){
    u16 ins = InstrFM_CopyAll;
    param_flag_fm(&ins, s, 0);
    param_flag_fm(&ins, d, 1);
    param_flag_fm(&ins, p, 2);
    emit_u16_fm(ins);
    emit_param_fm(s);
    emit_param_fm(d);
    emit_param_fm(p);
}

internal void
zip_fm(String_Param s, String_Param d){
    u16 ins = InstrFM_Copy;
    param_flag_fm(&ins, s, 0);
    param_flag_fm(&ins, d, 1);
    emit_u16_fm(ins);
    emit_param_fm(s);
    emit_param_fm(d);
}

internal void
cd_fm(String_Param n){
    u16 ins = InstrFM_Copy;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_param_fm(n);
}

internal void
command_fm(String_Param n){
    u16 ins = InstrFM_Command;
    param_flag_fm(&ins, n, 0);
    emit_u16_fm(ins);
    emit_param_fm(n);
}

// OLD VERSION
#if 0
#if defined(IS_WINDOWS)
#  define ONLY_WINDOWS(x) x
#  define ONLY_LINUX(x) (void)0

#define SLASH "\\"
static char platform_correct_slash = '\\';

#elif defined(IS_LINUX)
#  define ONLY_WINDOWS(x) (void)0
#  define ONLY_LINUX(x) x

#define SLASH "/"
static char platform_correct_slash = '/';

#else
#  error File moving is not setup on this OS.
#endif

static char SF_CMD[4096];
static i32 error_state = 0;
static i32 prev_error = 0;

#define systemf(...) do{                                       \
    int32_t n = snprintf(SF_CMD, sizeof(SF_CMD), __VA_ARGS__); \
    AllowLocal(n);                                             \
    Assert(n < sizeof(SF_CMD));                                \
    /** printf("%s\n", SF_CMD); /**/                          \
    prev_error = system(SF_CMD);                               \
    if (prev_error != 0) error_state = 1;                      \
}while(0)

static void init_time_system();
static u64  get_time();
static i32  get_current_directory(char *buffer, i32 max);
static void execute_in_dir(char *dir, char *str, char *args);

static void make_folder_if_missing(char *dir, char *folder);
static void clear_folder(char *folder);
static void delete_file(char *file);
static void copy_file(char *path, char *file, char *folder1, char *folder2, char *newname);
static void copy_all(char *source, char *tag, char *folder);
static void zip(char *parent, char *folder, char *dest);

static void slash_fix(char *path);
#define DECL_STR(n,s) char n[] = s; slash_fix(n)

typedef struct Temp_Dir{
    char dir[512];
} Temp_Dir;

static Temp_Dir pushdir(char *dir);
static void popdir(Temp_Dir temp);

#endif

#if defined(FTECH_FILE_MOVING_IMPLEMENTATION) && !defined(FTECH_FILE_MOVING_IMPL_GUARD)
#define FTECH_FILE_MOVING_IMPL_GUARD

#if defined(IS_WINDOWS)

typedef uint32_t DWORD;
typedef int32_t  LONG;
typedef int64_t  LONGLONG;
typedef char*    LPTSTR;
typedef char*    LPCTSTR;
typedef int32_t  BOOL;
typedef void*    LPSECURITY_ATTRIBUTES;
typedef union    _LARGE_INTEGER {
    struct {
        DWORD LowPart;
        LONG  HighPart;
    };
    struct {
        DWORD LowPart;
        LONG  HighPart;
    } u;
    LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

#define WINAPI

extern "C"{
    DWORD WINAPI GetCurrentDirectoryA(_In_  DWORD  nBufferLength, _Out_ LPTSTR lpBuffer);
    BOOL WINAPI SetCurrentDirectoryA(_In_ LPCTSTR lpPathName);
    
    BOOL WINAPI QueryPerformanceCounter(_Out_ LARGE_INTEGER *lpPerformanceCount);
    
    BOOL WINAPI QueryPerformanceFrequency(_Out_ LARGE_INTEGER *lpFrequency);
    
    BOOL WINAPI CreateDirectoryA(_In_ LPCTSTR lpPathName, _In_opt_ LPSECURITY_ATTRIBUTES lpSecurityAttributes);
    
    BOOL WINAPI CopyFileA(_In_ LPCTSTR lpExistingFileName, _In_ LPCTSTR lpNewFileName, _In_ BOOL bFailIfExists);
}

static uint64_t perf_frequency;

static Temp_Dir
pushdir(char *dir){
    Temp_Dir temp = {0};
    GetCurrentDirectoryA(sizeof(temp.dir), temp.dir);
    SetCurrentDirectoryA(dir);
    return(temp);
}

static void
popdir(Temp_Dir temp){
    SetCurrentDirectoryA(temp.dir);
}

static void
init_time_system(){
    LARGE_INTEGER lint;
    if (QueryPerformanceFrequency(&lint)){
        perf_frequency = lint.QuadPart;
    }
}

static uint64_t
get_time(){
    uint64_t time = 0;
    LARGE_INTEGER lint;
    if (QueryPerformanceCounter(&lint)){
        time = lint.QuadPart;
        time = (time * 1000000) / perf_frequency;
    }
    return(time);
}

static int32_t
get_current_directory(char *buffer, int32_t max){
    int32_t result = GetCurrentDirectoryA(max, buffer);
    return(result);
}

static void
execute_in_dir(char *dir, char *str, char *args){
    if (dir){
        Temp_Dir temp = pushdir(dir);
        if (args){
            systemf("call \"%s\" %s", str, args);
        }
        else{
            systemf("call \"%s\"", str);
        }
        popdir(temp);
    }
    else{
        if (args){
            systemf("call \"%s\" %s", str, args);
        }
        else{
            systemf("call \"%s\"", str);
        }
    }
}

static void
slash_fix(char *path){
    if (path){
        for (int32_t i = 0; path[i]; ++i){
            if (path[i] == '/') path[i] = '\\';
        }
    }
}

static void
make_folder_if_missing(char *dir, char *folder){
    char space[1024];
    String path = make_fixed_width_string(space);
    append_sc(&path, dir);
    if (folder){
        append_sc(&path, "\\");
        append_sc(&path, folder);
    }
    terminate_with_null(&path);
    
    char *p = path.str;
    for (; *p; ++p){
        if (*p == '\\'){
            *p = 0;
            CreateDirectoryA(path.str, 0);
            *p = '\\';
        }
    }
    CreateDirectoryA(path.str, 0);
}

static void
clear_folder(char *folder){
    systemf("del /S /Q /F %s\\* & rmdir /S /Q %s & mkdir %s",
            folder, folder, folder);
}

static void
delete_file(char *file){
    systemf("del %s", file);
}

static void
copy_file(char *path, char *file, char *folder1, char *folder2, char *newname){
    char src[256], dst[256];
    String b = make_fixed_width_string(src);
    if (path){
        append_sc(&b, path);
        append_sc(&b, "\\");
    }
    append_sc(&b, file);
    terminate_with_null(&b);
    
    b = make_fixed_width_string(dst);
    append_sc(&b, folder1);
    append_sc(&b, "\\");
    if (folder2){
        append_sc(&b, folder2);
        append_sc(&b, "\\");
    }
    if (newname){
        append_sc(&b, newname);
    }
    else{
        append_sc(&b, file);
    }
    terminate_with_null(&b);
    
    CopyFileA(src, dst, 0);
}

static void
copy_all(char *source, char *tag, char *folder){
    if (source){
        systemf("copy %s\\%s %s\\*", source, tag, folder);
    }
    else{
        systemf("copy %s %s\\*", tag, folder);
    }
}

static void
zip(char *parent, char *folder, char *dest){
    char cdir[512];
    get_current_directory(cdir, sizeof(cdir));
    
    Temp_Dir temp = pushdir(parent);
    systemf("%s\\zip %s\\4tech_gobble.zip", cdir, cdir);
    popdir(temp);
    
    systemf("copy %s\\4tech_gobble.zip %s & del %s\\4tech_gobble.zip", cdir, dest, cdir);
}

#elif defined(IS_LINUX)

#include <time.h>
#include <unistd.h>

static Temp_Dir
pushdir(char *dir){
    Temp_Dir temp;
    char *result = getcwd(temp.dir, sizeof(temp.dir));
    int32_t chresult = chdir(dir);
    if (result == 0 || chresult != 0){
        printf("trying pushdir %s\n", dir);
        assert(result != 0);
        assert(chresult == 0);
    }
    return(temp);
}

static void
popdir(Temp_Dir temp){
    chdir(temp.dir);
}

static void
init_time_system(){
    // NOTE(allen): do nothing
}

static uint64_t
get_time(){
    struct timespec spec;
    uint64_t result;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    result = (spec.tv_sec * (uint64_t)(1000000)) + (spec.tv_nsec / (uint64_t)(1000));
    return(result);
}

static int32_t
get_current_directory(char *buffer, int32_t max){
    int32_t result = 0;
    char *d = getcwd(buffer, max);
    if (d == buffer){
        result = strlen(buffer);
    }
    return(result);
}

static void
execute_in_dir(char *dir, char *str, char *args){
    if (dir){
        if (args){
            Temp_Dir temp = pushdir(dir);
            systemf("%s %s", str, args);
            popdir(temp);
        }
        else{
            Temp_Dir temp = pushdir(dir);
            systemf("%s", str);
            popdir(temp);
        }
    }
    else{
        if (args){
            systemf("%s %s", str, args);
        }
        else{
            systemf("%s", str);
        }
    }
}

static void
slash_fix(char *path){}

static void
make_folder_if_missing(char *dir, char *folder){
    if (folder){
        systemf("mkdir -p %s/%s", dir, folder);
    }
    else{
        systemf("mkdir -p %s", dir);
    }
}

static void
clear_folder(char *folder){
    systemf("rm -rf %s*", folder);
}

static void
delete_file(char *file){
    systemf("rm %s", file);
}

static void
copy_file(char *path, char *file, char *folder1, char *folder2, char *newname){
    if (!newname){
        newname = file;
    }
    
    if (path){
        if (folder2){
            systemf("cp %s/%s %s/%s/%s", path, file, folder1, folder2, newname);
        }
        else{
            systemf("cp %s/%s %s/%s", path, file, folder1, newname);
        }
    }
    else{
        if (folder2){
            systemf("cp %s %s/%s/%s", file, folder1, folder2, newname);
        }
        else{
            systemf("cp %s %s/%s", file, folder1, newname);
        }
    }
}

static void
copy_all(char *source, char *tag, char *folder){
    if (source){
        systemf("cp -f %s/%s %s", source, tag, folder);
    }
    else{
        systemf("cp -f %s %s", tag, folder);
    }
}

static void
zip(char *parent, char *folder, char *file){
    Temp_Dir temp = pushdir(parent);
    printf("PARENT DIR: %s\n", parent);
    systemf("zip -r %s %s", file, folder);
    popdir(temp);
}

#else
#error This OS is not supported yet
#endif

#endif

#endif

// BOTTOM

