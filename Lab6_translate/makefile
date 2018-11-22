b : parse.o y.tab.o errormsg.o util.o symbol.o absyn.o  y.tab.o prabsyn.o lex.yy.o semant.o env.o table.o types.o
	cc -g parse.o errormsg.o util.o symbol.o absyn.o  y.tab.o prabsyn.o lex.yy.o semant.o env.o table.o types.o
a.out: parsetest.o y.tab.o lex.yy.o errormsg.o util.o symbol.o absyn.o table.o
	cc -g parsetest.o y.tab.o lex.yy.o errormsg.o util.o symbol.o absyn.o table.o
translate.o : tanslate.c absyn.h translate.h petiumframe.h frame.h temp.h tree.h util.h error.h
	cc -g -c tanslate.c
frame.o : petiumframe.c frame.h util.h tree.h temp.h
	cc -g -c petiumframe.c -o frame.o

find_escape.o : find_escape.c util.h absyn.h symbol.h table.h find_escape.h errormsg.h
	cc -g -c findescape.c

tree.o : tree.c tree.h util.h symbol.h temp.h
	cc -g -c tree.c

temp.o : temp.c temp.h  util.h symbol.h table.h
	cc -g -c temp.c

types.o : types.c
	cc -g -c types.c

parse.o : parse.c
	cc -g -c parse.c
env.o : env.c
	cc -g -c env.c
table.o : table.c 
	cc -g -c table.c

semant.o : semant.c 
	cc -g -c semant.c 

parsetest.o: parsetest.c errormsg.h util.h
	cc -g -c parsetest.c

absyn.o : absyn.c
	cc -g -c absyn.c

symbol.o : symbol.c
	cc -g -c symbol.c

table.o : table.c
	cc -g -c table.c

y.tab.o: y.tab.c
	cc -g -c y.tab.c

y.tab.c: tiger.grm
	yacc -dv tiger.grm

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

errormsg.o: errormsg.c errormsg.h util.h
	cc -g -c errormsg.c

lex.yy.o: lex.yy.c y.tab.h errormsg.h util.h
	cc -g -c lex.yy.c

lex.yy.c: tiger.lex
	lex tiger.lex

util.o: util.c util.h
	cc -g -c util.c

clean:
	rm -f a.out util.o parsetest.o lex.yy.o errormsg.o y.tab.c y.tab.h y.tab.o *.o *.output
