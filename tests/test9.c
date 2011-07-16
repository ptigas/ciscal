program main {
	declare
		f, n, level, calls
	enddeclare
	
	function max(in a, in b) {
		if (a>b) {
			return(a);
		}else{
			return(b);
		}
	}
	
	function fibonacci(in n, inout level) {
		declare
			a, b, l1, l2
		enddeclare

		if ( n = 1 or n = 0 ) {
			return(n);
		};
		
		l1 := level+1;
		l2 := level+1;
		
		calls := calls + 2;
		
		a := fibonacci(in n-1,inout l1);
		b := fibonacci(in n-2, inout l2);
		level := max(in l1, in l2);
		return(a+b);
	}
	
	n := 45;
	level := 1;
	calls := 1;
	f := fibonacci(in n, inout level);
	
	print(f);
	print(level);
	print(calls);
}