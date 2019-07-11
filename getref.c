/*
 * Copyright (C) 2016, 2017, 2018 "IoT.bzh"
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
#include <stdio.h>
#include <string.h>

#include <json-c/json.h>

#include "getref.h"

/**
 * Search for a reference of type "#/a/b/c" int the
 * parsed JSON root object
 */
int search$ref(struct json_object *root, const char *ref, struct json_object **result)
{
	char *d;
	struct json_object *i;

	/* does it match #/ at the beginning? */
	if (ref[0] != '#' || (ref[0] && ref[1] != '/')) {
		fprintf(stderr, "$ref invalid. Was: %s", ref);
		return 0;
	}

	/* search from root to target */
	i = root;
	d = strdupa(ref+2);
	d = strtok(d, "/");
	while(i && d) {
		if (!json_object_object_get_ex(i, d, &i))
			return 0;
		d = strtok(NULL, "/");
	}
	if (result)
		*result = i;
	return 1;
}

struct json_object *get$ref(struct json_object *root, const char *ref)
{
	struct json_object *result;
	return search$ref(root, ref, &result) ? result : NULL;
}
