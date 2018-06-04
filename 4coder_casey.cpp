/* NOTE(casey): This code is _extremely_ bad and is mostly just me hacking things
   around to put in features I want in advance of 4coder having them properly.
   Most of the time I haven't even taken enough time to read the 4coder API
   to know what I'm actually even doing.  So if you decide to use the code in
   here, be advised that it might be super crashy or break something or cause you
   to lose work or who knows what else!
   
   DON'T SAY I WE DIDN'T WARN YA: This custom extension provided "as is" without
   warranty of any kind, either express or implied, including without
   limitation any implied warranties of condition, uninterrupted use,
   merchantability, fitness for a particular purpose, or non-infringement.
*/

/* TODO(casey): Here are our current issues

   - High priority:
     - Seems to buggily break out of the search sometimes for no reason?  (eg., you hit the end and it just drops out of the search instead of stopping?)
       - Tracked this one down: I think it is because spurious mousewheel or other inputs break
         out of the search.  How can this be prevented?
     - Replace:
       - Needs to be case-insensitive, or at least have the option to be
       - Needs to replace using the case of the thing being replaced, or at least have the option to do so
     - Deleting lines in comments is busted right now, probably because of the way I implemented my D command.
       This is probably something I need to fix for myself, but it's basically a problem whereby deleting a line
       sucks the next line up and indents it weirdly.
     - I'd prefer it if file-open deleted per-character instead of deleting entire path sections.  Deleting a whole path segment should be a different key, maybe?
       - Simimlarly, it'd be nice to get another Emacs behavior in here, which is that if you start a new path name anywhere in the path, it
         discards the initial part.  This was where you'd have w:/temp/foo/whatever/p:/foo it would just lop off everything before the p:,
         similarly in unix you would have w:/temp/foo/whatever//foo it would lop off everything before the /foo.  This is to make it so
         you don't have to erase the whole current path that comes up just to start typing a new path from scratch.  It's nice!
     - Real, robust jump-to-function (eg., no crashes and supports filtering by substring like the file picker does)
     
   - Search:
     - Should highlight all matches in the buffer
     
   - Display:
     - Dialogs like query-replace can lead to obscuring the cursor position if it happened to be on the top lines of the display
     - When switching _back_ to a buffer, it seems like it loses the scroll position, instead preferring
       to center the cursor?  This is undesirable IMO... <<< Check this again
     - I'd like to be able to hide the mark in text entry mode, and show the whole highlighted
       region in edit mode - perhaps even with a magic split at the top or bottom that shows where the mark
       is if it's off screen?
     - NOTE / IMPORTANT / TODO highlighting?  Ability to customize?  Whatever.
     - Some kind of parentheses highlighting?  I can write this myself, but I
       would need some way of adding highlight information to the buffer.
     - Need a way of highlighting the current line like Emacs does for the benefit
       of people on The Stream(TM)
     - Some kind of matching brace display so in long ifs, etc., you can see
       what they match (maybe draw it directly into the buffer?)
       
   - Indentation:
     - Need to have better indentation / wrapping control for typing in comments.
       Right now it's a bit worse than Emacs, which does automatically put you at
       the same margin as the prev. line (4coder just goes back to column 1).  It'd
       be nice if it go _better_ than Emacs, with no need to manually flow comments,
       etc.
       
   - Buffer management:
     - I'd like to be able to set a buffer to "auto-revert", so it reloads automatically whenever it changes externally
     - If you undo back to where there are no changes, the "buffer changed" flag should be cleared
     
   - File system
     - When switching to a buffer that has changed on disk, notify?  Really this can just
       be some way to query the modification flag and then the customization layer can do it?
     - I'd prefer it if file-open could create new files, and that I could get called on that
       so I can insert my boilerplate headers on new files
       
   - Need auto-complete for things like "arbitrary command", with options listed, etc.,
     so this should either be built into 4ed, or the custom DLL should have the ability
     to display possible completions and iterate over internal cmdid's, etc.  Possibly
     the latter, for maximal ability of customizers to add their own commands?
     
   - Macro recording/playback
   
   - Arbitrary cool features:
     - Once you can highlight things in 4coder buffers, I could make it so that my
       metacompiler output _ranges_ for errors, so it highlights the whole token rather
       than putting the cursor in a spot.
     - Highlight on the screen what the completion would be if you hit TAB now (eg., if the string appears elsewhere on the screen)
     - LOC count for the buffer and for all buffers summed shown in the title bar?
     - Show auto-parsed #if/if/for/while/etc. statements at else and closing places.
     - Automatic highlighting of the region in side the parentheses / etc.
     - You should just implement a shell inside 4coder which can call all the 4coder
       stuff as well as execute system stuff, so that from now on you just write
       scripts "in 4coder", etc., so they are always portable everywhere 4coder runs?
       
   - Things I should write:
     - Ability to do "file open from same directory as the current buffer"
     - Spell-checker
     - Repeat last replace?
     - Maybe search could be a permanent thing, so instead of initiating a search,
       you're just _changing_ the search term with MODAL-S, and then there's _always_
       a next-of-these-in... and that could go through buffers in order, to...
*/

// NOTE(casey): Microsoft/Windows is poopsauce.

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "4coder_default_include.cpp"

#define internal static

struct Parsed_Error
{
    int exists;
    
    String target_file_name;
    int target_line_number;
    int target_column_number;
    
    int source_buffer_id;
    int source_position;
};

static bool GlobalEditMode;
static bool GlobalBrightMode;
static char *GlobalCompilationBufferName = "*compilation*";

// TODO(casey): If 4coder gets variables at some point, this would go in a variable.
static char BuildDirectory[4096] = "./";

#define ZeroStruct(a) memset(&(a), 0, sizeof(a))

enum token_type
{
    Token_Unknown,
    
    Token_OpenParen,
    Token_CloseParen,
    Token_Asterisk,
    Token_Minus,
    Token_Plus,
    Token_ForwardSlash,
    Token_Percent,
    Token_Colon,
    Token_Number,
    Token_Comma,
    
    Token_EndOfStream,
};
struct token
{
    token_type Type;
    
    size_t TextLength;
    char *Text;
};

struct tokenizer
{
    char *At;
};

inline bool
IsEndOfLine(char C)
{
    bool Result = ((C == '\n') ||
                   (C == '\r'));
    
    return(Result);
}

inline bool
IsWhitespace(char C)
{
    bool Result = ((C == ' ') ||
                   (C == '\t') ||
                   (C == '\v') ||
                   (C == '\f') ||
                   IsEndOfLine(C));
    
    return(Result);
}

inline bool
IsAlpha(char C)
{
    bool Result = (((C >= 'a') && (C <= 'z')) ||
                   ((C >= 'A') && (C <= 'Z')));
    
    return(Result);
}

inline bool
IsNumeric(char C)
{
    bool Result = ((C >= '0') && (C <= '9'));
    
    return(Result);
}

static void
EatAllWhitespace(tokenizer *Tokenizer)
{
    for(;;)
    {
        if(IsWhitespace(Tokenizer->At[0]))
        {
            ++Tokenizer->At;
        }
        else if((Tokenizer->At[0] == '/') &&
                (Tokenizer->At[1] == '/'))
        {
            Tokenizer->At += 2;
            while(Tokenizer->At[0] && !IsEndOfLine(Tokenizer->At[0]))
            {
                ++Tokenizer->At;
            }
        }
        else if((Tokenizer->At[0] == '/') &&
                (Tokenizer->At[1] == '*'))
        {
            Tokenizer->At += 2;
            while(Tokenizer->At[0] &&
                  !((Tokenizer->At[0] == '*') &&
                    (Tokenizer->At[1] == '/')))
            {
                ++Tokenizer->At;
            }
            
            if(Tokenizer->At[0] == '*')
            {
                Tokenizer->At += 2;
            }
        }
        else
        {
            break;
        }
    }
}

