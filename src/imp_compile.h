#ifndef X__IMP16_COMPILE_H__X
#define X__IMP16_COMPILE_H__X
#include <stdint.h>
#include "imp_parse.h"

typedef struct {
        uint8_t *contents;
        uint32_t length;
        uint32_t allocated;
} IMP_BYTE_BUFFER;

void imp_byte_buffer_create(IMP_BYTE_BUFFER *ret);
void imp_byte_buffer_append(IMP_BYTE_BUFFER *ret, const uint8_t *string, uint32_t length);
void imp_byte_buffer_append_string(IMP_BYTE_BUFFER *ret, const char *string);
void imp_byte_buffer_append_format(IMP_BYTE_BUFFER *ret, size_t max_added_len, const char *string, ...);
void imp_byte_buffer_destroy(IMP_BYTE_BUFFER *ret);

typedef uint64_t IMP_NUMBER;
typedef struct {
        IMP_NUMBER *contents;
        uint32_t length;
} IMP_NUMBER_ARRAY;

typedef struct {
        union {
                IMP_NUMBER  immediate;
                char       *name;
        } content;
        bool named;
} IMP_ADDRESS;

typedef struct {
        IMP_REGISTER_PREFIX      prefix;
        IMP_REGISTER_KIND        kind;
        char                    *name;
} IMP_BINDING;
typedef struct {
        IMP_BINDING *contents;
        uint32_t length;
} IMP_BINDING_ARRAY;

typedef struct {
        IMP_NUMBER_ARRAY  prefix;
        IMP_NUMBER_ARRAY  suffix;
        IMP_BINDING_ARRAY bindings;
        IMP_ADDRESS       entry;
        IMP_NUMBER        origin;
        IMP_NUMBER        size;
        uint16_t          nmspc;
        bool              has_prefix;
        bool              has_suffix;
} IMP_SEGMENT;
typedef struct {
        IMP_SEGMENT *contents;
        uint32_t length;
} IMP_SEGMENT_ARRAY;

void imp_segment_create(IMP_SEGMENT *segment);
void imp_segment_set_origin(IMP_SEGMENT *segment, IMP_NUMBER immediate_address);
void imp_segment_set_size(IMP_SEGMENT *segment, IMP_NUMBER size);
void imp_segment_prefix_append(IMP_SEGMENT *segment, IMP_NUMBER number);
void imp_segment_suffix_append(IMP_SEGMENT *segment, IMP_NUMBER number);
void imp_segment_set_entry(IMP_SEGMENT *segment, const IMP_ADDRESS *address);
void imp_segment_add_binding(IMP_SEGMENT *segment, IMP_REGISTER_PREFIX prefix, IMP_REGISTER_KIND kind, const char *name);
void imp_segment_destroy(IMP_SEGMENT *segment);

typedef struct {
        const IMP_NODE         *ast;
        const IMP_NODE         *current;
        IMP_BYTE_BUFFER        *ret;
        IMP_SEGMENT_ARRAY       segments;
} IMP_COMPILER_STATE;

void imp_compiler_state_create(IMP_COMPILER_STATE *ret, const IMP_NODE *ast, IMP_BYTE_BUFFER *product);
void imp_compiler_state_destroy(IMP_COMPILER_STATE *ret);

void imp_compile_program(IMP_BYTE_BUFFER *ret, const IMP_NODE *ast);
bool imp_compile_segment(IMP_COMPILER_STATE *state);
bool imp_compile_segment_parameter(IMP_COMPILER_STATE *state);
bool imp_compile_segment_block(IMP_COMPILER_STATE *state);
bool imp_compile_label(IMP_COMPILER_STATE *state);
bool imp_compile_block(IMP_COMPILER_STATE *state);
bool imp_compile_bind(IMP_COMPILER_STATE *state);
bool imp_compile_interrupt(IMP_COMPILER_STATE *state);
bool imp_compile_load(IMP_COMPILER_STATE *state);
bool imp_compile_halt(IMP_COMPILER_STATE *state);

const char *get_register_string(IMP_REGISTER_PREFIX prefix, IMP_REGISTER_KIND kind);

#endif
