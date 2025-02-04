#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <stdbool.h>
#include "imp_lex.h"

#define IMP_TOKEN_STR_CLASSIFY_UPDATED_TO 0x21
IMP_TOKEN_TYPE imp_token_str_classify(const char *string) {
        char *eptr;
        bool isname;
        uint32_t i;
        assert(IMP_TOKEN_STR_CLASSIFY_UPDATED_TO == IMP_TOKEN_TYPE__MAX && "imp_token_str_classify has not been updated to the current token_type_max.");
        if(!*string) return IMP_TOKEN_TYPE_NULL;
        else if(!strcmp(string,         ".at"))    return            IMP_TOKEN_TYPE_DAT;
        else if(!strcmp(string,         ".id"))    return            IMP_TOKEN_TYPE_DID;
        else if(!strcmp(string,         "end"))    return            IMP_TOKEN_TYPE_END;
        else if(!strcmp(string,         "int"))    return            IMP_TOKEN_TYPE_INT;
        else if(!strcmp(string,         "low"))    return            IMP_TOKEN_TYPE_LOW;
        else if(!strcmp(string,         "seg"))    return            IMP_TOKEN_TYPE_SEG;
        else if(!strcmp(string,        "base"))    return           IMP_TOKEN_TYPE_BASE;
        else if(!strcmp(string,        "bind"))    return           IMP_TOKEN_TYPE_BIND;
        else if(!strcmp(string,        "data"))    return           IMP_TOKEN_TYPE_DATA;
        else if(!strcmp(string,        "halt"))    return           IMP_TOKEN_TYPE_HALT;
        else if(!strcmp(string,        "high"))    return           IMP_TOKEN_TYPE_HIGH;
        else if(!strcmp(string,        "load"))    return           IMP_TOKEN_TYPE_LOAD;
        else if(!strcmp(string,       "extra"))    return          IMP_TOKEN_TYPE_EXTRA;
        else if(!strcmp(string,       "label"))    return          IMP_TOKEN_TYPE_LABEL;
        else if(!strcmp(string,       "stack"))    return          IMP_TOKEN_TYPE_STACK;
        else if(!strcmp(string,      ".entry"))    return         IMP_TOKEN_TYPE_DENTRY;
        else if(!strcmp(string,      "segend"))    return         IMP_TOKEN_TYPE_SEGEND;
        else if(!strcmp(string,      "source"))    return         IMP_TOKEN_TYPE_SOURCE;
        else if(!strcmp(string,     ".prefix"))    return        IMP_TOKEN_TYPE_DPREFIX;
        else if(!strcmp(string,     ".sizeof"))    return        IMP_TOKEN_TYPE_DSIZEOF;
        else if(!strcmp(string,     ".suffix"))    return        IMP_TOKEN_TYPE_DSUFFIX;
        else if(!strcmp(string,     "counter"))    return        IMP_TOKEN_TYPE_COUNTER;
        else if(!strcmp(string,     "general"))    return        IMP_TOKEN_TYPE_GENERAL;
        else if(!strcmp(string,     "pointer"))    return        IMP_TOKEN_TYPE_POINTER;
        else if(!strcmp(string,     "segment"))    return        IMP_TOKEN_TYPE_SEGMENT;
        else if(!strcmp(string, "accumulator"))    return    IMP_TOKEN_TYPE_ACCUMULATOR;
        else if(!strcmp(string, "destination"))    return    IMP_TOKEN_TYPE_DESTINATION;
        else if(!strcmp(string, ".prepend_entry")) return IMP_TOKEN_TYPE_DPREPEND_ENTRY;
        if(sizeof(unsigned long) == sizeof(uint64_t)) {
                if(string[0] == '-') {
                        if(string[1] == '$') strtoul(string + 2, &eptr, 0x10);
                        else                 strtoul(string,     &eptr, 0x0A);
                } else {
                        if(string[0] == '$') strtoul(string + 1, &eptr, 0x10);
                        else                 strtoul(string,     &eptr, 0x0A);
                }
        } else if(sizeof(unsigned long long) == sizeof(uint64_t)) {
                if(string[0] == '-') {
                        if(string[1] == '$') strtoull(string + 2, &eptr, 0x10);
                        else                 strtoull(string,     &eptr, 0x0A);
                } else {
                        if(string[0] == '$') strtoull(string + 1, &eptr, 0x10);
                        else                 strtoull(string,     &eptr, 0x0A);
                }
        }
        if(!*eptr) return IMP_TOKEN_TYPE_IMMEDIATE;

        isname = isalpha(string[0]) || string[0] == '_';
        for(i = 1; string[i] && (isname = isalpha(string[i]) || isdigit(string[i]) || string[i] == '_'); i++);

        return isname ? IMP_TOKEN_TYPE_NAME : IMP_TOKEN_TYPE_NULL;
}
#define IMP_TOKEN_CHR_CLASSIFY_UPDATED_TO 0x21
IMP_TOKEN_TYPE imp_token_chr_classify(const char *restrict source, uint32_t *restrict index) {
        assert(IMP_TOKEN_CHR_CLASSIFY_UPDATED_TO == IMP_TOKEN_TYPE__MAX && "imp_token_chr_classify has not been updated to the current token_type_max.");
        switch(source[*index]) {
        case ':': if(source[*index + 1] == '=') {
                        ++*index;
                        return IMP_TOKEN_TYPE_COLON_EQUAL;
                  }
                  return IMP_TOKEN_TYPE_COLON;
        default:  return IMP_TOKEN_TYPE_NULL;
        }
}