static token
GetToken(tokenizer *Tokenizer)
{
    EatAllWhitespace(Tokenizer);
    
    token Token = {};
    Token.TextLength = 1;
    Token.Text = Tokenizer->At;
    char C = Tokenizer->At[0];
    ++Tokenizer->At;
    switch(C)
    {
        case 0: {--Tokenizer->At; Token.Type = Token_EndOfStream;} break;
        
        case '(': {Token.Type = Token_OpenParen;} break;
        case ')': {Token.Type = Token_CloseParen;} break;
        case '*': {Token.Type = Token_Asterisk;} break;
        case '-': {Token.Type = Token_Minus;} break;
        case '+': {Token.Type = Token_Plus;} break;
        case '/': {Token.Type = Token_ForwardSlash;} break;
        case '%': {Token.Type = Token_Percent;} break;
        case ':': {Token.Type = Token_Colon;} break;
        case ',': {Token.Type = Token_Comma;} break;
        
        default:
        {
            if(IsNumeric(C))
            {
                // TODO(casey): Real number
                Token.Type = Token_Number;
                while(IsNumeric(Tokenizer->At[0]) ||
                      (Tokenizer->At[0] == '.') ||
                      (Tokenizer->At[0] == 'f'))
                {
                    ++Tokenizer->At;
                    Token.TextLength = Tokenizer->At - Token.Text;
                }
            }
            else
            {
                Token.Type = Token_Unknown;
            }
        } break;
    }
    
    return(Token);
}

static token
PeekToken(tokenizer *Tokenizer)
{
    tokenizer Tokenizer2 = *Tokenizer;
    token Result = GetToken(&Tokenizer2);
    return(Result);
}

inline bool
IsH(String extension)
{
    bool Result = (match(extension, make_lit_string("h")) ||
                   match(extension, make_lit_string("hpp")) ||
                   match(extension, make_lit_string("hin")));
    
    return(Result);
}

inline bool
IsCPP(String extension)
{
    bool Result = (match(extension, make_lit_string("c")) ||
                   match(extension, make_lit_string("cpp")) ||
                   match(extension, make_lit_string("cin")));
    
    return(Result);
}

inline bool
IsBee(String extension)
{
    bool Result = (match(extension, make_lit_string("bee")) != 0);
    
    return(Result);
}

inline bool
IsShader(String extension)
{
    bool Result = (match(extension, make_lit_string("ps")) ||
                   match(extension, make_lit_string("vs")) ||
                   match(extension, make_lit_string("cs")) ||
                   match(extension, make_lit_string("ts")) ||
                   match(extension, make_lit_string("gs")));
    
    return(Result);
}

inline bool
IsINL(String extension)
{
    bool Result = (match(extension, make_lit_string("inl")) != 0);
    
    return(Result);
}

inline bool
IsJavascript(String extension)
{
    bool Result = (match(extension, make_lit_string("js")) != 0);
    
    return(Result);
}

inline bool
IsBAT(String extension)
{
    bool Result = (match(extension, make_lit_string("bat")) != 0);
    
    return(Result);
}

inline bool
IsTXT(String extension)
{
    bool Result = (match(extension, make_lit_string("txt")) != 0);
    
    return(Result);
}

inline bool
IsCMirror(String extension)
{
    bool Result = (match(extension, make_lit_string("cmirror")) != 0);
    
    return(Result);
}

inline bool
IsMTD(String extension)
{
    bool Result = (match(extension, make_lit_string("mtd")) != 0);
    
    return(Result);
}

inline bool
IsOutline(String extension)
{
    bool Result = (match(extension, make_lit_string("tol")) != 0);
    
    return(Result);
}

inline bool
IsMollyWebMarkup(String extension)
{
    bool Result = (match(extension, make_lit_string("mwm")) != 0);
    
    return(Result);
}

inline bool
IsCode(String extension)
{
    bool Result = (IsJavascript(extension) || IsBee(extension) || IsH(extension) || IsCPP(extension) || IsINL(extension) || IsBAT(extension) || IsCMirror(extension) || IsShader(extension) || IsMTD(extension));
    
    return(Result);
}

inline bool
IsDoc(String extension)
{
    bool Result = (IsTXT(extension) || IsOutline(extension) || IsMollyWebMarkup(extension));
    
    return(Result);
}

CUSTOM_COMMAND_SIG(casey_open_in_other)
{
    exec_command(app, change_active_panel);
    exec_command(app, cmdid_interactive_open_or_new);
}

CUSTOM_COMMAND_SIG(casey_clean_and_save)
{
    exec_command(app, clean_all_lines);
    exec_command(app, eol_nixify);
    exec_command(app, cmdid_save);
}

CUSTOM_COMMAND_SIG(casey_newline_and_indent)
{
    // NOTE(allen): The idea here is that if the current buffer is
    // read-only, it cannot be edited anyway.  So instead let the return
    // key indicate an attempt to interpret the line as a location to jump to.
    
    View_Summary view = get_active_view(app, AccessProtected);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessProtected);
    
    if (buffer.lock_flags & AccessProtected){
        exec_command(app, goto_jump_at_cursor);
    }
    else{
        exec_command(app, write_character);
        exec_command(app, auto_tab_line_at_cursor);
    }
}

CUSTOM_COMMAND_SIG(casey_open_file_other_window)
{
    exec_command(app, change_active_panel);
    exec_command(app, cmdid_interactive_open_or_new);
}

CUSTOM_COMMAND_SIG(casey_switch_buffer_other_window)
{
    exec_command(app, change_active_panel);
    exec_command(app, cmdid_interactive_switch_buffer);
}

internal void
DeleteAfterMotion(struct Application_Links *app, Custom_Command_Function *motion)
{
    unsigned int access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    
    int pos2 = view.cursor.pos;
    motion(app);
    refresh_view(app, &view);
    int pos1 = view.cursor.pos;
    
    Range range = make_range(pos1, pos2);
    
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    // NOTE(allen|a4.0.19): This 'paragraph' should fix 85% of whitespace problems, but if
    // it has some issue, you can get the old behavior by just removing it. Doing it by
    // cursor motion always seemed to introduce other problems spots.
    if (range.min > 0 && range.max < buffer.size){
        char before = buffer_get_char(app, &buffer, range.min - 1);
        char after  = buffer_get_char(app, &buffer, range.max);
        
        if ((IsWhitespace(before) || before == '(') && IsWhitespace(after)){
            if (after == ' '){
                range.max += 1;
            }
            else if (before == ' '){
                range.min -= 1;
            }
        }
    }
    
    buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
}

CUSTOM_COMMAND_SIG(casey_delete_token_left)
{
    DeleteAfterMotion(app, seek_white_or_token_left);
}

CUSTOM_COMMAND_SIG(casey_delete_token_right)
{
    DeleteAfterMotion(app, seek_white_or_token_right);
}

CUSTOM_COMMAND_SIG(casey_kill_to_end_of_line)
{
    unsigned int access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    
    int pos2 = view.cursor.pos;
    exec_command(app, seek_end_of_line);
    refresh_view(app, &view);
    int pos1 = view.cursor.pos;
    
    Range range = make_range(pos1, pos2);
    if(pos1 == pos2)
    {
        range.max += 1;
    }
    
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    buffer_replace_range(app, &buffer, range.min, range.max, 0, 0);
    exec_command(app, auto_tab_line_at_cursor);
}

CUSTOM_COMMAND_SIG(casey_paste_and_tab)
{
    exec_command(app, paste);
    exec_command(app, auto_tab_range);
}

CUSTOM_COMMAND_SIG(casey_seek_beginning_of_line_and_tab)
{
    exec_command(app, seek_beginning_of_line);
    exec_command(app, auto_tab_line_at_cursor);
}

CUSTOM_COMMAND_SIG(casey_seek_beginning_of_line)
{
    exec_command(app, auto_tab_line_at_cursor);
    exec_command(app, seek_beginning_of_line);
}

struct switch_to_result
{
    bool Switched;
    bool Loaded;
    View_Summary view;
    Buffer_Summary buffer;
};

inline void
SanitizeSlashes(String Value)
{
    for(int At = 0;
        At < Value.size;
        ++At)
    {
        if(Value.str[At] == '\\')
        {
            Value.str[At] = '/';
        }
    }
}

