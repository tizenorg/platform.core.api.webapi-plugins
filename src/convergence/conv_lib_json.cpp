/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string>
#include <sstream>
#include <locale>
#include <iomanip>
#include <json-glib/json-glib.h>
#include "conv_lib_json.h"
#include <stdio.h>
#include <string.h>

#define PATH_DELIM	'.'
#define GVAR_VALUES	"values"
#define GVAR_TYPES	"types"

static double string_to_double(const char* in)
{
	//IF_FAIL_RETURN_TAG(in, 0, _E, "Parameter NULL");

	// Locale-independent string-to-double conversion
	double out;
	std::istringstream istr(in);
	istr.imbue(std::locale("C"));
	istr >> out;
	return out;
}

static std::string double_to_string(double in, int prec)
{
	// Locale-independent double-to-string conversion
	std::ostringstream ostr;
	ostr.imbue(std::locale("C"));
	ostr << std::setprecision(prec) << std::fixed << in;
	return ostr.str();
}

json::json()
{
	JsonObject *obj = json_object_new();
	//IF_FAIL_VOID_TAG(obj, _E, "Json object construction failed");

	json_node = json_node_new(JSON_NODE_OBJECT);
	if (!json_node) {
		json_object_unref(obj);
		//_E("Json object construction failed");
	}

	json_node_set_object(json_node, obj);
	json_object_unref(obj);
}

json::json(const json& j)
{
	json_node = json_node_copy(j.json_node);
	//IF_FAIL_VOID_TAG(json_node, _E, "Json object construction failed");
}

json::json(const char* s)
{
	if (s) {
		parse(s);
	} else {
		parse(EMPTY_JSON_OBJECT);
	}
}

json::json(const std::string& s)
{
	if (s.empty()) {
		parse(EMPTY_JSON_OBJECT);
	} else {
		parse(s.c_str());
	}
}

json::~json()
{
	release();
}

void json::parse(const char* s)
{
	//gboolean result;
	JsonParser *parser = NULL;
	JsonNode *root = NULL;

	parser = json_parser_new();
	//IF_FAIL_VOID_TAG(parser, _E, "Memory allocation failed");

	//result = json_parser_load_from_data(parser, s, -1, NULL);
	//IF_FAIL_CATCH_TAG(result, _E, "Parsing failed");

	root = json_parser_get_root(parser);
	//IF_FAIL_CATCH_TAG(root, _E, "Getting root failed");

	json_node = json_node_copy(root);
	//IF_FAIL_CATCH_TAG(json_node, _E, "Copying failed");

/*CATCH:
	if (parser)
		g_object_unref(parser);*/
}

void json::release()
{
	if (json_node) {
		json_node_free(json_node);
		json_node = NULL;
	}
}

json& json::operator=(const json& j)
{
	release();
	json_node = json_node_copy(j.json_node);
	if (!json_node) {
	//	_E("Json object copy failed");
	}
	return *this;
}

json& json::operator=(const char* s)
{
	release();
	if (s) {
		parse(s);
	} else {
		parse(EMPTY_JSON_OBJECT);
	}
	return *this;
}

json& json::operator=(const std::string& s)
{
	release();
	if (s.empty()) {
		parse(EMPTY_JSON_OBJECT);
	} else {
		parse(s.c_str());
	}
	return *this;
}

bool json::operator==(const json& rhs)
{
	return node_equals(json_node, rhs.json_node);
}

bool json::operator!=(const json& rhs)
{
	return !operator==(rhs);
}

/* TODO
bool json::contains(const json& subset) const
{
	return false;
}
*/

char* json::dup_cstr()
{
	//IF_FAIL_RETURN_TAG(json_node, NULL, _E, "Json object not initialized");

	JsonGenerator *jgen = NULL;
	char *output = NULL;

	jgen = json_generator_new();
	//IF_FAIL_CATCH(jgen);

	json_generator_set_root(jgen, json_node);
	output = json_generator_to_data(jgen, NULL);
	//IF_FAIL_CATCH(output);

	g_object_unref(jgen);
	return output;

/*CATCH:
	if (jgen) {
		g_object_unref(jgen);
	}

	//_E("Memory allocation failed");
	return NULL;*/
}

