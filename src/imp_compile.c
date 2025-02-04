#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include "imp_parse.h"
#include "imp_compile.h"

void imp_byte_buffer_create(IMP_BYTE_BUFFER *ret) {
        ret->contents  = malloc(1);
        ret->length    = 0;
        ret->allocated = 2;
}
void imp_byte_buffer_append(IMP_BYTE_BUFFER *ret, const uint8_t *string, uint32_t length) {
        ret->length += length;
        if(ret->length > ret->allocated) {
                while(ret->length > ret->allocated) ret->allocated += ret->allocated >> 1;
                ret->contents = realloc(ret->contents, ret->allocated * sizeof(*ret->contents));
        }
        memcpy(ret->contents + ret->length - length, string, length);
}
void imp_byte_buffer_append_string(IMP_BYTE_BUFFER *ret, const char *string) {
        imp_byte_buffer_append(ret, (const uint8_t *)string, strlen(string));
}
void imp_byte_buffer_append_format(IMP_BYTE_BUFFER *ret, size_t max_added_len, const char *format, ...) {
        va_list args;
        char *rstring;
        va_start(args, format);
        rstring = malloc(strlen(format)+max_added_len+1);
        vsnprintf(rstring, strlen(format)+max_added_len, format, args);
        imp_byte_buffer_append_string(ret, rstring);
        free(rstring);
        va_end(args);
}
void imp_byte_buffer_destroy(IMP_BYTE_BUFFER *ret) {
        free(ret->contents);
        memset(ret, 0, sizeof(*ret));
}

void imp_segment_create(IMP_SEGMENT *ret) {
        memset(ret, 0, sizeof(*ret));
        ret->bindings.contents = malloc(1);
        ret->bindings.length   = 0;
        ret->suffix.contents   = malloc(1);
        ret->suffix.length     = 0;
        ret->prefix.contents   = malloc(1);
        ret->prefix.length     = 0;
}
void imp_segment_set_origin(IMP_SEGMENT *ret, IMP_NUMBER immediate_address) {
        ret->origin = immediate_address;
}
void imp_segment_set_namespace(IMP_SEGMENT *ret, IMP_NUMBER immediate_address) {
        ret->nmspc = immediate_address;
}
void imp_segment_set_prepend_entry(IMP_SEGMENT *ret) {
        ret->prepend_entry = true;
}
void imp_segment_set_size(IMP_SEGMENT *ret, IMP_NUMBER size) {
        ret->size = size;
}
void imp_segment_prefix_append(IMP_SEGMENT *ret, IMP_NUMBER number) {
        ret->prefix.contents = realloc(ret->prefix.contents, ++ret->prefix.length * sizeof(*ret->prefix.contents));
        ret->prefix.contents[ret->prefix.length - 1] = number;
}
void imp_segment_suffix_append(IMP_SEGMENT *ret, IMP_NUMBER number) {
        ret->suffix.contents = realloc(ret->suffix.contents, ++ret->suffix.length * sizeof(*ret->suffix.contents));
        ret->suffix.contents[ret->suffix.length - 1] = number;
}
void imp_segment_set_entry(IMP_SEGMENT *ret, const IMP_ADDRESS *address) {
        ret->entry.named = address->named;
        if(address->named) {
                ret->entry.content.name = malloc(strlen(address->content.name)+1);
                strcpy(ret->entry.content.name, address->content.name);
        } else ret->entry.content.immediate = address->content.immediate;
}
void imp_segment_add_binding(IMP_SEGMENT *ret, IMP_REGISTER_PREFIX prefix, IMP_REGISTER_KIND kind, const char *name) {
        ret->bindings.contents = realloc(ret->bindings.contents, ++ret->bindings.length * sizeof(*ret->bindings.contents));
        ret->bindings.contents[ret->bindings.length - 1].prefix = prefix;
        ret->bindings.contents[ret->bindings.length - 1].kind   = kind; 
        ret->bindings.contents[ret->bindings.length - 1].name   = malloc(strlen(name)+1);
        strcpy(ret->bindings.contents[ret->bindings.length - 1].name, name);
}
void imp_segment_destroy(IMP_SEGMENT *ret) {
        uint32_t i;
        free(ret->prefix.contents);
        free(ret->suffix.contents);
        for(i = 0; i < ret->bindings.length; i++)
                free(ret->bindings.contents[i].name);
        free(ret->bindings.contents);
        if(ret->entry.named) free(ret->entry.content.name);
        memset(ret, 0, sizeof(*ret));
}

