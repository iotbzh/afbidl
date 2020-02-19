# Study for a formalism aimed to APIs description in the context of AGL

## Abstract

This document is a proposal of a formalism that allows to specify, define
and document AGL bindings APIs (Application Programming Interfaces).
We first define our needs, search for a candidate in the existing FOSS
pool, and eventually come up a custom solution (based on existing and
validated pieces of Free Software).

## Terminology

In this document we use the term **API Specification** as broad term, that
can actually be refined in more specific tasks : **Definition**,
**Specification**, and **Documentation**.

They are all related, but also different entities, with their respective
purposes.

Historically, and still often nowadays, **Specification** is done in a human,
natural language, with focus on high-level aspects, behavior and
responsibilities...

**Definition** refers to more fine-grained level, often describing the
messages exchanged by the API, using a more machine-oriented formalism that
allows automated processing.

Finally **Documentation** is aimed at developers, API users. It will often
compile information found in the former, synthesized in a convenient way,
for instance as a set of HTML pages, with hyperlinks, that can be conveniently
read in a web browser.

The formalism we propose in this document address those three aspect in a unified
and (hopefully) convenient way, making the distinction kind of obsolete.

### RFC 2119

Especially when written in capital letters,

> The key words "MUST", "MUST NOT", "REQUIRED", "SHALL", "SHALL
> NOT", "SHOULD", "SHOULD NOT", "RECOMMENDED",  "MAY", and
> "OPTIONAL" in this document are to be interpreted as described in
> RFC 2119.

**TO BE COMPLETED** : This MUST be achieved for a definitive version (1.0 and
further).
This SHALL be mostly correct for previous versions.

## Requirements for AGL

Well, First off: is API specification useful for AGL ?

The answer is a resounding *"YES !"*

Indeed, the micro service architecture of AGL and its flexible IPC mechanism
emphasizes on the decomposition of services or applications in tiny cooperative
parts. This has big advantages in terms of flexibility and development process,
but, subsequently, implies to correctly document interfaces.

In our opinion, we should avoid specifying using natural human language as
much as possible. We should favor the use of a machine-oriented formalism to
describe most of (if not all) the aspects of an API.

### Task automation

Doing so enables automation of various tasks such as :

- Generation of documentation (in many formats and using customizable styles)
- Generation of code (either minimal or elaborated)
- Adaptation to transport backend or protocol
- Generation of test cases
- Integration of advanced tools like supervision, tracing, spying
- Proof of system properties, in particular when assembling many API together

### Expected features

The formalism SHALL, at least allows to fulfill the following features:

- Describing JSON data of APIs
- Specifying APIs that throw events (expected, or spurious)
- Describing the internal states and their transitions (state-machines)
- Describing the security model based on permissions
- Being Suitable for the generation of:
  - Documentation in various formats:
    - HTML
    - pdf
    - markdown
    - ...
  - Code skeleton various languages (C, Python...) with optional features;
    - parameter validation
    - marshalling / unmarshalling
    - ...
  - Automated tests
  - ...

## Evaluation of existing tools

### Interface Description Languages

With the raise of distributed computing &#8212; using IPCs (InterProcess
Communication) and/or RPCs (Remote Procedure Calls) mechanisms &#8212; urged the
need for a way to describe the Programming Interfaces provided by an arbitrary
computing node.

Indeed, while a language like C, or Java, is self-descriptive at its source code
level, distributed computing can imply various hardware and software technologies
with no common standard. Furthermore, calls are often serialized into generic
protocol messages (such as TCP, UNIX sockets, ...) with no intrinsic structure.

IDL is an attempt to address such a problem. Various IDLs came up with different
approaches, and they have been maturing with years.

They are now found with a clear predominance in Web-based technologies.

### Older IDLs

One of the first shot was RPC language from Sun Microsystem, used for
implementing NFS (Network File System) using RPC (Remote Procedure Call).

