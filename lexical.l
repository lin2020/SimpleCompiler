%{
	#include "syntax.tab.h"
	#include <string.h>
	char mytext[32];
	int err=0;
	int yycolumn = 1;
	#define YY_USER_ACTION yylloc.first_line = yylloc.last_line = yylineno;\
		yylloc.first_column = yycolumn;\
		yylloc.last_column = yycolumn + yyleng - 1;\
		yycolumn += yyleng;
%}

%option yylineno
yylineno = 1;

int [+-]?[0-9]+
float [+-]?([0-9]*\.?[0-9]+|[0-9]+\.)
id [_A-Za-z][A-Za-z0-9_]*
type "int"|"float"

%%
{int} 	{/*printf("(INT:%d) ",INT);*/return INT;}
{float} {/*printf("(FLOAT:%d) ",FLOAT);*/return FLOAT;}
{type}	{/*printf("(TYPE:%d) ",TYPE);*/return TYPE;}
{id} 	{if(strcmp(yytext,"struct") == 0) {/*printf("(STRUCT:%d) ",STRUCT);*/return STRUCT;}
		else if(strcmp(yytext,"return") == 0) {/*printf("(RETURN:%d) ",RETURN);*/return RETURN;}
		else if(strcmp(yytext,"if") == 0) {/*printf("(IF:%d) ",IF);*/return IF;}
		else if(strcmp(yytext,"else") == 0) {/*printf("(ELSE:%d) ",ELSE);*/return ELSE;}
		else if(strcmp(yytext,"while") == 0) {/*printf("(WHILE:%d) ",WHILE);*/return WHILE;}
		else {/*printf("(ID:%d) ",ID);*/strcpy(mytext,yytext);init_c(&yylval,54,ID,yylineno,yytext);return ID;}}
";" 	{/*printf("(SEMI:%d) ",SEMI);*/return SEMI;}
"," 	{/*printf("(COMMA:%d) ",COMMA);*/return COMMA;}
"=" 	{/*printf("(ASSIGNOP:%d) ",ASSIGNOP);*/return ASSIGNOP;}
">"|"<"|">="|"<="|"=="|"!=" 	{
						/*printf("(RELOP:%d) ",RELOP);*/
						init_c(&yylval,-1,ID,yylineno,yytext);
						return RELOP;
					}
"+" 	{/*printf("(PLUS:%d) ",PLUS);*/return PLUS;}
"-" 	{/*printf("(MINUS:%d) ",MINUS);*/return MINUS;}
"*" 	{/*printf("(STAR:%d) ",STAR);*/return STAR;}
"/" 	{/*printf("(DIV:%d) ",DIV);*/return DIV;}
"&&" 	{/*printf("(AND:%d) ",AND);*/return AND;}
"||" 	{/*printf("(OR:%d) ",OR);*/return OR;}
"." 	{/*printf("(DOT:%d) ",DOT);*/return DOT;}
"!" 	{/*printf("(NOT:%d) ",NOT);*/return NOT;}
"(" 	{/*printf("(LP:%d) ",LP);*/return LP;}
")" 	{/*printf("(RP:%d) ",RP);*/return RP;}
"[" 	{/*printf("(LB:%d) ",LB);*/return LB;}
"]" 	{/*printf("(RB:%d) ",RB);*/return RB;}
"{" 	{/*printf("(LC:%d) ",LC);*/return LC;}
"}" 	{/*printf("(RC:%d) ",RC);*/return RC;}
"\n"	{/*printf("\n") ;*/yycolumn+=1;}
"//"	{
			char c;
			while((c=input())!='\n');
		}
"/*"	{
			char c,c1;
			loop:
				while((c=input())!='*' && c!=0);
				if((c1=input())!='/' && c!=0);
		}
" "		{}
"	"	{}
.		{printf("Error type A at Line %d: mysterious characters \"%s\".\n",yylineno,yytext);if(err==0)err=1;}
%%