std::string json::str()
{
	std::string output;
	char *_s = dup_cstr();
	//IF_FAIL_RETURN(_s, output = EMPTY_JSON_OBJECT);

	output = _s;
	g_free(_s);

	return output;
}

static char** tokenize_path(const char* path, int* length)
{
	//TODO: Re-implement this tokenizer using C++ stuff
	char** tokens;
	const char* pch;
	const char* begin;
	int i;
	//int j;
	int len;

	if (path == NULL || strlen(path) == 0) {
		*length = 0;
		return NULL;
	}

	*length = 1;

	for (pch = path; *pch != '\0'; pch++) {
		if (*pch == PATH_DELIM) {
			*length = *length + 1;
		}
	}

	tokens = static_cast<char**>(g_malloc((*length) * sizeof(char*)));
	//IF_FAIL_RETURN_TAG(tokens, NULL, _E, "Memory allocation failed");

	begin = path;
	i = 0;

	for (pch = path; ; pch++) {
		if (*pch == PATH_DELIM || *pch == '\0') {
			len = pch - begin;
			tokens[i] = static_cast<char*>(g_malloc((len+1) * sizeof(char)));
			//IF_FAIL_CATCH_TAG(tokens[i], _E, "Memory allocation failed");
			strncpy(tokens[i], begin, len);
			tokens[i][len] = '\0';
			i++;
			begin = pch + 1;
		}

		if (*pch == '\0') {
			break;
		}
	}

	return tokens;

/*CATCH:
	for (j=0; j<i; j++) {
		g_free(tokens[j]);
	}
	g_free(tokens);
	return NULL;*/
}

static void free_tokenized_path(int length, char** tokens)
{
	int i;
	if (tokens) {
		for (i=0; i<length; i++) {
			g_free(tokens[i]);
		}
		g_free(tokens);
	}
}

static JsonObject* traverse(JsonNode* jnode, const char* path, bool force)
{
	//IF_FAIL_RETURN_TAG(jnode, NULL, _E, "Invalid parameter");

	int length = 0;
	int depth = 0;
	char **path_token = NULL;
	JsonObject *jobj = NULL;
	JsonObject *child_obj = NULL;
	JsonNode *child_node = NULL;

	jobj = json_node_get_object(jnode);
	//IF_FAIL_RETURN(jobj, NULL);

	if (path) {
		path_token = tokenize_path(path, &length);
		//IF_FAIL_RETURN_TAG(path_token, NULL, _E, "Invalid path");
	}

	for (depth=0; depth<length; depth++) {
		if (!json_object_has_member(jobj, path_token[depth])) {
			if (force) {
				child_obj = json_object_new();
				//IF_FAIL_CATCH_TAG(child_obj, _E, "Memory allocation failed");
				json_object_set_object_member(jobj, path_token[depth], child_obj);
			} else {
				goto CATCH;
			}
		}
		child_node = json_object_get_member(jobj, path_token[depth]);
		//IF_FAIL_CATCH(child_node && json_node_get_node_type(child_node) == JSON_NODE_OBJECT);

		jobj = json_node_get_object(child_node);
		//IF_FAIL_CATCH(jobj);
	}

	free_tokenized_path(length, path_token);
	return jobj;

CATCH:
	free_tokenized_path(length, path_token);
	return NULL;
}

bool json::set(const char* path, const char* key, json& val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val.json_node, false, _E, "Invalid parameter");

	JsonObject *jobj = traverse(json_node, path, true);
	//IF_FAIL_RETURN(jobj, false);

	if (json_object_has_member(jobj, key))
		json_object_remove_member(jobj, key);

	json_object_set_member(jobj, key, val.json_node);
	val.json_node = NULL;
	val = json();

	return true;
}

bool json::set(const char* path, const char* key, int val)
{
	return set(path, key, static_cast<int64_t>(val));
}

