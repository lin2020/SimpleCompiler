%error-verbose
%{
	#include <stdio.h>	
	#include <string.h>
	#include "lex.yy.c"
	#include "MIPS.c"
	pnode root;
%}

/*declared types*/
%union {
	int type_int;
	float type_float;
	char type_char[32];
	struct NODE* type_node;
};

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

/*priority*/
%right ASSIGNOP
%left OR
%left AND
%left RELOP
%left PLUS MINUS
%left STAR DIV
%right NOT SMINUS
%left LP RP LB RB DOT

/*declared tokens*/
%token SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE
%token LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE INT FLOAT ID
%token T NT TYPE_INT TYPE_FLOAT

/*declared non-terminals*/
%type <type_node> Program ExtDefList ExtDef Specifier StructSpecifier OptTag Tag VarDec FunDec VarList
%type <type_node> CompSt ParamDec StmtList Stmt DefList Dec Def Exp Args DecList ExtDecList
%type <type_node> SEMI COMMA ASSIGNOP RELOP PLUS MINUS STAR DIV AND OR DOT NOT TYPE
%type <type_node> LP RP LB RB LC RC STRUCT RETURN IF ELSE WHILE INT FLOAT ID

%%
/*High-level Definitions*/
Program 
	: ExtDefList 		{init_c(&$$,1,NT,@1.first_line,"Program");$$->child=$1;root=$$;}
	;
ExtDefList 			
	: ExtDef ExtDefList	{init_c(&$$,2,NT,@1.first_line,"ExtDefList");$$->child=$1;$1->brother=$2;}
	| /* empty */		{init_c(&$$,3,NT,-1,"ExtDefList");}
	;
