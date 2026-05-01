#ifndef JSON_H
#define JSON_H
#include "parser.h"

void json_print(json_object object, int indent);
void json_object_free(json_object *object);
bool json_from_string(String file_content, json_object *object);

#endif
