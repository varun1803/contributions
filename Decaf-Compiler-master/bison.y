%{
#include <fstream>
#include "codegen.cpp"
#include <bits/stdc++.h>

using namespace std;

// stuff from flex that bison needs to know about:
extern "C" int yylex();
extern "C" int yyparse();
extern "C" FILE *yyin;
 
void yyerror(const char *s);

int var_type = 0;//unintialised

Program *root;

%}

// Bison fundamentally works by asking flex to get the next token, which it
// returns as an object of type "yystype".  But tokens could be of any
// arbitrary data type!  So we deal with that in Bison by defining a C union
// holding each of the types of tokens that Flex could return, and have Bison
// use that union instead of "int" for the definition of "yystype":

%define parse.error verbose

%union {
	string *sval;
	BoolLiteral *boo;
    	Literal *lit;
    	IntLiteral *lint;
    	Location *loc;
    	Expr *ex;
    	NormalMethodCall *nmc;
    	MethodCall *mc;
    	CalloutSub *cs;
    	vector<Expr>* fop;
    	vector<CalloutSub>* vcs;
    	AssignmentStmt *as;
    	Statement *stmt;
    	vector<Statement> *vstmt;
    	vector<string> *vstr;
    	VarsSub *vsub;
    	vector<VarsSub> *vvsub;
    	Block *blck;
    	IFStmt *ifelst;
    	MetDecl *mtdc;
    	vector<MetDecl> *vmtdc;
    	FieldDecl *fdl;
    	vector<FieldDecl> *vfdl;
    	Variable *vbl;
    	vector<Variable> *vvbl;	
}

// define the "terminal symbol" token types I'm going to use (in CAPS
// by convention), and associate each with a field of the union:

%token <sval> PROGRAM
%token <sval> INT
%token <sval> BOOLEAN
%token <sval> CHAR
%token <sval> STRING
%token <sval> CLASS
%token <sval> ID 
%token <sval> CALLOUT 
%token <sval> TRUE
%token <sval> FALSE
%token <sval> ROUND_OPEN 
%token <sval> ROUND_CLOSED 
%token <sval> CURLY_OPEN
%token <sval> CURLY_CLOSED
%token <sval> SQUARE_OPEN
%token <sval> SQUARE_CLOSED
%token <sval> IF
%token <sval> ELSE 
%token <sval> BREAK 
%token <sval> RETURN
%token <sval> FOR 
%token <sval> CONTINUE
%token <sval> PLUS 
%token <sval> MINUS	 
%token <sval> MULT
%token <sval> DIV 
%token <sval> MOD 
%token <sval> LT
%token <sval> GT
%token <sval> LE
%token <sval> GE
%token <sval> ASSIGNMENT
%token <sval> INCREMENT
%token <sval> DECREMENT
%token <sval> COMPARISON
%token <sval> NOTEQUAL
%token <sval> AND
%token <sval> OR
%token <sval> HEXADECIMAL
%token <sval> INTEGER
%token <sval> COMMA
%token <sval> SEMICOLON
%token <sval> NOT
%token <sval> VOID
%token <sval> UMINUS

%type <boo> bool_lit
%type <lit> literal
%type <lint> int_lit
%type <loc> location
%type <ex> expr
%type <fop> expr_calls_sub expr_calls
%type <mc> method_call
%type <cs> callout_args_sub
%type <vcs> callout_args
%type <stmt> statement
%type <sval> assign_op type
%type <vstmt> statement_block
%type <vstr> variable_block_list 
%type <vsub> vars_decls_block_sub vars_decl
%type <vvsub> vars_decls_block vars_decls
%type <blck> block else_block
%type <mtdc> met_decl
%type <vmtdc> met_decls
%type <fdl> field_decl
%type <vfdl> field_decls
%type <vbl> variable
%type <vvbl> id_list

%right ASSIGNMENT
%left OR
%left AND
%left COMPARISON NOTEQUAL
%left LT GT LE GE
%left PLUS MINUS
%left MULT DIV MOD
%right NOT
%left UMINUS

%%

