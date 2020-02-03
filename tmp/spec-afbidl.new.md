# API specification for AGL

## Abstract

This document is a proposal of a formalism that allows to specify, define
and document AGL bindings APIs (Application Programming Interfaces).
We first define our needs, search for a candidate in the existing FOSS
pool, and eventually set up a custom solution (based on existing and
validated pieces of Free Software).

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

First off: is API specification useful for AGL ?

The answer is a loud **YES !**

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
  - Documentation
  - Code skeleton with/without parameter validation
  - Automated tests

## Evaluation of existing tools

### OpenAPI

Micro service architectures on the Web mostly use OpenAPI for specifying and
documenting their API. For this reason, it was a
good candidate to evaluate.

Sadly, however, the API description language in OpenAPI never provided some of the
features required/expected by AGL for a such tool. It mainly lacks features to
describe eventing aspects and the security model is also out-of-scope.

ToBeDone : Possible Extensions  ?

ToBeRewritten: Using OpenAPI also had the disadvantage of implying
some twist of the model and then some ugliness
verbosity.

OpenAPI is not a language, unlike many IDL, but rather describes structured
data, in JSON format ([1][json-org], [2][json-rfc]).

### Others
ToBeRewritten:
Many IDL (Interface Description Language) exist
but they rarely fit all AGL requirements.

First of all, they generally are "languages",
meaning that they are difficult to parse, to
generate and to manipulate by tools.

ToBeDone : what has been evaluated ? -> SEE SPEC-1903 - also used in automotive: Genivi Franca IDL

## Proposal of a new formalism

As a side note, IoT.bzh proposal includes advanced designs introduced after some
discussions with Joël Champeau and Philippe Dhaussy, researchers at ENSTA
Bretagne (National Institute of Advanced Technologies of Brittany) specialized
in system modeling and formal verification.

### Foundation of the new formalism

OpenAPI is not a language, unlike many IDL, but rather consist of structured
data, which is much simpler to deal with. There is no need to write down
a generative grammar, a lexer, and other tedious components to write compiler parts.

OpenAPI is a specification made by a data structure of JSON format
[1][json-org], [2][json-rfc].
Using JSON has the advantage that no parser has to be written because a standard
one exists.

Binding verbs payload is also formatted in JSON. An obvious choice to specify
them is then JSON Schema [3][json-schema].
This allows to describe complex values and their constraints using a format easy
to integrate in tools.

### Toward even more human-friendliness

JSON is a bit more human-friendly than its predecessors like XML, but can still
be a bit tedious to deal with. Another format like YAML [4][yaml] is easier to deal with. As stated previously, we'll rely, internally, on JSON and
JSON-Schemas, but it's worth noting that their is a quasi-isomorphism between both YAML and JSON data models.

Comments are an exception, as strict JSON forbids them, whereas YAML welcomes them quite happily. If a user writes down comments in the YAML "source"
specification file, they'll be lost while converting to JSON but it's not really a showstopper.

We still need to define a technical solution to report an error at the very line that
caused it IN THE SOURCE file. Something like a "reverse lookup table", indexing
source YAML line number, for each line in the converted JSON.

NOTE : It may happen that some line in the JSON doesn't have an equivalent; for
instance, a closing curly brace that closes an object will have an equivalent line
in the YAML source. This is not problem because that type of line should NOT be
a source of error - given the YAML is syntaxically valid, and the converter
works fine.

There are example of such YAML (and other) to JSON and vice-versa converters like [5][any-json].

[json-org]:    http://json.org/                    "JSON format"
[json-rfc]:    https://tools.ietf.org/html/rfc8259 "JSON format RFC"
[json-schema]: https://json-schema.org/            "JSON Schema"
[yaml]:        https://yaml.org/                   "YAML format"
[any-json]:    http://json.org/                    "any-JSON"
