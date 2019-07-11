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

#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>

#include <json-c/json.h>

#include "getref.h"
#include "exprefs.h"
#include "json2c.h"

#define TEST(x)   ((x) && *(x))
#define OOM(x) do{if(!(x)){fprintf(stderr,"out of memory\n");exit(1);}}while(0)

/**
 * root of the JSON being parsed
 */
int version = 3;
struct json_object *root = NULL;
struct json_object *d_perms = NULL;
struct json_object *a_perms = NULL;
const char *preinit = NULL;
const char *init = NULL;
const char *onevent = NULL;
const char *api = NULL;
const char *scope = NULL;
const char *prefix = NULL;
const char *postfix = NULL;
const char *provideclass = NULL;
const char *requireclass = NULL;
const char *requireapi = NULL;
const char *info = NULL;
char *capi = NULL;
int priv = -1;
int noconc = -1;
int cpp = 0;
enum idl {
	idl_afbidl = 0,
	idl_openapi = 1
} idl = idl_openapi;



/* create c name by replacing non alpha numeric characters with underscores */
char *cify(const char *str)
{
	char *r = strdup(str);
	int i = 0;
	while (r && r[i]) {
		if (!isalnum(r[i]))
			r[i] = '_';
		i++;
	}
	return r;
}

/* get the permission odescription if set */
struct json_object *permissions_of_verb(struct json_object *obj)
{
	struct json_object *x, *y;

	if (idl == idl_afbidl)
		return json_object_object_get_ex(obj, "permissions", &x) ? x : NULL;

	if (json_object_object_get_ex(obj, "x-permissions", &x))
		return x;

	if (json_object_object_get_ex(obj, "get", &x))
		if (json_object_object_get_ex(x, "x-permissions", &y))
			return y;

	return NULL;
}

/* output the array of permissions */
void print_perms()
{
	int i, n;
	const char *fmtstr = cpp ? "\t%s" : "\t{ %s }";

	n = a_perms ? (int)json_object_array_length(a_perms) : 0;
	if (n) {
		printf("static const struct afb_auth _afb_auths_%s[] = {\n" , capi);
		i = 0;
		while (i < n) {
			printf(fmtstr, json_object_get_string(json_object_array_get_idx(a_perms, i)));
			printf(",\n"+(++i == n));
		}
		printf("};\n\n");
	}
}

/*
 * search in the global object 'd_perm' the computed representation
 * of the permission described either by 'obj' or 'desc'
 */
struct json_object *new_perm(struct json_object *obj, const char *desc)
{
	const char *tag;
	char *b;
	struct json_object *x, *y;

	tag = obj ? json_object_to_json_string_ext(obj, 0) : desc;
	if (!json_object_object_get_ex(d_perms, tag, &y)) {

		/* creates the d_perms dico and the a_perms array */
		if (!d_perms) {
			d_perms = json_object_new_object();
			a_perms = json_object_new_array();
		}

		/* creates the reference in the structure */
		asprintf(&b, "&_afb_auths_%s[%d]", capi, (int)json_object_array_length(a_perms));
		x = json_object_new_string(desc);
		y = json_object_new_string(b);
		json_object_array_add(a_perms, x);
		json_object_object_add(d_perms, tag, y);
		free(b);
	}
	return y;
}

struct json_object *decl_perm(struct json_object *obj);

enum optype { And, Or };

/* recursive declare and/or permissions */
struct json_object *decl_perm_a(enum optype op, struct json_object *obj)
{
	int i, n;
	char *a;
	const char *opstr, *fmtstr;
	struct json_object *x, *y;

	if (cpp) {
		fmtstr = "afb::auth_%s(%s, %s)";
		opstr = op==And ? "and" : "or";
	} else {
		fmtstr = ".type = afb_auth_%s, .first = %s, .next = %s";
		opstr = op==And ? "And" : "Or";
	}
	x = NULL;
	i = n = obj ? (int)json_object_array_length(obj) : 0;
	while (i) {
		y = decl_perm(json_object_array_get_idx(obj, --i));
		if (!y)
			;
		else if (!x)
			x = y;
		else if (x != y) {
			asprintf(&a, fmtstr, opstr, json_object_get_string(y), json_object_get_string(x));
			x = new_perm(NULL, a);
			free(a);
		}
	}
	return x;
}

