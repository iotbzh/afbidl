# API specification for AGL

## Abstract

This document is a proposal of a formalism that allows to specify, define
and document AGL bindings APIs (Application Programming Interfaces).
We first define our needs, search for a candidate in the existing FOSS
pool, and eventually set up a custom solution (based on existing and
validated pieces of Free Software).

## Notations and Definitions

In this document the <b font-family="monospace">bold monospace font</b> will be used to indicate we refer
to one of the following definition, as per the JSON RFC [][json-rfc]:

> JSON can represent four primitive types (<b font-family="monospace">strings
> </b>, <b font-family="monospace">numbers</b>, <b font-family="monospace">
> booleans</b>, and <b font-family="monospace">null</b>) and two structured 
> types (<b font-family="monospace">objects</b> and> <b font-family="monospace">
> arrays</b>).

Although, in order to allow other representation than JSON (see below), we
don't stick to the lexical definitions of those types as stated in this RFC,
any representation used to describe our APIs must comply with the following
definitions:

- <b font-family="monospace">number</b>: this type is similar to that used in
   most programming languages. It can be a signed integer, or a floating point
   representation. It is represented in base 10, as per JSON RFC requirement.
- <b font-family="monospace">string</b>: this type is similar to conventions
  used in the C family of programming languages.
- <b font-family="monospace">boolean</b> is either **true** or **false**
- <b font-family="monospace">null</b> is the empty element
- <b font-family="monospace">array</b>: an ordered list of zero or more
  values, each of which may be of any type.
- <b font-family="monospace">object</b>: an unordered collection of name–value
  pairs  where the names (also called keys) are strings. Objects are intended
  to represent associative arrays, where each key is unique within an object.

## About API specification

API Specification is here a broad term that can be understood as : Definition,
Specification, or Documentation.
They are all related, but also different entities, with their respective
purposes.
In some context, Specification will done in a human natural language,
with focus on high-level aspects, behavior and responsibilities...

Definition is a more fine-grained level, often describing the messages
exchanged by the API, using a more machine-oriented formalism that allows
automated processing.

Documentation is aimed at developers, API users. It will often compile
information found in the former, synthesized in a convenient way, for instance
as a set of HTML pages, with hyperlinks, that can be conveniently read in
a web browser.

As for now, we will use the word specification in its board meaning, unless
explicitly stated.

## Requirements for AGL

First off: is API specification useful for AGL ? The answer is a resounding
*"YES !"*

Indeed, the micro service architecture of AGL and its flexible IPC mechanism
emphasizes on the decomposition of services or applications in tiny cooperative
parts. This has big advantages in terms of flexibility and development process,
but, subsequently, implies to correctly document interfaces.

In our opinion, we should avoid specifying using natural human language as
much as possible. We should favor the use of a machine-oriented formalism to
describe most of the aspects of an API.

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
- Describing the security model based on permissions
- Being Suitable for the generation of:
  - Documentation (in various format: HTML, pdf, markdown, ...)
  - Code skeleton with various optionnal features
    - parameter validation
    - marshalling / unmarshalling
  - Automated tests

## Evaluation of existing tools

### Odler IDLs

IDLs are getting mature since years. Their primary use is found in describing
IPCs (InterProcess Communication) and RPCs (Remote Procedure Calls) mechanisms,
in distributed computing, often between different nodes through a network.
They are now found with clear predominance in Web-based technologies.

One of the first shot was released with the CORBA 2 standard, where the urge
to interface various technologies (mainly C++ and Java) led to an
implementation-agnostic description formalism of the interfaces provided by
system components, regardless of their respective actual implementation.

By the meantime, WebServices (based on XML RPCs over HTTP, aka SOAP) and the
next decade RESTFul APIs (said "Web Applications") started to spread, with
their own needs of describing nodes interfaces for seamless interoperability.

WSDL (Web Service Description Language) was used to describe SOAP interfaces,
while WADL (Web Application Description Language) and RAML (Restful Application
Modeling Language), and swagger, among others were aimed at RESTFul APIs.

