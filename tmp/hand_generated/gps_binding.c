#include "gps_binding.h"
#include "genutils.h"

static afb_event_t location_event;

typedef struct opaque_location_context_s {
    deallocator_t timestamp_deallocator;
} opaque_location_context_t;

typedef struct opaque_record_reply_context_s {
    deallocator_t filename_deallocator;
} opaque_record_reply_context_t;

typedef union opaque_context_u {
    opaque_location_context_t location;
    opaque_record_reply_context_t record_reply;
} opaque_context_t;

// *************** MARSHALLING / UNMARSHALLING ********************************


// subscription_desc

// TODO: turn this into a hashmap ?
// Binary search (dichotomy) ? => must be sorted. TBD
// TODO: generated version SHOULD provide size
// TODO: extrn forward declaration in the header, definition in .c
const char *gps_subsription_desc_value_enum[] = {
    "location"
};


static int parse_subscription_desc(json_object *request, gps_subsription_desc_t *result, const char **parse_error) {
    static const char* parse_subscription_desc_reason_1 = "format request is invalid";
    static const char* parse_subscription_desc_reason_2 = "request must include field 'value'";
    static const char* parse_subscription_desc_reason_3 = "request field 'value' must be a string";
    static const char* parse_subscription_desc_reason_4 = "request field 'value' must be in ['location',]";

    json_object * gps_subsription_desc_value = NULL;

    if (json_object_object_length(request) != 1) {
        *parse_error = parse_subscription_desc_reason_1;
        return 1;
    }

    gps_subsription_desc_value = json_object_object_get(request, "value");
    if (gps_subsription_desc_value == NULL) {
        *parse_error = parse_subscription_desc_reason_2;
        return 2;
    }

    if (json_object_get_type(gps_subsription_desc_value) != json_type_string) {
        *parse_error = parse_subscription_desc_reason_3;
        return 3;
    }

    const char *value = json_object_get_string(gps_subsription_desc_value);
    if (!str_is_in(value, gps_subsription_desc_value_enum)) {
        *parse_error = parse_subscription_desc_reason_4;
        return 4;
    }
    result->value = value;
    return 0;
}

static int serialize_subscription_desc(const gps_subsription_desc_t * subscription_desc, json_object ** result) {
    if (!result)
        return 1;
    if (!subscription_desc)
        return 2;
    if (!subscription_desc->value)
        return 3;
    if (!str_is_in(subscription_desc->value, gps_subsription_desc_value_enum))
        return 4;
    *result = json_object_new_object();
    json_object_object_add(*result,
                           "value",
                           json_object_new_string(subscription_desc->value));
    return 0;
}


// location

/*
int parse_location(json_object * request, gps_location_t * result, const char ** parse_error) {
    // ...
}
*/

const char *gps_location_timestamp_pattern = "gps_\\d{4}-[01][0-9]-[0-3][0-9]T[012][0-9]:[0-5][0-9]:[0-5][0-9].*";

static int serialize_location(const gps_location_t * location, json_object ** result) {
    if (!result)
        return 1;
    if (!location)
        return 2;
    if (!location->timestamp)
        return 3;
    if (!check_pattern(gps_location_timestamp_pattern, location->timestamp))
        return 4;
    *result = json_object_new_object();
    json_object_object_add(*result,
                           "altitude",
                           json_object_new_double(location->altitude));
    json_object_object_add(*result,
                           "latitude",
                           json_object_new_double(location->latitude));
    json_object_object_add(*result,
                           "longitude",
                           json_object_new_double(location->longitude));
    json_object_object_add(*result,
                           "speed",
                           json_object_new_double(location->speed));
    json_object_object_add(*result,
                           "track",
                           json_object_new_double(location->track));
    json_object_object_add(*result,
                           "timestamp",
                           json_object_new_string(location->timestamp));
    // who's reading my comments ? https://www.youtube.com/watch?v=PD3RCwNseqQ
    return 0;
}


void set_gps_location_timestamp(afb_req_t req, gps_location_t *location, char *timestamp, void (*deallocator)(void*)) {
    opaque_context_t *context = afb_req_context_get(req);

    if (!context) {
        context = malloc(sizeof(opaque_context_t));
        afb_req_context_set(req, context, free);
    }

    context->location.timestamp_deallocator = deallocator;
    location->timestamp = timestamp;
}

// record/request and record/reply

const char *gps_record_request_state_enum[] = {
    "on"
};

static int parse_record_request(json_object *request, gps_record_request_t *result, const char **parse_error) {
    static const char* parse_record_request_reason_1 = "format request is invalid";
    static const char* parse_record_request_reason_2 = "request must include field 'state'";
    static const char* parse_record_request_reason_3 = "request field 'state' must be a string";
    static const char* parse_record_request_reason_4 = "request field 'state' must be in ['location',]";

    json_object * gps_subsription_desc_state = NULL;

    if (json_object_object_length(request) != 1) {
        *parse_error = parse_record_request_reason_1;
        return 1;
    }

    gps_subsription_desc_state = json_object_object_get(request, "state");
    if (gps_subsription_desc_state == NULL) {
        *parse_error = parse_record_request_reason_2;
        return 2;
    }

    if (json_object_get_type(gps_subsription_desc_state) != json_type_string) {
        *parse_error = parse_record_request_reason_3;
        return 3;
    }

    const char *state = json_object_get_string(gps_subsription_desc_state);
    if (!str_is_in(state, gps_record_request_state_enum)) {
        *parse_error = parse_record_request_reason_4;
        return 4;
    }
    result->state = state;
    return 0;
}


