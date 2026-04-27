#ifndef LEXER_H
#define LEXER_H

#include "smol.h"

typedef enum {
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_LBRACK,
    TOKEN_RBRACK,
    TOKEN_COLON,
    TOKEN_COMMA,

    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_BOOLEAN,
    TOKEN_NULL,

    TOKEN_EOF,
    TOKEN_ILLEGAL
} token_kind;

extern char *token_lit[];

typedef struct {
    token_kind kind;
    String value;
    size_t pos;
} token;

typedef struct {
    String buffer;
    size_t pos;
    char ch;
} lexer;

typedef struct {
    size_t line;
    size_t col;
} line_col;

line_col calc_line_col(String s, size_t pos);
void lexer_init(lexer *lexer, String text);
bool lexer_get_token(lexer *lexer, token *token);
void token_free(token *token);
#endif
