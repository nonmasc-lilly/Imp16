#include <stdio.h>
#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "imp_lex.h"
#include "imp_parse.h"


IMP_NODE *imp_node_create(IMP_NODE *parent, IMP_NODE_TYPE type, IMP_NODE_VALUE *value) {
        IMP_NODE *ret;
        if(parent) {
                parent->children = realloc(parent->children, ++parent->children_length * sizeof(*parent->children));
                ret = parent->children + parent->children_length - 1; 
                memset(ret, 0, sizeof(*ret));
        } else ret = calloc(1,sizeof(*ret));
        ret->type               = type;
        ret->parent             = parent;
        ret->children           = malloc(1);
        ret->children_length    = 0;
        if(value) switch(type) {
        case IMP_NODE_TYPE_REGISTER:
        case IMP_NODE_TYPE_IMMEDIATE:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_AT:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_SIZEOF:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_LITERAL:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ID:
        case IMP_NODE_TYPE_INTERRUPT:
                memcpy(&ret->value, value, sizeof(*value));
                break;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_PREFIX:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_SUFFIX:
        case IMP_NODE_TYPE_IMMEDIATE_LIST:
                ret->value.number_array.length   = value->number_array.length;
                ret->value.number_array.contents = malloc(value->number_array.length * sizeof(*value->number_array.contents));
                memcpy(
                        ret->value.number_array.contents,
                        value->number_array.contents,
                        value->number_array.length * sizeof(*ret->value.number_array.contents)
                );
                break;
        case IMP_NODE_TYPE_NAME:
        case IMP_NODE_TYPE_LABEL:
        case IMP_NODE_TYPE_LOAD:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_NAMED:
                ret->value.string = malloc(strlen(value->string)+1);
                strcpy(ret->value.string, value->string);
                break;
        }
        return ret;
}
void imp_node_destroy(IMP_NODE *ret) {
        uint32_t i;
        for(i = 0; i < ret->children_length; i++) imp_node_destroy(ret->children+i);
        free(ret->children);
        switch(ret->type) {
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_PREFIX:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_SUFFIX:
        case IMP_NODE_TYPE_IMMEDIATE_LIST:
                free(ret->value.number_array.contents);
                break;
        case IMP_NODE_TYPE_NAME:
        case IMP_NODE_TYPE_LABEL:
        case IMP_NODE_TYPE_LOAD:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_NAMED:
                free(ret->value.string);
                break;
        }
        if(ret->parent == NULL) free(ret);
}
void imp_node_print(const IMP_NODE *tree, uint32_t index) {
        uint32_t i;
        static const char *node_type[IMP_NODE_TYPE__MAX] = {
                "NULL",                 "program",                      "segment",
                "segment .at",          "segment .sizeof",              "segment .prefix",
                "segment .suffix",      "segment .entry (immediate)",   "segment .entry (string)",
                "immediate list",       "segment block",                "primary statement",
                "label",                "block",                        "statement",
                "bind",                 "register",                     "interrupt",
                "load",                 "halt",                         "immediate",
                "name",                 "segment .prepend_entry",       "segment .id"
        };
        fprintf(stderr, "\033[0;90m");
        for(i = 0; i < index; i++) fprintf(stderr, "|");
        fprintf(stderr, "> [children: $%04X] %s", tree->children_length, node_type[tree->type]);
        switch(tree->type) {
        case IMP_NODE_TYPE_REGISTER:
                switch(tree->value.reg.prefix) {
                case IMP_REGISTER_PREFIX_HIGH:          fprintf(stderr, "[ high : ");   break;
                case IMP_REGISTER_PREFIX_LOW:           fprintf(stderr, "[ low : ");    break;
                case IMP_REGISTER_PREFIX_SEG:           fprintf(stderr, "[ seg : ");    break;
                case IMP_REGISTER_PREFIX_NULL:          fprintf(stderr, "[  : ");       break;
                }
                switch(tree->value.reg.kind) {
                case IMP_REGISTER_KIND_ACCUMULATOR:     fprintf(stderr, "accumulator ]\n");     break;
                case IMP_REGISTER_KIND_POINTER:         fprintf(stderr, "pointer ]\n");         break;
                case IMP_REGISTER_KIND_COUNTER:         fprintf(stderr, "counter ]\n");         break;
                case IMP_REGISTER_KIND_GENERAL:         fprintf(stderr, "general ]\n");         break;
                case IMP_REGISTER_KIND_SOURCE:          fprintf(stderr, "source ]\n");          break;
                case IMP_REGISTER_KIND_DESTINATION:     fprintf(stderr, "destination ]\n");     break;
                case IMP_REGISTER_KIND_STACK:           fprintf(stderr, "stack ]\n");           break;
                case IMP_REGISTER_KIND_BASE:            fprintf(stderr, "base ]\n");            break;
                case IMP_REGISTER_KIND_EXTRA:           fprintf(stderr, "extra ]\n");           break;
                case IMP_REGISTER_KIND_DATA:            fprintf(stderr, "data ]\n");            break;
                }
                break;
        case IMP_NODE_TYPE_IMMEDIATE:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ID:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_AT:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_SIZEOF:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_LITERAL:
        case IMP_NODE_TYPE_INTERRUPT: fprintf(stderr, " := %08X\n", tree->value.number); break;
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_SUFFIX:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_PREFIX:
        case IMP_NODE_TYPE_IMMEDIATE_LIST:
                fprintf(stderr, " := <");
                for(i = 0; i < tree->value.number_array.length; i++)
                        fprintf(stderr, " %08X", tree->value.number_array.contents[i]);
                fprintf(stderr, " >\n");
                break;
        case IMP_NODE_TYPE_NAME:
        case IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_NAMED:
        case IMP_NODE_TYPE_LOAD:
        case IMP_NODE_TYPE_LABEL: fprintf(stderr, " := %s\n", tree->value.string); break;
        default: fprintf(stderr, "\n"); break;
        }
        fprintf(stderr, "\033[m");
        for(i = 0; i < tree->children_length; i++)
                imp_node_print(tree->children+i, index+1);
}

