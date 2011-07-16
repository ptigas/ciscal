program test {
	declare
		a, b, c
	enddeclare

	function test2(in a) {
		return(a);
	}
	
	a := test2(in test2(in test2(in test2(in test2(in test2(in 1) + 1) + 1) + 1) + 1) + 1);
	
	print(a);
}