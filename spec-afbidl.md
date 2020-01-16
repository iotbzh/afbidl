# API specification for AGL

Micro service architectures on web mostly use OpenAPI
for specifying and documenting their API (Application
Programming Interface).\
Following that use, AGL's binder provides a tool to
translate OpenAPI specifications to a code skeleton
written either in C or C++ languages.

However, the API description language OpenAPI never
provided some the requirement that AGL expected for
a such tool:

- Specify API that throws events
- Describe the permission based security model

Using OpenAPI also had the disadvantage of implying
some twist of the model and then some ugliness
verbosity.

Unfortunately, search for a replacement of OpenAPI
that would fullfil below requirements failed.

- Describe JSON data of APIs
- Describe events
- Describe permission based security (even as extension)
- Suitable for generating:
  - Documentation
  - Code skeleton with/without parameter validation
  - Automated test

Consequently, a new API specification formalism has
to be proposed.

This document is the proposal for that new formalism.\
For the best, that proposal includes advanced designs
introduced after discussions with Joël Champeau and
Philippe Dhaussy, researchers at ENSTA Bretagne (National
Institute of Advanced Technologies of Brittany) specialized
in system modeling and formal verification.

## The goals of specifying APIs

The micro service architecture of AGL and its flexible
IPC mechanism emphasis the decomposition of services or
applications in tiny cooperative parts.\
This has big advantages in terms of flexibility and
development process but, conversely, implies to
correctly document interfaces.\
Documenting or specifying API are the same thing, except
that, traditionally, specifying comes forward and
documenting afterward.

Specifying API can be done using simple text documents.\
Using text documents is great for humans but not for
computers.\
For this reason, because machines can't exploit human
structured texts, the use of simple text documents
should be avoided as much as possible.\
In effect, here is the list of all items that computers
can do based on API specifications:

- Automatic generation of documentation
   in many formats and using customizable styles

- Automatic generation of code either
   minimal or elaborate

- Automatic adaptation to transport backend or
   protocol

- Automatic generation of test cases

- Integration of advanced tools like supervision,
   tracing, spying

- Proof of system properties, in particular
   when assembling many API together

Many IDL (Interface Description Language) exist
but it rarely fit all that requirements.\
First of all, they generally are "languages",
meaning that they are difficult to parse, to
generate and to manipulate by tools.

OpenAPI is not a language.\
It is a specification made by a data structure
 of JSON format [1][json-org],[2][json-rfc].\
Using JSON has the advantage that no parser has
to be written because a standard one exists.\
JSON is not human friendly but its data model
is quasi-isomorph with the one of YAML [3][yaml]
that is more human friendly.

For this reasons, the below proposal describes
an API specification format based on YAML.

Nevertheless, for specifying the values expected
by APIs the format will use the JSON Schema formalism
[4][json-schema].\
This allows to describe complex
values and their constraints using a format easy to
integrate in tools.

[json-org]:    http://json.org/                    "JSON format"
[json-rfc]:    https://tools.ietf.org/html/rfc8259 "JSON format RFC"
[yaml]:        https://yaml.org/                   "YAML format"
[json-schema]: https://json-schema.org/            "JSON Schema"

## Content of API specifications

### Top level structure

An API specification has the following structure:

```yaml
%YAML 1.3
---
afbidl: "0.1"
info: # description the content of the specification
tools: # items for tools (doc, afb-genskel, ..)
verbs: # description of verbs of the API
events: # description of the events emitted by the API
state-machines: # description of the state machines of the API
examples: # examples of usage with or without timings
schemas: # place holder for description of the types of items
```

The specification is designed to describe only one API.\
If needed (example: simulation of a complex system), the aggregation
of multiple API descriptions can be done but externally with some
other description.

The heading line *%YAML 1.3* is recommended but not mandatory.

The main item of the description is an object. Its fields are:

- afbidl: this field indicates that the description follows that
   specification and the value precise what version of the specification
   is used. Current version is 0.1

- info: this field contains an object that give information about the
   API. Its mandatory fields are:

  - apiname: name of the API
  - title: short explanation of the API
  - description: long description of the API
  - version: version of the API

   Other fields are accepted, example:
  - author
  - maintainer
  - homepage
  - site
  - copyright
  - license
  - ...

- tools: this fields contains an object that can set properties for
   processing tools. The fields are the names of tool to setup.

- verbs: this field contains an object whose fields are the names
   of the verbs of the API. For each verb the value attached to the
   field of the verb is the description of the verb.

