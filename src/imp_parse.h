#ifndef X__IMP_PARSE_H__X
#define X__IMP_PARSE_H__X
#include <stdint.h>
#include <stdbool.h>
#include "imp_lex.h"

typedef enum {
        IMP_NODE_TYPE_NULL                              =       0x00,   /* value class = None           */
        IMP_NODE_TYPE_PROGRAM                           =       0x01,   /* value class = None           */
        IMP_NODE_TYPE_SEGMENT                           =       0x02,   /* value class = None           */
        IMP_NODE_TYPE_SEGMENT_PARAMETER_AT              =       0x03,   /* value class = Immediate      */
        IMP_NODE_TYPE_SEGMENT_PARAMETER_SIZEOF          =       0x04,   /* value class = Immediate      */
        IMP_NODE_TYPE_SEGMENT_PARAMETER_PREFIX          =       0x05,   /* value class = Immediate List */
        IMP_NODE_TYPE_SEGMENT_PARAMETER_SUFFIX          =       0x06,   /* value class = Immediate List */
        IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_LITERAL   =       0x07,   /* value class = Immediate      */
        IMP_NODE_TYPE_SEGMENT_PARAMETER_ENTRY_NAMED     =       0x08,   /* value class = String         */
        IMP_NODE_TYPE_IMMEDIATE_LIST                    =       0x09,   /* value class = Immediate List */
        IMP_NODE_TYPE_SEGMENT_BLOCK                     =       0x0A,   /* value class = None           */
        IMP_NODE_TYPE_PRIMARY_STATEMENT                 =       0x0B,   /* value class = None           */
        IMP_NODE_TYPE_LABEL                             =       0x0C,   /* value class = String         */
        IMP_NODE_TYPE_BLOCK                             =       0x0D,   /* value class = None           */
        IMP_NODE_TYPE_STATEMENT                         =       0x0E,   /* value class = None           */
        IMP_NODE_TYPE_BIND                              =       0x0F,   /* value class = None           */
        IMP_NODE_TYPE_REGISTER                          =       0x10,   /* value class = Register       */
        IMP_NODE_TYPE_INTERRUPT                         =       0x11,   /* value class = Immediate      */
        IMP_NODE_TYPE_LOAD                              =       0x12,   /* value class = String         */
        IMP_NODE_TYPE_HALT                              =       0x13,   /* value class = None           */
        IMP_NODE_TYPE_IMMEDIATE                         =       0x14,   /* value class = Immediate      */
        IMP_NODE_TYPE_NAME                              =       0x15,   /* value class = String         */

        IMP_NODE_TYPE__MAX
} IMP_NODE_TYPE;

/*PREFIX & KIND == Unique register value*/

typedef enum {
        IMP_REGISTER_KIND_NULL          =       0x000,
        IMP_REGISTER_KIND_ACCUMULATOR   =       0x008,
        IMP_REGISTER_KIND_POINTER       =       0x010,
        IMP_REGISTER_KIND_COUNTER       =       0x020,
        IMP_REGISTER_KIND_GENERAL	=       0x040,
        IMP_REGISTER_KIND_SOURCE	=       0x080,
        IMP_REGISTER_KIND_DESTINATION	=       0x100,
        IMP_REGISTER_KIND_STACK		=       0x110,
        IMP_REGISTER_KIND_BASE		=       0x120,
        IMP_REGISTER_KIND_EXTRA         =       0x140,
        IMP_REGISTER_KIND_DATA          =       0x180,
        
        IMP_REGISTER_KIND__MAX
} IMP_REGISTER_KIND;

typedef enum {
        IMP_REGISTER_PREFIX_NULL        =       0x00,
        IMP_REGISTER_PREFIX_HIGH        =       0x01,
        IMP_REGISTER_PREFIX_LOW         =       0x02,
        IMP_REGISTER_PREFIX_SEG         =       0x04,

        IMP_REGISTER_PREFIX__MAX
} IMP_REGISTER_PREFIX;

typedef union {
        char     *string;
        uint64_t  number;
        struct {
                IMP_REGISTER_PREFIX prefix;
                IMP_REGISTER_KIND   kind;
        } reg;
        struct {
                uint64_t *contents;
                uint32_t length;
        } number_array;
        
} IMP_NODE_VALUE;

typedef struct imp_node {
        struct imp_node *parent;
        struct imp_node *children;
        IMP_NODE_VALUE value;
        uint32_t children_length;
        IMP_NODE_TYPE type;
} IMP_NODE;

IMP_NODE *imp_node_create(/*Nullable*/ IMP_NODE *parent, IMP_NODE_TYPE type, IMP_NODE_VALUE *value);
void imp_node_destroy(IMP_NODE *ret);
void imp_node_print(const IMP_NODE *ret, uint32_t index);

typedef struct {
        const IMP_TOKENS *tokens;
        IMP_NODE *ret, *current;
        uint32_t tindex;
} IMP_PARSE_STATE;

void imp_parse_state_create(IMP_PARSE_STATE *ret, const IMP_TOKENS *tokens, IMP_NODE *product);
void imp_parse_state_destroy(IMP_PARSE_STATE *ret);

void imp_assert(bool condition, bool error, uint32_t line, const char *message, ...);

void imp_parse_program(IMP_NODE **ret, const IMP_TOKENS *tokens);
bool imp_parse_segment(IMP_PARSE_STATE *state);
bool imp_parse_segment_parameter(IMP_PARSE_STATE *state);
bool imp_parse_segment_block(IMP_PARSE_STATE *state);
bool imp_parse_label(IMP_PARSE_STATE *state);
bool imp_parse_block(IMP_PARSE_STATE *state);
bool imp_parse_bind(IMP_PARSE_STATE *state);
bool imp_parse_int(IMP_PARSE_STATE *state);
bool imp_parse_load(IMP_PARSE_STATE *state);
bool imp_parse_halt(IMP_PARSE_STATE *state);

bool imp_parse_register(IMP_PARSE_STATE *state);


#endif
