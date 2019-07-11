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
#include <limits.h>
#include <assert.h>
#include <stdio.h>

#include <json-c/json.h>

static size_t s2c(const char *str, const char *prefix, int width, int lprf, char *out)
{
	char c, buf[4];
	size_t len;
	int i, pos;

#define P(x) do{ if (out) out[len] = (x); len++; pos++; }while(0)
	/* translate the string */
	len = pos = 0;
	for(;;) {
		/* get the char to convert */
		c = *str++;

		/* set buf with next char */
		switch(c) {
		case '\\':
			c = *str++;
			if (c) {
				if (c == '/') {
					/* remove ugly \/ put by json-c */
					buf[0] = '/';
					buf[1] = 0;
				} else {
					buf[0] = '\\';
					buf[1] = c;
					buf[2] = 0;
				}
				break;
			}
			/*@fallthrough@*/
		case 0:
			if (!len) P('"');
			if (!len || pos) {
				P('"');
				if (prefix) P('\n');
			}
			P(0);
			return len;
		case '"':
			buf[0] = '\\';
			buf[1] = '"';
			buf[2] = 0;
			break;
		case '\n':
			buf[0] = '\\';
			buf[1] = 'n';
			buf[2] = 0;
			break;
		case '\t':
			buf[0] = '\\';
			buf[1] = 't';
			buf[2] = 0;
			break;
		default:
			if (0 < c && c < ' ') {
				buf[0] = '\\';
				buf[1] = (char)('0' + ((c >> 3) & 7));
				buf[2] = (char)('0' + ((c >> 0) & 7));
				buf[3] = 0;
			} else {
				buf[0] = c;
				buf[1] = 0;
				break;
			}
			break;
		}
		/* add the char in the output */
		if (pos == 0) {
			for(i = 0 ; i < lprf ; i++)
				P(prefix[i]);
			P('"');
		}
		for(i = 0 ; buf[i] ; i++)
			P(buf[i]);
		if (pos >= width - 1) {
			P('"');
			P('\n');
			pos = 0;
		}
	}
	while(c);
#undef P
}

char *str2c(const char *str, const char *prefix, int width)
{
	size_t len;
	int lprf;
	char *out;

	/* ensure defaults */
	len = prefix ? strlen(prefix) : 0;
	lprf = len > INT_MAX ? INT_MAX : (int)len;
	width = width <= 0 || width - 2 <= lprf ? INT_MAX : width;

	/* compute final size*/
	len = s2c(str, prefix, width, lprf, NULL);

	/* allocate the memory */
	out = malloc(len);
	if (!out)
		return NULL;

	/* make the output */
	s2c(str, prefix, width, lprf, out);
	return out;
}

char *str2c_std(const char *str)
{
	return str2c(str, "\t", 71);
}

char *str2c_inl(const char *str)
{
	return str2c(str, 0, 0);
}

char *json2c(struct json_object *object, const char *prefix, int width)
{
	return str2c(json_object_to_json_string_ext(object, 0), prefix, width);
}

char *json2c_std(struct json_object *object)
{
	return str2c_std(json_object_to_json_string_ext(object, 0));
}

char *json2c_inl(struct json_object *object)
{
	return str2c_inl(json_object_to_json_string_ext(object, 0));
}