An other early shot was released with the CORBA 2 standard, where the need
for interfacing various technologies (mainly C++ and Java) led to an
implementation-agnostic description formalism of the interfaces provided by
system components ("agnostic" just meaning "regardless of their respective
actual implementation").

CORBA 2 IDL already allowed some automatic processing, but unlike modern
IDLs made of structured data trees, CORBA's IDL is a "full-featured" language
than need a compiler-like program to be interpreted (which is not easy to
write and quite heavy to maintain). Also, for the record, CORBA used a
specific binary format for message marshalling.

By the meantime, WebServices (based on XML RPCs over HTTP, aka SOAP) and the
next decade RESTFul APIs (so-called "Web Applications") spread, with their own
needs of describing node interfaces for the sake of interoperability.

WSDL (Web Service Description Language) was used to describe SOAP interfaces.

On the other hand, various IDLs like WADL (Web Application Description
Language) and RAML (Restful Application Modeling Language), and
swagger/OpenAPI, among others were aimed at Web Applications (RESTFul APIs).

It is worth to note is that from the beginning and until now, each new RPC
technology came up with its own specific IDL (whenever it provides one !). 
And, although all IDLs are all sharing common concepts, no unifying / general
purpose language made any major break-through...

### State-of-the-art IDLs

By time passed, as from 2010 or so, JSON ([1][json-org], [2][json-rfc]) started
to widely replace XML for structured data representation in the messages
exchanges. Various IDLs evolved to reflect this change.

As AGL uses JSON for message (verb request and replies, events) payload
encoding, using JSON (or another equivalent format) to describes the messages
payload format sounds attractive.

As per JIRA SPEC-1903 [3][spec1903], 3 main IDLs were identified.

All of them are using JSON (or equivalent format to describe interfaces):

- OpenAPI 3.0 [4][openapi]
- asyncAPI 1.2 [5][asyncapi]
- GENIVI Franca [6][franca]
- Others

#### OpenAPI

OpenAPI is not a language, unlike many IDL, but rather describes structured
data, in JSON format.

Micro service architectures on the Web mostly use OpenAPI for specifying and
documenting their API. For this reason, it was a good candidate to evaluate.

Sadly, however, the API description language in OpenAPI never provided some of
the features required/expected by AGL for a such tool. It mainly lacks features
to describe event aspects and the security model has mismatches.

#### AsyncAPI

This IDL is very similar to OpenAPI. It add features for event management.
But it also doesn't fit much the requirements.

#### GENIVI Franca

GENIVI Franca is very close to our expectations in terms of features, but also
closely tighten to eclipse / Xtext[7][xtext].

#### Others

Many IDL (Interface Description Language) exist but they rarely fit all AGL
requirements.

First of all, they generally are "languages", meaning that they are difficult
to parse, to generate and to manipulate by tools. 

Many of them are also type oriented and this typing is not convenient when
the generation of code targets more than one language.

## Proposal of a new formalism

Although no suitable solution is available off the shelf, this research taught
its lessons and some ideas stemmed from those investigations.

First, both of them are using structured data representation to encode the API
description, allowing JSON and more human-friendly format as YAML.

This proposal includes advanced designs introduced after some discussions with
Joël Champeau and Philippe Dhaussy, researchers at ENSTA Bretagne (National
Institute of Advanced Technologies of Brittany) specialized in system modeling
and formal verification: description of internal states, scenarios of interaction.
All of this have counterpart in UML design methodology.

### JSON Schema

One of the goals of specifying our APIs is to manipulate AGL binding verbs
payloads (validation, parsing, etc). As a matter of fact, this payload
is encoded in JSON.

As stated above, other IDLs like OpenAPI and AsyncAPI do rely on JSON-Schema
[8][json-schema] for this kind of task (NB : a subset extension for OpenAPI,
a super-set for AsyncAPI).

JSON-Schemas enables a normalized description of the syntax a given chuck of
JSON. The schema itself being written in JSON, (or can be converted to) it can
be, in turn, validated against a so-called "meta-schema". Meta-schemas are also
JSON-Schemas, but a "special" kind as they also validate themselves.
It allows to describe complex values and their constraints using a format easy
to integrate in tools.

One big advantage of using JSON Schema is that it is extensively used, so it
comes with a whole ecosystem of tools (parsers, validators, ...), that can be
taken off-the-shelf and used as-is, as part of a bigger system.

JSON-Schema can't be used to describe those whole API but focuses on
describing messages structure (request and replies "schemas").

### Toward even more human-friendliness

JSON is a bit more human-friendly than its predecessors like XML, but can
still be a bit tedious to deal with. Another format like YAML [9][yaml] has a
softer syntax that avoid the need of managing opening and closing tokens (often
curly braces `{...}` ). 

As stated previously, we'll rely, internally, on JSON and JSON-Schemas, but
it's worth to note that their is a quasi-isomorphism between YAML and JSON data
models.

Comments are an exception, as strict JSON forbids them, whereas YAML welcomes
them quite happily. If a user writes down comments in the YAML "source"
specification file, they'll be lost while converting to JSON but it's not
really a show-stopper, as we don't need comments to achieve its processing.

We still need to define a technical solution to report an error at the very
line that caused it _in the source file_. Something like a *reverse lookup
table"* indexing source YAML line number, for each line in the converted JSON.

**TO BE DEFINED** : the very specific tool chosen may have some impact here.

**NOTE**: It may happen that some line in the JSON doesn't have an equivalent;
for instance, a closing curly brace that closes an object will have an
equivalent line in the YAML source. This is not problem because that type of
line should NOT be a source of error  given the YAML is syntactically valid,
and the converter works fine.

There are example of such YAML (and other) to JSON and vice-versa converters
like any-json [10][any-json].

## API description file structure

The API specification document will be written in YAML, JSON or any
equivalent, supported format. The list is still to be defined.

A proposition is to use any-json (see their website for the supported format
list)

### JSON objects

As a reminder, JSON defines an `object` as:
> an unordered collection of name–value pairs where the names (also called
> keys) are `strings`. Objects are intended to represent associative arrays,
> where each key is unique within an object.

*Source:* *[12][wikip-json]*

As `objects` can be nested inside another `object` item value, they are
convenient to represent trees.

At a syntax level, the API description document consist at its root level of
an `object`, which items are called **sections**. The value of each section
consists of an `object`, which content depends on the section it describes
(see below).

### afbidl document sections

API description document describe one and only API. 

As a reminder, on the other hand an AGL bindin cann implements multiple 
interfaces.

The API description document SHOULD/MUST provide, the following sections:

- **afbidl** (mandatory): identifies afbidl document type and specify version.
- **info** (mandatory): General information about the API.
- **tools** (optional): Specific information aimed to automatic processing tools
- **verbs** (mandatory): List of API verbs, for each, gives the request and
  possible replies.
- **events** (optional): List events that can be sent by the API. NB: for now we
  focus on "static" events, known at the binding's startup. Dynamic events
  (i.e. generated at runtime) will be addressed later on.
- **state-machines** (optional): Internal states described as Finite State
  Machines (state-transition automates).
- **scenarios** (optional): Example of exchanges between clients and services.
  Scenarios are intended to document typical exchanges (similarly to sequence
  diagrams of UML)

Those section names are _reserved_.

User can still add arbitrary sections, with arbitrary names, AS LONG AS this
name do not match one the keywords listed above.

In examples provided below, we define a **schemas** section that gathers and
organizes the various schemas used to describes **verbs** and **events**.
As a reminder, **schemas** section is optional, can be freely renamed and/or
be split into multiple sections (**TBD:** *and even maybe ext.ref...*)

### afbidl

This section indicates that the document is written in afbidl format and
complies to this specification.

It is `string` (made of integers concatenated with dots).

As for now the value MUST BE `"0.1"`.

### info (General Information)

This section is commonly found in most IDLs (either info or information). It
gathers various information about the API : name, description, version, author, website...
In this section, items values are mostly simple types (`strings`, and possibly
`numbers` and `booleans`).

This section MUST AT LEAST provide:

- `apiname`: a `string` identifier (unique) that reference the API.
- `version`: a `string` (made of integers concatenated with dots). It matches
  the revision of the API described in the document.

Here is an example of an **info** section, written in YAML (describing the GPS
API):

```YAML
info:
  apiname: gps
  title: Service for geolocation
  description:
        GPS service reports current WGS84 coordinates from GNSS devices
        via the gpsd application.
  version: "0.1"
  author: AGL
  maintainer: John Difool
  homepage: https://doc.automotivelinux.org/...
```

### tools

This section will hold the configuration for automatic processing (e.g. code
and doc generation).
It type is an `object`.

In this section, item keys are identifiers referencing an external tool.
The value itself will probably be an `object` as
well, in keys will identify some configuration parameter... But there is no
hard constraint on this. It's really a bunch of config options.
The exact format depends on the tool aimed by the item.

Here is an example of a **tools** section, written in YAML, defining some
options for a C code skeleton generator:

```YAML
tools:
    afb-genskel:
        scope: static
        prefix: req_
        postfix: _cb
        init: init

```

Those options may be superseded by command line option when invoking the tool.

### verbs

This section describes the verbs exposed by the API, and implemented in a
binding. Each item describe one (and only one verb).
Each item key is the verb name (identifier).
Its value is an `object`, describing:

1. The request schema for this verb (that may include optional parameters)
2. The possible replies (success, failure, etc.), along with their respective
schemas

Each message (request and replies) refers to a schema. This schema can be:

- inlined. The schema is directly described after the message (request, reply)
  it describes. It is convenient for simple, short messages but prevent
  factorization by reusing a schema in various message with different roles.
- an internal document reference (a "path" of nodes, under a user-defined
  *schemas* section). In this case, the reference is a string beginning with
  `$/schemas/` keyword, followed by of path of children node concatenated with
  slashes (`/`).

  **NOTE:** *A future revision of the afbidl specification may also alow the
  schema to be an external reference (to another document, schema libraries,
  ...), but further work is needed to sort out a few syntactic and semantic
  issues between JSON, YAML, and JSON-Schema, making all them collaborate
  happily).*

