
/*
; compute the C presentation of the string of 'value
; prefix is the string to prepend at the beginning
; width if the expected width in characters (tab is 1 char)
*/
function C_string_fmt(value, prefix, width) {
	function esc(c, fun) {
		switch(c) {
			case '"':
			case '\\': fun('\\'); fun(c); break;
			case '\n': fun('\\'); fun('n'); break;
			case '\r': fun('\\'); fun('r'); break;
			case '\t': fun('\\'); fun('t'); break;
			default: fun(c);
		}
	}
	function process(str, fun) {
		var w = 0;
		function start(f) {
			w = 0;
			for(var i = 0; i < prefix.length; i++)
				f(prefix.charAt(i));
			f('"');
		}
		function stop(f) {
			f('"');
		}
		function start_if(f, limit) {
			if (w >= limit) {
				stop(f);
				fun('\n');
				start(f);
			}
		}
		function emit(c, f) {
			f(c);
			w++;
		}
		function width_emit(c, f) {
			start_if(f, width);
			esc(c, function(c) { emit(c, f); });
		}
		start(fun);
		for(var i = 0; i < str.length; i++)
			width_emit(str.charAt(i), fun);
		stop(fun);
	}
	var result = [];
	process(value, function(c) { result.push(c); });
	return result.join('');
}


// default C formating for inline values
function C_string(value) { return C_string_fmt(value, "", 256); }

// default formating or NULL
function C_string_null(value) { return value ? C_string(value) : "NULL"; }

// return a C compatible identifier for name
function C_ify(name) {
	return name.replace(/\W/g, "_");
}

exports.string = C_string;
exports.stringNull = C_string_null;
exports.stringFmt = C_string_fmt;
exports.id = C_ify;
