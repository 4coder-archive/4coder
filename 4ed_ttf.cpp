/*
 * YOUR INFO HERE!
 */

// TOP

typedef unsigned int Fixed16_16;

struct Offset_Table{
    Fixed16_16 version;
    unsigned short num_tables;
    unsigned short search_range;
    unsigned short entry_selector;
    unsigned short range_shift;
};

struct Table_Directory_Entry{
    unsigned long tag;
    unsigned long check_sum;
    unsigned long offset;
    unsigned long length;
};

#include <stdio.h>
#include <stdlib.h>

struct Data{
    char *data;
    int size;
};

Data
open_file(const char *filename){
    Data result = {};
    FILE *file;
    file = fopen(filename, "rb");
    if (file){
        fseek(file, 0, SEEK_END);
        result.size = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (result.size > 0){
            result.data = (char*)malloc(result.size);
            fread(result.data, result.size, 1, file);
        }
        fclose(file);
    }
    return(result);
}

void
print(Offset_Table *offset){
    printf("version %d\n", offset->version >> 16);
    printf("number of tables %d\n", (int)(offset->num_tables));
    printf("search range %d\n", (int)(offset->search_range));
    printf("entry selector %d\n", (int)(offset->entry_selector));
    printf("range shift %d\n", (int)(offset->range_shift));
}

void
print(Table_Directory_Entry *entry){
    printf("tag %.4s\n", &entry->tag);
    printf("check sum %08x\n", entry->check_sum);
    printf("offset %d\n", entry->offset);
    printf("length %d\n", entry->length);
}

void
byte_reverse(void *ptr, int len){
    char *c;
    int i,j;
    char t;
    c = (char*)ptr;
    for (i = 0, j = len-1; i < j; ++i, --j){
        t = c[i];
        c[i] = c[j];
        c[j] = t;
    }
}

void
byte_fix(Offset_Table *offset){
    byte_reverse(&offset->version, 4);
    byte_reverse(&offset->num_tables, 2);
    byte_reverse(&offset->search_range, 2);
    byte_reverse(&offset->entry_selector, 2);
    byte_reverse(&offset->range_shift, 2);
}

void
byte_fix(Table_Directory_Entry *entry){
    byte_reverse(&entry->check_sum, 4);
    byte_reverse(&entry->offset, 4);
    byte_reverse(&entry->length, 4);
}

struct cmap_Header{
    unsigned short version;
    unsigned short num_subtables;
};

struct cmap_Subtable_Entry{
    unsigned short plat_id;
    unsigned short plat_encoding_id;
    unsigned long offset_from_cmap;
};

void
byte_fix(cmap_Header *header){
    byte_reverse(&header->version, 2);
    byte_reverse(&header->num_subtables, 2);
}

void
print(cmap_Header *header){
    printf("cmap tables:\n");
    printf("\tversion %d\n", (int)(header->version));
    printf("\tsubtables %d\n", (int)(header->num_subtables));
}

void
byte_fix(cmap_Subtable_Entry *entry){
    byte_reverse(&entry->plat_id, 2);
    byte_reverse(&entry->plat_encoding_id, 2);
    byte_reverse(&entry->offset_from_cmap, 4);
}

struct cmap_Subtable_Format4_Header{
    unsigned short format;
    unsigned short length;
    unsigned short version;
    unsigned short segment_count_2;
    unsigned short search_range;
    unsigned short entry_selector;
    unsigned short range_shift;
};

void
print(cmap_Subtable_Entry *entry){
    printf("\tplatform id %d\n", (int)(entry->plat_id));
    printf("\tencoding id %d\n", (int)(entry->plat_encoding_id));
    printf("\toffset from cmap %d\n", (int)(entry->offset_from_cmap));
}

void
byte_fix(cmap_Subtable_Format4_Header *header){
    byte_reverse(&header->length, 2);
    byte_reverse(&header->version, 2);
    byte_reverse(&header->segment_count_2, 2);
    byte_reverse(&header->search_range, 2);
    byte_reverse(&header->entry_selector, 2);
    byte_reverse(&header->range_shift, 2);
}

void
print(cmap_Subtable_Format4_Header *header){
    printf("\t\tlength %d\n", header->length);
    printf("\t\tversion %d\n", header->version);
    printf("\t\tsegment count doubled %d\n", header->segment_count_2);
    printf("\t\tsearch range %d\n", header->search_range);
    printf("\t\tentry selector %d\n", header->entry_selector);
    printf("\t\trange shift %d\n", header->range_shift);
}

struct cmap_Subtable_Format4_Segments{
    unsigned short *end_code, *start_code;
    unsigned short *id_delta, *id_range_offset;
};

void
byte_fix(cmap_Subtable_Format4_Segments segs, int segment_count){
    for (int i = 0; i < segment_count; ++i){
        byte_reverse(segs.end_code + i, 2);
    }
    
    for (int i = 0; i < segment_count; ++i){
        byte_reverse(segs.start_code + i, 2);
    }
    
    for (int i = 0; i < segment_count; ++i){
        byte_reverse(segs.id_delta + i, 2);
    }
    
    for (int i = 0; i < segment_count; ++i){
        byte_reverse(segs.id_range_offset + i, 2);
    }
}