- events: this field contains an object whose fields are the names
   of the events thrown by the API. For each event the value attached
   to the field describes the event.

- state-machines: this field contains an object whose fields are the
   names of the state-machines of the API. For each state-machine the
   value attached to the field describes the state machine.

- examples: TO BE SPECIFIED - object of named sequences/scenarios -

- schemas: this optional field is intended to group the schema of
   the common types used by API.

### Describing verbs

The verbs are described using an object containing the fields

```yaml
title: # short explanation of the verb
description: # detailed description of the verb
permissions: # required permissions
request: # schema of the request parameters
reply: # describe the reply
```

### Describing events

The events are described using an object containing the fields

```yaml
schema: # description of the data associated with the event
when-state: # condition of emitting the event
set-state: # when the event is associated to a state change
```

### Describing state machine

#### Example of the API gps

```yaml
%YAML 1.3
---
afbidl: "0.1"

info:
  apiname: gps
  title: Service for geo location
  description:
        GPS service reports current WGS84 coordinates from GNSS devices
        via the gpsd application.
  version: "0.1"
  author: AGL
  maintainer: John Difool
  homepage: https://doc.automotivelinux.org/...

tools:

    afb-genskel:
        scope: static
        prefix: req_
        postfix: _cb
        init: init_gps

    doc:
        id: gps-api
        keywords: gps
        author:
        version:
        src_prefix: api-gps
        chapters:
        - name: Abstract
          url: abstract.md
        - name: User Guide
          url: userguide.md

verbs:

    subscribe:
        description: subscribe to gps/gnss events
        request: $/schemas/subscription-desc
        reply:
            success:
                schema: $/schemas/none
                set-state:
                    listening: yes

    unsubscribe:
        description: unsubscribe to gps/gnss events
        request: $/schemas/subscription-desc
        reply:
            success:
                schema: $/schemas/none
                set-state:
                    listening: no

    location:
        description: get current gps/gnss coordinates
        request: $/schemas/none
        reply:
            success: $/schemas/location
            _: An error can be returned when the service isn't ready

    record:
        description: |
            Entering *record* mode you must send **{"state": "on"}** with the **record**
            verb which will have a JSON response of **{"filename": "gps_YYYYMMDD_hhmm.log"}**
            pointing to log under *app-data/agl-service-gps*

            Now to enter *replaying* mode you must symlink or copy a GPS dump to
            *app-data/agl-service-gps/recording.log* and restart the service.
            From then on out the previously recorded GPS data will loop infinitely
            which is useful for testing or demonstration purposes.
        request: $/schemas/record/request
        reply:
            success:
                schema: $/schemas/record/reply
                set-state:
                    recording: yes
            _: An error can be returned when the service isn't ready

events:
    location:
        schema: $/schemas/location
        when-state:
            listening: yes

state-machines:
    listening:
        states: [ no, yes ]
        initial: no
    recording:
        states: [ no, yes ]
        initial: no

# Follow JsonSchema specification (https://json-schema.org/)
schemas:
    subscription-desc:
        title: Description of the event subscribed or unsubscribed
        type: object
        properties:
            value: { enum: [ location ] }
        required: [ value ]

    location:
        title: the location
        type: object
        properties:
            altitude:
                title: the altitude in meters above the normal geoid
                type: number
                minimum: -20000
                maximum: 20000
            latitude:
                title: the latitude in degrees
                type: number
                minimum: -90
                maximum: 90
            longitude:
                title: the longitude in degrees
                type: number
                minimum: -180
                maximum: 180
            speed:
                title: the speed in meter per seconds m/s
                type: number
                minimum: 0
                maximum: 6000
            track:
                title: the heading in degrees
                type: number
                minimum: 0
                maximum: 360
            timestamp:
                title: time stamp of the location as a ISO8601 date
                type: string #ISO8601
                pattern: \d{4,}-[01][0-9]-[0-3][0-9]T[012][0-9]:[0-5][0-9]:[0-5][0-9].*

    record:
        request:
            type: object
            properties:
                state: { const: "on" }
            required: [ state ]

        reply:
            type: object
            properties:
                filename:
                    title: the name of the file that records the data of format gps_YYYYMMDD_hhmm.log
                    type: string
                    pattern: gps_\d{4}\d{2}\d{2}_\d{2}\d{2}.log
            required: [ filename ]

    none:
        title: no value, just null
        const: null
```
