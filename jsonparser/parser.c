#include "parser.h"
#include "json.h"
#include "lexer.h"
#include "stb_ds.h"
#include <string.h>

char *object_lit[] = {"STRING", "NUMBER", "BOOLEAN", "NULL", "ARRAY", "MAP"};

parser *parser_init(String content) {
    parser *parserptr = (parser *)malloc(sizeof(parser));
    parserptr->lexer = (lexer){0};
    parserptr->current_token = (token){0};
    lexer_init(&parserptr->lexer, content);
    lexer_get_token(&parserptr->lexer, &parserptr->current_token);
    return parserptr;
}

void parser_free(parser *parser) {
    // free the contents of parser
    da_free(&parser->lexer.buffer);
    token_free(&parser->current_token);
    // free the pointer itself
    free(parser);
}

// advances current token
static void parser_next(parser *parser) {
    token_free(&parser->current_token);
    parser->current_token = (token){0};
    lexer_get_token(&parser->lexer, &parser->current_token);
}

static bool parser_parse_array(parser *parser, json_object *object) {
    // [ 1, 2, 3, "abcd" ]
    // ^                  <- start post

    bool result = true;
    // advance past opening bracket
    // either at first element or closing bracket
    parser_next(parser);

    json_object element = {0};
    while (parser->current_token.kind != TOKEN_RBRACK) {
        if (!parser_parse(parser, &element)) {
            return_defer(false);
        }
        da_append(&object->value.array, element);

        parser_next(parser);

        if (parser->current_token.kind == TOKEN_RBRACK)
            break;

        if (parser->current_token.kind != TOKEN_COMMA) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current_token.pos);
            LOG(ERROR,
                "Parser: expected COMMA but found %s at line %zu, col %zu",
                token_lit[parser->current_token.kind], lc.line, lc.col);
            return_defer(false);
        }

        parser_next(parser);
    }

defer:
    if (!result)
        json_object_free(&element);

    return result;
}

static bool parser_parse_map(parser *parser, json_object *object) {
    // { "key": "value" }
    // ^                 <- start pos

    bool result = true;
    parser_next(parser);

    json_object key_element = (json_object){0};
    json_object value_element;
    while (parser->current_token.kind != TOKEN_RBRACE) {
        if (!parser_parse(parser, &key_element)) {
            return_defer(false);
        }
        if (key_element.kind != OBJECT_STRING) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current_token.pos);
            LOG(ERROR,
                "Parser: map key has to be a string but found %s at line "
                "%zu, "
                "col %zu",
                object_lit[key_element.kind], lc.line, lc.col);
            return_defer(false);
        }

        parser_next(parser);

        if (parser->current_token.kind != TOKEN_COLON) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current_token.pos);
            LOG(ERROR,
                "Parser: expected COLON but found %s at line %zu, col %zu",
                token_lit[parser->current_token.kind], lc.line, lc.col);
            return_defer(false);
        }

        parser_next(parser);

        value_element = (json_object){0};
        if (!parser_parse(parser, &value_element)) {
            return_defer(false);
        }
        json_object *value = malloc(sizeof(json_object));
        *value = value_element;
        hmput(object->value.map, key_element.value.string, value);

        parser_next(parser);
        if (parser->current_token.kind == TOKEN_RBRACE)
            break;

        if (parser->current_token.kind != TOKEN_COMMA) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current_token.pos);
            LOG(ERROR,
                "Parser: expected COMMA but found %s at line %zu, col %zu",
                token_lit[parser->current_token.kind], lc.line, lc.col);
            return_defer(false);
        }
        parser_next(parser);
    }

defer:
    if (!result) {
        json_object_free(&key_element);
        json_object_free(&value_element);
    }
    return result;
}

bool parser_parse(parser *parser, json_object *object) {
    bool result = true;
    token token = parser->current_token;

    switch (token.kind) {
    case TOKEN_LBRACK:
        object->kind = OBJECT_ARRAY;
        if (!parser_parse_array(parser, object)) {
            return_defer(false);
        }
        break;
    case TOKEN_LBRACE:
        object->kind = OBJECT_MAP;
        object->value.map = NULL; // need to null-initialize for stb_ds.h
        if (!parser_parse_map(parser, object)) {
            return_defer(false);
        }
        break;
    case TOKEN_STRING:
        object->kind = OBJECT_STRING;
        // copy from token.value to object->value
        string_append0(&token.value);
        object->value.string = strdup(token.value.items);
        assert(object->value.string != NULL);
        break;
    case TOKEN_NUMBER:
        object->kind = OBJECT_NUMBER;
        string_append0(&token.value);
        object->value.number = strtod(token.value.items, NULL);
        break;
    case TOKEN_BOOLEAN:
        object->kind = OBJECT_BOOLEAN;
        switch (token.value.items[0]) {
        case 't':
            object->value.boolean = true;
            break;
        case 'f':
            object->value.boolean = false;
            break;
        }
        break;
    case TOKEN_NULL:
        object->kind = OBJECT_NULL;
        break;
    default:
        if (token.kind != TOKEN_ILLEGAL) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current_token.pos);
            LOG(ERROR, "Unexpected token type: %s at line %zu, column %zu",
                token_lit[token.kind], lc.line, lc.col);
        }
        return false;
        break;
    }

defer:
    return result;
}