inline switch_to_result
SwitchToOrLoadFile(struct Application_Links *app, String FileName, bool CreateIfNotFound = false)
{
    switch_to_result Result = {};
    
    SanitizeSlashes(FileName);
    
    unsigned int access = AccessAll;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer_by_name(app, FileName.str, FileName.size, access);
    
    Result.view = view;
    Result.buffer = buffer;
    
    if(buffer.exists)
    {
        view_set_buffer(app, &view, buffer.buffer_id, 0);
        Result.Switched = true;
    }
    else
    {
        if(file_exists(app, FileName.str, FileName.size) || CreateIfNotFound)
        {
            // NOTE(allen): This opens the file and puts it in &view
            // This returns false if the open fails.
            view_open_file(app, &view, expand_str(FileName), false);
            
            Result.buffer = get_buffer_by_name(app, FileName.str, FileName.size, access);
            
            Result.Loaded = true;
            Result.Switched = true;
        }
    }
    
    return(Result);
}

CUSTOM_COMMAND_SIG(casey_load_todo)
{
    int size = app->memory_size/2;
    String dir = make_string(app->memory, 0, size);
    String command = make_string((char*)app->memory + size, 0, size);
    
    append(&dir, BuildDirectory);
    for(int At = 0;
        At < dir.size;
        ++At)
    {
        if(dir.str[At] == '/')
        {
            dir.str[At] = '\\';
        }
    }
    
    append(&command, dir);
    
    if(append(&command, "todo.tol"))
    {
        SwitchToOrLoadFile(app, command, true);
    }
}

CUSTOM_COMMAND_SIG(casey_build_search)
{
    int keep_going = 1;
    int old_size;
    // TODO(allen): It's fine to get memory this way for now, eventually
    // we should properly suballocating from app->memory.
    String dir = make_string(app->memory, 0, app->memory_size);
    dir.size = directory_get_hot(app, dir.str, dir.memory_size);
    
    while (keep_going)
    {
        old_size = dir.size;
        append(&dir, "build.bat");
        
        if (file_exists(app, dir.str, dir.size))
        {
            dir.size = old_size;
            memcpy(BuildDirectory, dir.str, dir.size);
            BuildDirectory[dir.size] = 0;
            
            // TODO(allen): There are ways this could be boiled down
            // to one print message which would be better.
            print_message(app, literal("Building with: "));
            print_message(app, BuildDirectory, dir.size);
            print_message(app, literal("build.bat\n"));
            
            return;
        }
        
        dir.size = old_size;
        
        if (directory_cd(app, dir.str, &dir.size, dir.memory_size, literal("..")) == 0)
        {
            keep_going = 0;
            print_message(app, literal("Did not find a build.bat\n"));
        }
    }
}

CUSTOM_COMMAND_SIG(casey_find_corresponding_file)
{
    unsigned int access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    String extension = file_extension(make_string(buffer.file_name, buffer.file_name_len));
    if (extension.str)
    {
        char *HExtensions[] =
        {
            "hpp",
            "hin",
            "h",
        };
        
        char *CExtensions[] =
        {
            "c",
            "cin",
            "cpp",
        };
        
        int ExtensionCount = 0;
        char **Extensions = 0;
        if(IsH(extension))
        {
            ExtensionCount = ArrayCount(CExtensions);
            Extensions = CExtensions;
        }
        else if(IsCPP(extension) || IsINL(extension))
        {
            ExtensionCount = ArrayCount(HExtensions);
            Extensions = HExtensions;
        }
        
        int MaxExtensionLength = 3;
        int Space = (int)(buffer.file_name_len + MaxExtensionLength);
        String FileNameStem = make_string(buffer.file_name, (int)(extension.str - buffer.file_name), 0);
        String TestFileName = make_string(app->memory, 0, Space);
        for(int ExtensionIndex = 0;
            ExtensionCount;
            ++ExtensionIndex)
        {
            TestFileName.size = 0;
            append(&TestFileName, FileNameStem);
            append(&TestFileName, Extensions[ExtensionIndex]);
            
            if(SwitchToOrLoadFile(app, TestFileName, ((ExtensionIndex + 1) == ExtensionCount)).Switched)
            {
                break;
            }
        }
    }
}

CUSTOM_COMMAND_SIG(casey_find_corresponding_file_other_window)
{
    unsigned int access = AccessProtected;
    View_Summary old_view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, old_view.buffer_id, access);
    
    exec_command(app, change_active_panel);
    View_Summary new_view = get_active_view(app, AccessAll);
    view_set_buffer(app, &new_view, buffer.buffer_id, 0);
    
    //    exec_command(app, casey_find_corresponding_file);
}

CUSTOM_COMMAND_SIG(casey_save_and_make_without_asking)
{
    exec_command(app, change_active_panel);
    
#if 0
    Buffer_Summary buffer = {};
    
    unsigned int access = AccessAll;
    for(buffer = get_buffer_first(app, access);
        buffer.exists;
        get_buffer_next(app, &buffer, access))
    {
        save_buffer(app, &buffer, buffer.file_name, buffer.file_name_len, 0);
    }
#endif
    save_all_dirty_buffers(app);
    
    // NOTE(allen): The parameter pushing made it a little easier
    // to deal with this particular pattern where two similar strings
    // were both used.  Now both strings need to exist at the same
    // time on the users side.
    
    int size = app->memory_size/2;
    String dir = make_string(app->memory, 0, size);
    String command = make_string((char*)app->memory + size, 0, size);
    
    append(&dir, BuildDirectory);
    for(int At = 0;
        At < dir.size;
        ++At)
    {
        if(dir.str[At] == '/')
        {
            dir.str[At] = '\\';
        }
    }
    
    append(&command, dir);
    
    if(append(&command, "build.bat"))
    {
        unsigned int access = AccessAll;
        View_Summary view = get_active_view(app, access);
        exec_system_command(app, &view,
                            buffer_identifier(GlobalCompilationBufferName, (int)strlen(GlobalCompilationBufferName)),
                            dir.str, dir.size,
                            command.str, command.size,
                            CLI_OverlapWithConflict);
        lock_jump_buffer(GlobalCompilationBufferName, str_size(GlobalCompilationBufferName));
    }
    exec_command(app, change_active_panel);
    
    ZeroStruct(prev_location);
}

#if 1
CUSTOM_COMMAND_SIG(casey_goto_previous_error)
{
    goto_prev_error_no_skips(app);
}

CUSTOM_COMMAND_SIG(casey_goto_next_error)
{
    goto_next_error_no_skips(app);
}
#else
CUSTOM_COMMAND_SIG(casey_goto_previous_error)
{
    seek_error(app, &global_part, true, false, -1);
}

CUSTOM_COMMAND_SIG(casey_goto_next_error)
{
    seek_error(app, &global_part, true, false, 1);
}
#endif

CUSTOM_COMMAND_SIG(casey_imenu)
{
    // TODO(casey): Implement!
}

//
// TODO(casey): Everything below this line probably isn't possible yet
//

CUSTOM_COMMAND_SIG(casey_call_keyboard_macro)
{
    // TODO(casey): Implement!
}

CUSTOM_COMMAND_SIG(casey_begin_keyboard_macro_recording)
{
    // TODO(casey): Implement!
}

CUSTOM_COMMAND_SIG(casey_end_keyboard_macro_recording)
{
    // TODO(casey): Implement!
}

CUSTOM_COMMAND_SIG(casey_fill_paragraph)
{
    // TODO(casey): Implement!
}

enum calc_node_type
{
    CalcNode_UnaryMinus,
    
    CalcNode_Add,
    CalcNode_Subtract,
    CalcNode_Multiply,
    CalcNode_Divide,
    CalcNode_Mod,
    
    CalcNode_Constant,
};
struct calc_node
{
    calc_node_type Type;
    double Value;
    calc_node *Left;
    calc_node *Right;
};

