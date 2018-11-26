//
// Created by lenovo on 2018/9/13.
//


#ifndef TIGER_REMOTE_ENV_H
#define TIGER_REMOTE_ENV_H


typedef struct E_enventry_* E_enventry;


struct E_enventry_{
    enum {E_varEntry,E_funEntry }kind ;
    union{
        struct{Ty_ty ty;Tr_access  access;} var ;
        struct{Tr_level level;Temp_label label ; Ty_tyList formals ; Ty_ty ty;} func ;
    }u;
};

  E_enventry E_VarEntry(Tr_access  access, Ty_ty ty);
  /*
   *  一个函数的词条要包含
   *  函数参数列表 ，返回值信息
   *  函数的标号，   层次信息
   */
  E_enventry E_FunEntry(Ty_tyList formals, Ty_ty ty,Temp_label label,Tr_level level);

S_table E_base_tenv(void);
S_table E_base_venv(void);

#endif //TIGER_REMOTE_ENV_H
