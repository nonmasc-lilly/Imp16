#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "imp_lex.h"
#include "imp_parse.h"
#include "imp_compile.h"

void help_menu(const char *a0, int ecode) {
        fprintf(stderr, "\033[1;33mUsage:\033[m %s [options] input_file [options]\n", a0);
        fputs("Options:\n", stderr);
        fputs("\t-h\t\t::\tOpen this menu and exit successfully.\n", stderr);
        fputs("\t-d\t\t::\tAllow compiler debug text to be printed to stderr.\n", stderr);
        fputs("\t-o <ofile>\t::\tSet the return file to ofile.\n", stderr);
        exit(ecode);
}

int main(int argc, char **argv) {
        bool debug_text;
        FILE *fp;
        char *content;
        uint32_t i;
        const char *input_file, *output_file;
        IMP_TOKENS lexed_tokens;
        IMP_NODE  *ast;
        IMP_BYTE_BUFFER buffer;
        debug_text = false;
        input_file = NULL;
        output_file = "./a.out";
        for(i = 1; i < argc; i++) {
                if(argv[i][0] != '-') {
                        input_file = argv[i];
                        continue;
                }
                switch(argv[i][1]) {
                case 'h': help_menu(argv[0], 0);   break;
                case 'o': output_file = argv[++i]; break;
                case '-': if(!strcmp(argv[i]+2, "help")) help_menu(argv[0], 0);
                          else if(!strcmp(argv[i]+2, "debug")) debug_text = 1;
                          else goto _invalid_usage;
                          break;
                _invalid_usage:
                default:  fprintf(stderr, "\033[1;31mError:\033[m Invalid usage!\n");
                          help_menu(argv[0], 1);
                }
        }
        if(!input_file) {
                fprintf(stderr, "\033[1;31mMissing input file!\n\033[m");
                help_menu(argv[0], 1);
        }
        fp = fopen(input_file, "r");
        if(!fp) {
                fprintf(stderr, "\033[1;31mInput file %s cant be opened for reading\033[m", input_file);
                help_menu(argv[0], 1);
        }
        fseek(fp, 0L, SEEK_END);
        i = ftell(fp);
        content = calloc(1,i+1);
        fseek(fp, 0L, SEEK_SET);
        fread(content, 1, i, fp);
        fclose(fp);
        if(debug_text) fprintf(stderr, "\033[90m%s\n\033[m", content);
        imp_tokens_create(&lexed_tokens);
        imp_string_tokenize(&lexed_tokens, content);
        if(debug_text) imp_tokens_print(&lexed_tokens);
        imp_parse_program(&ast, &lexed_tokens);
        if(debug_text) imp_node_print(ast, 0);
        imp_compile_program(&buffer, ast);
        if(debug_text) {
                fprintf(stderr, "\033[90m");
                for(i = 0; i < buffer.length; i++) {
                        fputc(buffer.contents[i], stderr);
                }
                fprintf(stderr, "\033[m");
        }
        fp = fopen(output_file, "w");
        if(!fp) {
                fprintf(stderr, "\033[1;31mInput file %s cant be opened for writing\033[m", output_file);
                imp_byte_buffer_destroy(&buffer);
                imp_node_destroy(ast);
                imp_tokens_destroy(&lexed_tokens);
                free(content);
                help_menu(argv[0], 1);
        }
        fwrite(buffer.contents, 1, buffer.length, fp);
        fclose(fp);
        imp_byte_buffer_destroy(&buffer);
        imp_node_destroy(ast);
        imp_tokens_destroy(&lexed_tokens);
        free(content);
        exit(0);
}
