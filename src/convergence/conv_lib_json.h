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

#ifndef __CONV_LIB_JSON_H__
#define __CONV_LIB_JSON_H__

#include <sys/types.h>
#include <glib.h>
#include <string>
#include <list>
//#include "common.h"

#define _J(cmt, jobj) \
do { \
	_SD("%s: %s", (cmt), jobj.str().c_str()); \
} while(0)

#define EMPTY_JSON_OBJECT	"{}"
#define DEFAULT_PRECISION	3

class json {
private:
	typedef struct _JsonNode json_node_t;
	json_node_t *json_node;
public:
	void parse(const char* s);
private:
	void release();

	/* For json vs json comparison */
	bool get_member_list(json_node_t* node, std::list<std::string>& list);
	bool node_equals(json_node_t* lhs, json_node_t* rhs);
	bool value_equals(json_node_t* lhs, json_node_t* rhs);
	bool object_equals(json_node_t* lhs, json_node_t* rhs);
	bool array_equals(json_node_t* lhs, json_node_t* rhs);

public:
	json();
	json(const char* s);
	json(const std::string& s);

	/* This json(const json& j) only copies the reference to the underlying json node.
	 * Therefore, changes applied to a json object affect the other.
	 * If you need to create a 'real' copy of a json, which can be manipulated separately,
	 * utilize the str() function, e.g., ctx::json copy(original.str());
	 */
	json(const json& j);

	~json();

	json& operator=(const char* s);
	json& operator=(const std::string& s);

	/* This operator=(const json& j) only copies the reference to the underlying json node.
	 * Therefore, changes applied to a json object affect the other.
	 * If you need to create a 'real' copy of a json, which can be manipulated separately,
	 * utilize the str() function, e.g., ctx::json copy = original.str();
	 */
	json& operator=(const json& j);

	bool operator==(const json& rhs);
	bool operator!=(const json& rhs);

	char* dup_cstr();
	std::string str();

	bool get_keys(std::list<std::string>* list);

	bool set(const char* path, const char* key, json& val);
	bool set(const char* path, const char* key, int val);
	bool set(const char* path, const char* key, int64_t val);
	bool set(const char* path, const char* key, double val, int prec = DEFAULT_PRECISION);
	bool set(const char* path, const char* key, std::string val);
	bool set(const char* path, const char* key, GVariant *val);

	bool get(const char* path, const char* key, json* val);
	bool get(const char* path, const char* key, int* val);
	bool get(const char* path, const char* key, int64_t* val);
	bool get(const char* path, const char* key, double* val);
	bool get(const char* path, const char* key, std::string* val);
	bool get(const char* path, const char* key, GVariant **val);

	bool remove(const char* path, const char* key);

	int array_get_size(const char* path, const char* key);

	bool array_append(const char* path, const char* key, json& val);
	bool array_append(const char* path, const char* key, int val);
	bool array_append(const char* path, const char* key, int64_t val);
	bool array_append(const char* path, const char* key, double val, int prec = DEFAULT_PRECISION);
	bool array_append(const char* path, const char* key, std::string val);

	bool array_set_at(const char* path, const char* key, int index, json& val);
	bool array_set_at(const char* path, const char* key, int index, int val);
	bool array_set_at(const char* path, const char* key, int index, int64_t val);
	bool array_set_at(const char* path, const char* key, int index, double val, int prec = DEFAULT_PRECISION);
	bool array_set_at(const char* path, const char* key, int index, std::string val);

	bool get_array_elem(const char* path, const char* key, int index, json* val);
	bool get_array_elem(const char* path, const char* key, int index, int* val);
	bool get_array_elem(const char* path, const char* key, int index, int64_t* val);
	bool get_array_elem(const char* path, const char* key, int index, double* val);
	bool get_array_elem(const char* path, const char* key, int index, std::string* val);
};

#endif // __CONV_LIB_JSON_H__

