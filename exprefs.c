/*
 * Copyright (C) 2016-2019 "IoT.bzh"
 * Author José Bollo <jose.bollo@iot.bzh>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * This simple program expands the object { "$ref": "#/path/to/a/target" }
 *
 * For example:
 *
 *  {
 *    "type":{
 *      "a": "int",
 *      "b": { "$ref": "#/type/a" }
 *    }
 *  }
 *
 * will be exapanded to
 *
 *  {
 *    "type":{
 *      "a": "int",
 *      "b": "int"
 *    }
 *  }
 *
 * Invocation:   program  [file|-]...
 *
 * without arguments, it reads the input.
 */

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>

#include <json-c/json.h>

#include "getref.h"
#include "exprefs.h"

/**
 * records path to the expanded node
 */
struct path
{
	struct json_object *object;	/**< node being expanded */
	const struct path *upper;	/**< link to upper expanded nodes */
};

/**
 * Returns the top object
 */
static inline struct json_object *top(const struct path *path)
{
	while (path->upper)
		path = path->upper;
	return path->object;
}

/**
 * Expands the node designated by path and returns its expanded form
 */
static struct json_object *expand(const struct path *upper)
{
	struct path here;
	struct json_object *object, *x;
	int n, i;
	struct json_object_iterator ji, jn;

	/* expansion depends of the type of the node */
	here.upper = upper;
	object = upper->object;
	switch (json_object_get_type(object)) {
	case json_type_object:
		/* for object, look if it contains a property "$ref" */
		if (json_object_object_get_ex(object, "$ref", &here.object)) {
			/* check that "$ref" value is a string */
			if (!json_object_is_type(here.object, json_type_string)) {
				fprintf(stderr, "found a $ref not being string. Is: %s\n", json_object_get_string(here.object));
				exit(1);
			}
			/* yes, reference, try to substitute its target */
			i = search$ref(top(upper), json_object_get_string(here.object), &x);
			if (!i) {
				fprintf(stderr, "$ref not found. Was: %s\n", json_object_get_string(here.object));
				exit(1);
			}
			/* check not recursive */
			upper = &here;
			while(upper) {
				if (x == upper->object) {
					fprintf(stderr, "$ref recursive. Was: %s\n", json_object_get_string(here.object));
					exit(1);
				}
				upper = upper->upper;
			}
			/* found. return a new instance of the target */
			return x;
		}
		/* no, expand the values */
		ji = json_object_iter_begin(object);
		jn = json_object_iter_end(object);
		while (!json_object_iter_equal(&ji, &jn)) {
			here.object = json_object_iter_peek_value(&ji);
			x = expand(&here);
			if (x != here.object)
				json_object_object_add(object, json_object_iter_peek_name(&ji), json_object_get(x));
			json_object_iter_next(&ji);
		}
		break;
	case json_type_array:
		/* expand the values of arrays */
		i = 0;
		n = (int)json_object_array_length(object);
		while (i != n) {
			here.object = json_object_array_get_idx(object, i);
			x = expand(&here);
			if (x != here.object)
				json_object_array_put_idx(object, i, json_object_get(x));
			i++;
		}
		break;
	default:
		/* otherwise no expansion */
		break;
	}
	/* return the given node */
	return object;
}

struct json_object *exp$refs(struct json_object *root)
{
	struct path top = { .object = root, .upper = NULL };
	return expand(&top);
}

static int is_tree(struct json_object *object, const struct path *upper)
{
	struct path here;
	int n, i;
	struct json_object_iterator ji, jn;

	switch (json_object_get_type(object)) {
	case json_type_object:
	case json_type_array:
		/* check recursive */
		here.upper = upper;
		while (upper) {
			if (upper->object == object)
				return 0;
			upper = upper->upper;
		}
		here.object = object;
		switch (json_object_get_type(object)) {
		case json_type_object:
			ji = json_object_iter_begin(object);
			jn = json_object_iter_end(object);
			while (!json_object_iter_equal(&ji, &jn)) {
				if (!is_tree(json_object_iter_peek_value(&ji), &here))
					return 0;
				json_object_iter_next(&ji);
			}
			break;
		case json_type_array:
			i = 0;
			n = (int)json_object_array_length(object);
			while (i != n) {
				if (!is_tree(json_object_array_get_idx(object, i), &here))
					return 0;
				i++;
			}
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	return 1;
}

int exp$refs_is_tree(struct json_object *root)
{
	return is_tree(root, NULL);
}
