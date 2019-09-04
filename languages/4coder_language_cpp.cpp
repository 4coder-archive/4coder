/*
4coder_language_cpp.cpp - C++ language parser.
*/

// TOP

internal Token_Array
lex_cpp_initial(Base_Allocator *allocator, String_Const_u8 contents){
    Token_Array result = {};
    result.tokens = base_array(allocator, Token, 2);
    result.count = 2;
    result.max = 2;
    result.tokens[0].pos = 0;
    result.tokens[0].size = contents.size;
    result.tokens[0].kind = TokenBaseKind_COUNT;
    result.tokens[0].sub_kind = 0;
    result.tokens[1].pos = contents.size;
    result.tokens[1].size = 0;
    result.tokens[1].kind = TokenBaseKind_EOF;
    return(result);
}

// BOTTOM

