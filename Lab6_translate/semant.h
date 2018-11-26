//
// Created by lenovo on 2018/9/13.
//


#ifndef TIGER_REMOTE_SEMANT_H
#define TIGER_REMOTE_SEMANT_H


void SEM_transProg(A_exp exp);

struct expty transVar(Tr_level lev,S_table venv, S_table tenv,A_var v,Temp_label breakbl);
struct expty transExp(Tr_level lev,S_table venv, S_table tenv,A_exp a,Temp_label breakbl);
void         transDec(Tr_level lev,S_table venv,S_table tenv, A_dec d,Temp_label breakbl);
Ty_ty transTy(S_table tenv, A_ty t);




#endif //TIGER_REMOTE_SEMANT_H
