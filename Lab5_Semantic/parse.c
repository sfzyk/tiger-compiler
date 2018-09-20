/*
 * parse.c - Parse source file.
 */
#include <stdlib.h>
#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "prabsyn.h"
#include "errormsg.h"
#include "parse.h"
#include "semant.h"
extern int yyparse(void);
extern A_exp absyn_root;

/* parse source file fname; 
   return abstract syntax data structure */
A_exp parse(string fname) 
{EM_reset(fname);
 if (yyparse() == 0) /* parsing worked */
   return absyn_root;
 else return NULL;
}

int main(int argc, char ** argv){
	if(argc!=2){
		fprintf(stderr,"input error");
		exit(1);
	}
	parse(argv[1]);
	//pr_exp(stderr,absyn_root,0);
	SEM_transProg(absyn_root);
	fprintf(stderr,"\n");
	return 0;
}
