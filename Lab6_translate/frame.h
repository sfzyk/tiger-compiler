//
// Created by sf on 18-9-24.
//

#ifndef FRAME_H
#define FRAME_H
typedef struct F_frmae_ *F_frame;
typedef struct F_access_ *F_access;
typedef struct F_accessList_ * F_accessList;

struct F_accessList_ {F_access head;F_accessList tail;};



//获取新的栈帧
F_frame F_newFrame(Temp_label name,U_boolList formals);

//
temp_label F_name(F_frame f);

F_accessList F_formals(F_frame f);
F_access   F_allocLocal(F_frame f,bool escape);

Temp_temp F_FP(void);
extern const int F_wordSize;
/*
 * tanslate 模块调用 F_exp 将一个访问转换成IR 表示
 * 这是模块分离的较好表现
 */
T_exp F_exp(F_access acc,T_exp framePtr);

/*
 * 栈帧指针 查找上一个栈帧指针
 */
T_exp F_FPExp(T_exp);

/*
 * 调用外部函数
 */
T_exp F_externalCall(string,T_expList);

#endif //LAB5_SEMANTIC_FRAME_H
