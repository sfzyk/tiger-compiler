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
Temp_label F_name(F_frame f);

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

Temp_temp F_RV(void);//用作返回值的单元

T_stm F_procEntryExit1(F_frame,T_stm);//处理函数入口代码

typedef struct F_frag_ * F_frag;
struct F_frag_{
    F_frame enum{F_stringFrag,F_procFrag}kind;
    union{
        struct{Temp_label label;
        string str;
        }stringg;
        struct{
            T_stm body;F_frame frame;
        }proc;
    }u;
};

F_frag F_StringFrag(Temp_label,string str);
F_frag F_procEntryExit(Tr_level level,Tr_exp body,Tr_accessList formals);

typedef struct F_fragList_* F_fragList;
struct F_fragList_ {F_frag head,F_fragList tail;};
F_fragList F_FragList(F_frag,F_fragList);

#endif //LAB5_SEMANTIC_FRAME_H
