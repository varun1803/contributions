rootdir = $(realpath .)

# - is used at the begining of a command to denote that the exit status of
#   that command is not important
# @ is used at the begining of a command to silent the printing of that command on terminal after execution


bison.tab.c bison.tab.h: bison.y
	@bison -d bison.y

lex.yy.c: syntax_analyser.l
	@flex syntax_analyser.l


all: clean lex.yy.c bison.tab.c
	@g++ -o decaf_compiler bison.tab.c lex.yy.c -lfl `llvm-config --libs core jit native --cxxflags core` `llvm-config --ldflags` -pthread -ltinfo -ldl 

clean_intermediate:
	@-rm -f llvm.o bison_output.txt flex_output.txt bison.output bison.tab.h bison.tab.c lex.yy.c decaf_analyser

clean:
	@-rm -f llvm.o bison_output.txt flex_output.txt bison.output bison.tab.h bison.tab.c lex.yy.c decaf_analyser XML_visitor.txt decaf_compiler IR.txt ll.s

debug:
	@-rm -f bison.output
	@bison -t -d bison.y --report=state