internal double
ExecCalcNode(calc_node *Node)
{
    double Result = 0.0f;
    
    if(Node)
    {
        switch(Node->Type)
        {
            case CalcNode_UnaryMinus: {Result = -ExecCalcNode(Node->Left);} break;
            case CalcNode_Add: {Result = ExecCalcNode(Node->Left) + ExecCalcNode(Node->Right);} break;
            case CalcNode_Subtract: {Result = ExecCalcNode(Node->Left) - ExecCalcNode(Node->Right);} break;
            case CalcNode_Multiply: {Result = ExecCalcNode(Node->Left) * ExecCalcNode(Node->Right);} break;
            case CalcNode_Divide: {/*TODO(casey): Guard 0*/Result = ExecCalcNode(Node->Left) / ExecCalcNode(Node->Right);} break;
            case CalcNode_Mod: {/*TODO(casey): Guard 0*/Result = fmod(ExecCalcNode(Node->Left), ExecCalcNode(Node->Right));} break;
            case CalcNode_Constant: {Result = Node->Value;} break;
            default: {Assert(!"AHHHHH!");}
        }
    }
    
    return(Result);
}

internal void
FreeCalcNode(calc_node *Node)
{
    if(Node)
    {
        FreeCalcNode(Node->Left);
        FreeCalcNode(Node->Right);
        free(Node);
    }
}

internal calc_node *
AddNode(calc_node_type Type, calc_node *Left = 0, calc_node *Right = 0)
{
    calc_node *Node = (calc_node *)malloc(sizeof(calc_node));
    Node->Type = Type;
    Node->Value = 0;
    Node->Left = Left;
    Node->Right = Right;
    return(Node);
}

internal calc_node *
ParseNumber(tokenizer *Tokenizer)
{
    calc_node *Result = AddNode(CalcNode_Constant);
    
    token Token = GetToken(Tokenizer);
    Result->Value = atof(Token.Text);
    
    return(Result);
}

internal calc_node *
ParseConstant(tokenizer *Tokenizer)
{
    calc_node *Result = 0;
    
    token Token = PeekToken(Tokenizer);
    if(Token.Type == Token_Minus)
    {
        Token = GetToken(Tokenizer);
        Result = AddNode(CalcNode_UnaryMinus);
        Result->Left = ParseNumber(Tokenizer);
    }
    else
    {
        Result = ParseNumber(Tokenizer);
    }
    
    return(Result);
}

#pragma warning(disable:4456)

internal calc_node *
ParseMultiplyExpression(tokenizer *Tokenizer)
{
    calc_node *Result = 0;
    
    token Token = PeekToken(Tokenizer);
    if((Token.Type == Token_Minus) ||
       (Token.Type == Token_Number))
    {
        Result = ParseConstant(Tokenizer);
        token Token = PeekToken(Tokenizer);
        if(Token.Type == Token_ForwardSlash)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Divide, Result, ParseNumber(Tokenizer));
        }
        else if(Token.Type == Token_Asterisk)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Multiply, Result, ParseNumber(Tokenizer));
        }
    }
    
    return(Result);
}

internal calc_node *
ParseAddExpression(tokenizer *Tokenizer)
{
    calc_node *Result = 0;
    
    token Token = PeekToken(Tokenizer);
    if((Token.Type == Token_Minus) ||
       (Token.Type == Token_Number))
    {
        Result = ParseMultiplyExpression(Tokenizer);
        token Token = PeekToken(Tokenizer);
        if(Token.Type == Token_Plus)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Add, Result, ParseMultiplyExpression(Tokenizer));
        }
        else if(Token.Type == Token_Minus)
        {
            GetToken(Tokenizer);
            Result = AddNode(CalcNode_Subtract, Result, ParseMultiplyExpression(Tokenizer));
        }
    }
    
    return(Result);
}

internal calc_node *
ParseCalc(tokenizer *Tokenizer)
{
    calc_node *Node = ParseAddExpression(Tokenizer);
    
    return(Node);
}

CUSTOM_COMMAND_SIG(casey_quick_calc)
{
    unsigned int access = AccessOpen;
    View_Summary view = get_active_view(app, access);
    
    Range range = get_view_range(&view);
    
    size_t Size = range.max - range.min;
    char *Stuff = (char *)malloc(Size + 1);
    Stuff[Size] = 0;
    
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    buffer_read_range(app, &buffer, range.min, range.max, Stuff);
    
    tokenizer Tokenizer = {Stuff};
    calc_node *CalcTree = ParseCalc(&Tokenizer);
    double ComputedValue = ExecCalcNode(CalcTree);
    FreeCalcNode(CalcTree);
    
    char ResultBuffer[256];
    int ResultSize = sprintf(ResultBuffer, "%f", ComputedValue);
    
    buffer_replace_range(app, &buffer, range.min, range.max, ResultBuffer, ResultSize);
    
    free(Stuff);
}

internal char *
GetNextString(char *Dest, int DestSize, char *&At)
{
    char *Result = 0;
    
    if(*At)
    {
        Result = Dest;
        while((--DestSize > 0) && *At && (*At != '\n'))
        {
            *Dest++ = *At++;
        }
        *Dest = 0;
        
        while(*At && (*At != '\n'))
        {
            ++At;
        }
        
        while(*At && (*At == '\n'))
        {
            ++At;
        }
    }
    
    return(Result);
}

internal void
OpenProject(Application_Links *app, char *Contents)
{
    int TotalOpenAttempts = 0;
    char *At = Contents;
    
    if(GetNextString(BuildDirectory, sizeof(BuildDirectory) - 1, At))
    {
        size_t BuildDirSize = strlen(BuildDirectory);
        if((BuildDirSize) && (BuildDirectory[BuildDirSize - 1] == '\n'))
        {
            --BuildDirSize;
        }
        
        if((BuildDirSize) && (BuildDirectory[BuildDirSize - 1] != '/'))
        {
            BuildDirectory[BuildDirSize++] = '/';
            BuildDirectory[BuildDirSize] = 0;
        }
    }
    
    char SourceFileDirectoryName[4096];
    char FileDirectoryName[4096];
    while(GetNextString(SourceFileDirectoryName, sizeof(SourceFileDirectoryName) - 1, At))
    {
        // NOTE(allen|a3.4.4): Here we get the list of files in this directory.
        // Notice that we free_file_list at the end.
        String dir = make_string(FileDirectoryName, 0, sizeof(FileDirectoryName));
        append(&dir, SourceFileDirectoryName);
        if(dir.size && dir.str[dir.size-1] == '\n')
        {
            --dir.size;
        }
        
        if(dir.size && dir.str[dir.size-1] != '/')
        {
            dir.str[dir.size++] = '/';
        }
        
        File_List list = get_file_list(app, dir.str, dir.size);
        int dir_size = dir.size;
        
        for (int unsigned i = 0; i < list.count; ++i)
        {
            File_Info *info = list.infos + i;
            if (!info->folder)
            {
                String filename = make_string(info->filename, info->filename_len);
                String extension = file_extension(filename);
                if (IsCode(extension) || IsDoc(extension))
                {
                    // NOTE(allen): There's no way in the 4coder API to use relative
                    // paths at the moment, so everything should be full paths.  Which is
                    // managable.  Here simply set the dir string size back to where it
                    // was originally, so that new appends overwrite old ones.
                    dir.size = dir_size;
                    append(&dir, info->filename);
                    
                    open_file(app, 0, dir.str, dir.size, true, true);
                    ++TotalOpenAttempts;
                }
            }
        }
        
        free_file_list(app, list);
    }
}

inline int
IsCodeLegal(int32_t Codepoint)
{
    int Result = ((Codepoint == '\n') ||
                  (Codepoint == '\t') ||
                  (Codepoint == '\r') ||
                  ((Codepoint >= 32)  &&
                   (Codepoint <= 126)));
    
    return(Result);
}

