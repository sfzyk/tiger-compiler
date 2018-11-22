
// Created by sf on 18-9-24.
//

#ifndef TRANSLATE_H
#define TRANSLATE_H
typedef struct Tr_access_      *Tr_access;
typedef struct Tr_level_       *Tr_level;
typedef struct Tr_accessList_  *Tr_accessList;
typedef struct Tr_exp_         *Tr_exp;
typedef struct Tr_expList_     *Tr_expList;

Tr_accessList Tr_AccessList(Tr_accessList head,Tr_accessList tail);

Tr_expList Tr_ExpList(Tr_exp head,Tr_expList tail);

Tr_level Tr_newLevel(Tr_level,temp_Label,U_boolList);
Tr_level Tr_outermost(void);

Tr_access Tr_accessList();
Tr_access Tr_allocLocal(Tr_level,bool);


// IR tree
Tr_exp Tr_nullEx();
Tr_exp Tr_nullCx();
Tr_exp Tr_nullNx();

Tr_exp Tr_nilExp();
Tr_exp Tr_intExp(A_exp e);
Tr_exp Tr_stringExp(string str);

Tr_exp Tr_simpleVar(Tr_access a, Tr_level l);
Tr_exp Tr_fieldVar(Tr_exp var, int fieldIndex, Tr_level l);
Tr_exp Tr_subscriptVar(Tr_exp var, Tr_exp sub, Tr_level l);

Tr_exp Tr_arOpExp(A_oper o, Tr_exp left, Tr_exp right);
Tr_exp Tr_condOpExp(A_oper o, Tr_exp left, Tr_exp right);
Tr_exp Tr_strOpExp(A_oper o, Tr_exp left, Tr_exp right);
Tr_exp Tr_assignExp(Tr_exp var, Tr_exp exp);

Tr_exp Tr_ifExp(Tr_exp test, Tr_exp then, Tr_exp elsee, Ty_ty ifty);
Tr_exp Tr_whileExp(Tr_exp exp, Tr_exp body, Temp_label breaklbl);
Tr_exp Tr_forExp(Tr_exp explo, Tr_exp exphi, Tr_exp body, Temp_label breaklbl);
Tr_exp Tr_breakExp(Temp_label breaklbl);

Tr_exp Tr_arrayExp(Tr_exp init, Tr_exp size);
Tr_exp Tr_recordExp(Tr_expList el, int fieldCount);

Tr_exp Tr_callExp(Temp_label name, Tr_expList expList);

#endif //LAB5_SEMANTIC_TRANSLATE_H