Here is an example of a simple **verbs** section, containing only one verb.
Calling this verb can lead to two possible outcomes, success or failure.
Failure is denoted by the `_` special property and no schema is needed as the
reply object will just be empty.

```YAML
verbs:
    example:
        description: "fake verb just for an example"
        request: $/schemas/example/request
        reply:
            succces: $/schemas/example/reply_success
            _: This verb may fail in some cases...
        triggers: some-condition
```

NOTE: this verbs **triggers** a transition in a Finite State Machine, defined
in the **state-machines** section (see below).

### state-machines

This section describes internal states of the binding implementing the API.

As a reminder, a Finite State Machine is an abstract machine that can be in
exactly one of a finite number of states at any given time. The FSM can change
from one state to another in response to some inputs; the change from one state
to another is called a transition. An FSM is defined by a list of its states,
its initial state, and the inputs that trigger each transition.

Lets look at a simple example state machine (related to the example verb defined
above):

```YAML
state-machines:
    example-state-machine:
        description: This is an example state machine
        scope: global
        states: [initial-state, another-state]
        initial: initial-state
        transitions:
            initial-to-another:
                from: initial-state
                to: another-state
                triggered-by: some-condition
            back-to-initial:
                from: another-state
                to: initial-state
                triggered-by: another-condition
```