bool json::set(const char* path, const char* key, int64_t val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key, false, _E, "Invalid parameter");

	JsonObject *jobj = traverse(json_node, path, true);
	//IF_FAIL_RETURN(jobj, false);

	if (json_object_has_member(jobj, key))
		json_object_remove_member(jobj, key);

	json_object_set_int_member(jobj, key, val);
	return true;
}

bool json::set(const char* path, const char* key, double val, int prec)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key, false, _E, "Invalid parameter");

	JsonObject *jobj = traverse(json_node, path, true);
	//IF_FAIL_RETURN(jobj, false);

	if (json_object_has_member(jobj, key)) {
		json_object_remove_member(jobj, key);
	}

	//NOTE: json-glib causes a precision issue while handling double values
	json_object_set_string_member(jobj, key, double_to_string(val, prec).c_str());
	return true;
}

bool json::set(const char* path, const char* key, std::string val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key, false, _E, "Invalid parameter");

	JsonObject *jobj = traverse(json_node, path, true);
	//IF_FAIL_RETURN(jobj, false);

	if (json_object_has_member(jobj, key)) {
		json_object_remove_member(jobj, key);
	}

	json_object_set_string_member(jobj, key, val.c_str());
	return true;
}

bool json::set(const char* path, const char* key, GVariant *val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val, false, _E, "Invalid parameter");

	const gchar *type_str = g_variant_get_type_string(val);
	//IF_FAIL_RETURN_TAG(type_str, false, _E, "GVariant manipulation failed");

	json_node_t *node = json_gvariant_serialize(val);
	//IF_FAIL_RETURN_TAG(node, false, _E, "GVariant manipulation failed");

	json gvar_json;
	gvar_json.set(NULL, GVAR_TYPES, type_str);
	json_object_set_member(json_node_get_object(gvar_json.json_node), GVAR_VALUES, node);

	return set(path, key, gvar_json);
}

bool json::remove(const char* path, const char* key)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key, false, _E, "Invalid parameter");

	JsonObject *jobj = traverse(json_node, path, true);
	//IF_FAIL_RETURN(jobj, false);

	if (json_object_has_member(jobj, key)) {
			json_object_remove_member(jobj, key);
	}

	return true;
}

bool json::get(const char* path, const char* key, json* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val, false, _E, "Invalid parameter");

	JsonObject *jobj = NULL;
	JsonNode *node = NULL;

	jobj = traverse(json_node, path, false);
	//IF_FAIL_RETURN(jobj && json_object_has_member(jobj, key), false);

	node = json_object_dup_member(jobj, key);
	//IF_FAIL_RETURN_TAG(node, false, _E, "Memory allocation failed");

	if (val->json_node) {
		json_node_free(val->json_node);
	}
	val->json_node = node;

	return true;
}

static JsonNode* search_value_node(JsonNode* jnode, const char* path, const char* key)
{
	JsonNode *node = NULL;
	JsonObject *jobj = NULL;
	//JsonNodeType ntype;

	jobj = traverse(jnode, path, false);
	//IF_FAIL_RETURN(jobj && json_object_has_member(jobj, key), NULL);

	node = json_object_get_member(jobj, key);
	//ntype = json_node_get_node_type(node);
	//IF_FAIL_RETURN(ntype == JSON_NODE_VALUE, NULL);

	return node;
}

bool json::get(const char* path, const char* key, int* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val, false, _E, "Invalid parameter");

	int64_t v;

	if (get(path, key, &v)) {
		*val = v;
		return true;
	}

	return false;
}

bool json::get(const char* path, const char* key, int64_t* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val, false, _E, "Invalid parameter");

	JsonNode *node = search_value_node(json_node, path, key);
	//IF_FAIL_RETURN(node, false);

	GType vtype = json_node_get_value_type(node);
	if (vtype == G_TYPE_INT64) {
		*val = json_node_get_int(node);
	} else if (vtype == G_TYPE_STRING) {
		//TODO: if the string is not a number?
		*val = static_cast<int64_t>(string_to_double(json_node_get_string(node)));
	} else {
		return false;
	}

	return true;
}

