program programma{
	declare a, b, c, d, e
	enddeclare
	
	procedure alpha(in a, in b, inout ret) {
		print(a+b);
		print(ret);
		do {
			a := a + 1;
			ret := ret + 10*b;
			print(ret);
			if(ret>50 and 1+1+1+1+1+1+1+1+1+1>10) {
				exit;
			}
			
		} while(a < 10)
		
	}
	
	function beta(in a, in b) {
		procedure test(in a, inout b) {
			do {
				if(a<0) {
					exit;
				};
				a := a + 1;
				b := b + 10;
			} while(a>0);
			b := 2*a;
		} 
		
		call test(in a, inout b);
		
		return(a+b);
	}
	b := 1;
	a := 1;
	call alpha(in 1, in 2, inout a);
	print(a);
}