void imp_parse_state_create(IMP_PARSE_STATE *ret, const IMP_TOKENS *tokens, IMP_NODE *product) {
        memset(ret, 0, sizeof(*ret));
        ret->tokens     = tokens;
        ret->ret        = product;
        ret->current    = product;
}
void imp_parse_state_destroy(IMP_PARSE_STATE *ret) {
        memset(ret, 0, sizeof(*ret));
}

void imp_assert(bool condition, bool error, uint32_t line, const char *message, ...) {
        va_list args;
        if(condition) return;
        va_start(args, message);
        if(error) {
                fprintf(stderr, "\033[1;31mError (\033[0;31mon line %u)\033[1;31m:\033[m ", line);
                vfprintf(stderr, message, args);
                fprintf(stderr, "\n");
                va_end(args);
                exit(1);
        }
        fprintf(stderr, "\033[1;33mWarning (\033[0;33mon line %u)\033[1;31m:\033[m ", line);
        vfprintf(stderr, message, args);
        fprintf(stderr, "\n");
        va_end(args);
}


static IMP_TOKEN_TYPE get_token_type(IMP_PARSE_STATE *state) {
        return state->tokens->types[state->tindex];
}
static uint32_t get_token_line(IMP_PARSE_STATE *state) {
        return state->tokens->lines[state->tindex];
}
static uint64_t get_token_numeric_value(IMP_PARSE_STATE *state) {
        return state->tokens->values[state->tindex].number;
}
static const char *get_token_string_value(IMP_PARSE_STATE *state) {
        return state->tokens->values[state->tindex].string;
}
static void consume_token(IMP_PARSE_STATE *state) {
        state->tindex++;
}