CUSTOM_COMMAND_SIG(casey_force_codelegal_characters)
{
    View_Summary view = get_active_view(app, AccessOpen);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, AccessOpen);
    
    int32_t line_count = buffer.line_count;
    int32_t edit_max = line_count;
    
    if (edit_max*(int32_t)sizeof(Buffer_Edit) < app->memory_size){
        Buffer_Edit *edits = (Buffer_Edit*)app->memory;
        
        char data[1024];
        Stream_Chunk chunk = {0};
        
        int32_t i = 0;
        int32_t last_utf = 0;
        int32_t run = 0;
        
        if (init_stream_chunk(&chunk, app, &buffer, i, data, sizeof(data))){
            Buffer_Edit *edit = edits;
            
            do
            {
                for (; i < chunk.end; ++i)
                {
                    if(IsCodeLegal(chunk.data[i]))
                    {
                        if(run)
                        {
                            edit->str_start = 0;
                            edit->len = 1;
                            edit->start = last_utf;
                            edit->end = i;
                            ++edit;
                            
                            run = false;
                        }
                    }
                    else if(!run)
                    {
                        last_utf = i;
                        
                        run = true;
                    }
                }
            } while(forward_stream_chunk(&chunk));
            
            if(run)
            {
                edit->str_start = 0;
                edit->len = 1;
                edit->start = last_utf;
                edit->end = i;
                ++edit;
                
                run = false;
            }
            
            int32_t edit_count = (int32_t)(edit - edits);
            buffer_batch_edit(app, &buffer, " ", 1, edits, edit_count, BatchEdit_PreserveTokens);
        }
    }
}

CUSTOM_COMMAND_SIG(casey_execute_arbitrary_command)
{
    Query_Bar bar;
    char space[1024], more_space[1024];
    bar.prompt = make_lit_string("Command: ");
    bar.string = make_fixed_width_string(space);
    
    if (!query_user_string(app, &bar)) return;
    end_query_bar(app, &bar, 0);
    
    if(match(bar.string, make_lit_string("codelegal")))
    {
        exec_command(app, casey_force_codelegal_characters);
    }
    else if(match(bar.string, make_lit_string("open menu")))
    {
        //        exec_command(app, cmdid_open_menu);
    }
    else
    {
        bar.prompt = make_fixed_width_string(more_space);
        append(&bar.prompt, make_lit_string("Unrecognized: "));
        append(&bar.prompt, bar.string);
        bar.string.size = 0;
        
        start_query_bar(app, &bar, 0);
        get_user_input(app, EventOnAnyKey | EventOnButton, 0);
    }
}

static void
casey_list_all_functions(Application_Links *app, Partition *part, Buffer_Summary *buffer, Buffer_Summary *decls_buffer){
    
    Temp_Memory temp = begin_temp_memory(part);
    
    struct Function_Positions{
        int32_t sig_start_index;
        int32_t sig_end_index;
        int32_t open_paren_pos;
    };
    
    Function_Positions *positions_array = push_array(part, Function_Positions, (4<<10)/sizeof(Function_Positions));
    int32_t positions_count = 0;
    
    Partition extra_memory_ = partition_sub_part(part, (4<<10));
    Partition *extra_memory = &extra_memory_;
    char *str = (char*)partition_current(part);
    int32_t part_size = 0;
    int32_t size = 0;
    
    static const int32_t token_chunk_size = 512;
    Cpp_Token token_chunk[token_chunk_size];
    Stream_Tokens token_stream = {0};
    
    if (init_stream_tokens(&token_stream, app, buffer, 0, token_chunk, token_chunk_size)){
        Stream_Tokens start_position_stream_temp = begin_temp_stream_token(&token_stream);
        
        int32_t token_index = 0;
        int32_t nest_level = 0;
        int32_t paren_nest_level = 0;
        
        int32_t first_paren_index = 0;
        int32_t first_paren_position = 0;
        int32_t last_paren_index = 0;
        
        bool32 still_looping = false;
        
        // Look for the next token at global scope that might need to be printed.
        mode1: do{
            for (; token_index < token_stream.end; ++token_index){
                Cpp_Token *token = &token_stream.tokens[token_index];
                
                if (!(token->flags & CPP_TFLAG_PP_BODY)){
                    switch (token->type){
                        case CPP_TOKEN_BRACE_OPEN:
                        {
                            ++nest_level;
                        }break;
                        
                        case CPP_TOKEN_BRACE_CLOSE:
                        {
                            if (nest_level > 0){
                                --nest_level;
                            }
                        }break;
                        
                        case CPP_TOKEN_PARENTHESE_OPEN:
                        {
                            if (nest_level == 0){
                                first_paren_index = token_index;
                                first_paren_position = token->start;
                                goto paren_mode1;
                            }
                        }break;
                    }
                }
            }
            still_looping = forward_stream_tokens(&token_stream);
        }while(still_looping);
        goto end;
        
        // Look for a closing parenthese to mark the end of a function signature.
        paren_mode1:
        paren_nest_level = 0;
        do{
            for (; token_index < token_stream.end; ++token_index){
                Cpp_Token *token = &token_stream.tokens[token_index];
                
                if (!(token->flags & CPP_TFLAG_PP_BODY)){
                    switch (token->type){
                        case CPP_TOKEN_PARENTHESE_OPEN:
                        {
                            ++paren_nest_level;
                        }break;
                        
                        case CPP_TOKEN_PARENTHESE_CLOSE:
                        {
                            --paren_nest_level;
                            if (paren_nest_level == 0){
                                last_paren_index = token_index;
                                goto paren_mode2;
                            }
                        }break;
                    }
                }
            }
            still_looping = forward_stream_tokens(&token_stream);
        }while(still_looping);
        goto end;
        
        // Look backwards from an open parenthese to find the start of a function signature.
        paren_mode2: {
            Stream_Tokens backward_stream_temp = begin_temp_stream_token(&token_stream);
            int32_t local_index = first_paren_index;
            int32_t signature_start_index = 0;
            
            do{
                for (; local_index >= token_stream.start; --local_index){
                    Cpp_Token *token = &token_stream.tokens[local_index];
                    if ((token->flags & CPP_TFLAG_PP_BODY) || (token->flags & CPP_TFLAG_PP_DIRECTIVE) || token->type == CPP_TOKEN_BRACE_CLOSE || token->type == CPP_TOKEN_SEMICOLON || token->type == CPP_TOKEN_PARENTHESE_CLOSE){
                        ++local_index;
                        signature_start_index = local_index;
                        goto paren_mode2_done;
                    }
                }
                still_looping = backward_stream_tokens(&token_stream);
            }while(still_looping);
            // When this loop ends by going all the way back to the beginning set the signature start to 0 and fall through to the printing phase.
            signature_start_index = 0;
            
            paren_mode2_done:;
            {
                Function_Positions positions;
                positions.sig_start_index = signature_start_index;
                positions.sig_end_index = last_paren_index;
                positions.open_paren_pos = first_paren_position;
                positions_array[positions_count++] = positions;
            }
            
            end_temp_stream_token(&token_stream, backward_stream_temp);
            goto mode1;
        }
        
        end:;
        end_temp_stream_token(&token_stream, start_position_stream_temp);
        // Print the results
        String buffer_name = make_string(buffer->buffer_name, buffer->buffer_name_len);
        for (int32_t i = 0; i < positions_count; ++i){
            Function_Positions *positions = &positions_array[i];
            Temp_Memory extra_temp = begin_temp_memory(extra_memory);
            
            int32_t local_index = positions->sig_start_index;
            int32_t end_index = positions->sig_end_index;
            int32_t open_paren_pos = positions->open_paren_pos;
            
            do{
                for (; local_index < token_stream.end; ++local_index){
                    Cpp_Token *token = &token_stream.tokens[local_index];
                    if (!(token->flags & CPP_TFLAG_PP_BODY)){
                        if (token->type != CPP_TOKEN_COMMENT){
                            bool32 delete_space_before = false;
                            bool32 space_after = false;
                            
                            switch (token->type){
                                case CPP_TOKEN_COMMA:
                                case CPP_TOKEN_PARENTHESE_OPEN:
                                case CPP_TOKEN_PARENTHESE_CLOSE:
                                {
                                    delete_space_before = true;
                                }break;
                            }
                            
                            switch (token->type){
                                case CPP_TOKEN_IDENTIFIER:
                                case CPP_TOKEN_COMMA:
                                case CPP_TOKEN_STAR:
                                {
                                    space_after = true;
                                }break;
                            }
                            if (token->flags & CPP_TFLAG_IS_KEYWORD){
                                space_after = true;
                            }
                            
                            if (delete_space_before){
                                int32_t pos = extra_memory->pos - 1;
                                char *base = ((char*)(extra_memory->base));
                                if (pos >= 0 && base[pos] == ' '){
                                    extra_memory->pos = pos;
                                }
                            }
                            
                            char *token_str = push_array(extra_memory, char, token->size + space_after);
                            
                            buffer_read_range(app, buffer, token->start, token->start + token->size, token_str);
                            if (space_after){
                                token_str[token->size] = ' ';
                            }
                        }
                    }
                    
                    if (local_index == end_index){
                        goto finish_print;
                    }
                }
                still_looping = forward_stream_tokens(&token_stream);
            }while(still_looping);
            
            finish_print:;
            {
                int32_t sig_size = extra_memory->pos;
                String sig = make_string(extra_memory->base, sig_size);
                
                int32_t line_number = buffer_get_line_number(app, buffer, open_paren_pos);
                int32_t line_number_len = int_to_str_size(line_number);
                
                int32_t append_len = buffer_name.size + 1 + line_number_len + 1 + 1 + sig_size + 1;
                
                char *out_space = push_array(part, char, append_len);
                if (out_space == 0){
                    buffer_replace_range(app, decls_buffer, size, size, str, part_size);
                    size += part_size;
                    
                    end_temp_memory(temp);
                    temp = begin_temp_memory(part);
                    
                    part_size = 0;
                    out_space = push_array(part, char, append_len);
                }
                
                part_size += append_len;
                String out = make_string(out_space, 0, append_len);
                append(&out, buffer_name);
                append(&out, ':');
                append_int_to_str(&out, line_number);
                append(&out, ':');
                append(&out, ' ');
                append(&out, sig);
                append(&out, '\n');
            }
            
            end_temp_memory(extra_temp);
        }
        
        buffer_replace_range(app, decls_buffer, size, size, str, part_size);
        
        View_Summary view = get_active_view(app, AccessAll);
        view_set_buffer(app, &view, decls_buffer->buffer_id, 0);
        
        lock_jump_buffer(decls_buffer->buffer_name, decls_buffer->buffer_name_len);
        
        end_temp_memory(temp);
    }
}

