### SimpleCompiler ###

#### Introduce ####
This is a simple compiler for a c-like language written in c, compile the c-like language into assembly language.
It consists of four parts: lexical analysis, grammar analysis, intermediate code generation and optimization, object code generation.

#### Feature ####

* Comment Type:  
    c style comment:
``` c
    // comment
	/* comment */
```

* Variable Type:  
    support integers(int) and float
``` c
    int a = 0;
    int b = a;
    float c = 1.1;
    float d = c;
```

* Flow Control:
``` c
    // if 
    if (cond1) {
        // statement
    } else if (cond2) {
        // statement
    } else {
        // statement
    }

    // while 
    while (cond) {
        // statement
    }
```

* Function:
``` c++
    // define
    int max(int a, int b) {
		if(a>b){
			return a;
		}else{
			return b;
		}
    }

    // call
    int c = max(1, 5);
```

* System Functions:
``` c
    // prototype: input()
    // usage: input a integer , and return it to a variable
    a = input();

    // prototype: output(int)
    // usage: output a integer 'a'
    output(a);
```

#### Example ####
download the codes, build and run:
``` shell
$./run.sh
$./a.out code.c
```
"a.out" is our simple compiler;  
"code.c" is the source code;
after compile,our source code will be compile into assembly language and we can run it under spim or just run it in a mips cpu; 
``` c
// code.c
// input n,output fibonacci sequence
int main()
{
	int a = 0, b = 1, i = 0, n;
	n = read();
	while(i < n)
	{
		int c = a + b;
		write(b);
		a = b;
		b = c;
		i = i + 1;
	}
	return 0;
}

```
input 1, get output: 1  
input 5, get output: 1 1 2 3 5  
input 8, get output: 1 1 2 3 5 8 13 21  


#### License ####
MIT