#define IMP_TOKEN_STR_EVALUATE_UPDATED_TO 0x21
void imp_token_str_evaluate(IMP_TOKEN_VALUE *restrict ret, const char *restrict string) {
        uint64_t value;
        char    *eptr;
        IMP_TOKEN_TYPE type;
        type = imp_token_str_classify(string);
        assert(IMP_TOKEN_STR_EVALUATE_UPDATED_TO == IMP_TOKEN_TYPE__MAX && "imp_token_str_evaluate has not been updated to the current token_type_max.");
        if(type == IMP_TOKEN_TYPE_IMMEDIATE) {
                if(sizeof(unsigned long) == sizeof(uint64_t)) {
                        if(string[0] == '-') {
                                if(string[1] == '$') value = strtoul(string + 2, &eptr, 0x10);
                                else                 value = strtoul(string,     &eptr, 0x0A);
                                value = ~value;
                                value += 1;
                        } else {
                                if(string[0] == '$') value = strtoul(string + 1, &eptr, 0x10);
                                else                 value = strtoul(string,     &eptr, 0x0A);
                        }
                } else if(sizeof(unsigned long long) == sizeof(uint64_t)) {
                ull:
                        if(string[0] == '-') {
                                if(string[1] == '$') value = strtoull(string + 2, &eptr, 0x10);
                                else                 value = strtoull(string,     &eptr, 0x0A);
                                value = ~value;
                                value += 1;
                        } else {
                                if(string[0] == '$') value = strtoull(string + 1, &eptr, 0x10);
                                else                 value = strtoull(string,     &eptr, 0x0A);
                        }
                } else {
                        fputs("\033[1;33mEnvironment long long size < 64, integer size may be restricted.\n\033[m", stderr);
                        goto ull;
                }
                ret->number = value;
        } else if(type == IMP_TOKEN_TYPE_NAME) {
                ret->string = malloc(strlen(string)+1);
                strcpy(ret->string, string);
        } else memset(ret, 0, sizeof(*ret));
}