internal void
ClearDeclsBuffer(Application_Links *app, Buffer_Summary *decls_buffer)
{
    String search_name = make_lit_string("*decls*");
    *decls_buffer = get_buffer_by_name(app, search_name.str, search_name.size, AccessAll);
    if (!decls_buffer->exists){
        *decls_buffer = create_buffer(app, search_name.str, search_name.size, BufferCreate_AlwaysNew);
        buffer_set_setting(app, decls_buffer, BufferSetting_Unimportant, true);
        buffer_set_setting(app, decls_buffer, BufferSetting_ReadOnly, true);
        buffer_set_setting(app, decls_buffer, BufferSetting_WrapLine, false);
    }
    else{
        buffer_replace_range(app, decls_buffer, 0, decls_buffer->size, 0, 0);
    }
}

CUSTOM_COMMAND_SIG(casey_list_all_functions_current_buffer){
    uint32_t access = AccessProtected;
    View_Summary view = get_active_view(app, access);
    Buffer_Summary buffer = get_buffer(app, view.buffer_id, access);
    
    Buffer_Summary decls_buffer;
    ClearDeclsBuffer(app, &decls_buffer);
    casey_list_all_functions(app, &global_part, &buffer, &decls_buffer);
}

CUSTOM_COMMAND_SIG(casey_list_all_functions_globally){
    uint32_t access = AccessProtected;
    
    Buffer_Summary decls_buffer;
    ClearDeclsBuffer(app, &decls_buffer);
    
    for (Buffer_Summary buffer_it = get_buffer_first(app, access);
         buffer_it.exists;
         get_buffer_next(app, &buffer_it, access))
    {
        casey_list_all_functions(app, &global_part, &buffer_it, &decls_buffer);
    }
}

internal void
UpdateModalIndicator(Application_Links *app)
{
    int unsigned Background = (GlobalBrightMode ? 0xFFFFFF : 0x161616);
    int unsigned Default = (GlobalBrightMode ? 0x000000 : 0xA08563);
    int unsigned Constant = 0x6B8E23;
    
    Theme_Color normal_colors[] =
    {
        {Stag_Cursor, 0x40FF40},
        {Stag_At_Cursor, 0x161616},
        {Stag_Mark, 0x808080},
        //{Stag_Margin, 0x262626},
        //{Stag_Margin_Hover, 0x333333},
        //{Stag_Margin_Active, 0x404040},
        {Stag_Bar, 0xCACACA}
    };
    
    Theme_Color edit_colors[] =
    {
        {Stag_Cursor, 0xFF0000},
        {Stag_At_Cursor, 0x00FFFF},
        {Stag_Mark, 0xFF6F1A},
        //{Stag_Margin, 0x33170B},
        //{Stag_Margin_Hover, 0x49200F},
        //{Stag_Margin_Active, 0x934420},
        {Stag_Bar, 0xCACACA}
        // {Stag_Bar, 0x934420}
    };
    
    if (GlobalEditMode)
    {
        set_theme_colors(app, edit_colors, ArrayCount(edit_colors));
    }
    else
    {
        set_theme_colors(app, normal_colors, ArrayCount(normal_colors));
    }
    
    Theme_Color common_colors[] =
    {
        {Stag_Comment, 0x7D7D7D},
        {Stag_Keyword, 0xCD950C},
        {Stag_Preproc, 0xDAB98F},
        {Stag_Include, Constant},
        {Stag_Back, Background},
        {Stag_Margin, Background},
        {Stag_Margin_Hover, Background},
        {Stag_Margin_Active, Background},
        {Stag_List_Item,Background},
        {Stag_List_Item_Hover, 0x934420},
        {Stag_List_Item_Active, 0x934420},
        {Stag_Default, Default},
        
        {Stag_Str_Constant, Constant},
        {Stag_Char_Constant, Constant},
        {Stag_Int_Constant, Constant},
        {Stag_Float_Constant, Constant},
        {Stag_Bool_Constant, Constant},
    };
    set_theme_colors(app, common_colors, ArrayCount(common_colors));
}

CUSTOM_COMMAND_SIG(begin_free_typing)
{
    GlobalEditMode = false;
    UpdateModalIndicator(app);
}

CUSTOM_COMMAND_SIG(end_free_typing)
{
    GlobalEditMode = true;
    UpdateModalIndicator(app);
}

CUSTOM_COMMAND_SIG(toggle_bright_mode)
{
    GlobalBrightMode = !GlobalBrightMode;
    UpdateModalIndicator(app);
}

#define DEFINE_FULL_BIMODAL_KEY(binding_name,edit_code,normal_code) \
CUSTOM_COMMAND_SIG(binding_name) \
{ \
    if(GlobalEditMode) \
    { \
        edit_code;            \
    } \
    else \
    { \
        normal_code; \
    } \
}

#define DEFINE_BIMODAL_KEY(binding_name,edit_code,normal_code) DEFINE_FULL_BIMODAL_KEY(binding_name,exec_command(app,edit_code),exec_command(app,normal_code))
#define DEFINE_MODAL_KEY(binding_name,edit_code) DEFINE_BIMODAL_KEY(binding_name,edit_code,write_character)

//    paste_next ?
//    cmdid_history_backward,
//    cmdid_history_forward,
//    toggle_line_wrap,