bool json::get(const char* path, const char* key, double* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val, false, _E, "Invalid parameter");

	JsonNode *node = search_value_node(json_node, path, key);
	//IF_FAIL_RETURN(node, false);

	GType vtype = json_node_get_value_type(node);
	if (vtype == G_TYPE_DOUBLE) {
		*val = json_node_get_double(node);
	} else if (vtype == G_TYPE_INT64) {
		*val = json_node_get_int(node);
	} else if (vtype == G_TYPE_STRING) {
		//NOTE: json-glib causes a precision issue while handling double values
		*val = string_to_double(json_node_get_string(node));
	} else {
		return false;
	}

	return true;
}

bool json::get(const char* path, const char* key, std::string* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val, false, _E, "Invalid parameter");

	JsonNode *node = search_value_node(json_node, path, key);
	//IF_FAIL_RETURN(node, false);

	//GType vtype = json_node_get_value_type(node);
	//IF_FAIL_RETURN(vtype == G_TYPE_STRING, false);

	const char *str_val = json_node_get_string(node);
	//IF_FAIL_RETURN_TAG(str_val, false, _E, "Getting string failed");

	*val = str_val;
	return true;
}

bool json::get(const char* path, const char* key, GVariant **val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val, false, _E, "Invalid parameter");

	//bool ret;
	json gvar_json;
	//ret =
	get(path, key, &gvar_json);
	//IF_FAIL_RETURN(ret, false);

	std::string gvar_types;
	//ret =
	gvar_json.get(NULL, GVAR_TYPES, &gvar_types);
	//IF_FAIL_RETURN(ret, false);

	json gvar_values;
	//ret =
	gvar_json.get(NULL, GVAR_VALUES, &gvar_values);
	//IF_FAIL_RETURN(ret, false);

	GError *gerr = NULL;
	*val = json_gvariant_deserialize(gvar_values.json_node, gvar_types.c_str(), &gerr);
	//HANDLE_GERROR(gerr);
	//IF_FAIL_RETURN(*val, false);

	return true;
}

static JsonArray* search_array(JsonNode* jnode, const char* path, const char* key, bool force)
{
	JsonNode *node = NULL;
	JsonArray *arr = NULL;
	JsonObject *jobj = NULL;

	jobj = traverse(jnode, path, force);
	///IF_FAIL_RETURN(jobj, NULL);

	if (!json_object_has_member(jobj, key)) {
		if (force) {
			arr = json_array_new();
			//IF_FAIL_RETURN_TAG(arr, NULL, _E, "Memory allocation failed");
			json_object_set_array_member(jobj, key, arr);
		} else {
			return NULL;
		}
	}
	node = json_object_get_member(jobj, key);
	//IF_FAIL_RETURN_TAG(node && json_node_get_node_type(node) == JSON_NODE_ARRAY,
		//	NULL, _W, "Type mismatched: %s", key);

	return json_node_get_array(node);
}

int json::array_get_size(const char* path, const char* key)
{
	//IF_FAIL_RETURN_TAG(this->json_node, -1, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key, -1, _E, "Invalid parameter");

	JsonArray *jarr = search_array(json_node, path, key, false);
	//IF_FAIL_RETURN_TAG(jarr, -1, _D, "Mismatched data type");

	return json_array_get_length(jarr);
}

bool json::array_append(const char* path, const char* key, json& val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val.json_node, false, _E, "Invalid parameter");

	JsonArray *arr = search_array(json_node, path, key, true);
	//IF_FAIL_RETURN(arr, false);

	json_array_add_element(arr, val.json_node);
	val.json_node = NULL;
	val = json();

	return true;
}

bool json::array_append(const char* path, const char* key, int val)
{
	return array_append(path, key, static_cast<int64_t>(val));
}

bool json::array_append(const char* path, const char* key, int64_t val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key, false, _E, "Invalid parameter");

	JsonArray *arr = search_array(json_node, path, key, true);
	//IF_FAIL_RETURN(arr, false);

	json_array_add_int_element(arr, val);
	return true;
}