/* declare the permission for obj */
struct json_object *decl_perm(struct json_object *obj)
{
	char *a;
	const char *fmt;
	struct json_object *x, *y;

	if (json_object_object_get_ex(d_perms, json_object_to_json_string_ext(obj, 0), &x))
		return x;

	if (json_object_object_get_ex(obj, "permission", &x)) {
		if (cpp)
			fmt = "afb::auth_permission(\"%s\")";
		else
			fmt = ".type = afb_auth_Permission, .text = \"%s\"";
		asprintf(&a, fmt, json_object_get_string(x));
		y = new_perm(obj, a);
		free(a);
	}
	else if (json_object_object_get_ex(obj, "anyOf", &x)) {
		y = decl_perm_a(Or, x);
	}
	else if (json_object_object_get_ex(obj, "allOf", &x)) {
		y = decl_perm_a(And, x);
	}
	else if (json_object_object_get_ex(obj, "not", &x)) {
		x = decl_perm(x);
		if (cpp)
			fmt = "afb::auth_not(%s)";
		else
			fmt = ".type = afb_auth_Not, .first = %s";
		asprintf(&a, fmt, json_object_get_string(x));
		y = new_perm(obj, a);
		free(a);
	}
	else if (json_object_object_get_ex(obj, "LOA", &x))
		y = NULL;
	else if (json_object_object_get_ex(obj, "session", &x))
		y = NULL;
	else
		y = NULL;

	return y;
}

void declare_permissions(const char *name, struct json_object *obj)
{
	struct json_object *p;

	p = permissions_of_verb(obj);
	if (p)
		decl_perm(p);
}


#define SESSION_CLOSE  0x000001
#define SESSION_RENEW  0x000010
#define SESSION_CHECK  0x000100
#define SESSION_LOA_1  0x001000
#define SESSION_LOA_2  0x011000
#define SESSION_LOA_3  0x111000
#define SESSION_MASK   0x111111


int get_session(struct json_object *obj);

int get_session_a(int and, struct json_object *obj)
{
	int i, n, x, y;

	n = obj ? (int)json_object_array_length(obj) : 0;
	if (n == 0)
		return 0;

	i = n;
	x = get_session(json_object_array_get_idx(obj, --i));
	while (i) {
		y = get_session(json_object_array_get_idx(obj, --i));
		if (and)
			x &= y;
		else
			x |= y;
	}
	return x;
}

int get_session(struct json_object *obj)
{
	int y;
	const char *a;
	struct json_object *x;

	y = 0;
	if (json_object_object_get_ex(obj, "anyOf", &x)) {
		y = get_session_a(1, x);
	}
	else if (json_object_object_get_ex(obj, "allOf", &x)) {
		y = get_session_a(0, x);
	}
	else if (json_object_object_get_ex(obj, "not", &x)) {
		y = ~get_session(x) & SESSION_MASK;
	}
	else if (json_object_object_get_ex(obj, "LOA", &x)) {
		switch (json_object_get_int(x)) {
		case 3: y = SESSION_LOA_3; break;
		case 2: y = SESSION_LOA_2; break;
		case 1: y = SESSION_LOA_1; break;
		default: break;
		}
	}
	else if (json_object_object_get_ex(obj, "session", &x)) {
		a = json_object_get_string(x);
		if (!strcmp(a, "check"))
			y = SESSION_CHECK;
		else if (!strcmp(a, "close"))
			y = SESSION_CLOSE;
	}
	else if (json_object_object_get_ex(obj, "token", &x)) {
		a = json_object_get_string(x);
		if (!strcmp(a, "refresh"))
			y = SESSION_RENEW;
	}

	return y;
}

