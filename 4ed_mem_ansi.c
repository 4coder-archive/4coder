/*
* Mr. 4th Dimention - Allen Webster
 *
 * 30.08.2016
 *
 * Replacements for common memory block managing functions.
 */

// TOP

// TODO(allen): make these as fast as possible
// with per architecture implementations.
void
block_copy(void *a, void *b, int32_t size){
    int32_t size8 = size/8;
    uint64_t *a8 = (uint64_t*)a;
    uint64_t *b8 = (uint64_t*)b;
    uint64_t *a8_end = a8 + size8;
    uint8_t *a1 = 0, *b1 = 0;
    
    for (;a8 < a8_end; ++a8, ++b8){
        *a8 = *b8;
    }
    
    a1 = (uint8_t*)a8;
    b1 = (uint8_t*)b8;
    
    switch (size % 8){
        case 7: *a1++ = *b1++;
        case 6: *a1++ = *b1++;
        case 5: *a1++ = *b1++;
        case 4: *a1++ = *b1++;
        case 3: *a1++ = *b1++;
        case 2: *a1++ = *b1++;
        case 1: *a1 = *b1;
    }
}

int64_t
block_compare(void *a, void *b, int32_t size){
    int64_t r = 0;
    int32_t size8 = size/4;
    uint32_t *a8 = (uint32_t*)a;
    uint32_t *b8 = (uint32_t*)b;
    uint32_t *a8_end = a8 + size8;
    uint8_t *a1 = 0, *b1 = 0;
    
    for (;a8 < a8_end; ++a8, ++b8){
        if (*a8 != *b8){
            r = (*a8 - *b8);
            goto end;
        }
    }
    
    a1 = (uint8_t*)a8;
    b1 = (uint8_t*)b8;
    
    switch (size % 4){
        case 3: if (*a1 != *b1){r = (*a1 - *b1); goto end;} ++a1, ++b1;
        case 2: if (*a1 != *b1){r = (*a1 - *b1); goto end;} ++a1, ++b1;
        case 1: if (*a1 != *b1){r = (*a1 - *b1); goto end;}
    }
    
    end:;
    return(r);
}

void
block_zero(void *a, int32_t size){
    int32_t size8 = size/8;
    uint64_t *a8 = (uint64_t*)a;
    uint64_t *a8_end = a8 + size8;
    uint8_t *a1 = 0;
    
    for (;a8 < a8_end; ++a8){
        *a8 = 0;
    }
    
    a1 = (uint8_t*)a8;
    
    switch (size % 8){
        case 7: *a1++ = 0;
        case 6: *a1++ = 0;
        case 5: *a1++ = 0;
        case 4: *a1++ = 0;
        case 3: *a1++ = 0;
        case 2: *a1++ = 0;
        case 1: *a1 = 0;
    }
}

// BOTTOM