bool json::array_append(const char* path, const char* key, double val, int prec)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key, false, _E, "Invalid parameter");

	JsonArray *arr = search_array(json_node, path, key, true);
	//IF_FAIL_RETURN(arr, false);

	//NOTE: json-glib causes a precision issue while handling double values
	json_array_add_string_element(arr, double_to_string(val, prec).c_str());
	return true;
}

bool json::array_append(const char* path, const char* key, std::string val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key, false, _E, "Invalid parameter");

	JsonArray *arr = search_array(json_node, path, key, true);
	//IF_FAIL_RETURN(arr, false);

	json_array_add_string_element(arr, val.c_str());
	return true;
}

static JsonNode* search_array_elem(JsonNode* jnode, const char* path, const char* key, int index)
{
	JsonArray *jarr = search_array(jnode, path, key, false);
	//IF_FAIL_RETURN_TAG(jarr, NULL, _W, "Mismatched data type");

	//int size = json_array_get_length(jarr);
	//IF_FAIL_RETURN(size > index, NULL);

	JsonNode *node = json_array_get_element(jarr, index);
	//IF_FAIL_RETURN_TAG(node, NULL, _E, "Failed to get an array element");

	return node;
}

bool json::array_set_at(const char* path, const char* key, int index, json& val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(val.json_node && key && index >= 0, false, _E, "Invalid parameter");

	JsonNode *node = search_array_elem(json_node, path, key, index);
	//IF_FAIL_RETURN_TAG(node, false, _W, "Out of range");
	//IF_FAIL_RETURN_TAG(json_node_get_node_type(node) == JSON_NODE_OBJECT, false, _E, "Type mismatched: %s[%d]", key, index);

	JsonObject *obj = json_node_get_object(val.json_node);
	//IF_FAIL_RETURN_TAG(obj, false, _E, "Getting object failed");

	json_node_set_object(node, obj);
	json_node_free(val.json_node);
	val.json_node = NULL;
	val = json();

	return true;
}

bool json::array_set_at(const char* path, const char* key, int index, int val)
{
	return array_set_at(path, key, index, static_cast<int64_t>(val));
}

bool json::array_set_at(const char* path, const char* key, int index, int64_t val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && index >= 0, false, _E, "Invalid parameter");

	JsonNode *node = search_array_elem(json_node, path, key, index);
	//IF_FAIL_RETURN_TAG(node, false, _W, "Out of range");
	//IF_FAIL_RETURN_TAG(json_node_get_node_type(node) == JSON_NODE_VALUE, false, _E, "Type mismatched: %s[%d]", key, index);
	//IF_FAIL_RETURN_TAG(json_node_get_value_type(node) == G_TYPE_INT64, false, _E, "Type mismatched: %s[%d]", key, index);

	json_node_set_int(node, val);
	return true;
}

bool json::array_set_at(const char* path, const char* key, int index, double val, int prec)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && index >= 0, false, _E, "Invalid parameter");

	JsonNode *node = search_array_elem(json_node, path, key, index);
	//IF_FAIL_RETURN_TAG(node, false, _W, "Out of range");
	//IF_FAIL_RETURN_TAG(json_node_get_node_type(node) == JSON_NODE_VALUE, false, _E, "Type mismatched: %s[%d]", key, index);
	//IF_FAIL_RETURN_TAG(json_node_get_value_type(node) == G_TYPE_STRING, false, _E, "Type mismatched: %s[%d]", key, index);

	json_node_set_string(node, double_to_string(val, prec).c_str());
	return true;
}

bool json::array_set_at(const char* path, const char* key, int index, std::string val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && index >= 0, false, _E, "Invalid parameter");

	JsonNode *node = search_array_elem(json_node, path, key, index);
	//IF_FAIL_RETURN_TAG(node, false, _W, "Out of range");
	//IF_FAIL_RETURN_TAG(json_node_get_node_type(node) == JSON_NODE_VALUE, false, _E, "Type mismatched: %s[%d]", key, index);
	//IF_FAIL_RETURN_TAG(json_node_get_value_type(node) == G_TYPE_STRING, false, _E, "Type mismatched: %s[%d]", key, index);

	json_node_set_string(node, val.c_str());
	return true;
}

