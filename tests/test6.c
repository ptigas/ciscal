program main {
	
	function abs(in a) {
		if ( a < 0 ) {
			return(-a);
		} else {
			return(a);
		}
	}
	
	function gcd(in a, in b) {
		if ( b = 0 ) {
			return(a);
		} else {
			return(gcd(in b, in a-b*(a/b)));
		}
	}
	
	function lcm(in a, in b) {
		return(abs(in a*b)/gcd(in a, in b))
	}
	
	print(abs(in -2));
	print(gcd(in 42, in 56));
	print(lcm(in 42, in 56));
}