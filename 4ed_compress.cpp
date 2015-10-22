/*
 * Mr. 4th Dimention - Allen Webster
 *
 * 20.10.2015
 *
 * Code compression for 4coder
 *
 */

// TOP

internal i32
compress_code(u8 *data, i32 size, void *dest){
    *(i32*)dest = size;
    dest = (i32*)dest + 1;
    memcpy(dest, data, size);
    return size + 4;
}

internal i32
decompress_code_size(void *compressed){
    i32 size = *(i32*)compressed;
    return size;
}

internal void
decompress_code(void *compressed, u8 *data, i32 size){
    Assert(size == *(i32*)compressed);
    compressed = (i32*)compressed + 1;
    memcpy(data, compressed, size);
}

// BOTTOM