ExtDef 
	: Specifier ExtDecList SEMI	{init_c(&$$,4,NT,@1.first_line,"ExtDef");init_c(&$3,-1,T,0,"SEMI");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	| Specifier SEMI			{init_c(&$$,5,NT,@1.first_line,"ExtDef");init_c(&$2,-1,T,0,"SEMI");$$->child=$1;$1->brother=$2;}
	| Specifier FunDec CompSt	{init_c(&$$,6,NT,@1.first_line,"ExtDef");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	| Specifier FunDec SEMI 	{init_c(&$$,7,NT,@1.first_line,"ExtDef");init_c(&$3,-1,T,0,"SEMI");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	;
ExtDecList 
	: VarDec					{init_c(&$$,8,NT,@1.first_line,"ExtDecList");$$->child=$1;}	
	| VarDec COMMA ExtDecList	{init_c(&$$,9,NT,@1.first_line,"ExtDecList");init_c(&$2,-1,T,0,"COMMA");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	;

/*Specifers*/
Specifier 
	: TYPE 							{init_c(&$$,10,NT,@1.first_line,"Specifier");init_c(&$1,-1,strcmp(yytext,"int")==0?TYPE_INT:TYPE_FLOAT,0,"TYPE");$$->child=$1;}  	
	| StructSpecifier				{init_c(&$$,11,NT,@1.first_line,"Specifier");$$->child=$1;}
	;
StructSpecifier 
	: STRUCT OptTag LC DefList RC	{init_c(&$$,12,NT,@1.first_line,"StructSpecifier");init_c(&$1,-1,T,0,"STRUCT");init_c(&$3,-1,T,0,"LC");init_c(&$5,-1,T,0,"RC");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;$4->brother=$5;}
	| STRUCT Tag 					{init_c(&$$,13,NT,@1.first_line,"StructSpecifier");init_c(&$1,-1,T,0,"STRUCT");$$->child=$1;$1->brother=$2;}
	;
OptTag 
	: ID 							{init_c(&$$,14,NT,@1.first_line,"OptTag");/*init_c(&$1,-1,ID,0,mytext);*/$$->child=$1;}		
	| /*empty*/						{init_c(&$$,15,NT,-1,"OptTag");}
	;
Tag 
	: ID 							{init_c(&$$,16,NT,@1.first_line,"Tag");/*init_c(&$1,-1,ID,0,mytext);*/$$->child=$1;}	
	;

/*Declerators*/
VarDec 
	: ID 							{init_c(&$$,17,NT,@1.first_line,"VarDec");/*init_c(&$1,-1,ID,0,mytext);*/$$->child=$1;}		
	| VarDec LB INT RB 				{init_c(&$$,18,NT,@1.first_line,"VarDec");init_c(&$2,-1,T,0,"LB");init_i(&$3,-1,INT,0,atoi(yytext));init_c(&$4,-1,T,0,"RB");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;}
	;
FunDec 	
	: ID LP VarList RP 				{init_c(&$$,19,NT,@1.first_line,"FunDec");/*init_c(&$1,-1,ID,0,mytext);*/init_c(&$2,-1,T,0,"LP");init_c(&$4,-1,T,0,"RP");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;}	
	| ID LP RP 						{init_c(&$$,20,NT,@1.first_line,"FunDec");/*init_c(&$1,-1,ID,0,mytext);*/init_c(&$2,-1,T,0,"LP");init_c(&$3,-1,T,0,"RP");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	;
VarList 
	: ParamDec COMMA VarList 		{init_c(&$$,21,NT,@1.first_line,"VarList");init_c(&$2,-1,T,0,"COMMA");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	| ParamDec 						{init_c(&$$,22,NT,@1.first_line,"VarList");$$->child=$1;}
	;
ParamDec 
	: Specifier VarDec				{init_c(&$$,23,NT,@1.first_line,"ParamDec");$$->child=$1;$1->brother=$2;}
	;

/*Statements*/
CompSt 
	: LC DefList StmtList RC		{init_c(&$$,24,NT,@1.first_line,"CompSt");init_c(&$1,-1,T,0,"LC");init_c(&$4,-1,T,0,"RC");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;}
	| error RC						{err = 1;yyerrok;}
	;
StmtList 
	: Stmt StmtList 				{init_c(&$$,25,NT,@1.first_line,"StmtList");$$->child=$1;$1->brother=$2;}
	| /*empty*/						{init_c(&$$,26,NT,-1,"StmtList");}	
	;
Stmt 
	: Exp SEMI 						{init_c(&$$,27,NT,@1.first_line,"Stmt");init_c(&$2,-1,T,0,"SEMI");$$->child=$1;$1->brother=$2;}
	| Exp error SEMI				{err = 1;yyerrok;}
	| CompSt 						{init_c(&$$,28,NT,@1.first_line,"Stmt");$$->child=$1;}			
	| RETURN Exp SEMI 				{init_c(&$$,29,NT,@1.first_line,"Stmt");init_c(&$1,-1,T,0,"RETURN");init_c(&$3,-1,T,0,"SEMI");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	| IF LP Exp RP Stmt %prec LOWER_THAN_ELSE	{init_c(&$$,30,NT,@1.first_line,"Stmt");init_c(&$1,-1,T,0,"IF");init_c(&$2,-1,T,0,"LP");init_c(&$4,-1,T,0,"RP");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;$4->brother=$5;}
	| IF LP Exp RP Stmt ELSE Stmt 	{init_c(&$$,31,NT,@1.first_line,"Stmt");init_c(&$1,-1,T,0,"IF");init_c(&$2,-1,T,0,"LP");init_c(&$4,-1,T,0,"RP");init_c(&$6,-1,T,0,"ELSE");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;$4->brother=$5;$5->brother=$6;$6->brother=$7;}
	| WHILE LP Exp RP Stmt  		{init_c(&$$,32,NT,@1.first_line,"Stmt");init_c(&$1,-1,T,0,"WHILE");init_c(&$2,-1,T,0,"LP");init_c(&$4,-1,T,0,"RP");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;$4->brother=$5;}
	| error SEMI					{err = 1;yyerrok;}
	;

/*Local Definit_cions*/
DefList 
	: Def DefList 				{init_c(&$$,33,NT,@1.first_line,"DefList");$$->child=$1;$1->brother=$2;}
	| /*empty*/					{init_c(&$$,34,NT,-1,"DefList");}			
	;
Def 
	: Specifier DecList SEMI 	{init_c(&$$,35,NT,@1.first_line,"Def");init_c(&$3,-1,T,0,"SEMI");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	;
DecList 
	: Dec 						{init_c(&$$,36,NT,@1.first_line,"DecList");$$->child=$1;}
	| Dec COMMA DecList 		{init_c(&$$,37,NT,@1.first_line,"DecList");init_c(&$2,-1,T,0,"COMMA");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	;
Dec 
	: VarDec 					{init_c(&$$,38,NT,@1.first_line,"Dec");$$->child=$1;}
	| VarDec ASSIGNOP Exp   	{init_c(&$$,39,NT,@1.first_line,"Dec");init_c(&$2,-1,T,0,"ASSIGNOP");$$->child=$1;$1->brother=$2;$2->brother=$3;}
	;

/*Expressions*/
Exp 
	: Exp ASSIGNOP Exp  	{init_c(&$$,40,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"ASSIGNOP");$$->child=$1;$1->brother=$2;$2->brother=$3;}    	
	| Exp AND Exp   		{init_c(&$$,41,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"AND");$$->child=$1;$1->brother=$2;$2->brother=$3;}  			
	| Exp OR Exp     		{init_c(&$$,42,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"OR");$$->child=$1;$1->brother=$2;$2->brother=$3;}  			
	| Exp RELOP Exp         {init_c(&$$,43,NT,@1.first_line,"Exp");/*init_c(&$2,-1,T,0,"RELOP");*/$$->child=$1;$1->brother=$2;$2->brother=$3;}  
	| Exp PLUS Exp    		{init_c(&$$,44,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"PLUS");$$->child=$1;$1->brother=$2;$2->brother=$3;}       	
	| Exp MINUS Exp   		{init_c(&$$,45,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"MINUS");$$->child=$1;$1->brother=$2;$2->brother=$3;}       	
	| Exp STAR Exp    		{init_c(&$$,46,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"STAR");$$->child=$1;$1->brother=$2;$2->brother=$3;}        	
	| Exp DIV Exp  			{init_c(&$$,47,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"DIV");$$->child=$1;$1->brother=$2;$2->brother=$3;}           	 
	| LP Exp RP    			{init_c(&$$,48,NT,@1.first_line,"Exp");init_c(&$1,-1,T,0,"LP");init_c(&$3,-1,T,0,"RP");$$->child=$1;$1->brother=$2;$2->brother=$3;}           
	| MINUS Exp %prec SMINUS     		{init_c(&$$,49,NT,@1.first_line,"Exp");init_c(&$1,-1,T,0,"MINUS");$$->child=$1;$1->brother=$2;}  				  
	| NOT Exp        		{init_c(&$$,50,NT,@1.first_line,"Exp");init_c(&$1,-1,T,0,"NOT");$$->child=$1;$1->brother=$2;}  		
	| ID LP Args RP         {init_c(&$$,51,NT,@1.first_line,"Exp");/*init_c(&$1,-1,ID,0,mytext);*/init_c(&$2,-1,T,0,"LP");init_c(&$4,-1,T,0,"RP");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;} 
	| ID LP RP 		        {init_c(&$$,52,NT,@1.first_line,"Exp");/*init_c(&$1,-1,ID,0,mytext);*/init_c(&$2,-1,T,0,"LP");init_c(&$3,-1,T,0,"RP");$$->child=$1;$1->brother=$2;$2->brother=$3;} 
	| Exp LB Exp RB  		{init_c(&$$,53,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"LB");init_c(&$4,-1,T,0,"RB");$$->child=$1;$1->brother=$2;$2->brother=$3;$3->brother=$4;} 
	| Exp DOT ID     		{init_c(&$$,54,NT,@1.first_line,"Exp");init_c(&$2,-1,T,0,"DOT");init_c(&$3,-1,ID,0,mytext);$$->child=$1;$1->brother=$2;$2->brother=$3;} 
	| ID 					{init_c(&$$,55,NT,@1.first_line,"Exp");/*init_c(&$1,-1,ID,0,mytext);*/$$->child=$1;}  							 
	| INT 					{init_c(&$$,56,NT,@1.first_line,"Exp");init_i(&$1,-1,INT,0,atoi(yytext));$$->child=$1;}  		 
	| FLOAT        			{init_c(&$$,57,NT,@1.first_line,"Exp");init_f(&$1,-1,FLOAT,0,atof(yytext));$$->child=$1;}  
	| error RP				{err = 1;yyerrok;fprintf(stderr, "\n");}		
	;
Args 
	: Exp COMMA Args		{init_c(&$$,58,NT,@1.first_line,"Args");init_c(&$2,-1,T,0,"COMMA");$$->child=$1;$1->brother=$2;$2->brother=$3;}  
	| Exp            		{init_c(&$$,59,NT,@1.first_line,"Args");$$->child=$1;}  
	;

%%
int main(int argc, char* argv[]){
	if(argc >  1) 
	{
		if(!(yyin = fopen(argv[1], "r"))){
			perror(argv[1]);
			return 1;
		}
	}
	yyparse();

	//exp 1
	/*printf("\n");
	if(err==0)	display(root,0);*/

	//exp 2
	Program(root);

	//exp3
	generateLIR(root);

	//exp4
	translate();
	return 0;
}

yyerror(const char* msg){
	fprintf(stderr, "\nError type B at Line %d: %s at \"%s\",", yylineno,msg,yytext);
	err = 1;
}