prog: CLASS ID CURLY_OPEN field_decls met_decls CURLY_CLOSED 
      { root = new Program(*$4, *$5);  }	
      | CLASS ID CURLY_OPEN  met_decls CURLY_CLOSED 
      { root = new Program(*$4); }	
      | CLASS ID CURLY_OPEN field_decls  CURLY_CLOSED 
      { root = new Program(*$4); }	
      | CLASS ID CURLY_OPEN  CURLY_CLOSED 
      { root = new Program(); }	


field_decls: field_decl { $$ = new vector<FieldDecl>();$$->push_back(*$1); } 
	     | field_decls field_decl { $1->push_back(*$2); $$ = $1; }


field_decl: type id_list SEMICOLON { $$ = new FieldDecl(*$1,*$2); }


id_list:variable { $$ = new vector<Variable>();$$->push_back(*$1); }
	| id_list COMMA variable{ $1->push_back(*$3); $$ = $1; } 


variable: ID { $$ = new Variable(*$1); }
	  | ID SQUARE_OPEN int_lit SQUARE_CLOSED { $$ = new Variable(*$1,*$3); }


met_decls: met_decl{ $$ = new vector<MetDecl>();$$->push_back(*$1); }
	  | met_decls met_decl
	   { $1->push_back(*$2); $$ = $1; }


met_decl: VOID ID ROUND_OPEN ROUND_CLOSED block
       	{ $$ = new MetDecl("void", *$2, *$5); }
	 | type ID ROUND_OPEN ROUND_CLOSED block
       	{ $$ = new MetDecl(*$1, *$2, *$5); }
	 | type ID ROUND_OPEN vars_decls ROUND_CLOSED block  
       	{ $$ = new MetDecl(*$1, *$2, *$4, *$6); }
	 | VOID ID ROUND_OPEN vars_decls ROUND_CLOSED block
       	{ $$ = new MetDecl("void", *$2, *$4, *$6); }
;


vars_decls: vars_decl{ $$ = new vector<VarsSub>(); $$->push_back(*$1); } 
	    | vars_decls COMMA vars_decl{ $1->push_back(*$3); $$ = $1; } 


vars_decl: type ID { vector<string> st; st.push_back(*$2); $$ = new VarsSub(*$1, st); }


block: CURLY_OPEN vars_decls_block statement_block CURLY_CLOSED{ $$ = new Block(*$2, *$3); } 
       | CURLY_OPEN statement_block CURLY_CLOSED{ $$ = new Block(*$2); }


vars_decls_block: vars_decls_block_sub { $$=new vector<VarsSub>(); $$->push_back(*$1); }
	          | vars_decls_block vars_decls_block_sub { $1->push_back(*$2); $$=$1; }


vars_decls_block_sub: type variable_block_list SEMICOLON { $$ = new VarsSub(*$1, *$2); }


variable_block_list: ID { $$ = new vector<string>(); $$->push_back(*$1); }
		     | variable_block_list COMMA ID { $1->push_back(*$3); $$ = $1; }


statement_block:{ $$ = new vector<Statement>(); } 
		 | statement_block statement{ $1->push_back(*$2); $$=$1; } 


type: INT{ $$ = new string("int"); } 
      | BOOLEAN{ $$ = new string("boolean"); }


statement: location assign_op expr SEMICOLON
	 {AssignmentStmt *assign = new AssignmentStmt(*$1, *$2, *$3); $$ = new Statement(*assign);  }
    	   | method_call SEMICOLON
	{ $$ = new Statement(*$1); }
	   | IF ROUND_OPEN expr ROUND_CLOSED block else_block
	{ IFStmt *ifstmt = new IFStmt(*$3, *$5, *$6); $$ = new Statement(*ifstmt); } 
	   | FOR ID ASSIGNMENT expr COMMA expr block 
	{ Assign *assign = new Assign(*$2, *$4); ForStmt *forstmt = new ForStmt(*assign, *$6, *$7); $$ = new Statement(*forstmt); } 
	   | RETURN SEMICOLON 
	 { ReturnStmt *ret = new ReturnStmt(); $$ = new Statement(*ret); }
	   | RETURN expr SEMICOLON
	 { ReturnStmt *ret = new ReturnStmt(*$2); $$ = new Statement(*ret); }
	   | BREAK SEMICOLON 
	 { BreakStmt *bre = new BreakStmt(); $$ = new Statement(*bre); }
	   | CONTINUE SEMICOLON 
	 { ContStmt *cont = new ContStmt(); $$ = new Statement(*cont); }
  	   | block 
	 { $$ = new Statement(*$1); }


