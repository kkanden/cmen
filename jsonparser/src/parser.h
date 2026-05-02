#ifndef PARSER_H
#define PARSER_H
#include "json.h"
#include "lexer.h"

typedef struct {
    lexer lexer;
    token current_token;
} parser;

parser *parser_init(String content);
void parser_free(parser *parser);
bool parser_parse(parser *parser, json_object *object);
#endif
