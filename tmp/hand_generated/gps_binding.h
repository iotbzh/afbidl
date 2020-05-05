#ifndef _GPS_BINDING_SKELETON_H_
#define _GPS_BINDING_SKELETON_H_

#include <stdio.h>
// #include <json_object.h>
#define AFB_BINDING_VERSION 3
#include <afb/afb-binding.h>

// TODO : const char* vs char*
// is it enough if the struct itself is const ?
// char* stinks malloc()...

// the location
typedef struct gps_location_s {
    // the altitude in meters above the normal geoide
    double altitude; // [-20000 .. 20000]
    // the latitude in degrees
    double latitude; // [-90 .. 90]
    // the longitude in degrees
    double longitude; // [-180 .. 180]
    // the speed in meter per seconds m/s
    double speed; // [0 .. 6000]
    // the heading in degrees
    double track; //[0 .. 360]
    // time stamp of the location as a ISO8601 date
    char * timestamp; // gps_\d{4,}-[01][0-9]-[0-3][0-9]T[012][0-9]:[0-5][0-9]:[0-5][0-9].*
} gps_location_t;



typedef void (*deallocator_t)(void*);
// special value for deallocator_t
#define NO_DEALLOCATOR (void(*0)(void))

void set_gps_location_timestamp(afb_req_t req, gps_location_t *location, char *timestamp, deallocator_t deallocator);


/*****************************************************************
 * NOTE : the 'value' in gps_subsription_desc_t points to one of *
 *        the item of gps_subsription_desc_value_enum array      *
 *****************************************************************/

// Description of the event subscribed or unsubscribed
typedef struct gps_subsription_desc_s {
    const char *value;
} gps_subsription_desc_t;

typedef struct gps_record_request_s {
    const char *state; // "on", mandatory
} gps_record_request_t;

typedef struct gps_record_reply_s {
    // the name of the file that records the data of format gps_YYYYMMDD_hhmm.log
    char *filename; // gps_\d{4}\d{2}\d{2}_\d{2}\d{2}.log, mandatory
} gps_record_reply_t;


void set_gps_record_reply_filename(afb_req_t req, gps_record_reply_t *reply, char *filename, void (*deallocator)(void*));



// ======== USER IMPLEMENTED FUNCTIONS ==============================

// Functions to be implemented by the binding.
// MUST return 0 if ok (error_reason will be ignored, SHOULD be NULL)
// Any other value <room to more spec here>, and error_reason SHOULD point to a char* that survives the function scope (not an automatic variable!).
// error reason is forwarded in verb's reply.


void gps_subscribe(const gps_subsription_desc_t * subscription_desc);

void gps_unsubscribe(const gps_subsription_desc_t * subscription_desc);

typedef enum {
    gps_location_success = 0,
    gps_location_error = 1
} gps_location_returncode_t;

int gps_location(afb_req_t request,
                 /*out*/gps_location_t * location);

typedef enum {
    gps_record_success = 0,
    gps_record_not_ready = 1
} gps_record_returncode_t;

int gps_record(afb_req_t request,
               /*in*/const gps_record_request_t * record_request,
               /*out*/gps_record_reply_t * record_reply);

// special function called at binfding init
int gps_user_init(afb_api_t api);

#endif /*_GPS_BINDING_SKELETON_H_*/