bool json::get_array_elem(const char* path, const char* key, int index, json* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val && index >= 0, false, _E, "Invalid parameter");

	JsonNode *node = search_array_elem(json_node, path, key, index);
	//IF_FAIL_RETURN(node, false);

	JsonNode *node_copy = json_node_copy(node);
	//IF_FAIL_RETURN_TAG(node_copy, false, _E, "Memory allocation failed");

	if (val->json_node) {
		json_node_free(val->json_node);
	}
	val->json_node = node_copy;

	return true;
}

bool json::get_array_elem(const char* path, const char* key, int index, int* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val && index >= 0, false, _E, "Invalid parameter");

	int64_t v;
	if (get_array_elem(path, key, index, &v)) {
		*val = v;
		return true;
	}

	return false;
}

bool json::get_array_elem(const char* path, const char* key, int index, int64_t* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val && index >= 0, false, _E, "Invalid parameter");

	JsonNode *node = search_array_elem(json_node, path, key, index);
	//IF_FAIL_RETURN(node, false);

	//JsonNodeType ntype = json_node_get_node_type(node);
	//IF_FAIL_RETURN_TAG(ntype == JSON_NODE_VALUE, false, _E, "Type mismatched: %s", key);

	GType vtype = json_node_get_value_type(node);
	if (vtype == G_TYPE_INT64) {
		*val = json_node_get_int(node);
	} else if (vtype == G_TYPE_STRING) {
		*val = static_cast<int64_t>(string_to_double(json_node_get_string(node)));
	} else {
		//_E("Type mismatched: %s", key);
		return false;
	}

	return true;
}

bool json::get_array_elem(const char* path, const char* key, int index, double* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val && index >= 0, false, _E, "Invalid parameter");

	JsonNode *node = search_array_elem(json_node, path, key, index);
	//IF_FAIL_RETURN(node, false);

	//JsonNodeType ntype = json_node_get_node_type(node);
	//IF_FAIL_RETURN_TAG(ntype == JSON_NODE_VALUE, false, _E, "Type mismatched: %s", key);

	GType vtype = json_node_get_value_type(node);
	if (vtype == G_TYPE_DOUBLE) {
		*val = json_node_get_double(node);
	} else if (vtype == G_TYPE_INT64) {
		*val = json_node_get_int(node);
	} else if (vtype == G_TYPE_STRING) {
		//NOTE: json-glib causes a precision issue while handling double values
		*val = string_to_double(json_node_get_string(node));
	} else {
		//_E("Type mismatched: %s", key);
		return false;
	}

	return true;
}

bool json::get_array_elem(const char* path, const char* key, int index, std::string* val)
{
	//IF_FAIL_RETURN_TAG(this->json_node, false, _E, "Json object not initialized");
	//IF_FAIL_RETURN_TAG(key && val && index >= 0, false, _E, "Invalid parameter");

	JsonNode *node = search_array_elem(json_node, path, key, index);
	//IF_FAIL_RETURN(node, false);

	//JsonNodeType ntype = json_node_get_node_type(node);
	//IF_FAIL_RETURN_TAG(ntype == JSON_NODE_VALUE, false, _E, "Type mismatched: %s", key);

	//GType vtype = json_node_get_value_type(node);
	//IF_FAIL_RETURN_TAG(vtype == G_TYPE_STRING, false, _E, "Type mismatched: %s", key);

	const char *str_val = json_node_get_string(node);
	//IF_FAIL_RETURN_TAG(str_val, false, _E, "Getting string failed");

	*val = str_val;
	return true;
}

