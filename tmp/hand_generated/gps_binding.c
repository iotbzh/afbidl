#include "gps_binding.h"
#include "genutils.h"

static afb_event_t location_event;


// *************** MARSHALLING / UNMARSHALLING ********************************


// subscription_desc

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

static int serialize_location(const gps_location_t * location, json_object ** result) {
    if (!result)
        return 1;
    if (!location)
        return 2;
    if (!location->timestamp)
        return 3;
    // TODO : check rational exp match
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


// record/request and record/reply

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


static int serialize_record_reply(gps_record_request_t * record_reply, json_object ** result) {
    if (!result)
        return 1;
    if (!record_reply)
        return 2;
    if (!record_reply->filename)
        return 3;
    if (!check_pattern("gps_\d{4}\d{2}\d{2}_\d{2}\d{2}.log", record_reply->filename))
        return 4;

    *result = json_object_new_object();
    json_object_object_add(*result,
                           "filename",
                           json_object_new_string(record_reply->filename));
    return 0;
}





// ********************** BINDER's CALLBACK ***********************************

static void req_gps_subscribe_cb(afb_req_t request) {
    struct json_object * req = afb_req_json(request);
    const char * req_gps_subscribe_error_reason = NULL;
    gps_subsription_desc_t gps_subsription_desc;

    if (parse_subscription(req, &gps_subsription_desc, &req_gps_subscribe_error_reason) == 0) {
        if (gps_subscribe(&gps_subsription_desc, &req_gps_subscribe_error_reason) == 0) {
            afb_req_success(request, NULL, NULL);
        }
    }

    afb_req_fail(request, "failed", req_gps_subscribe_error_reason);
}


static void req_gps_unsubscribe_cb(afb_req_t request) {
    struct json_object * req = afb_req_json(request);
    const char * req_gps_unsubscribe_error_reason = NULL;
    gps_subsription_desc_t gps_subsription_desc;

    if (parse_subscription(req, &gps_subsription_desc, &req_gps_unsubscribe_error_reason) == 0) {
        if (gps_unsubscribe(&gps_subsription_desc, &req_gps_unsubscribe_error_reason) == 0) {
            afb_req_success(request, NULL, NULL);
        }
    }

    afb_req_fail(request, "failed", req_gps_unsubscribe_error_reason);
}


static void req_gps_location_cb(afb_req_t request) {
    static const char* req_gps_location_error_reason_1 = "format request is invalid: Must have no field";

    if (json_object_object_length(afb_req_json(request)) != 0) {
        afb_req_fail(request, "failed", req_gps_location_error_reason_1);
        return;
    }

    gps_location_t location;
    const char * req_gps_location_error_reason = NULL;
    if (gps_location(&location, &req_gps_location_error_reason) == 0) {

    }

    afb_req_fail(request, "failed", req_gps_unsubscribe_error_reason);
}


static void req_gps_record_cb((afb_req_t request) {
    struct json_object * req = afb_req_json(request);
    const char * req_gps_unsubscribe_error_reason = NULL;
    gps_record_request_t record_request;
    gps_record_reply_t record_reply;

    if (parse_record_request(req,
                           &record_request,
                           &req_gps_unsubscribe_error_reason) == 0) {
        if (gps_record(&gps_subsription_desc,
                            &record_reply,
                            &req_gps_unsubscribe_error_reason) == 0) {
            json_object * jresp = serialize_record_reply(&record_reply);
            afb_req_success(request, jresp, NULL);
        }
    }

    afb_req_fail(request, "failed", req_gps_unsubscribe_error_reason);
}


static int init(afb_api_t api)
{
    int ret;
    location_event = afb_daemon_make_event("location");

    return gps_user_init(api);
}

// Binder's definition.


static const struct afb_verb_v3 binding_verbs[] = {
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
    .verbs = binding_verbs,
    .init = init,
};