// const char *gps_record_reply_filename_pattern = "gps_[:digit:]{4}[:digit:]{2}[:digit:]{2}_[:digit:]{2}[:digit:]{2}.log";
const char *gps_record_reply_filename_pattern =  "gps_[0-9]{4}[0-9]{2}[0-9]{2}_[0-9]{2}[0-9]{2}.log";

static int serialize_record_reply(gps_record_reply_t * record_reply, json_object ** result) {
    if (!result)
        return 1;
    if (!record_reply)
        return 2;
    if (!record_reply->filename)
        return 3;
    if (!check_pattern(gps_record_reply_filename_pattern, record_reply->filename))
        return 4;

    *result = json_object_new_object();
    json_object_object_add(*result,
                           "filename",
                           json_object_new_string(record_reply->filename));
    return 0;
}


void set_gps_record_reply_filename(afb_req_t req, gps_record_reply_t *reply, char *filename, void (*deallocator)(void*)) {
    opaque_context_t *context = afb_req_context_get(req);

    if (!context) {
        context = malloc(sizeof(opaque_context_t));
        afb_req_context_set(req, context, free);
    }

    context->record_reply.filename_deallocator = deallocator;
    reply->filename = filename;
}



// ********************** BINDER's CALLBACK ***********************************

static void req_gps_subscribe_cb(afb_req_t request) {
    struct json_object * req = afb_req_json(request);
    const char * req_gps_subscribe_error_reason = NULL;
    gps_subsription_desc_t gps_subsription_desc;

    if (parse_subscription_desc(req,
                                &gps_subsription_desc,
                                &req_gps_subscribe_error_reason) == 0) {
        gps_subscribe(&gps_subsription_desc);
        afb_req_success(request, NULL, NULL);
    }

    afb_req_fail(request, "failed", req_gps_subscribe_error_reason);
}


static void req_gps_unsubscribe_cb(afb_req_t request) {
    struct json_object * req = afb_req_json(request);
    const char * req_gps_unsubscribe_error_reason = NULL;
    gps_subsription_desc_t gps_subsription_desc;

    if (parse_subscription_desc(req,
                                &gps_subsription_desc,
                                &req_gps_unsubscribe_error_reason) == 0) {
        gps_unsubscribe(&gps_subsription_desc);
        afb_req_success(request, NULL, NULL);
    }

    afb_req_fail(request, "failed", req_gps_unsubscribe_error_reason);
}


static void req_gps_location_cb(afb_req_t request) {
    static const char* req_gps_location_error_reason_1 = "format request is invalid: Must be empty";
    opaque_context_t *context = NULL;

    if (json_object_object_length(afb_req_json(request)) != 0) {
        afb_req_fail(request, "failed", req_gps_location_error_reason_1);
        goto cleaning;
    }


    gps_location_t location;
    const char * req_gps_location_error_reason = NULL;
    if (gps_location(request, &location) == 0) {
        json_object * jresp = NULL;
        if (serialize_location(&location, &jresp)) {
            afb_req_success(request, jresp, NULL);
            goto cleaning;
        } else {
            req_gps_location_error_reason = "ERROR : could not serialize location";
        }
    }

    afb_req_fail(request, "failed", req_gps_location_error_reason);

  cleaning:
    context = afb_req_context_get(request);
    if (context) {
        if (context->location.timestamp_deallocator) {
            context->location.timestamp_deallocator(location.timestamp);
        }
    }
}


static void req_gps_record_cb(afb_req_t request) {
    struct json_object * req = afb_req_json(request);
    const char * req_gps_record_error_reason = NULL;
    gps_record_request_t record_request;
    gps_record_reply_t record_reply;
    opaque_context_t *context = NULL;

    if (parse_record_request(req,
                             &record_request,
                             &req_gps_record_error_reason) == 0) {
        if (gps_record(request,
                       &record_request,
                       &record_reply) == 0) {
            json_object * jresp = NULL;
            int rc = -1;
            if (serialize_record_reply(&record_reply, &jresp) == 0) {
                afb_req_success(request, jresp, NULL);
                goto cleaning;
            } else {
                printf("Serialisation error :%d\n", rc);
                req_gps_record_error_reason = "ERROR : could not serialize record/reply";
            }
        }
    }

    afb_req_fail(request, "failed", req_gps_record_error_reason);

  cleaning:
    context = afb_req_context_get(request);
    if (context) {
        if (context->record_reply.filename_deallocator) {
            context->record_reply.filename_deallocator(record_reply.filename);
        }
    }
}

extern afb_api_t the_api;

static int gps_init(afb_api_t api)
{
    location_event = afb_daemon_make_event("location");
    the_api = api;

    return gps_user_init(api);
}

// Binder's definition.


static const struct afb_verb_v3 gps_binding_verbs[] = {
    { .verb = "location",    .callback = req_gps_location_cb,     .info = "Get GNSS data" },
    { .verb = "record",      .callback = req_gps_record_cb,       .info = "Record GPS data" },
    { .verb = "subscribe",   .callback = req_gps_subscribe_cb,    .info = "Subscribe to GNSS events" },
    { .verb = "unsubscribe", .callback = req_gps_unsubscribe_cb,  .info = "Unsubscribe to GNSS events" },
    { }
};

/*
 * binder API description
 */
const struct afb_binding_v3 afbBindingV3 = {
    .api = "gps",
    .specification = "GNSS/GPS API",
    .verbs = gps_binding_verbs,
    .init = gps_init,
};