bool json::get_member_list(json_node_t* node, std::list<std::string>& list)
{
	//IF_FAIL_RETURN(node, false);
	list.clear();

	JsonObject *jobj = json_node_get_object(node);
	//IF_FAIL_RETURN_TAG(jobj, false, _E, "Getting json object failed");

	GList *members = json_object_get_members(jobj);
	//IF_FAIL_RETURN(members, true);

	for (GList *it = g_list_first(members); it; it = g_list_next(it)) {
		const char *key = static_cast<const char*>(it->data);
		if (!key) {
			list.clear();
			g_list_free(members);
			//_E("Member list extraction failed");
			return false;
		}

		list.push_back(key);
	}

	g_list_free(members);
	return true;
}

bool json::get_keys(std::list<std::string>* list)
{
	//IF_FAIL_RETURN_TAG(list, false, _E, "Invalid parameter");
	return get_member_list(json_node, *list);
}

bool json::node_equals(json_node_t* lhs, json_node_t* rhs)
{
	//IF_FAIL_RETURN(lhs && rhs, false);

	JsonNodeType ltype = json_node_get_node_type(lhs);
	//JsonNodeType rtype = json_node_get_node_type(rhs);
	//IF_FAIL_RETURN(ltype == rtype, false);

	switch (ltype) {
		case JSON_NODE_VALUE:
			//IF_FAIL_RETURN(value_equals(lhs, rhs), false);
			break;
		case JSON_NODE_OBJECT:
			//IF_FAIL_RETURN(object_equals(lhs, rhs), false);
			break;
		case JSON_NODE_ARRAY:
			//IF_FAIL_RETURN(array_equals(lhs, rhs), false);
			break;
		default:
			//_W("Unsupported type");
			return false;
	}

	return true;
}

bool json::value_equals(json_node_t* lhs, json_node_t* rhs)
{
	GType ltype = json_node_get_value_type(lhs);
	//GType rtype = json_node_get_value_type(rhs);
	//IF_FAIL_RETURN(ltype == rtype, false);

	switch (ltype) {
		case G_TYPE_INT64:
			return json_node_get_int(lhs) == json_node_get_int(rhs);
		case G_TYPE_DOUBLE:
			return json_node_get_double(lhs) == json_node_get_double(rhs);
		case G_TYPE_STRING:
			//return STR_EQ(json_node_get_string(lhs), json_node_get_string(rhs));
			return (strcmp(json_node_get_string(lhs), json_node_get_string(rhs)) == 0);
		default:
			//_W("Unsupported type");
			return false;
	}
}

bool json::object_equals(json_node_t* lhs, json_node_t* rhs)
{
	std::list<std::string> lm, rm;
	//IF_FAIL_RETURN(get_member_list(lhs, lm), false);
	//IF_FAIL_RETURN(get_member_list(rhs, rm), false);
	//IF_FAIL_RETURN(lm.size() == rm.size(), false);

	lm.sort();
	rm.sort();

	std::list<std::string>::iterator lit, rit;
	lit = lm.begin();
	rit = rm.begin();

	while (lit != lm.end()) {
		//IF_FAIL_RETURN(*lit == *rit, false);

		//json_node_t *lhs_child = json_object_get_member(json_node_get_object(lhs), (*lit).c_str());
		//json_node_t *rhs_child = json_object_get_member(json_node_get_object(rhs), (*rit).c_str());
		//IF_FAIL_RETURN(node_equals(lhs_child, rhs_child), false);

		++lit;
		++rit;
	}

	return true;
}

bool json::array_equals(json_node_t* lhs, json_node_t* rhs)
{
	/*JsonArray *larr =*/ json_node_get_array(lhs);
	/*JsonArray *rarr =*/ json_node_get_array(rhs);

	//int size = json_array_get_length(larr);
	//IF_FAIL_RETURN(size == static_cast<int>(json_array_get_length(rarr)), false);

/*	for (int i=0; i<size; ++i) {
		//json_node_t *lhs_child = json_array_get_element(larr, i);
		//json_node_t *rhs_child = json_array_get_element(rarr, i);
		//IF_FAIL_RETURN(node_equals(lhs_child, rhs_child), false);
	}*/

	return true;
}