void imp_compiler_state_create(IMP_COMPILER_STATE *ret, const IMP_NODE *ast, IMP_BYTE_BUFFER *product) {
        ret->ast = ast;
        ret->current = ast;
        ret->ret = product;
        ret->segments.contents = malloc(1);
        ret->segments.length   = 0;
}
void imp_state_segment_create(IMP_COMPILER_STATE *ret) {
        ret->segments.contents = realloc(ret->segments.contents, ++ret->segments.length * sizeof(*ret->segments.contents));
        imp_segment_create(ret->segments.contents + ret->segments.length - 1);
}
void imp_compiler_state_destroy(IMP_COMPILER_STATE *ret) {
        uint32_t i;
        for(i = 0; i < ret->segments.length; i++)
                imp_segment_destroy(ret->segments.contents+i);
        free(ret->segments.contents);
        memset(ret, 0, sizeof(*ret));
}

void imp_compile_program(IMP_BYTE_BUFFER *ret, const IMP_NODE *ast) {
        IMP_COMPILER_STATE state;
        uint32_t i;
        imp_byte_buffer_create(ret);
        imp_byte_buffer_append_string(ret, "use16\norg 0x00\n");
        imp_compiler_state_create(&state, ast, ret);
        for(i = 0; i < ast->children_length; i++) {
                state.current = ast->children+i;
                imp_compile_segment(&state);
        }
        imp_compiler_state_destroy(&state);
}
bool imp_compile_segment(IMP_COMPILER_STATE *state) {
        uint32_t i;
        IMP_SEGMENT *current_segment;
        if(state->current->type != IMP_NODE_TYPE_SEGMENT) return false;
        imp_state_segment_create(state);
        current_segment = state->segments.contents + state->segments.length - 1;
        for(i = 0; state->current->children[i].type != IMP_NODE_TYPE_SEGMENT_BLOCK; i++) {
                state->current = state->current->children + i;
                imp_compile_segment_parameter(state);
                state->current = state->current->parent;
        }
        state->current = state->current->children + i;
        imp_byte_buffer_append_format(
                state->ret,
                8,
                "org 0x%08X\n",
                current_segment->origin
        );
        if(current_segment->prepend_entry) {
                if(current_segment->entry.named) {
                        imp_byte_buffer_append_format(
                                state->ret,
                                strlen(current_segment->entry.content.name)+4,
                                "jmp near %s@%04X\n",
                                current_segment->entry.content.name,
                                current_segment->nmspc
                        );
                } else imp_byte_buffer_append_format(state->ret, 8, "jmp near 0x%08x\n", current_segment->entry.content.immediate);
        }
        if(current_segment->prefix.length) {
                imp_byte_buffer_append_format(state->ret, 2, "db 0x%02X", current_segment->prefix.contents[0]);
                for(i = 1; i < current_segment->prefix.length; i++)
                        imp_byte_buffer_append_format(state->ret, 2, ", 0x%02X", current_segment->prefix.contents[i]);
                imp_byte_buffer_append_string(state->ret, "\n");
        }
        if(!current_segment->prepend_entry) {
                if(current_segment->entry.named) {
                        imp_byte_buffer_append_format(
                                state->ret,
                                strlen(current_segment->entry.content.name)+4,
                                "jmp near %s@%04X\n",
                                current_segment->entry.content.name,
                                current_segment->nmspc
                        );
                } else imp_byte_buffer_append_format(state->ret, 8, "jmp near 0x%08x\n", current_segment->entry.content.immediate);
        }
        
        imp_compile_segment_block(state);
        imp_byte_buffer_append_format(
                state->ret,
                8,
                "times 0x%08X - $ + $$ db 0x00\n",
                current_segment->size-current_segment->suffix.length
        );
        if(current_segment->suffix.length) {
                imp_byte_buffer_append_format(state->ret, 2, "db 0x%02X", current_segment->suffix.contents[0]);
                for(i = 1; i < current_segment->suffix.length; i++)
                        imp_byte_buffer_append_format(state->ret, 2, ", 0x%02X", current_segment->suffix.contents[i]);
                imp_byte_buffer_append_string(state->ret, "\n");
        }
        state->current = state->current->parent;
        return true;
}
bool imp_compile_segment_parameter(IMP_COMPILER_STATE *state) {
        uint32_t i;
        IMP_ADDRESS addr;
        IMP_SEGMENT *current_segment;
        current_segment = state->segments.contents + state->segments.length - 1;
        switch(state->current->type) {
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_AT:     imp_segment_set_origin(current_segment, state->current->value.number); return true;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ID:     imp_segment_set_namespace(current_segment, state->current->value.number); return true;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_SIZEOF: imp_segment_set_size(current_segment, state->current->value.number);   return true;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_PREPEND_ENTRY: imp_segment_set_prepend_entry(current_segment);   return true;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_PREFIX:
                for(i = 0; i < state->current->value.number_array.length; i++)
                        imp_segment_prefix_append(current_segment, state->current->value.number_array.contents[i]);
                return true;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_SUFFIX:
                for(i = 0; i < state->current->value.number_array.length; i++)
                        imp_segment_suffix_append(current_segment, state->current->value.number_array.contents[i]);
                return true;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_LITERAL:
                addr.named = false;
                addr.content.immediate = state->current->value.number;
                imp_segment_set_entry(current_segment, &addr);
                return true;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_NAMED:
                addr.named = true;
                addr.content.name = malloc(strlen(state->current->value.string)+1);
                strcpy(addr.content.name, state->current->value.string);
                imp_segment_set_entry(current_segment, &addr);
                free(addr.content.name);
                return true;
        default: return false;
        }
}
bool imp_compile_segment_block(IMP_COMPILER_STATE *state) {
        bool (*functions[])(IMP_COMPILER_STATE *) = {imp_compile_label};
        bool has_run;
        uint32_t i, j;
        if(state->current->type != IMP_NODE_TYPE_SEGMENT_BLOCK) return false;
        for(i = 0; i < state->current->children_length; i++) {
                state->current = state->current->children+i;
                has_run = false;
                for(j = 0; j < sizeof(functions) / sizeof(functions[0]); j++) 
                        if(has_run = functions[j](state)) break;
                state->current = state->current->parent;
                if(!has_run) return false;
        }
        return true;
}
bool imp_compile_label(IMP_COMPILER_STATE *state) {
        if(state->current->type != IMP_NODE_TYPE_LABEL) return false;
        imp_byte_buffer_append_format(
                state->ret,
                strlen(state->current->value.string),
                "%s@%04X:\n",
                state->current->value.string,
                state->segments.contents[state->segments.length - 1].nmspc
        );
        state->current = state->current->children;
        imp_compile_block(state);
        state->current = state->current->parent;
        return true;
}
bool imp_compile_block(IMP_COMPILER_STATE *state) {
        bool (*functions[])(IMP_COMPILER_STATE *) = {imp_compile_bind, imp_compile_interrupt, imp_compile_load, imp_compile_halt};
        bool has_run;
        uint32_t i, j;
        if(state->current->type != IMP_NODE_TYPE_BLOCK) return false;
        for(i = 0; i < state->current->children_length; i++) {
                state->current = state->current->children+i;
                has_run = false;
                for(j = 0; j < sizeof(functions) / sizeof(functions[0]); j++) 
                        if(has_run = functions[j](state)) break;
                state->current = state->current->parent;
                if(!has_run) return false;
        }
        return true;
}
bool imp_compile_bind(IMP_COMPILER_STATE *state) {
        if(state->current->type != IMP_NODE_TYPE_BIND) return false;
        imp_segment_add_binding(
                state->segments.contents + state->segments.length - 1,
                state->current->children[1].value.reg.prefix,
                state->current->children[1].value.reg.kind,
                state->current->children[0].value.string
        );
        return true;
}
bool imp_compile_interrupt(IMP_COMPILER_STATE *state) {
        if(state->current->type != IMP_NODE_TYPE_INTERRUPT) return false;
        imp_byte_buffer_append_format(
                state->ret,
                2,
                "int 0x%02X\n",
                state->current->value.number
        );
        return true;
}
bool imp_compile_load(IMP_COMPILER_STATE *state) {
        uint32_t i;
        IMP_SEGMENT *current_segment;
        IMP_BINDING *binding;
        if(state->current->type != IMP_NODE_TYPE_LOAD) return false;
        current_segment = state->segments.contents + state->segments.length - 1;
        for(i = 0; i < current_segment->bindings.length; i++)
                if(!strcmp(current_segment->bindings.contents[i].name, state->current->value.string)) break;
        binding = current_segment->bindings.contents + i;
        if(binding->prefix == IMP_REGISTER_PREFIX_SEG) {
                imp_byte_buffer_append_format(
                        state->ret,
                        8 + strlen(get_register_string(binding->prefix, binding->kind)),
                        "push 0x%08X\n",
                        "pop %s\n",
                        state->current->children[0].value.number,
                        get_register_string(binding->prefix, binding->kind)
                );
                return true;
        }
        imp_byte_buffer_append_format(
                state->ret,
                8 + strlen(get_register_string(binding->prefix, binding->kind)),
                "mov %s, 0x%08X\n",
                get_register_string(binding->prefix, binding->kind),
                state->current->children[0].value.number
        );
        return true;
}
bool imp_compile_halt(IMP_COMPILER_STATE *state) {
        if(state->current->type != IMP_NODE_TYPE_HALT) return false;
        imp_byte_buffer_append_string(
                state->ret,
                "jmp $\n"
        );
        return true;
}

