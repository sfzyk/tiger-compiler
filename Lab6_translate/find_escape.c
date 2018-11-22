//
// Created by lenovo on 2018/11/12.
//
#include "util.h"
#include "absyn.h"
#include "symbol.h"
#include "table.h"
#include "find_escape.h"
#include "errormsg.h"
#include <assert.h>

// 变量产生于下面三个区域
// let 中的变量定义
// for 中的隐式变量定义
// 函数调用中的形式参数
// 所以escape解决的就是这些变量的逃逸问题

struct EscapeEntry_{
    int depth;
    bool * esc;
};

typedef struct EscapeEntry_* EscapeEntry;

EscapeEntry escapeEntry(int depth,bool *esc){
    EscapeEntry p=checked_malloc(sizeof(*p));
    p->depth=depth;
    p->esc=esc;
    return p;
}

static void traverseExp(S_table env,int depth,A_exp exp);
static void traverseDec(S_table env,int depth,A_dec dec);
static void traverseVar(S_table env,int depth,A_var var);

void Esc_findEscape(A_exp exp){
    S_table env=S_empty();
    traverseExp(env,0,exp);
}



static void traverseExp(S_table env,int depth,A_exp exp){
    switch (exp->kind){
        case A_varExp:
            traverseVar(env,depth+1,exp->u.var);
            break;
        case A_callExp:
            A_expList args=exp->u.call.args;
            for(;args;args=args->tail){
                // 参数中的exp 是本层的 出现了不意味着逃逸
                traverseExp(env,depth,args->tail);
            }
            break;
        case A_opExp:
            traverseExp(env,depth,exp->u.op.left);
            traverseExp(env,depth,exp->u.op.right);
            break;
        case A_recordExp:
            A_efieldList f= exp->u.record.fields;
            for(;f;f=f->tail){
                traverseExp(env,depth,f->head->exp);
            }
            break;
        case A_seqExp:
            A_expList stp=exp->u.seq;
            for(;stp;stp=stp->tail){
                traverseExp(env,depth,stp->head);
            }
            break;
        case A_assignExp:
            traverseVar(env,depth,exp->u.assign.var);
            traverseExp(env,depth,exp->u.assign.exp);
            break;
        case A_ifExp:
            traverseExp(env,depth,exp->u.iff.test);
            traverseExp(env,depth,exp->u.iff.elsee);
            traverseExp(env,depth,exp->u.iff.then);
            break;
        case A_whileExp:
            traverseExp(env,depth,exp->u.whilee.test);
            traverseExp(env,depth,exp->u.whilee.body);
            break;
        case A_forExp:
            traverseExp(env,depth,exp->u.forr.body);
            traverseExp(env,depth,exp->u.forr.lo);
            traverseExp(env,depth,exp->u.forr.hi);
            traverseVar(env,depth,exp->u.forr.var);
            break;
        case A_letExp:
            S_beginScope(env);

            A_decList decs=exp->u.let.decs;
            for(;decs;decs=decs->tail) {
                traverseDec(env, depth+1,decs->head);
            }
            traverseExp(env,depth+1,exp->u.let.body);
            S_endScope(env);
            break;
        case A_arrayExp:
            traverseExp(env,depth,exp->u.array.size);
            traverseExp(env,depth,exp->u.array.init);
            break;
        case A_nilExp:
        case A_breakExp:
        case A_stringExp:
        case A_intExp:
            break;
        default:
            assert(0);
    }
    return;
}

static void traverseDec(S_table env,int depth,A_dec dec){
    switch(dec->kind){
        case A_varDec:
            dec->u.var.escape=false;
            S_enter(env,dec->u.var.var,escapeEntry(depth,&dec->u.var.escape));
            traverseExp(env,depth,dec->u.var.init);
            break;
        case A_functionDec:
            A_fundecList f= dec->u.function;
            for(;f;f=f->tail){ //对于所有的函数定义
                A_fieldList f2 = f->head->params;
                S_beginScope(env);
                for(;f2;f2=f2->tail){
                    f2->head->escape=false;
                    S_enter(env,f2->head->name,escapeEntry(depth+1,&(f2->head->escape)));
                }
                traverseExp(env,depth+1,f->head->body);
                S_endScope();
            }
            break;
        case A_typeDec:
            break;// 不涉及变量逃逸的设置
        default:
            assert(0);
    }
    return;
}


static void traverseVar(S_table env,int depth,A_var var){
    switch(var->kind){
        case A_simpleVar:
            EscapeEntry p = (EscapeEntry)S_look(env,var->u.simple);
            if(!p){
                EM_error(0,"undefined var");
            }else{
                //只有这种情况下是的
                if(depth > p->depth ){
                    *(p->esc)= true;
                }
            }
            break;
        case A_fieldVar:
            traverseVar(env,depth,var->u.field.var);
            break;
        case A_subscriptVar:
            traverseVar(env,depth,var->u.subscript.var);
            break;
        default:
            assert(0);
    }
}