else_block:{ $$ = new Block(); }
	   |ELSE block{ $$ = $2; } 


assign_op: ASSIGNMENT { $$ = new string("="); }
	   | INCREMENT { $$ = new string("+="); }
	   | DECREMENT { $$ = new string ("-="); }


method_call: ID ROUND_OPEN expr_calls ROUND_CLOSED 
	   { NormalMethodCall *normal = new NormalMethodCall(*$1, *$3); $$ = new MethodCall(*normal); }
	     | CALLOUT ROUND_OPEN STRING callout_args ROUND_CLOSED 
	   { $3=new string($3->substr(1, $3->length() - 2)); CalloutMethodCall *call=new CalloutMethodCall(*$3, *$4); $$ = new MethodCall(*call); }


expr_calls: { $$ = new vector<Expr>(); }
	    | expr_calls_sub { $$ = $1; }


expr_calls_sub: expr { $$ = new vector<Expr>(); $$->push_back(*$1); }
		| expr_calls_sub COMMA expr{ $1->push_back(*$3); $$=$1; } 


location: ID { $$ = new Location(*$1);   }
	  | ID SQUARE_OPEN expr SQUARE_CLOSED { $$ = new Location(*$1, *$3); } 


callout_args: COMMA callout_args_sub { $$ = new vector<CalloutSub>(); $$->push_back(*$2); }
	      | callout_args COMMA callout_args_sub { $1->push_back(*$3); $$=$1; }


callout_args_sub: expr { $$ = new CalloutSub(*$1); }
		  | STRING{ $1=new string($1->substr(1, $1->length() - 2)); $$ = new CalloutSub(*$1);  } 


expr: expr OR expr 
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr AND expr 
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr LT expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr GT expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr LE expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr GE expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr COMPARISON expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr NOTEQUAL expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr PLUS expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr MINUS expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr MULT expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr DIV expr
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | expr MOD expr 
	{ Binexpr *bin = new Binexpr(*$1, *$2, *$3); $$ = new Expr(*bin); }
      | MINUS expr %prec UMINUS 
	{ Unexpr *uni = new Unexpr(*$1, *$2); $$ = new Expr(*uni); }
      | NOT expr 
	{ Unexpr *uni = new Unexpr(*$1, *$2); $$ = new Expr(*uni); }
      | ROUND_OPEN expr ROUND_CLOSED  { $$ = $2; }
      | literal	 { $$ = new Expr(*$1); }
      | location { $$ = new Expr(*$1); }
      | method_call { $$ = new Expr(*$1); }


literal: int_lit { $$ = new Literal((*$1).val); }
	 | CHAR { $$ = new Literal(*$1); }
	 | bool_lit { $$ = new Literal((*$1).val); }


int_lit: INTEGER  { $$ = new IntLiteral(*$1);  }
	 | HEXADECIMAL { $$ = new IntLiteral(*$1); }


bool_lit: TRUE 	{ $$ = new BoolLiteral(*$1); }
	  |FALSE { $$ = new BoolLiteral(*$1); }


%%

int main(int argc, char *argv[]) {

	// open a file handle to a particular file:
	FILE *myfile = fopen(argv[1], "r");
	// make sure it is valid:
	if (!myfile) {
	//	cout << "Can't locate an input file" << endl;
		return -1;
	}
	// set flex to read from it instead of defaulting to STDIN:
	yyin = myfile;
	
	// parse through the input until there is no more:
	do {
		yyparse();
	} while (!feof(yyin));
	
	CodeGenContext context;
        context.generateCode(root);
	context.module->dump();

	return 0;
}

void yyerror(const char *s) {
//	cout << "EEK, parse error!  Message: " << s << endl;
	printf("Syntax Error\n");
	// might as well halt now:
	exit(-1);
	
}
