
const ctools = require('./c-tools.js');
const otools = require('./o-tools.js');
const fs = require('fs');

// get the arguments

var inFile = process.argv[2];
console.log("processing file %s", inFile);

// open the input JSON file
console.log("reading file %s", inFile);
var inContent = fs.readFileSync(inFile);
console.log("content of file %s is [[[%s]]]", inFile, inContent);

// process the json input
console.log("parsing content of $%s...", inFile);
var inDesc = JSON.parse(inContent);

// show the input as pretty JSON
console.log("got for ${inFile}: %s", JSON.stringify(inDesc,null,3));

// compute the C presentation
var outCColumn = 120;
var outCTabs = "   ";
var cDesc = ctools.stringFmt(JSON.stringify(inDesc), outCTabs, outCColumn - 1 - outCTabs.length);
console.log("inDescC:\n%s", cDesc);

// make the Doc
var doc = new otools.ODoc(inDesc);

// initialize the variable
var vars = {};
[
   [ "preinit",      "#/tools/afb-genskel/preinit" ],
   [ "init",         "#/tools/afb-genskel/init" ],
   [ "onevent",      "#/tools/afb-genskel/onevent" ],
   [ "scope",        "#/tools/afb-genskel/scope",        "static" ],
   [ "prefix",       "#/tools/afb-genskel/prefix",       "afb_verb_" ],
   [ "postfix",      "#/tools/afb-genskel/postfix",      "_cb" ],
   [ "provideclass", "#/tools/afb-genskel/provide-class" ],
   [ "requireclass", "#/tools/afb-genskel/require-class" ],
   [ "requireapi",   "#/tools/afb-genskel/require-api" ],
   [ "priv",         "#/tools/afb-genskel/private",       false ],
   [ "noconc",       "#/tools/afb-genskel/noconcurrency", false ],
   [ "api",          "#/info/apiname" ],
   [ "title",        "#/info/title",                      "?" ],
   [ "desc",         "#/info/description" ]

].forEach(function(item){
   var v = doc.nodeAt(item[1]);
   if (v === undefined && item[2] !== undefined)
      v = item[2];
   vars[item[0]] = v;
});


function forEachVerb(fun) { doc.forEachChild(["verbs"], fun); }
function forEachEvent(fun) { doc.forEachChild(["events"], fun); }
function forEachStateMachine(fun) { doc.forEachChild(["state-machines"], fun); }


function funOfVerb(verb) { return vars['prefix'] + ctools.id(verb) +  vars['postfix']; }

// declare ver implementation
forEachVerb(function(n) {
   console.log(vars['scope'] + " void " + funOfVerb(n.name) + "(afb_req req);\n");
});

// print the array of verbs
console.log("");
console.log("static const struct afb_verb_v3 _afb_verbs_" + ctools.id(vars['api']) + "[] =");
console.log("{");
forEachVerb(function(n) {
   console.log("   {");
   console.log("      .verb = " + ctools.string(n.name) + ",");
   console.log("      .callback = " + funOfVerb(n.name) + ",");
   console.log("      .info = " + ctools.stringNull(n.nodeAt(['description'])) + ",");
   console.log("      .auth = NULL,");
   console.log("      .vcbdata = NULL,");
   console.log("      .session = 0,");
   console.log("      .glob = 0");
   console.log("   },");
});
console.log("   { .verb = NULL, .callback = NULL } /* END OF THE VERB ARRAY */");
console.log("};");
console.log("");
   