void
print(cmap_Subtable_Format4_Segments segs, int i){
    printf("\t\tsegment %d\n", i);
    printf("\t\tend code %d\n", (int)(segs.end_code[i]));
    printf("\t\tstart code %d\n", (int)(segs.start_code[i]));
    printf("\t\tid delta %d\n", (int)(segs.id_delta[i]));
    printf("\t\tid range offset %d\n", (int)(segs.id_range_offset[i]));
}

void
parse_cmap_subtable4(char *start){
    char *cursor = start;
    cmap_Subtable_Format4_Header *header =
        (cmap_Subtable_Format4_Header*)cursor;
    cursor = (char*)(header + 1);
    
    byte_fix(header);
    print(header);

    int segment_count = (header->segment_count_2 >> 1);

    cmap_Subtable_Format4_Segments segs;
    
    segs.end_code = (unsigned short*)cursor;
    cursor = (char*)(segs.end_code + segment_count);
    cursor = cursor + sizeof(unsigned short);
    
    segs.start_code = (unsigned short*)cursor;
    cursor = (char*)(segs.start_code + segment_count);

    segs.id_delta = (unsigned short*)cursor;
    cursor = (char*)(segs.id_delta + segment_count);

    segs.id_range_offset = (unsigned short*)cursor;
    cursor = (char*)(segs.id_range_offset + segment_count);

    byte_fix(segs, segment_count);
    for (int i = 0; i < segment_count; ++i){
        printf("\n");
        print(segs, i);
    }
}

void
parse_cmap_subtable(char *start){
    char *cursor = start;
    short *format = (short*)cursor;
    byte_reverse(format, 2);
    printf("\t\tformat %d\n", (int)(*format));

    switch (*format){
    case 4:
        parse_cmap_subtable4(start);
        break;
    }
}

void
parse_cmap(char *start){
    char *cursor = start;
    cmap_Header *header = (cmap_Header*)cursor;
    cursor = (char*)(header + 1);

    byte_fix(header);
    print(header);

    cmap_Subtable_Entry *entry = (cmap_Subtable_Entry*)cursor;
    for (int i = 0; i < header->num_subtables; ++i, ++entry){
        byte_fix(entry);
        printf("\n");
        print(entry);

        if (entry->plat_id == 3 && entry->plat_encoding_id == 1){
            printf("\n\tMicrosoft Unicode table:\n");
            parse_cmap_subtable(start + entry->offset_from_cmap);
        }
    }
}

struct glyf_Glyph_Header{
    short num_contours;
    short x_min;
    short x_max;
    short y_min;
    short y_max;
};

void
byte_fix(glyf_Glyph_Header *header){
    byte_reverse(&header->num_contours, 2);
    byte_reverse(&header->x_min, 2);
    byte_reverse(&header->x_max, 2);
    byte_reverse(&header->y_min, 2);
    byte_reverse(&header->y_max, 2);
}

void
print(glyf_Glyph_Header *header){
    printf("\tcontours %d\n", (int)(header->num_contours));
    printf("\tx min %d\n", (int)(header->x_min));
    printf("\tx max %d\n", (int)(header->x_max));
    printf("\ty min %d\n", (int)(header->y_min));
    printf("\ty max %d\n", (int)(header->y_max));
}

void
parse_glyf(char *start){
    char *cursor = start;
    glyf_Glyph_Header *header = (glyf_Glyph_Header*)cursor;
    cursor = (char*)(header + 1);

    byte_fix(header);
    print(header);
}

#define TTF_Tag(a,b,c,d) ((a) + ((b) << 8) + ((c) << 16) + ((d) << 24))
#define TTF_Tag_cmap TTF_Tag('c', 'm', 'a', 'p')

#define TTF_Tag_glyf TTF_Tag('g', 'l', 'y', 'f')
#define TTF_Tag_fpgm TTF_Tag('f', 'p', 'g', 'm')
#define TTF_Tag_prep TTF_Tag('p', 'r', 'e', 'p')
#define TTF_Tag_cvt TTF_Tag('c', 'v', 't', ' ')
#define TTF_Tag_maxp TTF_Tag('m', 'a', 'x', 'p')


int
main(){
    Data file;
    char *filename;

    filename = "test.ttf";
    file = open_file(filename);
    if (file.data == 0){
        printf("could not open %s\n", filename);
        return (1);
    }

    char *cursor;
    Offset_Table *offset;
    
    cursor = file.data;
    offset = (Offset_Table*)cursor;
    cursor = (char*)(offset + 1);


    Table_Directory_Entry *directory_entries = (Table_Directory_Entry*)cursor;

    byte_fix(offset);
    print(offset);
    
    int table_number = offset->num_tables;

    Table_Directory_Entry *entry = directory_entries;
    for (int i = 0; i < table_number; ++i, ++entry){
        printf("\n");
        byte_fix(entry);
        print(entry);

        switch (entry->tag){
        case TTF_Tag_cmap:
            parse_cmap(file.data + entry->offset);
            break;

        case TTF_Tag_glyf:
            parse_glyf(file.data + entry->offset);
            break;
        }
    }
    
    return (0);
}

// BOTTOM