DEFINE_MODAL_KEY(modal_space, set_mark);
DEFINE_MODAL_KEY(modal_back_slash, casey_clean_and_save);
DEFINE_MODAL_KEY(modal_single_quote, casey_call_keyboard_macro);
DEFINE_MODAL_KEY(modal_comma, seek_whitespace_down);
DEFINE_MODAL_KEY(modal_period, casey_fill_paragraph);
DEFINE_MODAL_KEY(modal_forward_slash, change_active_panel);
DEFINE_MODAL_KEY(modal_semicolon, seek_white_or_token_right);
DEFINE_BIMODAL_KEY(modal_open_bracket, casey_begin_keyboard_macro_recording, write_and_auto_tab);
DEFINE_BIMODAL_KEY(modal_close_bracket, casey_end_keyboard_macro_recording, write_and_auto_tab);
DEFINE_MODAL_KEY(modal_a, write_character); // TODO(casey): Arbitrary command + casey_quick_calc
DEFINE_MODAL_KEY(modal_b, cmdid_interactive_switch_buffer);
DEFINE_MODAL_KEY(modal_c, casey_find_corresponding_file);
DEFINE_MODAL_KEY(modal_d, casey_kill_to_end_of_line);
DEFINE_MODAL_KEY(modal_e, write_character);
DEFINE_MODAL_KEY(modal_f, casey_paste_and_tab);
DEFINE_MODAL_KEY(modal_g, goto_line);
DEFINE_MODAL_KEY(modal_h, seek_white_or_token_left);
DEFINE_MODAL_KEY(modal_i, move_up);
DEFINE_MODAL_KEY(modal_j, casey_list_all_functions_globally);
DEFINE_MODAL_KEY(modal_k, list_all_locations);
DEFINE_MODAL_KEY(modal_l, list_all_substring_locations_case_insensitive);
DEFINE_MODAL_KEY(modal_m, casey_save_and_make_without_asking);
DEFINE_MODAL_KEY(modal_n, goto_next_error);
DEFINE_MODAL_KEY(modal_o, query_replace);
DEFINE_MODAL_KEY(modal_p, replace_in_range);
DEFINE_MODAL_KEY(modal_q, copy);
DEFINE_MODAL_KEY(modal_r, reverse_search);
DEFINE_MODAL_KEY(modal_s, search);
DEFINE_MODAL_KEY(modal_t, casey_load_todo);
DEFINE_MODAL_KEY(modal_u, cmdid_undo);
DEFINE_MODAL_KEY(modal_v, casey_switch_buffer_other_window);
DEFINE_MODAL_KEY(modal_w, cut);
DEFINE_MODAL_KEY(modal_x, casey_find_corresponding_file_other_window);
DEFINE_MODAL_KEY(modal_y, cmdid_redo);
DEFINE_MODAL_KEY(modal_z, cmdid_interactive_open_or_new);

DEFINE_MODAL_KEY(modal_1, casey_build_search); // TODO(casey): Shouldn't need to bind a key for this?
DEFINE_MODAL_KEY(modal_2, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_3, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_4, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_5, toggle_bright_mode); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_6, auto_tab_range); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_7, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_8, seek_whitespace_up); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_9, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_0, cmdid_kill_buffer);
DEFINE_MODAL_KEY(modal_minus, write_character); // TODO(casey): Available
DEFINE_MODAL_KEY(modal_equals, casey_execute_arbitrary_command);

DEFINE_BIMODAL_KEY(modal_backspace, casey_delete_token_left, backspace_char);
DEFINE_BIMODAL_KEY(modal_up, move_up, move_up);
DEFINE_BIMODAL_KEY(modal_down, move_down, move_down);
DEFINE_BIMODAL_KEY(modal_left, seek_white_or_token_left, move_left);
DEFINE_BIMODAL_KEY(modal_right, seek_white_or_token_right, move_right);
DEFINE_BIMODAL_KEY(modal_delete, casey_delete_token_right, delete_char);
DEFINE_BIMODAL_KEY(modal_home, casey_seek_beginning_of_line, casey_seek_beginning_of_line_and_tab);
DEFINE_BIMODAL_KEY(modal_end, seek_end_of_line, seek_end_of_line);
DEFINE_BIMODAL_KEY(modal_page_up, page_up, seek_whitespace_up);
DEFINE_BIMODAL_KEY(modal_page_down, page_down, seek_whitespace_down);
DEFINE_BIMODAL_KEY(modal_tab, word_complete, word_complete);

OPEN_FILE_HOOK_SIG(casey_file_settings)
{
    // NOTE(allen|a4): As of alpha 4 hooks can have parameters which are
    // received through functions like this get_parameter_buffer.
    // This is different from the past where this hook got a buffer
    // from get_active_buffer.
    unsigned int access = AccessAll;
    //Buffer_Summary buffer = get_parameter_buffer(app, 0, access);
    Buffer_Summary buffer = get_buffer(app, buffer_id, access);
    
    int treat_as_code = 0;
    int treat_as_project = 0;
    int treat_as_doc = 0;
    int treat_as_outline = 0;
    int wrap_lines = 1;
    int WrapPosition = 1200;
    
    if (buffer.file_name && buffer.size < (16 << 20))
    {
        String ext = file_extension(make_string(buffer.file_name, buffer.file_name_len));
        treat_as_code = IsCode(ext);
        treat_as_project = match(ext, make_lit_string("prj"));
        treat_as_doc = IsDoc(ext);
        treat_as_outline = IsOutline(ext);
    }
    
    if(treat_as_outline)
    {
        buffer_set_setting(app, &buffer, BufferSetting_VirtualWhitespace, 1);
        buffer_set_setting(app, &buffer, BufferSetting_LexWithoutStrings, 1);
    }
    else if(treat_as_code)
    {
        buffer_set_setting(app, &buffer, BufferSetting_Lex, treat_as_code);
        buffer_set_setting(app, &buffer, BufferSetting_WrapLine, 0);
    }
    else if(treat_as_doc)
    {
        buffer_set_setting(app, &buffer, BufferSetting_WrapIndicator, WrapIndicator_Hide);
        buffer_set_setting(app, &buffer, BufferSetting_WrapLine, 1);
        WrapPosition = 600;
    }
    else
    {
        buffer_set_setting(app, &buffer, BufferSetting_WrapLine, wrap_lines);
    }
    
    buffer_set_setting(app, &buffer, BufferSetting_MapID, mapid_file);
    buffer_set_setting(app, &buffer, BufferSetting_WrapPosition, WrapPosition);
    
    if(treat_as_project)
    {
        int size = buffer.size;
        char *ParsingRegion = (char *)malloc(size + 1);
        buffer_read_range(app, &buffer, 0, size, ParsingRegion);
        ParsingRegion[size] = 0;
        OpenProject(app, ParsingRegion);
        free(ParsingRegion);
    }
    
    return(0);
}

bool
CubicUpdateFixedDuration1(float *P0, float *V0, float P1, float V1, float Duration, float dt)
{
    bool Result = false;
    
    if(dt > 0)
    {
        if(Duration < dt)
        {
            *P0 = P1 + (dt - Duration)*V1;
            *V0 = V1;
            Result = true;
        }
        else
        {
            float t = (dt / Duration);
            float u = (1.0f - t);
            
            float C0 = 1*u*u*u;
            float C1 = 3*u*u*t;
            float C2 = 3*u*t*t;
            float C3 = 1*t*t*t;
            
            float dC0 = -3*u*u;
            float dC1 = -6*u*t + 3*u*u;
            float dC2 =  6*u*t - 3*t*t;
            float dC3 =  3*t*t;
            
            float B0 = *P0;
            float B1 = *P0 + (Duration / 3.0f) * *V0;
            float B2 = P1 - (Duration / 3.0f) * V1;
            float B3 = P1;
            
            *P0 = C0*B0 + C1*B1 + C2*B2 + C3*B3;
            *V0 = (dC0*B0 + dC1*B1 + dC2*B2 + dC3*B3) * (1.0f / Duration);
        }
    }
    
    return(Result);
}

struct Casey_Scroll_Velocity
{
    float x, y, t;
};

Casey_Scroll_Velocity casey_scroll_velocity_[16] = {0};
Casey_Scroll_Velocity *casey_scroll_velocity = casey_scroll_velocity_ - 1;

SCROLL_RULE_SIG(casey_smooth_scroll_rule){
    Casey_Scroll_Velocity *velocity = casey_scroll_velocity + view_id;
    int result = 0;
    if(is_new_target)
    {
        if((*scroll_x != target_x) ||
           (*scroll_y != target_y))
        {
            velocity->t = 0.1f;
        }
    }
    
    if(velocity->t > 0)
    {
        result = !(CubicUpdateFixedDuration1(scroll_x, &velocity->x, target_x, 0.0f, velocity->t, dt) ||
                   CubicUpdateFixedDuration1(scroll_y, &velocity->y, target_y, 0.0f, velocity->t, dt));
    }
    
    velocity->t -= dt;
    if(velocity->t < 0)
    {
        velocity->t = 0;
        *scroll_x = target_x;
        *scroll_y = target_y;
    }
    
    return(result);
}