void imp_tokens_create(IMP_TOKENS *ret) {
        ret->types  = malloc(1);
        ret->lines  = malloc(1);
        ret->values = malloc(1);
        ret->length = 0;
}
#define IMP_TOKENS_DESTROY_UPDATED_TO 0x21
void imp_tokens_destroy(IMP_TOKENS *ret) {
        uint32_t i;
        assert(IMP_TOKENS_DESTROY_UPDATED_TO == IMP_TOKEN_TYPE__MAX && "imp_tokens_destroy has not been updated to the current token_type_max.");
        for(i = 0; i < ret->length; i++) {
                if(ret->types[i] != IMP_TOKEN_TYPE_NAME) continue;
                free(ret->values[i].string);
        }
        free(ret->types);
        free(ret->lines);
        free(ret->values);
        memset(ret, 0, sizeof(*ret));
}
void imp_tokens_append(IMP_TOKENS *restrict ret, IMP_TOKEN_TYPE type, const IMP_TOKEN_VALUE *restrict value, uint32_t line) {
        ret->length += 1;
        ret->types   = realloc(ret->types,  ret->length *  sizeof(*ret->types));
        ret->lines   = realloc(ret->lines,  ret->length *  sizeof(*ret->lines));
        ret->values  = realloc(ret->values, ret->length * sizeof(*ret->values));
        ret->types[ret->length - 1] = type;
        ret->lines[ret->length - 1] = line;
        if(value) switch(type) {
        case IMP_TOKEN_TYPE_NAME:
                ret->values[ret->length - 1].string = malloc(strlen(value->string)+1);
                strcpy(ret->values[ret->length - 1].string, value->string);
                break;
        case IMP_TOKEN_TYPE_IMMEDIATE:
                memcpy(ret->values + ret->length - 1, value, sizeof(*value));
                break;
        default: ret->values[ret->length - 1].string = NULL; break;
        } else ret->values[ret->length - 1].string = NULL;
}
#define IMP_TOKENS_PRINT_UPDATED_TO 0x21
void imp_tokens_print(const IMP_TOKENS *tokens) {
        uint32_t i;
        static const char *token_type_repr[IMP_TOKEN_TYPE__MAX] = {
                "NULL",         "segment",              "segend",
                ".at",          ".sizeof",              ".prefix",
                ".suffix",      ".entry",               "label",
                "end",          "bind",                 "low",
                "high",         "seg",                  "accumulator",
                "pointer",      "counter",              "general",
                "source",       "destination",          "stack",
                "base",         "extra",                "data",
                "int",          "halt",                 ":",
                ":=",           "<name>",               "<immediate>",
                "load",         ".prepend_entry",       ".id"
        };
        assert(IMP_TOKENS_PRINT_UPDATED_TO == IMP_TOKEN_TYPE__MAX && "imp_tokens_print has not been updated to the current token_type_max");
        fputs("\033[90m", stderr);
        for(i = 0; i < tokens->length; i++) {
                fprintf(stderr, "%010d||[line: %10u]  %15s :: ", i, tokens->lines[i], token_type_repr[tokens->types[i]]);
                if(tokens->types[i] == IMP_TOKEN_TYPE_NAME) fprintf(stderr, "%15s\n", tokens->values[i].string);
                else if(tokens->types[i] == IMP_TOKEN_TYPE_IMMEDIATE) fprintf(stderr, "$%014x\n", tokens->values[i].number);
                else fprintf(stderr, "%15s\n", " ");
        }
        fputs("\033[m", stderr);
}

void imp_string_tokenize(IMP_TOKENS *restrict ret, const char *restrict string) {
        char *buffer;
        uint32_t i, buffer_padding, buffer_length, line;
        IMP_TOKEN_VALUE tmp_value;
        IMP_TOKEN_TYPE tmp_type[2] = {0};
        buffer = malloc(0x10);
        buffer_length  = 0;
        buffer_padding = 0x10;
        line = 0;
        for(i = 0; string[i]; i++) {
                line += string[i] == '\n';
                if(!isspace(string[i]) && string[i] != '\\' && !(tmp_type[0] = imp_token_chr_classify(string, &i))) {
                        buffer_length += 1;
                        if(buffer_length >= buffer_padding) {
                                buffer_padding += buffer_padding >> 1;
                                buffer = realloc(buffer, buffer_padding);
                        }
                        buffer[buffer_length - 1] = string[i];
                        continue;
                }
                if(buffer_length) {
                        buffer[buffer_length] = 0;
                        tmp_type[1] = imp_token_str_classify(buffer);
                        if(!tmp_type[1])
                                fprintf(stderr, "\033[1;31m Could not classify token %s on line %u, omitting it.\n\033[m", buffer, line);
                        imp_token_str_evaluate(&tmp_value, buffer);
                        imp_tokens_append(ret, tmp_type[1], &tmp_value, line);
                        if(tmp_type[1] == IMP_TOKEN_TYPE_NAME) free(tmp_value.string);
                        buffer_length = 0;
                }
                if(tmp_type[0]) imp_tokens_append(ret, tmp_type[0], NULL, line);
                tmp_type[0] = IMP_TOKEN_TYPE_NULL;
                if(string[i] == '\\') {for(i++; string[i] && string[i] != '\\'; i++); if(string[i]) i++; }
        }
        free(buffer);
}
