#include "parser.h"
#include "lexer.h"
#include "stb_ds.h"

char *object_lit[] = {"STRING", "NUMBER", "BOOLEAN", "NULL", "ARRAY", "MAP"};

void parser_init(parser *parser, String content) {
    lexer_init(&parser->lexer, content);
    lexer_get_token(&parser->lexer, &parser->current);
}

void parser_free(parser *parser) {
    da_free(parser->lexer.buffer);
    token_free(&parser->current);
}

// advances current token
static void parser_next(parser *parser) {
    lexer_get_token(&parser->lexer, &parser->current);
}

static bool parser_parse_array(parser *parser, json_object *object) {
    // [ 1, 2, 3, "abcd" ]
    // ^                  <- start post

    // advance past opening bracket
    // either at first element or closing bracket
    parser_next(parser);

    while (parser->current.kind != TOKEN_RBRACK) {
        json_object element = {0};
        if (!parser_parse(parser, &element)) {
            return false;
        }
        da_append(&object->value.array, element);

        parser_next(parser);

        if (parser->current.kind == TOKEN_RBRACK)
            break;

        if (parser->current.kind != TOKEN_COMMA) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current.pos);
            LOG(ERROR,
                "Parser: expected COMMA but found %s at line %zu, col %zu",
                token_lit[parser->current.kind], lc.line, lc.col);
            return false;
        }

        parser_next(parser);
    }

    return true;
}

static bool parser_parse_map(parser *parser, json_object *object) {
    // { "key": "value" }
    // ^                 <- start pos

    parser_next(parser);

    while (parser->current.kind != TOKEN_RBRACE) {
        json_object key_element = {0};
        if (!parser_parse(parser, &key_element)) {
            return false;
        }
        if (key_element.kind != OBJECT_STRING) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current.pos);
            LOG(ERROR,
                "Parser: map key has to be a string but found %s at line "
                "%zu, "
                "col %zu",
                object_lit[key_element.kind], lc.line, lc.col);
            return false;
        }
        char *key = key_element.value.string;

        parser_next(parser);
        if (parser->current.kind != TOKEN_COLON) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current.pos);
            LOG(ERROR,
                "Parser: expected COLON but found %s at line %zu, col %zu",
                token_lit[parser->current.kind], lc.line, lc.col);
            return false;
        }
        parser_next(parser);

        json_object value_element = {0};
        if (!parser_parse(parser, &value_element)) {
            return false;
        }
        json_object *value = malloc(sizeof(json_object));
        *value = value_element;
        hmput(object->value.map, key, value);

        parser_next(parser);
        if (parser->current.kind == TOKEN_RBRACE)
            break;

        if (parser->current.kind != TOKEN_COMMA) {
            line_col lc =
                calc_line_col(parser->lexer.buffer, parser->current.pos);
            LOG(ERROR,
                "Parser: expected COMMA but found %s at line %zu, col %zu",
                token_lit[parser->current.kind], lc.line, lc.col);
            return false;
        }
        parser_next(parser);
    }

    return true;
}

bool parser_parse(parser *parser, json_object *object) {
    token token = parser->current;

    switch (token.kind) {
    case TOKEN_LBRACK:
        object->kind = OBJECT_ARRAY;
        if (!parser_parse_array(parser, object)) {
            return false;
        }
        break;
    case TOKEN_LBRACE:
        object->kind = OBJECT_MAP;
        if (!parser_parse_map(parser, object)) {
            return false;
        }
        break;
    case TOKEN_STRING:
        object->kind = OBJECT_STRING;
        string_append0(&token.value);
        object->value.string = token.value.items;
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
                calc_line_col(parser->lexer.buffer, parser->current.pos);
            LOG(ERROR, "Unexpected token type: %s at line %zu, column %zu",
                token_lit[token.kind], lc.line, lc.col);
        }
        return false;
        break;
    }

    return true;
}
