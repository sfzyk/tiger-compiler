%{
#include <string.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "y.tab.h"
#include "errormsg.h"

int charPos=1;

char str[10];

int yywrap(void)
{
 charPos=1;
 return 1;
}


void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

void intstr(char * strp){
	strp[0]='\0';
}

%}

%x comment_sta string_sta
%%
<comment_sta>{
	"*/"	{
		adjust();
		BEGIN(INITIAL);
	}
	.	{
		adjust();
	}
}
<string_sta>{
	\"	{
		adjust();
		yylval.sval=String(str);
		intstr(str);
		BEGIN(INITIAL);
		return STRING;
	}
	.	{
		adjust();
		strcat(str,yytext);
	}
}

"/*" {adjust();BEGIN(comment_sta);}
\"	 {adjust();BEGIN(string_sta);}
"("  {adjust();return LPAREN;}
")"  {adjust();return RPAREN;}
"{"  {adjust();return LBRACE;}
"}"  {adjust();return RBRACE;}
"["  {adjust();return LBRACK;}
"]"	 {adjust();return RBRACK;}
"."  {adjust();return DOT;}
"+"  {adjust();return PLUS;}
"-"  {adjust();return MINUS;}
"*"  {adjust();return TIMES;}
"/"  {adjust();return DIVIDE;}
"="  {adjust();return EQ;}
"<>" {adjust();return NEQ;}
"<"  {adjust();return LT;}
"<=" {adjust();return LE;}
">"  {adjust(); return GT;}
">=" {adjust(); return GE;}
":=" {adjust(); return ASSIGN;}
","  {adjust(); return COMMA;}
";"  {adjust(); return SEMICOLON;}
":"  {adjust(); return COLON;}

if {adjust(); return IF;}
then {adjust(); return THEN;}
else {adjust(); return ELSE;}
to {adjust(); return TO;}
do {adjust(); return DO;}
let {adjust(); return LET;}
in {adjust(); return IN;}
end {adjust(); return END;}
of  {adjust(); return OF;}
break {adjust();return BREAK;}
function {adjust();return FUNCTION;}
while {adjust(); return WHILE;}
nil {adjust();return NIL;}
type {adjust(); return TYPE;}
var {adjust(); return VAR;}
array {adjust();return ARRAY;}

" "	 {adjust(); continue;}
\n	 {adjust(); EM_newline(); continue;}
\t   {adjust(); continue;}



for  	 {adjust(); return FOR;}
[0-9]+	 {adjust(); yylval.ival=atoi(yytext); return INT;}
[a-zA-Z][a-zA-Z0-9_]* {adjust();yylval.sval=String(yytext); return ID;}
.	 {adjust();printf("unkown : %s",yytext);EM_error(EM_tokPos,"illegal token");}

