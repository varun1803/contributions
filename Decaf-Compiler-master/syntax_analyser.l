%{
#include "ast.h"
#include "bison.tab.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <list>
using namespace std;
#define YY_DECL extern "C" int yylex()

FILE *outfile = fopen("flex_output.txt", "w+");
//[ \t\n,\[\]\{\};\+\=\-\/\*\(\)]	;
%}
%%

[ ]*[\t]*\/\/.*			{ fprintf(outfile, "COMMENT: %s\n", yytext); }
[,]				{ yylval.sval = new string(yytext,yyleng); return COMMA; }
[\;]				{ yylval.sval = new string(yytext, yyleng); return SEMICOLON; }
[\(]				{ yylval.sval = new string(yytext, yyleng); return ROUND_OPEN; }
[\)]				{ yylval.sval = new string(yytext, yyleng); return ROUND_CLOSED; }
[\{]				{ yylval.sval = new string(yytext, yyleng); return CURLY_OPEN; }
[\}]				{ yylval.sval = new string(yytext, yyleng); return CURLY_CLOSED; }
[\[]				{ yylval.sval = new string(yytext, yyleng); return SQUARE_OPEN; }
[\]]				{ yylval.sval = new string(yytext, yyleng); return SQUARE_CLOSED; }
if				{ yylval.sval = new string(yytext, yyleng); return IF; }
else				{ yylval.sval = new string(yytext, yyleng); return ELSE; }
break				{ yylval.sval = new string(yytext, yyleng); return BREAK; }
return				{ yylval.sval = new string(yytext, yyleng); return RETURN; }
for				{ yylval.sval = new string(yytext, yyleng); return FOR; }
continue			{ yylval.sval = new string(yytext, yyleng); return CONTINUE; }
void				{ yylval.sval = new string(yytext, yyleng); return VOID; }
[\+]				{ yylval.sval = new string(yytext, yyleng); return PLUS; }
[\-]				{ yylval.sval = new string(yytext, yyleng); return MINUS; }
[\*]				{ yylval.sval = new string(yytext, yyleng); return MULT; }
[\/]				{ yylval.sval = new string(yytext, yyleng); return DIV; } 
[\%]				{ yylval.sval = new string(yytext, yyleng); return MOD; }
\<				{ yylval.sval = new string(yytext, yyleng); return LT; }
\>				{ yylval.sval = new string(yytext, yyleng); return GT; }
\<\=				{ yylval.sval = new string(yytext, yyleng); return LE; }
\>\=				{ yylval.sval = new string(yytext, yyleng); return GE; }
\=				{ yylval.sval = new string(yytext, yyleng); return ASSIGNMENT; }
\+\=				{ yylval.sval = new string(yytext, yyleng); return INCREMENT; }
\-\=				{ yylval.sval = new string(yytext, yyleng); return DECREMENT; }
\=\=				{ yylval.sval = new string(yytext, yyleng); return COMPARISON; }
\!\=				{ yylval.sval = new string(yytext, yyleng); return NOTEQUAL; }
\!				{ yylval.sval = new string(yytext, yyleng); return NOT; }
\&\&				{ yylval.sval = new string(yytext, yyleng); return AND; }
\|\|				{ yylval.sval = new string(yytext, yyleng); return OR; }
boolean				{ fprintf(outfile, "BOOLEAN_DECLARATION\n"); yylval.sval = new string(yytext, yyleng);
				  return BOOLEAN; }
callout 			{ fprintf(outfile, "CALLOUT\n"); yylval.sval = new string(yytext, yyleng);  return CALLOUT; }
class   			{ fprintf(outfile, "CLASS\n"); yylval.sval = new string(yytext, yyleng); return CLASS; }
false				{ fprintf(outfile, "BOOLEAN: false\n"); yylval.sval = new string(yytext, yyleng);  return FALSE;  }
int				{ fprintf(outfile, "INT_DECLARATION\n"); yylval.sval = new string(yytext, yyleng); return INT; }
true				{ fprintf(outfile, "BOOLEAN: true\n"); yylval.sval = new string(yytext, yyleng);return TRUE;  }
[a-zA-Z][a-zA-Z0-9]*    	{ fprintf(outfile, "ID: %s\n", yytext); yylval.sval = new string(yytext, yyleng);  return ID; }
0x[a-fA-F0-9]+         		{ fprintf(outfile, "HEX: %s\n", yytext); yylval.sval = new string(yytext, yyleng); return HEXADECIMAL;}
-?[0-9]+          		{ fprintf(outfile, "INT: %s\n", yytext); yylval.sval = new string(yytext, yyleng); return INTEGER;}
[a-zA-Z0-9]\'    		{ fprintf(outfile, "CHAR: %s\n", yytext); yylval.sval = new string(yytext, yyleng); return CHAR; }
\"[^"]*\"    			{ fprintf(outfile, "STRING: %s\n", yytext); yylval.sval = new string(yytext, yyleng);  return STRING; }
[\n]				yylineno++;
[\t]				;
%%
/*iint main(int, char**) {

	// lex through the input:
	FILE *infile = fopen("test_file/test_program", "r");
	// make sure it's valid:
	if (!infile) {
		printf("Can't open input program file\n");
		return -1;
	}
	// set lex to read from it instead of defaulting to STDIN:
	yyin = infile;
	
	// lex through the input:
	yylex();
	fclose(outfile);
	fclose(infile);
}
*/
