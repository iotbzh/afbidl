#include <stdio.h>
#include <time.h> /* for struct tm, localtime_r */
#include <sys/time.h> /* for gettimeofday */
#include "gps_binding.h"


// TODO : record/filname is gps_<timestamp>.log, not ONLY timestamp !

//typedef char timestamp_t[29];
static const char timestamp_length = 29;

// timestamp_t test_ts = "YYYY-MM-ddTHH:mm:ss.SSS+0000";

void fill_timestamp(char* timestamp) {
// adapted from:
// https://github.com/jordansissel/experiments/blob/master/c/time/iso8601.c
  struct timeval tv;
  struct tm tm;


  /* Get the current time at high precision; could also use clock_gettime() for
   * even higher precision times if we want it. */
  gettimeofday(&tv, NULL);

  /* convert to time to 'struct tm' for use with strftime */
  localtime_r(&tv.tv_sec, &tm);

  /* format the time */
  strftime(timestamp, timestamp_length, "%Y-%m-%dT%H:%M:%S,000%z", &tm);
  printf("intermediate : %s\n", timestamp);
  /* but, since strftime() can't subsecond precision, we have to hack it
   * in manually. '20' is the string offset of the subsecond value in our
   * timestamp string. Also, because sprintf always writes a null, we have to
   * write the subsecond value as well as the rest of the string already there.
   */
  sprintf(timestamp + 20, "%03ld%s", tv.tv_usec / 1000, timestamp + 23);
}



void gps_subscribe(const gps_subsription_desc_t* subscription_desc) {
    //afb_event_subscribe(...);
    printf("gps_subscribe: %s\n", subscription_desc->value);
}

void gps_unsubscribe(const gps_subsription_desc_t* subscription_desc) {
    //afb_event_subscribe(...);
    printf("gps_unsubscribe: %s\n", subscription_desc->value);
}

int gps_location(afb_req_t req, /*out*/gps_location_t * location) {
    char *currtime = malloc(/*sizeof(timestamp_t)*/timestamp_length);
    location->altitude = 25.6;
    location->latitude = 47.747678;
    location->longitude = -3.357529;
    location->speed = 0.0;
    location->track = 42.0;
    fill_timestamp(currtime);
    set_gps_location_timestamp(req, location, currtime, free);
    return gps_location_success;

    // return gps_location_error
}

int gps_record(afb_req_t req,
               /*in*/const gps_record_request_t * record_request,
               /*out*/gps_record_reply_t * record_reply) {
    char datetime[14];
    struct timeval tv;
    struct tm tm;


    /* Get the current time at high precision; could also use clock_gettime() for
    * even higher precision times if we want it. */
    gettimeofday(&tv, NULL);

    /* convert to time to 'struct tm' for use with strftime */
    localtime_r(&tv.tv_sec, &tm);

    /* format the time */
    strftime(datetime, 14, "%Y%m%d_%H%M", &tm);
    printf("datetime : %s\n", datetime);


    char *fn = malloc(22);
    snprintf(fn, 22, "gps_%s.log", datetime);
    printf("filename : %s\n", fn);
    set_gps_record_reply_filename(req, record_reply, fn, free);

    return gps_record_success;

    //return gps_record_not_ready;
}

int gps_user_init(afb_api_t api) {
    AFB_API_NOTICE(api,"GPS API gps_user_init() : do nothing !");
    return 0;
}