One thing that is worth to notice is that from the beginning and until now,
each new RPC technology came up with its own specific IDL (even if IDLs
are all sharing common concept, no unifying / general purpose language
made any major break-through...

### State-of-the-art IDLs

By time passed, as from 2010 or so, JSON started to widely replace XML for
structured data representation in the messages exchanges. Various IDLs
evolved to reflect this change.

As per JIRA SPEC-1903 [][spec1903], 3 main IDLs were identified:

- OpenAPI 3.0 [][openapi]
- asyncAPI 1.2 [][asyncapi]
- GENIVI Franca [][]

  GENIVI Franca very close of expectations in terms of features, but closely
  tighten to eclipse / Xtext.

#### OpenAPI

Micro service architectures on the Web mostly use OpenAPI for specifying and
documenting their API. For this reason, it was a good candidate to evaluate.

Sadly, however, the API description language in OpenAPI never provided some of
the features required/expected by AGL for a such tool. It mainly lacks features
to describe eventing aspects and the security model is also out-of-scope.

ToBeDone : Possible Extensions  ?

ToBeRewritten: Using OpenAPI also had the disadvantage of implying
some twist of the model and then some ugliness
verbosity.

OpenAPI is not a language, unlike many IDL, but rather describes structured
data, in JSON format ([][json-org], [][json-rfc]).

#### Others

ToBeRewritten:
Many IDL (Interface Description Language) exist
but they rarely fit all AGL requirements.

First of all, they generally are "languages",
meaning that they are difficult to parse, to
generate and to manipulate by tools.

ToBeDone : what has been evaluated ? -> SEE SPEC-1903 - also used in automotive: Genivi Franca IDL

## Proposal of a new formalism

Althrough no suitable solution is available off the shelf, this research
taught its lessons and some ideas stemmed from those investigations.

First, both of them are using structured data representation to encode the
API description, allowing JSON and more human-friendly format as YAML.
Furthermore both are relying on JSON Schema [][json-schema], for the
description (and runtime validation) of exchanged messages payload.

As a side note, this proposal includes advanced designs introduced after some
discussions with Joël Champeau and Philippe Dhaussy, researchers at ENSTA
Bretagne (National Institute of Advanced Technologies of Brittany) specialized
in system modeling and formal verification.

### JSON Schema

One of the goals of specifying our APIs is to manipulate AGL binding verbs
payloads (validation, parsing, etc.). As a matter of fact, this payload
is encoded in JSON.

As stated above, other IDLs like OpenAPI and AsyncAPI do rely on JSON-Schema
for this kind of task (NB : a subset extension for OpenAPI, a super-set for
AsyncAPI).

JSON Schema enables a normalized description of the syntax a given chuck of JSON.
The schema itself being written in JSON, (or can be converted to) it can be, in
turn, validated against a so-called "meta-schema". Meta-schemas are also
JSON-Schemas, but a "special" kind as they also validates themselves.
It allows to describe complex values and their constraints using a format easy
to integrate in tools.

One big advantage of using JSON Schema is that it extensively used, so it comes
with a whole ecosystem of tools (parsers, validators, ...), that can be taken
off the shelf and used as is, as part of a bigger system.

Furthermore, we stick at structured data, and we don't implement a
full-featured language, implying to define its generative grammar, a lexer,
and other compiler parts (whose are tedious to write and maintain).

JSON-Schema can't be used to describe those whole API but focuses on
describing messages structure (request and replies "schemas").

They will be found in the "schemas" section of the API description.

### Toward even more human-friendliness

JSON is a bit more human-friendly than its predecessors like XML, but can
still be a bit tedious to deal with. Another format like YAML [][yaml] is
easier to deal with. As stated previously, we'll rely, internally, on JSON and
JSON-Schemas, but it's worth noting that their is a quasi-isomorphism between
YAML and JSON data models.

Comments are an exception, as strict JSON forbids them, whereas YAML welcomes
them quite happily. If a user writes down comments in the YAML "source"
specification file, they'll be lost while converting to JSON but it's not
really a showstopper.

We still need to define a technical solution to report an error at the very
line that caused it IN THE SOURCE file. Something like a "reverse lookup
table", indexing source YAML line number, for each line in the converted JSON.

NOTE : It may happen that some line in the JSON doesn't have an equivalent;
for instance, a closing curly brace that closes an object will have an
equivalent line in the YAML source. This is not problem because that type of
line should NOT be a source of error - given the YAML is syntactically valid,
and the converter works fine.

There are example of such YAML (and other) to JSON and vice-versa converters
like [][any-json].

### API description file structure

Either written in YAML, JSON or any equivalent format, the description file
should/must provide, at its root level the following sections:

- **afbidl** (mandatory): identifies afbidl document type and specify version.
- **info** (mandatory): General information about the API.
- **tools** (optional): Specific information aimed to automatic processing tools
- **verbs** (mandatory): List of API verbs, for each, gives the request and
  possible replies.
- **events** (optional): List events that can be sent by the API. NB: for now we
  focus on "static" events, known at the binding's startup. Dynamic events
  (i.e. generated at runtime) will be addressed later on.
- **state-machines** (optional): Internal states described as Finite State
  Machines (state-transition automats).
- **schemas** (mandatory): describe the content of messages (verb requests and
  replies, events), complying to the JSON-Schema formalism.

At a syntax level, the API description document consist of an
<b font-family="monospace">object</b>, which items are the sections discussed
above. The key of each section/item is one of the keywords defined in the former
list. The value of each section consists of an <b font-family="monospace">object
</b>, which content depends on the section it describes.

### afbidl

This section indicates that the document is written in afbidl format and complies
to this specification.

It is <b font-family="monospace">string</b> (made of integers concatenated with dots).
As for now the value MUST BE `"0.1"`.

### info (Genral Information)

This section is commonly found in most IDLs (either info or information). It 
gathers various information about the API : name, description, version, author, website...
In this section, items values are mostly simple types (strings, and possibly
numbers and booleans).

This section MUST AT LEAST provide:

- `apiname`: a <b font-family="monospace">string</b> identifier (unique) that
  reference the API.
- `version`: a <b font-family="monospace">string</b> (made of integers
  concatenated with dots). It matches the revision of the API described in the
  document.

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
  maintainer: Scott Rifenbark
  homepage: https://doc.automotivelinux.org/...
```

### tools

This section will hold the configuration for automatic processing (e.g. code
and doc generation).
It type is an <b font-family="monospace">object</b>.

In this section, item keys are identifiers referencing an external tool.
The value itself will probably be an <b font-family="monospace">object</b> as
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

### verbs

This section describes the verbs exposed by the API, and implemented in a
binding. Each item describe one (and only one verb).
Each item key is the verb name (identifier).
Its value is an <b font-family="monospace">object</b>, describing:

1. The request schema for this verb (that may include optional parameters)
2. The possible replies (success, failure, etc.), along with their respective
schemas

Each message (request and replies) refers to a schema in the **schemas**
section (it is technically possible, despite not being best practice, and then
discouraged, to directly explicit the schema without a reference to the
**schemas** section).

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

As a reminder, a Finite State Machine is an abstract machine that can be in exactly one of a finite number of states at any given time. The FSM can change from one state to another in response to some inputs; the change from one state to another is called a transition. An FSM is defined by a list of its states, its initial state, and the inputs that trigger each transition.

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

- create an event: .. TBD
- emit an event: ... TBD
- subscribe another binding as a client for an event: `afb_req_subscribe()`
- cancel a subscription: ... TBD

Bindings API often have a subscribe/unsubscribe verb pair that wraps this
mechanics in a push/pull-like model, but as stated above, this is not a
requirement.
As previous section, the **events** section is an
<b font-family="monospace">object</b>. Each item of this
<b font-family="monospace">object</b> describes an event, the key being its
identifier, and the value an <b font-family="monospace">object</b>, that
MUST at least contain an item which key is `schema`, and it's value a JSON
schema (the best practice being, once again, a reference to the **schemas**
section).

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

The `when-state` is an <b font-family="monospace">object</b>, representing a
tree that encodes the logical condition. Each leaf of this tree MUST BE a
state as per **state-machines**, and each node MUST BE a logical operator, 
as per JSON-Schema definition, so either:

- `allOf`: logical AND
- `anyOf`: logical OR (at least one)
- `oneOf`: logical XOR (one and only one)

If multiple states are used as a condition without explicitly specifying a
logical operator, `allOf` is implied by default.

Another example, using the example state machine defined above, could be:

```YAML
        when-state:
            example-state-machine: another-state
``` 

### schemas


[json-org]:    http://json.org/                                     "JSON format"
[json-rfc]:    https://tools.ietf.org/html/rfc8259                  "JSON format RFC"
[json-schema]: https://json-schema.org/                             "JSON Schema"
[yaml]:        https://yaml.org/                                    "YAML format"
[any-json]:    http://json.org/                                     "any-JSON"
[openapi]:     https://www.openapis.org/                            "Open API"
[asyncapi]:    https://www.asyncapi.com/                            "Async API"
[franca]:      https://franca.github.io/franca/                     "Franca"
[spec1903]:    https://jira.automotivelinux.org/browse/SPEC-1903    "SPEC-1903"