Conditions can be a simple trigger identifier (commonly, fired by a verb), or
a more complex combination of triggers in a logical expression.

Logical expressions a represented by trees (TDB : factorization with **events**)

### events

This section describes the events,i.e. messages the binding implementing the
API will emit asynchronously, when some arbitrary conditions are met during
the runtime. Those conditions can include inputs from outside the boundaries
of the binding, internal state machine transition, achievement of a given
tasks, or whatsoever...

Events can be subscribed explicitly by the client (like in a push/pull model),
but this a rigid requirement: indeed, the binding can "decide" itself to
register another binding as a subscriber of a given event.

As a reminder, AGL's Application Framework Binder provides 4 main methods
related to events, allowing to:

- create an event: `afb_deamon_make_event()`
- emit an event: `afb_event_push()`
- subscribe another binding as a client for an event: `afb_req_subscribe()`
- cancel a subscription: `afb_req_unsubscribe()`

Bindings API often have a subscribe/unsubscribe verb pair that wraps this
mechanics in a push/pull-like model, but as stated above, this is not a
requirement.
As previous section, the **events** section is an`object`. Each item of this
`object` describes an event, the key being its identifier, and the value an
`object`, that MUST at least contain an item which key is `schema`, and it's
value a JSON-Schema (inlined, or as a reference).

