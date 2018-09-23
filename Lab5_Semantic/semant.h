//
// Created by lenovo on 2018/9/13.
//


#ifndef TIGER_REMOTE_SEMANT_H
#define TIGER_REMOTE_SEMANT_H

typedef void * Tr_exp;
void SEM_transProg(A_exp exp);
struct expty transVar(S_table venv, S_table tenv,A_var v);
struct expty transExp(S_table venv, S_table tenv,A_exp a);
void         transDec(S_table venv,S_table tenv, A_dec d);
Ty_ty transTy(S_table tenv, A_ty t);




#endif //TIGER_REMOTE_SEMANT_H