void imp_parse_program(IMP_NODE **ret, const IMP_TOKENS *tokens) {
        IMP_PARSE_STATE state;
        *ret = imp_node_create(NULL, IMP_NODE_TYPE_PROGRAM, NULL);
        imp_parse_state_create(&state, tokens, *ret);
        while(state.tindex < tokens->length) {
                imp_assert(imp_parse_segment(&state), true, get_token_line(&state), "Non-segment statement outside of segment.");
        }
}
bool imp_parse_segment(IMP_PARSE_STATE *state) {
        if(get_token_type(state) != IMP_TOKEN_TYPE_SEGMENT) return false;
        consume_token(state);
        state->current = imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT, NULL);
        while(imp_parse_segment_parameter(state));
        imp_assert(imp_parse_segment_block(state), true, get_token_line(state), "Expected segment block after `segment`.");
        state->current = state->current->parent;
        return true;
}
bool imp_parse_segment_parameter(IMP_PARSE_STATE *state) {
        IMP_NODE_VALUE value;
        switch(get_token_type(state)) {
        case IMP_TOKEN_TYPE_DAT:
                consume_token(state);
                imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE, true, get_token_line(state), "Expected immediate after `.at`.");
                value.number = get_token_numeric_value(state);
                imp_assert(value.number < 0x100000, false, get_token_line(state), "Immediate of .at is larger than 20 bits.");
                imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_PARAMETER_AT, &value);
                consume_token(state);
                return true;
        case IMP_TOKEN_TYPE_DSIZEOF:
                consume_token(state);
                imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE, true, get_token_line(state), "Expected immediate after `.sizeof`.");
                value.number = get_token_numeric_value(state);
                imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_PARAMETER_SIZEOF, &value);
                consume_token(state);
                return true;
        case IMP_TOKEN_TYPE_DPREFIX:
                consume_token(state);
                imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE, true, get_token_line(state), "Expected immediates after `.prefix`.");
                value.number_array.contents = malloc(1);
                value.number_array.length   = 0;
                while(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE) {
                        value.number_array.contents = realloc(
                                value.number_array.contents,
                                ++value.number_array.length * sizeof(*value.number_array.contents)
                        );
                        value.number_array.contents[value.number_array.length - 1] = get_token_numeric_value(state);
                        imp_assert(get_token_numeric_value(state) < 0x100, false, get_token_line(state), "Immediate byte in .prefix is larger than 8 bits.");
                        consume_token(state);
                }
                imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_PARAMETER_PREFIX, &value);
                free(value.number_array.contents);
                return true;
        case IMP_TOKEN_TYPE_DSUFFIX:
                consume_token(state);
                imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE, true, get_token_line(state), "Expected immediates after `.suffix`.");
                value.number_array.contents = malloc(1);
                value.number_array.length   = 0;
                while(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE) {
                        value.number_array.contents = realloc(
                                value.number_array.contents,
                                ++value.number_array.length * sizeof(*value.number_array.contents)
                        );
                        value.number_array.contents[value.number_array.length - 1] = get_token_numeric_value(state);
                        imp_assert(get_token_numeric_value(state) < 0x100, false, get_token_line(state), "Immediate byte in .suffix is larger than 8 bits.");
                        consume_token(state);
                }
                imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_PARAMETER_SUFFIX, &value);
                free(value.number_array.contents);
                return true;
        case IMP_TOKEN_TYPE_DENTRY:
                consume_token(state);
                imp_assert(
                        get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE || get_token_type(state) == IMP_TOKEN_TYPE_NAME,
                        true,
                        get_token_line(state),
                        "Expected immediate or label after `.entry`."
                );
                if(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE) {
                        value.number = get_token_numeric_value(state);
                        imp_assert(value.number < 0x100000, false, get_token_line(state), "Immediate of .entry is larger than 20 bits.");
                        imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_LITERAL, &value);
                } else if(get_token_type(state) == IMP_TOKEN_TYPE_NAME) {
                        value.string = (char*)get_token_string_value(state);
                        imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_NAMED, &value);
                }
                consume_token(state);
                return true;
        case IMP_TOKEN_TYPE_DPREPEND_ENTRY:
                consume_token(state);
                imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_PARAMETER_PREPEND_ENTRY, NULL);
                return true;
        case IMP_TOKEN_TYPE_DID:
                consume_token(state);
                imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE, true, get_token_line(state), "Expected immediate after `.id`.");
                value.number = get_token_numeric_value(state);
                imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_PARAMETER_ID, &value);
                consume_token(state);
                return true;
        default: return false;
        }
}
bool imp_parse_segment_block(IMP_PARSE_STATE *state) {
        bool (*functions[])(IMP_PARSE_STATE *) = {imp_parse_label};
        bool has_run;
        uint32_t i;
        state->current = imp_node_create(state->current, IMP_NODE_TYPE_SEGMENT_BLOCK, NULL);
        while(get_token_type(state) != IMP_TOKEN_TYPE_SEGEND) {
                for(i = 0; i < sizeof(functions) / sizeof(functions[0]); i++)
                        if(has_run = functions[i](state)) break;
                imp_assert(has_run, get_token_line(state), true, "Expected primary statement in segment block.");
        }
        consume_token(state);
        state->current = state->current->parent;
        return true;
}
bool imp_parse_label(IMP_PARSE_STATE *state) {
        IMP_NODE_VALUE value;
        if(get_token_type(state) != IMP_TOKEN_TYPE_LABEL) return false;
        consume_token(state);
        imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_NAME, true, get_token_line(state), "Expected identifier for label.");
        value.string = (char*)get_token_string_value(state);
        state->current = imp_node_create(state->current, IMP_NODE_TYPE_LABEL, &value);
        consume_token(state);
        imp_assert(imp_parse_block(state), true, get_token_line(state), "Expected block of label.");
        state->current = state->current->parent;
        return true;
}
bool imp_parse_block(IMP_PARSE_STATE *state) {
        bool (*functions[])(IMP_PARSE_STATE *) = {imp_parse_bind, imp_parse_int, imp_parse_load, imp_parse_halt};
        bool has_run;
        uint32_t i;
        state->current = imp_node_create(state->current, IMP_NODE_TYPE_BLOCK, NULL);
        while(get_token_type(state) != IMP_TOKEN_TYPE_END) {
                for(i = 0; i < sizeof(functions) / sizeof(functions[0]); i++)
                        if(has_run = functions[i](state)) break;
                imp_assert(has_run, true, get_token_line(state), "Expected statement in block.");
        }
        consume_token(state);
        state->current = state->current->parent;
        return true;
}
bool imp_parse_bind(IMP_PARSE_STATE *state) {
        IMP_NODE_VALUE value;
        if(get_token_type(state) != IMP_TOKEN_TYPE_BIND) return false;
        state->current = imp_node_create(state->current, IMP_NODE_TYPE_BIND, NULL);
        consume_token(state);
        imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_NAME, true, get_token_line(state), "Expected identifier to bind to.");
        value.string = (char*)get_token_string_value(state);
        imp_node_create(state->current, IMP_NODE_TYPE_NAME, &value);
        consume_token(state);
        imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_COLON, true, get_token_line(state), "Expected punctuator `:` in bind statement.");
        consume_token(state);
        imp_assert(imp_parse_register(state), true, get_token_line(state), "Expected register with which to bind.");
        state->current = state->current->parent;
        return true;
}
bool imp_parse_int(IMP_PARSE_STATE *state) {
        IMP_NODE_VALUE value;
        if(get_token_type(state) != IMP_TOKEN_TYPE_INT) return false;
        consume_token(state);
        imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE, true, get_token_line(state), "Expected interrupt number.");
        value.number = get_token_numeric_value(state);
        imp_assert(value.number < 0x100, false, get_token_line(state), "Immediate of int is larger than 8 bits.");
        imp_node_create(state->current, IMP_NODE_TYPE_INTERRUPT, &value);
        consume_token(state);
        return true;
}
bool imp_parse_load(IMP_PARSE_STATE *state) {
        IMP_NODE_VALUE value;
        IMP_NODE *load_node;
        if(get_token_type(state) != IMP_TOKEN_TYPE_LOAD) return false;
        consume_token(state);
        imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_NAME, true, get_token_line(state), "Expected name which shall be loaded into.");
        value.string = (char*)get_token_string_value(state);
        load_node = imp_node_create(state->current, IMP_NODE_TYPE_LOAD, &value);
        consume_token(state);
        imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_COLON_EQUAL, true, get_token_line(state), "Expected punctuator `:=` in load statement.");
        consume_token(state);
        imp_assert(get_token_type(state) == IMP_TOKEN_TYPE_IMMEDIATE, true, get_token_line(state), "Expected immediate with which to load.");
        value.number = get_token_numeric_value(state);
        /* TODO: keep track of bound registers and the immediate size given to them */
        imp_node_create(load_node, IMP_NODE_TYPE_IMMEDIATE, &value);
        consume_token(state);
        return true;
}
bool imp_parse_halt(IMP_PARSE_STATE *state) {
        if(get_token_type(state) != IMP_TOKEN_TYPE_HALT) return false;
        imp_node_create(state->current, IMP_NODE_TYPE_HALT, NULL);
        consume_token(state);
        return true;
}

