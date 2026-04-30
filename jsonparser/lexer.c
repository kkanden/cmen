#include <ctype.h>
#include <stddef.h>
#include <stdio.h>

#include "lexer.h"

char *token_lit[] = {"LBRACE",  "RBRACE", "LBRACK", "RBRACK",
                     "COLON",   "COMMA",  "STRING", "NUMBER",
                     "BOOLEAN", "NULL",   "EOF",    "ILLEGAL"};

// get (line, col) pair from pos, both one-based
line_col calc_line_col(String s, size_t pos) {
    assert(pos <= s.count);
    size_t line = 1;
    size_t col = 1;
    for (size_t i = 0; i < pos; i++) {
        if (s.items[i] != '\n') {
            col++;
        } else {
            line++;
            col = 1;
        }
    }
    return (line_col){line, col};
}

void token_free(token *tokenptr) {
    da_free(&tokenptr->value);
    // zero out the pointer
    *tokenptr = (token){0};
}

static char lexer_peek(lexer *lexer) {
    if (lexer->pos >= lexer->buffer.count)
        return EOF;
    return lexer->buffer.items[lexer->pos];
}

static char lexer_read(lexer *lexer) {
    lexer->ch = lexer_peek(lexer);
    lexer->pos++;
    return lexer->ch;
}

static void lexer_skip_ws(lexer *lexer) {
    while (isspace(lexer->ch)) { // space, newline, tab, \r, \f \v
        lexer_read(lexer);
    }
}

static bool lexer_read_literal(lexer *lexer, char *expected_literal) {
    // lexer is at the first character of the literal
    // true  |  false  | null
    // ^        ^        ^
    size_t expected_len = strlen(expected_literal);
    for (size_t i = 0; i < expected_len; i++) {
        if (lexer->ch != expected_literal[i]) {
            return false;
        }
        lexer_read(lexer);
    }

    return true;
}

// str must be empty
static bool lexer_read_string(lexer *lexer, String *str) {
    // lexer is at the first quotation
    // "dupa"
    // ^
    // we will advance the lexer until the other quotation and extract the
    // string
    assert(str->count == 0);
    size_t first_quote = lexer->pos;

    // consume first quote
    lexer_read(lexer);
    while (lexer->ch != '"') {
        if (lexer->ch == EOF || lexer->ch == '\n') // unterminated string
            return false;
        if (lexer->ch == '\\') // skip escaped character
            lexer_read(lexer);
        lexer_read(lexer);
    }

    assert(lexer->pos > first_quote);
    size_t length_inside_quote = lexer->pos - first_quote - 1;
    da_append_many(str, lexer->buffer.items + first_quote, length_inside_quote);

    // consume other quote
    lexer_read(lexer);

    return true;
}

static bool lexer_read_number(lexer *lexer, String *numstr) {
    // lexer is either at the first digit or the leading minus sign
    // 123.45  |  -5
    // ^          ^
    assert(numstr->count == 0);

    if (lexer->ch == '-') {
        da_append(numstr, lexer->ch);
        lexer_read(lexer);
        // minus has to be followed by a digit
        if (!isdigit(lexer->ch))
            return false;
    }

    while (isdigit(lexer->ch)) {
        da_append(numstr, lexer->ch);
        lexer_read(lexer);
    }

    if (lexer->ch == '.') {
        // read in the part after period
        da_append(numstr, lexer->ch);
        lexer_read(lexer);
        if (!isdigit(lexer->ch))
            return false;

        while (isdigit(lexer->ch)) {
            da_append(numstr, lexer->ch);
            lexer_read(lexer);
        }
    }

    return true;
}

void lexer_init(lexer *lexer, String text) {
    lexer->buffer = text;
    // read in first character
    lexer_read(lexer);
}

static void print_error_escaped(char c) {
    switch (c) {
    case '\n':
        fprintf(stderr, "\\n");
        break;
    case '\t':
        fprintf(stderr, "\\t");
        break;
    case '\r':
        fprintf(stderr, "\\r");
        break;
    case '\0':
        fprintf(stderr, "\\0");
        break;
    case '\\':
        fprintf(stderr, "\\\\");
        break;
    default:
        fprintf(stderr, "%c", c);
        break;
    }
}

static void lexer_error(lexer *lexer) {
    line_col lc = calc_line_col(lexer->buffer, lexer->pos - 1);
    fprintf(stderr, "[ERROR] Lexer: unexepected character `");
    print_error_escaped(lexer->ch);
    fprintf(stderr, "` at line %zu, col %zu\n", lc.line, lc.col);
}

bool lexer_get_token(lexer *lexer, token *token) {
    bool result = true;
    lexer_skip_ws(lexer);

    bool advance = true;

    char ch = lexer->ch;
    token->pos = lexer->pos - 1;
    if (ch == EOF) {
        token->kind = TOKEN_EOF;
        string_append_cstr(&token->value, "EOF");
    } else if (ch == '{') {
        token->kind = TOKEN_LBRACE;
        string_append_cstr(&token->value, "{");
    } else if (ch == '}') {
        token->kind = TOKEN_RBRACE;
        string_append_cstr(&token->value, "}");
    } else if (ch == '[') {
        token->kind = TOKEN_LBRACK;
        string_append_cstr(&token->value, "[");
    } else if (ch == ']') {
        token->kind = TOKEN_RBRACK;
        string_append_cstr(&token->value, "]");
    } else if (ch == ':') {
        token->kind = TOKEN_COLON;
        string_append_cstr(&token->value, ":");
    } else if (ch == ',') {
        token->kind = TOKEN_COMMA;
        string_append_cstr(&token->value, ",");
    } else if (ch == 't' || ch == 'f' || ch == 'n') {
        char *literal;
        switch (ch) {
        case 't':
            literal = "true";
            token->kind = TOKEN_BOOLEAN;
            break;
        case 'f':
            literal = "false";
            token->kind = TOKEN_BOOLEAN;
            break;
        case 'n':
            literal = "null";
            token->kind = TOKEN_NULL;
            break;
        }
        if (!lexer_read_literal(lexer, literal)) {
            lexer_error(lexer);
            token->kind = TOKEN_ILLEGAL;
            result = false;
        } else {
            string_append_cstr(&token->value, literal);
        }
        advance = false;
    } else if (ch == '"') {
        String str = {0};
        if (!lexer_read_string(lexer, &str)) {
            lexer_error(lexer);
            token->kind = TOKEN_ILLEGAL;
            result = false;
        } else {
            token->kind = TOKEN_STRING;
            string_cat(&token->value, &str);
        }
        // memory is copied to the token value so it's safe to free this
        da_free(&str);
        advance = false;
    } else if (isdigit(ch) || ch == '-') {
        String numstr = {0};
        if (!lexer_read_number(lexer, &numstr)) {
            lexer_error(lexer);
            token->kind = TOKEN_ILLEGAL;
            result = false;
        } else {
            token->kind = TOKEN_NUMBER;
            string_cat(&token->value, &numstr);
        }
        // memory is copied to the token value so it's safe to free this
        da_free(&numstr);
        advance = false;
    } else {
        lexer_error(lexer);
        token->kind = TOKEN_ILLEGAL;
        advance = false;
        result = false;
    }

    if (advance)
        lexer_read(lexer);

    return result;
}
