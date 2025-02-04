#ifndef X__IMP_LEX_H__X
#define X__IMP_LEX_H__X
#include <stdint.h>

typedef enum {
        IMP_TOKEN_TYPE_NULL             =       0x00,
        IMP_TOKEN_TYPE_SEGMENT          =       0x01,
        IMP_TOKEN_TYPE_SEGEND           =       0x02,
        IMP_TOKEN_TYPE_DAT              =       0x03,
        IMP_TOKEN_TYPE_DSIZEOF          =       0x04,
        IMP_TOKEN_TYPE_DPREFIX          =       0x05,
        IMP_TOKEN_TYPE_DSUFFIX          =       0x06,
        IMP_TOKEN_TYPE_DENTRY           =       0x07,
        IMP_TOKEN_TYPE_LABEL            =       0x08,
        IMP_TOKEN_TYPE_END              =       0x09,
        IMP_TOKEN_TYPE_BIND             =       0x0A,
        IMP_TOKEN_TYPE_LOW              =       0x0B,
        IMP_TOKEN_TYPE_HIGH             =       0x0C,
        IMP_TOKEN_TYPE_SEG              =       0x0D,
        IMP_TOKEN_TYPE_ACCUMULATOR      =       0x0E,
        IMP_TOKEN_TYPE_POINTER          =       0x0F,
        IMP_TOKEN_TYPE_COUNTER          =       0x10,
        IMP_TOKEN_TYPE_GENERAL          =       0x11,
        IMP_TOKEN_TYPE_SOURCE           =       0x12,
        IMP_TOKEN_TYPE_DESTINATION      =       0x13,
        IMP_TOKEN_TYPE_STACK            =       0x14,
        IMP_TOKEN_TYPE_BASE             =       0x15,
        IMP_TOKEN_TYPE_EXTRA            =       0x16,
        IMP_TOKEN_TYPE_DATA             =       0x17,
        IMP_TOKEN_TYPE_INT              =       0x18,
        IMP_TOKEN_TYPE_HALT             =       0x19,
        IMP_TOKEN_TYPE_COLON            =       0x1A,
        IMP_TOKEN_TYPE_COLON_EQUAL      =       0x1B,
        IMP_TOKEN_TYPE_NAME             =       0x1C,
        IMP_TOKEN_TYPE_IMMEDIATE        =       0x1D,
        IMP_TOKEN_TYPE_LOAD             =       0x1E,

        IMP_TOKEN_TYPE__MAX
} IMP_TOKEN_TYPE;

IMP_TOKEN_TYPE imp_token_str_classify(const char *string);
IMP_TOKEN_TYPE imp_token_chr_classify(const char *source, uint32_t *index);

typedef union {
        uint64_t number;
        char    *string;
} IMP_TOKEN_VALUE;

void imp_token_str_evaluate(IMP_TOKEN_VALUE *value, const char *string);

typedef struct {
        IMP_TOKEN_TYPE  *types;
        IMP_TOKEN_VALUE *values;
        uint32_t        *lines;
        uint32_t length;
} IMP_TOKENS;

void imp_tokens_create(IMP_TOKENS *ret);
void imp_tokens_destroy(IMP_TOKENS *ret);
void imp_tokens_append(IMP_TOKENS *ret, IMP_TOKEN_TYPE type, const IMP_TOKEN_VALUE *value, uint32_t line);
void imp_tokens_print(const IMP_TOKENS *tokens);

void imp_string_tokenize(IMP_TOKENS *ret, const char *string);



#endif
