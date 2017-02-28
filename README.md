### SimpleCompiler ###

#### Introduce ####
This is a simple compiler for a c-like language written in c, compile the c-like language into assembly language.
It consists of four parts: lexical and grammar analysis, semantic analysis，intermediate code generation and optimization, object code generation.

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
``` c
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

#### Achieve ####

* Lexical and grammar analysis:	
	integer, floating point, and the regular expression of the identifier are as follows:
``` c
    int  	[+-]?[0-9]+
    float  	[+-]?([0-9]*\.?[0-9]+|[0-9]+\.)
    id 		[_A-Za-z][A-Za-z0-9_]*
```

	The nodes that define the syntax tree are as follows:
``` c
    typedef struct NODE 	
	{
		int kind;			// node type
		int lineno;			// node line number
		int rule;			// the rules used for reduction
		union				
		{
			char c[32];		// identifier or keyword
			int i;			// int variable
			float f;		// float variable
		};
		struct NODE *child;			
		struct NODE *brother;		
	}node,*pnode,**ppnode;
```

	for more,you can see in lexical.l and syntic.y

* Semantic analysis:		
	The semantic analysis is realized by traversing the abstract syntax tree from top to bottom,supports detection of 18 error types.
	supports multi-scope,for example, when traversing a function within the time, that to enter a new scope, the scope of the stack, leaving the current scope that is to leave the function, the scope of the stack, the code fragment is as follows:
``` c
	IntoScope();  			// into a scope，Scope+1
	FuncNode* f = FunDec(p->child->brother, type);	// return type
	if(p->rule == 6) 		// ExtDef->Specifier FunDec CompSt, indicates that the function is defined at this time，and Def_flag is set to 1
	{
		f->Def_flag = TRUE;
		CompSt(p->child->brother->brother, type);
	}
	else if(p->rule == 7)	// ExtDef->Specifier FunDec SEMI, indicates that the function is declared at this time, undefined, and Def_flag is set to 0
	{
		f->Def_flag = FALSE;
	}
	OutScope();  			// exit a scope, Scope-1
```
	for more,you can see in semantic.h and semantic.c
	
* Intermediate code generation and optimization:	
	the node structure of the intermediate code is as follows:
``` c
	struct InterCode
	{
		enum ic_kind kind; 			// interCode kind
		pOperand op1,op2,op3,relop; // operands
	};
```
	code optimization is as follows:
``` c
	/* delete the code that can not be reached */
	RETURN n
	GOTO label4			// this interCode can be remove
	LABEL label3 :	
 
	/* variable propagation */
	t2 := t1; 
	t3 := t2 + #1;		// t2 is not necessary and can be remove
	t3 := t1 + #1;		// the above two intermediate codes can be combined into one
```

* Object code generation:	
	the key to object code generation is register allocation, parameter passing and variable protection.

	register allocation algorithm:	
	The register allocation uses the associated mapping table. All the variables are saved to the data segment, the use of the register group is s0 ~ s7, through the associated mapping table, the register with the data section of the variables associated with one. When you need to encounter a variable, first in the association table to find out whether there is an associated register, if so, you can directly use the associated register to operate, otherwise, see whether there is a register associated with the variables, if there is , Which associates the register with the current variable, otherwise it means that each register in the register group corresponds to a variable, and it is necessary to free the vacant space by removing one of them.
	
	parameter passing and variable protection:	
	parameter transfer using a0 ~ a3 register, if the parameters are more than four error, call the function before the parameters will be saved to a0 ~ a3 register, in the function, followed by a0 ~ a3 register parameters taken out to achieve the parameters of the transfer. The protection of the variables through the stack to protect, in turn, will protect the variables pushed stack, when the implementation of the function away from the time, the variables will be restored stack.

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