void print_session(struct json_object *p)
{
	int s, c, l;

	s = p ? get_session(p) : 0;
	c = 1;
	if (s & SESSION_CHECK) {
		printf("%s", "|AFB_SESSION_CHECK" + c);
		c = 0;
	}
	if (s & SESSION_LOA_3 & ~SESSION_LOA_2)
		l = 3;
	else if (s & SESSION_LOA_2 & ~SESSION_LOA_1)
		l = 2;
	else if (s & SESSION_LOA_1)
		l = 1;
	else
		l = 0;
	if (l) {
		printf("%s%d", "|AFB_SESSION_LOA_" + c, l);
		c = 0;
	}
	if (s & SESSION_CLOSE) {
		printf("%s", "|AFB_SESSION_CLOSE" + c);
		c = 0;
	}
	if (s & SESSION_RENEW) {
		printf("%s", "|AFB_SESSION_REFRESH" + c);
		c = 0;
	}
	if (c)
		printf("AFB_SESSION_NONE");
}

void print_verb(const char *name)
{
	printf("%s%s%s" , prefix, name, postfix);
}

void print_declare_verb(const char *name, struct json_object *obj)
{
	if (TEST(scope))
		printf("%s ", scope);
	printf("void ");
	print_verb(name);
	printf("(afb_req_t req);\n");
}

void print_struct_verb(const char *name, struct json_object *obj)
{
	struct json_object *p, *i;
	const char *info;

	info = NULL;
	if (json_object_object_get_ex(obj, "description", &i))
		info = json_object_get_string(i);

	p = permissions_of_verb(obj);
	printf(
		"    {\n"
		"        .verb = \"%s\",\n"
		"        .callback = "
		, name
	);
	print_verb(name);
	printf(
		",\n"
		"        .auth = %s,\n"
		"        .info = %s,\n"
		, p && decl_perm(p) ? json_object_get_string(decl_perm(p)) : "NULL"
		, info ? str2c_inl(info) : "NULL"
	);
	if (version == 3)
		printf(
			"        .vcbdata = NULL,\n"
		);
	printf(
		"        .session = "
	);
	print_session(p);
	if (version == 3)
		printf(
			",\n"
			"        .glob = 0"
		);
	printf(
		"\n"
		"    },\n"
	);
}

void getvarbool(int *var, const char *path, int defval)
{
	struct json_object *o;

	if (*var != 0 && *var != 1) {
		o = get$ref(root, path);
		if (o && json_object_is_type(o, json_type_boolean))
			*var = json_object_get_boolean(o);
		else
			*var = !!defval;
	}
}

void getvar(const char **var, const char *path, const char *defval)
{
	struct json_object *o;

	if (!*var) {
		o = get$ref(root, path);
		if (o && json_object_is_type(o, json_type_string))
			*var = json_object_get_string(o);
		else
			*var = defval;
	}
}

/******************************************************************************/

void openapi_getvars()
{
	getvar(&api, "#/info/x-binding-c-generator/api", NULL);
	getvar(&preinit, "#/info/x-binding-c-generator/preinit", NULL);
	getvar(&init, "#/info/x-binding-c-generator/init", NULL);
	getvar(&onevent, "#/info/x-binding-c-generator/onevent", NULL);
	getvar(&scope, "#/info/x-binding-c-generator/scope", "static");
	getvar(&prefix, "#/info/x-binding-c-generator/prefix", "afb_verb_");
	getvar(&postfix, "#/info/x-binding-c-generator/postfix", "_cb");
	getvar(&provideclass, "#/info/x-binding-c-generator/provide-class", NULL);
	getvar(&requireclass, "#/info/x-binding-c-generator/require-class", NULL);
	getvar(&requireapi, "#/info/x-binding-c-generator/require-api", NULL);
	getvarbool(&priv, "#/info/x-binding-c-generator/private", 0);
	getvarbool(&noconc, "#/info/x-binding-c-generator/noconcurrency", 0);
	getvar(&api, "#/info/title", "?");
	getvar(&info, "#/info/description", NULL);
}

