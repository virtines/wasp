function base64_encode(string) {
	var characters = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/=";
	var result = '';
	var i = 0;
	do {
		var a = string.charCodeAt(i++);
		var b = string.charCodeAt(i++);
		var c = string.charCodeAt(i++);

		a = a ? a : 0;
		b = b ? b : 0;
		c = c ? c : 0;

		var b1 = ( a >> 2 ) & 0x3F;
		var b2 = ( ( a & 0x3 ) << 4 ) | ( ( b >> 4 ) & 0xF );
		var b3 = ( ( b & 0xF ) << 2 ) | ( ( c >> 6 ) & 0x3 );
		var b4 = c & 0x3F;

		if( ! b ) {
			b3 = b4 = 64;
		} else if( ! c ) {
			b4 = 64;
		}

		result += characters.charAt( b1 ) + characters.charAt( b2 ) + characters.charAt( b3 ) + characters.charAt( b4 );
	} while ( i < string.length );

	return result;
}

var b64 = base64_encode("Hello There");
hcall_return(b64);