START_HOOK_SIG(casey_start)
{
    bool HandmadeHeroMachine = (strcmp(getenv("COMPUTERNAME"), "CASEYMPC2") == 0);
    
    // NOTE(allen): This initializes a couple of global memory
    // management structs on the custom side that are used in
    // some of the new 4coder features including building and
    // custom-side word complete.
    init_memory(app);
    
    exec_command(app, hide_scrollbar);
    if(!HandmadeHeroMachine) {exec_command(app, hide_filebar);}
    exec_command(app, open_panel_vsplit);
    exec_command(app, hide_scrollbar);
    if(!HandmadeHeroMachine) {exec_command(app, hide_filebar);}
    exec_command(app, change_active_panel);
    
    change_theme(app, literal("Handmade Hero"));
    set_global_face_by_name(app, literal("Droid Sans Mono"), true);
    UpdateModalIndicator(app);
    
    return(0);
}

extern "C" GET_BINDING_DATA(get_bindings)
{
    Bind_Helper context_actual = begin_bind_helper(data, size);
    Bind_Helper *context = &context_actual;
    
    set_start_hook(context, casey_start);
    set_command_caller(context, default_command_caller);
    set_open_file_hook(context, casey_file_settings);
    set_scroll_rule(context, casey_smooth_scroll_rule);
    set_end_file_hook(context, end_file_close_jump_list);
    
    begin_map(context, mapid_global);
    {
        bind(context, 'z', MDFR_NONE, cmdid_interactive_open_or_new);
        bind(context, 'x', MDFR_NONE, casey_open_in_other);
        bind(context, 't', MDFR_NONE, casey_load_todo);
        bind(context, '/', MDFR_NONE, change_active_panel);
        bind(context, 'b', MDFR_NONE, cmdid_interactive_switch_buffer);
        bind(context, key_page_up, MDFR_NONE, search);
        bind(context, key_page_down, MDFR_NONE, reverse_search);
        bind(context, 'm', MDFR_NONE, casey_save_and_make_without_asking);
        
        // NOTE(allen): Added this so mouse would keep working rougly as before.
        // Of course now there could be a modal click behavior if that will be useful.
        // As well as right click.
        bind(context, key_mouse_left, MDFR_NONE, click_set_cursor);
    }
    end_map(context);
    
    begin_map(context, mapid_file);
    
    bind_vanilla_keys(context, write_character);
    
    // TODO(casey): How can I bind something to just pressing the control key by itself?
    // bind(context, key_control, MDFR_NONE, end_free_typing);
    
    bind(context, key_insert, MDFR_NONE, begin_free_typing);
    bind(context, '`', MDFR_NONE, begin_free_typing);
    bind(context, key_esc, MDFR_NONE, end_free_typing);
    bind(context, '\n', MDFR_NONE, casey_newline_and_indent);
    bind(context, '\n', MDFR_SHIFT, casey_newline_and_indent);
    
    // NOTE(casey): Modal keys come here.
    bind(context, ' ', MDFR_NONE, modal_space);
    bind(context, ' ', MDFR_SHIFT, modal_space);
    
    bind(context, '\\', MDFR_NONE, modal_back_slash);
    bind(context, '\'', MDFR_NONE, modal_single_quote);
    bind(context, ',', MDFR_NONE, modal_comma);
    bind(context, '.', MDFR_NONE, modal_period);
    bind(context, '/', MDFR_NONE, modal_forward_slash);
    bind(context, ';', MDFR_NONE, modal_semicolon);
    bind(context, '[', MDFR_NONE, modal_open_bracket);
    bind(context, ']', MDFR_NONE, modal_close_bracket);
    bind(context, '{', MDFR_NONE, write_and_auto_tab);
    bind(context, '}', MDFR_NONE, write_and_auto_tab);
    bind(context, 'a', MDFR_NONE, modal_a);
    bind(context, 'b', MDFR_NONE, modal_b);
    bind(context, 'c', MDFR_NONE, modal_c);
    bind(context, 'd', MDFR_NONE, modal_d);
    bind(context, 'e', MDFR_NONE, modal_e);
    bind(context, 'f', MDFR_NONE, modal_f);
    bind(context, 'g', MDFR_NONE, modal_g);
    bind(context, 'h', MDFR_NONE, modal_h);
    bind(context, 'i', MDFR_NONE, modal_i);
    bind(context, 'j', MDFR_NONE, modal_j);
    bind(context, 'k', MDFR_NONE, modal_k);
    bind(context, 'l', MDFR_NONE, modal_l);
    bind(context, 'm', MDFR_NONE, modal_m);
    bind(context, 'n', MDFR_NONE, modal_n);
    bind(context, 'o', MDFR_NONE, modal_o);
    bind(context, 'p', MDFR_NONE, modal_p);
    bind(context, 'q', MDFR_NONE, modal_q);
    bind(context, 'r', MDFR_NONE, modal_r);
    bind(context, 's', MDFR_NONE, modal_s);
    bind(context, 't', MDFR_NONE, modal_t);
    bind(context, 'u', MDFR_NONE, modal_u);
    bind(context, 'v', MDFR_NONE, modal_v);
    bind(context, 'w', MDFR_NONE, modal_w);
    bind(context, 'x', MDFR_NONE, modal_x);
    bind(context, 'y', MDFR_NONE, modal_y);
    bind(context, 'z', MDFR_NONE, modal_z);
    
    bind(context, '1', MDFR_NONE, modal_1);
    bind(context, '2', MDFR_NONE, modal_2);
    bind(context, '3', MDFR_NONE, modal_3);
    bind(context, '4', MDFR_NONE, modal_4);
    bind(context, '5', MDFR_NONE, modal_5);
    bind(context, '6', MDFR_NONE, modal_6);
    bind(context, '7', MDFR_NONE, modal_7);
    bind(context, '8', MDFR_NONE, modal_8);
    bind(context, '9', MDFR_NONE, modal_9);
    bind(context, '0', MDFR_NONE, modal_0);
    bind(context, '-', MDFR_NONE, modal_minus);
    bind(context, '=', MDFR_NONE, modal_equals);
    
    bind(context, key_back, MDFR_NONE, modal_backspace);
    bind(context, key_back, MDFR_SHIFT, modal_backspace);
    
    bind(context, key_up, MDFR_NONE, modal_up);
    bind(context, key_up, MDFR_SHIFT, modal_up);
    
    bind(context, key_down, MDFR_NONE, modal_down);
    bind(context, key_down, MDFR_SHIFT, modal_down);
    
    bind(context, key_left, MDFR_NONE, modal_left);
    bind(context, key_left, MDFR_SHIFT, modal_left);
    
    bind(context, key_right, MDFR_NONE, modal_right);
    bind(context, key_right, MDFR_SHIFT, modal_right);
    
    bind(context, key_del, MDFR_NONE, modal_delete);
    bind(context, key_del, MDFR_SHIFT, modal_delete);
    
    bind(context, key_home, MDFR_NONE, modal_home);
    bind(context, key_home, MDFR_SHIFT, modal_home);
    
    bind(context, key_end, MDFR_NONE, modal_end);
    bind(context, key_end, MDFR_SHIFT, modal_end);
    
    bind(context, key_page_up, MDFR_NONE, modal_page_up);
    bind(context, key_page_up, MDFR_SHIFT, modal_page_up);
    
    bind(context, key_page_down, MDFR_NONE, modal_page_down);
    bind(context, key_page_down, MDFR_SHIFT, modal_page_down);
    
    bind(context, '\t', MDFR_NONE, modal_tab);
    bind(context, '\t', MDFR_SHIFT, modal_tab);
    
    bind(context, key_f4, MDFR_ALT, exit_4coder);
    
    end_map(context);
    
    end_bind_helper(context);
    return context->write_total;
}