void openapi_enum_verbs(void (*func)(const char *name, struct json_object *obj))
{
	struct json_object_iterator ji, jn;
	struct json_object *paths, *obj;
	const char *name;

	/* search the verbs */
	paths = get$ref(root, "#/paths");
	if (!paths)
		return;

	/* list the verbs and sort it */
	ji = json_object_iter_begin(paths);
	jn = json_object_iter_end(paths);
	while (!json_object_iter_equal(&ji, &jn)) {
		name = json_object_iter_peek_name(&ji);
		obj = json_object_iter_peek_value(&ji);
		name += (*name == '/');
		func(name, obj);
		json_object_iter_next(&ji);
	}
}

/******************************************************************************/

void afbidl_getvars()
{
	getvar(&preinit, "#/tools/afb-genskel/preinit", NULL);
	getvar(&init, "#/tools/afb-genskel/init", NULL);
	getvar(&onevent, "#/tools/afb-genskel/onevent", NULL);
	getvar(&scope, "#/tools/afb-genskel/scope", "static");
	getvar(&prefix, "#/tools/afb-genskel/prefix", "afb_verb_");
	getvar(&postfix, "#/tools/afb-genskel/postfix", "_cb");
	getvar(&provideclass, "#/tools/afb-genskel/provide-class", NULL);
	getvar(&requireclass, "#/tools/afb-genskel/require-class", NULL);
	getvar(&requireapi, "#/tools/afb-genskel/require-api", NULL);
	getvarbool(&priv, "#/tools/afb-genskel/private", 0);
	getvarbool(&noconc, "#/tools/afb-genskel/noconcurrency", 0);
	getvar(&api, "#/info/apiname", NULL);
	getvar(&api, "#/info/title", "?");
	getvar(&info, "#/info/description", NULL);
}

void afbidl_enum_verbs(void (*func)(const char *name, struct json_object *obj))
{
	struct json_object_iterator ji, jn;
	struct json_object *verbs, *obj;
	const char *name;

	/* search the verbs */
	verbs = get$ref(root, "#/verbs");
	if (!verbs)
		return;

	/* list the verbs and sort it */
	ji = json_object_iter_begin(verbs);
	jn = json_object_iter_end(verbs);
	while (!json_object_iter_equal(&ji, &jn)) {
		name = json_object_iter_peek_name(&ji);
		obj = json_object_iter_peek_value(&ji);
		func(name, obj);
		json_object_iter_next(&ji);
	}
}

/******************************************************************************/

void detectidl()
{
	struct json_object *o;

	o = get$ref(root, "#/openapi");
	if (o) {
		idl = idl_openapi;
		return;
	}
	o = get$ref(root, "#/afbidl");
	if (o) {
		idl = idl_afbidl;
		return;
	}
}

/**
 * process a file and prints its expansion on stdout
 */