Here is an example of a simple **events** section, written in YAML, describing
a unique event reporting a status (from the AGL's radio API)

```YAML
events:
    status:
        description: "event emitted when playing status changes."
        schema: $/schemas/playing/status
        when-state:
            signaling: on
```

The `when-state` property of events is a logical condition, combining internal
**state-machines** states, under which event emission is actually done. In
other words, the event is NOT emitted UNLESS this condition is met.

The `when-state` is an `object`, representing a tree that encodes the logical
expression. Each leaf of this tree MUST BE a state as per **state-machines**,
and each node MUST BE a logical operator.

As per JSON-Schema definition, available operators are:

- `allOf`: logical AND
- `anyOf`: logical OR (at least one)
- `oneOf`: logical XOR (one and only one)

When the `when-state` clause has multiple items, `allOf` operator is applied.

Another example, using the example state machine defined above, could be:

```YAML
        when-state:
            example-state-machine: another-state
```

### the "schemas" section(s)

Unlike other section keyword described above, `schemas` is NOT a reserved
keyword.

As a reminder JSON-Schemas can be defined in arbitrary sections, or directly
inlined in the message (verb or event definition).

In this document examples, we use a unique section (named **schemas** for the
sake of clarity) as a container storing various schemas.

## EXAMPLES

Here are some example of API specifications, written in AFBIDL formalism.
We tried to check the syntax as much as possible, but as the semantic is still
a "Work in Progress", they have to be be considered as proposal illustration and
nothing more (even if it intends to be contractual in a near future !)

**NOTE**: The 2 first examples, GPS and Radio, have been written in YAML syntax,
while the monitoring API as been described in a "native" JSON format.

### GPS API

[GPS API, link on IoTBZH afbild github](https://github.com/iotbzh/afbidl/blob/master/example-api-gps.yml)

### Radio API

[Radio API, link on IoTBZH afbild github](https://github.com/iotbzh/afbidl/blob/master/example-api-radio.yml)

### Monitor API

[Monitor API, link on IoTBZH afbild github](https://github.com/iotbzh/afbidl/blob/master/idl-monitor.json)




[json-org]:    http://json.org/                                    "JSON format"
[json-rfc]:    https://tools.ietf.org/html/rfc8259                 "JSON format RFC"
[json-schema]: https://json-schema.org/                            "JSON Schema"
[yaml]:        https://yaml.org/                                   "YAML format"
[any-json]:    http://json.org/                                    "any-JSON"
[openapi]:     https://www.openapis.org/                           "Open API"
[asyncapi]:    https://www.asyncapi.com/                           "Async API"
[franca]:      https://franca.github.io/franca/                    "Franca"
[spec1903]:    https://jira.automotivelinux.org/browse/SPEC-1903          "SPEC-1903"
[wikip-json]:  https://en.wikipedia.org/wiki/JSON#Data_types_and_syntax   "JSON on wikipedia"
[xtext]:       https://www.eclipse.org/Xtext/                      "Eclipse xtext"
