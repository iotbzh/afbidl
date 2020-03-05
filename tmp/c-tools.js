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
		function start(fun) {
			w = 0;
			for(var i = 0; i < s.length(); i++)
				fun(s.charAt(i));
			fun('"');
		}
		function stop(fun) {
			fun('"');
		}
		function start_if(fun, limit) {
			if (w >= limit) {
				stop(fun);
				fun('\n');
				start(fun);
			}
		}
		function emit(c, fun) {
			fun(c);
			w++;
		}
		function width_emit(c, fun) {
			start_if(fun, width);
			esc(c, function(c) { emit(c, fun); });
		}
		start(fun);
		for(var i = 0; i < str.length(); i++)
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
	for(var i = 0; i < str.length(); i++)
        (string-map (lambda (c)
                (if (or (char-alphabetic? c) (char-numeric? c)) c #\_))
                name))
}