void process(char *filename)
{
	char *desc;

	void (*getvars)();
	void (*enum_verbs)(void (*)(const char*, struct json_object*));

	/* translate - */
	if (!strcmp(filename, "-"))
		filename = "/dev/stdin";

	/* check access */
	if (access(filename, R_OK)) {
		fprintf(stderr, "can't access file %s\n", filename);
		exit(1);
	}

	/* read the file */
	root = json_object_from_file(filename);
	if (!root) {
		fprintf(stderr, "reading file %s produced null\n", filename);
		exit(1);
	}

	/* create the description (before expanding $ref ) */
	desc = json2c_std(root);

	/* expand references */
	root = exp$refs(root);

	/* detect the idl */
	detectidl();
	switch(idl) {
	default:
	case idl_afbidl:
		getvars = afbidl_getvars;
		enum_verbs = afbidl_enum_verbs;
		break;
	case idl_openapi:
		getvars = openapi_getvars;
		enum_verbs = openapi_enum_verbs;
		break;
	}

	/* get some names */
	getvars();
	capi = cify(api);

	/* get the API name */
	printf(
		"\n"
		"static const char _afb_description_%s[] =\n"
		"%s"
		";\n"
		"\n"
		, capi, desc
	);
	enum_verbs(declare_permissions);
	print_perms();
	enum_verbs(print_declare_verb);
	printf(
		"\n"
		"static const struct afb_verb_v%d _afb_verbs_%s[] = {\n"
                , version, capi
	);
	enum_verbs(print_struct_verb);
	printf(
		"    {\n"
		"        .verb = NULL,\n"
		"        .callback = NULL,\n"
		"        .auth = NULL,\n"
		"        .info = NULL,\n"
	);
	if (version == 3)
		printf(
			"        .vcbdata = NULL,\n"
		);
	printf(
		"        .session = 0"
	);
	if (version == 3)
		printf(
			",\n"
			"        .glob = 0"
		);
	printf(
		"\n"
		"    }\n"
		"};\n"
	);

	if (TEST(preinit) || TEST(init) || TEST(onevent)) {
		printf("\n");
		if (TEST(preinit)) {
			if (TEST(scope)) printf("%s ", scope);
			printf("int %s(%s);\n", preinit, version==3 ? "afb_api_t api" : "");
		}
		if (TEST(init)) {
			if (TEST(scope)) printf("%s ", scope);
			printf("int %s(%s);\n", init, version==3 ? "afb_api_t api" : "");
		}
		if (TEST(onevent)) {
			if (TEST(scope)) printf("%s ", scope);
			printf("void %s(%sconst char *event, struct json_object *object);\n",
					onevent, version==3 ? "afb_api_t api, " : "");
		}
	}

	printf(
		"\n"
		"%sconst struct afb_binding_v%d %s%s = {\n"
		"    .api = \"%s\",\n"
		"    .specification = _afb_description_%s,\n"
		"    .info = %s,\n"
		"    .verbs = _afb_verbs_%s,\n"
		"    .preinit = %s,\n"
		"    .init = %s,\n"
		"    .onevent = %s,\n"
		, priv ? "static " : ""
		, version
		, priv ? "_afb_binding_" : version==3 ? "afbBindingV3" : "afbBindingV2"
		, priv ? capi : ""
		, api
		, capi
		, info ? str2c_inl(info) : "NULL"
		, capi
		, TEST(preinit) ? preinit : "NULL"
		, TEST(init) ? init : "NULL"
		, TEST(onevent) ? onevent : "NULL"
	);


	if (version == 3)
		printf(
			"    .userdata = NULL,\n"
			"    .provide_class = %s%s%s,\n"
			"    .require_class = %s%s%s,\n"
			"    .require_api = %s%s%s,\n"
			, TEST(provideclass) ? "\"" : "", TEST(provideclass) ? provideclass : "NULL", TEST(provideclass) ? "\"" : ""
			, TEST(requireclass) ? "\"" : "", TEST(requireclass) ? requireclass : "NULL", TEST(requireclass) ? "\"" : ""
			, TEST(requireapi) ? "\"" : "", TEST(requireapi) ? requireapi : "NULL", TEST(requireapi) ? "\"" : ""
		);


	printf(
		"    .noconcurrency = %d\n"
		"};\n"
		"\n"
		, !!noconc
	);

	/* clean up */
	json_object_put(root);
	free(desc);
}

/** process the list of files or stdin if none */
int main(int ac, char **av)
{
	int r, w;
	av++;

	r = w = 0;
	while (av[r]) {
		if (!(strcmp(av[r], "-x") && strcmp(av[r], "--cpp"))) {
			cpp = 1;
			r++;
		} else if (!strcmp(av[r], "-2")) {
			version = 2;
			r++;
		} else if (!strcmp(av[r], "-3")) {
			version = 3;
			r++;
		} else {
			av[w++] = av[r++];
		}
	}
	av[w] = NULL;
	if (!*av)
		process("-");
	else {
		do { process(*av++); } while(*av);
	}
	return 0;
}