bool imp_parse_register(IMP_PARSE_STATE *state) {
        IMP_NODE_VALUE value;
        switch(get_token_type(state)) {
        case IMP_TOKEN_TYPE_LOW:        value.reg.prefix =  IMP_REGISTER_PREFIX_LOW; consume_token(state); break;
        case IMP_TOKEN_TYPE_HIGH:       value.reg.prefix = IMP_REGISTER_PREFIX_HIGH; consume_token(state); break;
        case IMP_TOKEN_TYPE_SEG:        value.reg.prefix =  IMP_REGISTER_PREFIX_SEG; consume_token(state); break;
        default:                        value.reg.prefix = IMP_REGISTER_PREFIX_NULL; break;
        }
        switch(get_token_type(state)) {
        case IMP_TOKEN_TYPE_ACCUMULATOR:        value.reg.kind = IMP_REGISTER_KIND_ACCUMULATOR; consume_token(state); break;
        case IMP_TOKEN_TYPE_POINTER:            value.reg.kind = IMP_REGISTER_KIND_POINTER;     consume_token(state); break;
        case IMP_TOKEN_TYPE_COUNTER:            value.reg.kind = IMP_REGISTER_KIND_COUNTER;     consume_token(state); break;
        case IMP_TOKEN_TYPE_GENERAL:            value.reg.kind = IMP_REGISTER_KIND_GENERAL;     consume_token(state); break;
        case IMP_TOKEN_TYPE_SOURCE:             value.reg.kind = IMP_REGISTER_KIND_SOURCE;      consume_token(state); break;
        case IMP_TOKEN_TYPE_DESTINATION:        value.reg.kind = IMP_REGISTER_KIND_DESTINATION; consume_token(state); break;
        case IMP_TOKEN_TYPE_STACK:              value.reg.kind = IMP_REGISTER_KIND_STACK;       consume_token(state); break;
        case IMP_TOKEN_TYPE_BASE:               value.reg.kind = IMP_REGISTER_KIND_BASE;        consume_token(state); break;
        case IMP_TOKEN_TYPE_EXTRA:              value.reg.kind = IMP_REGISTER_KIND_EXTRA;       consume_token(state); break;
        case IMP_TOKEN_TYPE_DATA:               value.reg.kind = IMP_REGISTER_KIND_DATA;        consume_token(state); break;
        default:                                return false;
        }
        imp_node_create(state->current, IMP_NODE_TYPE_REGISTER, &value);
        return true;
}
