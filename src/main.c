#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "imp_lex.h"
#include "imp_parse.h"
#include "imp_compile.h"

struct cmd_options {
        const char *input_file, *output_file, *a0;
        bool debug_text;
};

void parse_cmd_options(struct cmd_options *ret, int argc, const char **argv);
void help_menu(const char *a0, int ecode);
void read_input_file(char **content, struct cmd_options options);
void lex_input_string(IMP_TOKENS *tokens, const char *string, struct cmd_options options);
void parse_tokens(IMP_NODE **ast, const IMP_TOKENS *tokens, struct cmd_options options);
void compile_program(IMP_BYTE_BUFFER *output_buffer, const IMP_NODE *ast, struct cmd_options options);
bool write_output_buffer_to_file(IMP_BYTE_BUFFER output_buffer, struct cmd_options options);


int main(int argc, char **argv) {
        struct cmd_options      options;
        char                   *content;
        IMP_BYTE_BUFFER         output_buffer;
        IMP_TOKENS              tokens;
        IMP_NODE               *ast;

        parse_cmd_options(&options, argc, (const char**)argv);
        read_input_file(&content, options);
        lex_input_string(&tokens, content, options);
        parse_tokens(&ast, &tokens, options);
        compile_program(&output_buffer, ast, options);
        write_output_buffer_to_file(output_buffer, options);

        imp_byte_buffer_destroy(&output_buffer);
        imp_node_destroy(ast);
        imp_tokens_destroy(&tokens);
        free(content);
        exit(0);
}

void parse_cmd_options(struct cmd_options *ret, int argc, const char **argv) {
        uint32_t i;
        ret->a0 = argv[0];
        ret->debug_text = false;
        ret->input_file = ret->output_file = NULL;
        for(i = 1; i < argc; i++) {
                if(argv[i][0] != '-') {
                        if(ret->input_file) {
                                fprintf(stderr, "\033[1;31mError:\033[m Multiple input files: A = `%s`, B = `%s`.\n", ret->input_file, argv[i]);
                                exit(1);
                        }
                        ret->input_file = argv[i];
                        continue;
                }
                switch(argv[i][1]) {
                case 'h': help_menu(argv[0], 0); break;
                case 'o':
                        if(ret->output_file) {
                                fprintf(stderr, "\033[1;31mError:\033[m Multiple output files: A = `%s`, B = `%s`\n.", ret->output_file, argv[i]);
                                exit(1);
                        }
                        ret->output_file = argv[++i];
                        break;
                case '-':
                        if(!strcmp(argv[i]+2, "help")) help_menu(argv[0], 0);
                        else if(!strcmp(argv[i]+2, "debug")) ret->debug_text = true;
                        else goto _invalid_usage;
                        break;
                _invalid_usage:
                default:
                        fprintf(stderr, "\033[1;31mError:\033[m Invalid argument `%s`.\n", argv[i]);
                        help_menu(argv[0], 1);
                }
        }
        if(!ret->input_file) {
                fprintf(stderr, "\033[1;31mError:\033[m No input files.\n");
                help_menu(argv[0], 1);
        }
        if(!ret->output_file) ret->output_file = "./a.out";
}

void help_menu(const char *a0, int ecode) {
        fprintf(stderr, "\033[1;33mUsage:\033[m %s [options] input_file [options]\n", a0);
        fputs("Options:\n", stderr);
        fputs("\t-h\t\t::\tOpen this menu and exit successfully.\n", stderr);
        fputs("\t-d\t\t::\tAllow compiler debug text to be printed to stderr.\n", stderr);
        fputs("\t-o <ofile>\t::\tSet the return file to ofile.\n", stderr);
        exit(ecode);
}

void read_input_file(char **content, struct cmd_options options) {
        FILE *fp;
        uint32_t length;
        fp = fopen(options.input_file, "r");
        if(!fp) {
                fprintf(stderr, "\033[1;31mError:\033[m Input file %s cannot be opened for reading.\n", options.input_file);
                help_menu(options.a0, 1);
        }
        fseek(fp, 0L, SEEK_END);
        length = ftell(fp);
        *content = calloc(1, length+1);
        fseek(fp, 0L, SEEK_SET);
        fread(*content, 1, length, fp);
        fclose(fp);
        if(options.debug_text) fprintf(stderr, "\033[90m%s\n\033[m", *content);
}
void lex_input_string(IMP_TOKENS *tokens, const char *string, struct cmd_options options) {
        imp_tokens_create(tokens);
        imp_string_tokenize(tokens, string);
        if(options.debug_text) imp_tokens_print(tokens);
}
void parse_tokens(IMP_NODE **ast, const IMP_TOKENS *tokens, struct cmd_options options) {
        imp_parse_program(ast, tokens);
        if(options.debug_text) imp_node_print(*ast, 0);
}
void compile_program(IMP_BYTE_BUFFER *output_buffer, const IMP_NODE *ast, struct cmd_options options) {
        uint32_t i;
        imp_compile_program(output_buffer, ast);
        if(options.debug_text) {
                fprintf(stderr, "\033[90m");
                for(i = 0; i < output_buffer->length; i++)
                        fputc(output_buffer->contents[i], stderr);
                fprintf(stderr, "\033[m");
        }
}
bool write_output_buffer_to_file(IMP_BYTE_BUFFER output_buffer, struct cmd_options options) {
        FILE *fp;
        fp = fopen(options.output_file, "w");
        if(!fp) {
                fprintf(stderr, "\033[1;31mInput file %s cannot be opened for writing\033[m", options.output_file);
                return false;
        }
        fwrite(output_buffer.contents, 1, output_buffer.length, fp);
        fclose(fp);
}

