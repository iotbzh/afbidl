#include "gps_binding.h"
#include "genutils.h"

static afb_event_t location_event;

int  parse_subscription(json_object *request, gps_subsription_desc_t *result, const char **parse_error) {
    static const char* req_gps_subscribe_error_reason_1 = "format request is invalid";
    static const char* req_gps_subscribe_error_reason_2 = "request must include field 'value'";
    static const char* req_gps_subscribe_error_reason_3 = "request field 'value' must be a string";
    static const char* req_gps_subscribe_error_reason_4 = "request field 'value' must be in ['location',]";

    json_object * gps_subsription_desc_value = NULL;

    if (json_object_object_length(request) != 1) {
        *parse_error = req_gps_subscribe_error_reason_1;
        return 1;
    }

    gps_subsription_desc_value = json_object_object_get(request, "value");
    if (gps_subsription_desc_value == NULL) {
        *parse_error = req_gps_subscribe_error_reason_2;
        return 1;
    }

    if (json_object_get_type(gps_subsription_desc_value) != json_type_string) {
        *parse_error = req_gps_subscribe_error_reason_3;
        return 1;
    }

    const char *value = json_object_get_string(gps_subsription_desc_value);
    if (!str_is_in(value, gps_subsription_desc_value_enum)) {
        *parse_error = req_gps_subscribe_error_reason_4;
        return 1;
    }
    result->value = value;
    return 0;
}

int serialize_subscription_desc(const gps_subsription_desc_t * subscription_desc, json_object ** result) {
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



/*
int parse_location(json_object * request, gps_location_t * result, const char ** parse_error) {
    // ...
}
*/

int serialize_location(const gps_location_t * location, json_object ** result) {
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
    return 0;
}




void req_gps_subscribe_cb(afb_req_t request) {
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


void req_gps_unsubscribe_cb(afb_req_t request) {
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


void req_gps_location_cb(afb_req_t request) {
    static const char* req_gps_location_error_reason_1 = "format request is invalid: Must have no field";

    if (json_object_object_length(afb_req_json(request)) != 0) {
        afb_req_fail(request, "failed", req_gps_location_error_reason_1);
        return;
    }

    gps_location_t location;
    const char * req_gps_location_error_reason = NULL;
    gps_location(&location, &req_gps_location_error_reason);

}


/*
 * Test to see if in demo mode first, then enable if not enable gpsd
 */

static int init(afb_api_t api)
{
    int ret;
    location_event = afb_daemon_make_event("location");

    return 0;
}

static const struct afb_verb_v3 binding_verbs[] = {
    { .verb = "location",    .callback = req_gps_location_cb,     .info = "Get GNSS data" },
    //{ .verb = "record",      .callback = record,       .info = "Record GPS data" },
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
