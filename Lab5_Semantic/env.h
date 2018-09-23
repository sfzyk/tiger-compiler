//
// Created by lenovo on 2018/9/13.
//

#ifndef TIGER_REMOTE_ENV_H
#define TIGER_REMOTE_ENV_H


typedef struct E_enventry_* E_enventry;


struct E_enventry_{
    enum {E_varEntry,E_funEntry }kind ;
    union{
        struct{Ty_ty ty;} var ;
        struct{Ty_tyList formals ; Ty_ty ty;} func ;
    }u;
};

  E_enventry E_VarEntry(Ty_ty ty);
  E_enventry E_FunEntry(Ty_tyList formals, Ty_ty ty);

S_table E_base_tenv(void);
S_table E_base_venv(void);

#endif //TIGER_REMOTE_ENV_H