const char *get_register_string(IMP_REGISTER_PREFIX prefix, IMP_REGISTER_KIND kind) {
        /* Register Kinds
        * low   accumulator     --> al
        * low   pointer         --> bl
        * low   counter         --> cl
        * low   general         --> dl
        * high  accumulator     --> ah
        * high  pointer         --> bh
        * high  counter         --> ch
        * high  general         --> dh
        * seg   extra           --> es
        * seg   data            --> ds
        * seg   stack           --> ss
        *       accumulator     --> ax
        *       pointer         --> bx
        *       counter         --> cx
        *       general         --> dx
        *       source          --> si
        *       destination     --> di
        *       stack           --> sp
        *       base            --> bp
        */
        switch(prefix | kind) {
        case IMP_REGISTER_PREFIX_LOW  | IMP_REGISTER_KIND_ACCUMULATOR:  return "al";
        case IMP_REGISTER_PREFIX_LOW  | IMP_REGISTER_KIND_POINTER:      return "bl";
        case IMP_REGISTER_PREFIX_LOW  | IMP_REGISTER_KIND_COUNTER:      return "cl";
        case IMP_REGISTER_PREFIX_LOW  | IMP_REGISTER_KIND_GENERAL:      return "dl";
        case IMP_REGISTER_PREFIX_HIGH | IMP_REGISTER_KIND_ACCUMULATOR:  return "ah";
        case IMP_REGISTER_PREFIX_HIGH | IMP_REGISTER_KIND_POINTER:      return "bh";
        case IMP_REGISTER_PREFIX_HIGH | IMP_REGISTER_KIND_COUNTER:      return "ch";
        case IMP_REGISTER_PREFIX_HIGH | IMP_REGISTER_KIND_GENERAL:      return "dh";
        case IMP_REGISTER_PREFIX_SEG  | IMP_REGISTER_KIND_EXTRA:        return "es";
        case IMP_REGISTER_PREFIX_SEG  | IMP_REGISTER_KIND_DATA:         return "ds";
        case IMP_REGISTER_PREFIX_SEG  | IMP_REGISTER_KIND_STACK:        return "ss";
        case IMP_REGISTER_PREFIX_NULL | IMP_REGISTER_KIND_ACCUMULATOR:  return "ax";
        case IMP_REGISTER_PREFIX_NULL | IMP_REGISTER_KIND_POINTER:      return "bx";
        case IMP_REGISTER_PREFIX_NULL | IMP_REGISTER_KIND_COUNTER:      return "cx";
        case IMP_REGISTER_PREFIX_NULL | IMP_REGISTER_KIND_GENERAL:      return "dx";
        case IMP_REGISTER_PREFIX_NULL | IMP_REGISTER_KIND_SOURCE:       return "si";
        case IMP_REGISTER_PREFIX_NULL | IMP_REGISTER_KIND_DESTINATION:  return "di";
        case IMP_REGISTER_PREFIX_NULL | IMP_REGISTER_KIND_STACK:        return "sp";
        case IMP_REGISTER_PREFIX_NULL | IMP_REGISTER_KIND_BASE:         return "bp";
        default: return "---";
        }
}
