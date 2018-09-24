//
// Created by lenovo on 2018/9/13.
//
#include <stdio.h>
#include "util.h"
#include "symbol.h"
#include "absyn.h"
#include "errormsg.h"
#include "types.h"
#include "env.h"
#include "semant.h"

struct expty{Tr_exp exp;Ty_ty ty;};

struct expty expTy(Tr_exp exp,Ty_ty ty){
    struct expty e;
    e.exp=exp;
    e.ty=ty;
    return e;
}


void SEM_transProg(A_exp exp){
    S_table tenv=E_base_tenv();
    S_table venv=E_base_venv();

    transExp(venv,tenv,exp);
}

Ty_ty actual_ty(Ty_ty p){
    while(p && p->kind==Ty_name) {
        p=p->u.name.ty;
    }
    return p;
}
/*    将语义分析得到的  A_fieldList
 *    转换成类型分析所需要的 Ty_tyList
 *    实现上有一个非常ugly的 因为传入的param 是向后链接的单链表
 *    输出的也是向后链接的单链表 ， 自然的实现会把链表倒置
 *    只能使用这种实现方式乐
 */
Ty_tyList makeFormalTyList(S_table tenv, A_fieldList param){
    Ty_tyList p=NULL;           //前一个参数
    Ty_tyList n=NULL;           //当前参数
    Ty_tyList first=NULL;       //第一个参数
    while(param) {
        Ty_ty arg_ty = S_look(tenv, param->head->typ);
        if (!arg_ty) {
            EM_error(param->head->pos, "not such type");
            break;
        }

        n = Ty_TyList(arg_ty, NULL);
        if(p){
            p->tail=n;
        }
        if(!first){
            first=n;
        }

        p=n;
	    param = param -> tail;
    }
    return first;
}


/*
 *  接受的类型定义是 A_Ty 类型的
 *  A_Ty 类型也只会在语义解析中的 dec 中出现
 *  进入环境中需要的是新定义的 "类型"类型
 */
Ty_ty transTy(S_table tenv,A_ty ty){
    switch(ty->kind) {
        case A_recordTy: {
            A_fieldList p = ty->u.record;
            Ty_fieldList k = NULL;
            while (p) {
                Ty_ty fieldty = S_look(tenv, p->head->typ);
                if (fieldty) {
					Ty_field field= Ty_Field(p->head->name,fieldty);
                    k = Ty_FieldList(field, k);
                    p = p->tail;
                } else {
                    EM_error(ty->pos, "no such field type");
                    break;
                }
            }
            return Ty_Record(k);
        }
            break;
        case A_arrayTy: {

            return Ty_Array(S_look(tenv,ty->u.array));
        }
break;
        case A_nameTy: {

            return Ty_Name(ty->u.name, S_look(tenv, ty->u.name));
        }
            break;
        default:
            EM_error(ty->pos, "Type unknown type");
    }
}
void transDec(S_table venv,S_table tenv ,A_dec dec){

    switch (dec->kind) {
        case A_typeDec: {
            A_nametyList p = dec->u.type;

            /* 第一遍 扫描 加入类型的头 */
            while (p) {
                // 只进行后向查冲即可
                for(A_nametyList  i= p->tail;i;i=i->tail) {
                    if(!i)break ;
                    if(i->head->name == p->head->name){
                        EM_error(dec->pos,"type dec not allow same name");
                    }
                }
                S_enter(tenv, p->head->name, Ty_Name(p->head->name, NULL));
                p = p->tail;
            }
            /* 第二遍 扫描 加入全部的内容 */
            p = dec->u.type;
            while (p) {
                Ty_ty wait = S_look(tenv, p->head->name);
                /* 去除第一遍的值重新改写 */
				Ty_ty temp = transTy(tenv, p->head->ty);
				
                wait->u.name.ty = temp;
                /* 这里使用 actual_ty 是为了一般的 Namety 深入 不是为了嵌套
                 * 更一般的 使用两次扫描的 模式也是 为了在 actual_ty 的时候正确的返回
                 * */
                Ty_ty wait_true = actual_ty(wait);
                if(wait_true) {   //  所有的非 Namety 类型  actual_ty 的结果就是他自身 没有意义
                                  //  NameTy  类型执行actul_ty 不良的定义会导致这里的 wait_true 为空
                    S_enter(tenv, p->head->name, wait_true);
                }else{
                    EM_error(dec->pos,"bad dec Namety");
					break;
                }
				p=p->tail;
			    }
        }
        break;
            /*
             *  还是直接写成短情况和长情况分开的形式吧 省心
             */
        case A_varDec: {
            Ty_ty varty;
            varty=transExp(venv,tenv,dec->u.var.init).ty;



            if(dec->u.var.typ){
                Ty_ty mark_varty=S_look(tenv,dec->u.var.typ);
                if(varty->kind!=mark_varty->kind && varty->kind!=Ty_nil){
                    EM_error(dec->pos,"mismatch type for var dec");
                }
                S_enter(venv,dec->u.var.var,E_VarEntry(mark_varty));

            }else{
                if(varty->kind==Ty_nil){
                    EM_error(dec->pos,"short term can't be Nil");
                }
                S_enter(venv,dec->u.var.var,E_VarEntry(varty));
            }
        }
            break;
        case A_functionDec: {
            A_fundecList f= dec->u.function;
	    //第一遍扫描记录所有函数的头部信息
            while(f){
                for(A_fundecList i=f->tail;i;i=i->tail){
                    if(!i)break;
                    if(i->head->name == f->head->name){
                        EM_error(dec->pos,"func dec not allow same name");
                    }
                }


                Ty_tyList formals=makeFormalTyList(tenv,f->head->params);
                Ty_ty        retty;
		        if(f->head->result)
         		    retty= S_look(tenv,f->head->result);
		        else
			        retty= Ty_Void();

                if(!retty) {
                    EM_error(f->head->pos, "undefined return type");
                }

                E_enventry funenv=E_FunEntry(formals,retty);
                S_enter(venv,f->head->name,funenv);
                f=f->tail;
            }
		
	    //第二编扫描解决expbody的问题
            f= dec->u.function;
	        while(f){
                E_enventry funwait = S_look(venv, f->head->name );
                A_fieldList  params = f->head->params;
                S_beginScope(venv);
                S_beginScope(tenv);

                while(params){
                    Ty_ty argty = S_look(tenv,params->head->typ);
                    E_enventry ent=E_VarEntry(argty);
                    S_enter(venv,params->head->name,ent);
                    params=params->tail;
                }

                struct expty truety =transExp(venv,tenv,f->head->body);
                if(truety.ty!=funwait->u.func.ty){
                    EM_error(f->head->pos,"return type error");
                }

                S_endScope(venv);
                S_endScope(tenv);

                f=f->tail;
	        }

        }break;
    }
}

struct expty transVar(S_table venv,S_table tenv ,A_var a){
    switch(a->kind) {
        case A_simpleVar: {
            E_enventry x = S_look(venv, a->u.simple);
            if (x && x->kind == E_varEntry) {
                return expTy(NULL, actual_ty(x->u.var.ty));
            } else {
                EM_error(a->pos, "undefined variable", S_name(a->u.simple));
                return expTy(NULL, Ty_Int());
            }
        }
        break;
        case A_fieldVar:{
            struct expty e=transVar(venv,tenv,a->u.field.var);
            if(e.ty->kind==Ty_record) {
                Ty_fieldList p = e.ty->u.record;

                while (p && p->head->name != a->u.field.sym) {
                    p = p->tail;
                }

                if (p) {
                    return expTy(NULL, p->head->ty);
                } else {
                    EM_error(a->pos, "var has no such field");
                }
            }else{
                EM_error(a->pos,"invaild left value");
            }

            return expTy(NULL,Ty_Void());

        }
        break;
        case A_subscriptVar:{
            struct expty var=transVar(venv,tenv,a->u.subscript.var);
            struct expty sub=transExp(venv,tenv,a->u.subscript.exp);
            if(var.ty->kind==Ty_array ){
                if(sub.ty->kind==Ty_int){
                    return expTy(NULL,var.ty->u.array);
                }else{
                    EM_error(a->pos,"subscript not int");
                }
            }else{
                EM_error(a->pos,"not arrat type");
            }
        }
        break;
        default:
            EM_error(a->pos,"unknown type");
    }
}

struct expty transExp(S_table venv, S_table tenv,A_exp a){
    switch(a->kind){
        case A_opExp:{
            A_oper oper= a->u.op.oper ;
            struct expty left  = transExp(venv,tenv,a->u.op.left);
            struct expty right = transExp(venv,tenv,a->u.op.right);

            if (oper==A_plusOp | oper==A_minusOp | oper==A_timesOp | oper==A_divideOp | oper==A_leOp | oper==A_gtOp | oper==A_geOp){
                /* A_plusOp, A_minusOp, A_timesOp, A_divideOp,
                        A_eqOp, A_neqOp, A_ltOp, A_leOp, A_gtOp, A_geOp */
                    if (left.ty->kind != Ty_int) {
                        EM_error(a->u.op.left->pos, "i need integer");
                    }
                    if(right.ty->kind != Ty_int){
                        EM_error(a->u.op.left->pos,"i need interger");
                    }
                    return expTy(NULL,Ty_Int());
            }
            if(oper==A_eqOp|oper==A_neqOp){
                    if(left.ty->kind!=right.ty->kind && left.ty->kind!=Ty_nil && right.ty->kind!=Ty_nil){
                        EM_error(a->u.op.right->pos,"value type not match");
                    }
                    return expTy(NULL,Ty_Int());
            }
        }
        break;
        case A_varExp:{
            struct expty e=transVar(venv,tenv,a->u.var);
            return expTy(NULL,e.ty);
        }
        break;
        case A_seqExp:{
           A_expList p=a->u.seq;
           while(p && p->tail){
               transExp(venv,tenv,p->head);
               p=p->tail;
           }

           struct expty e;
            if(p) e = transExp(venv,tenv,p->head);
                else  e = expTy(NULL,Ty_Void());

           return expTy(NULL,e.ty);
        }
        break;
        case A_breakExp:{
            return expTy(NULL,Ty_Void());
        }
        break;
        case A_nilExp:{
            return expTy(NULL,Ty_Nil());
        }
        break;
		case A_intExp:{
            return expTy(NULL,Ty_Int());
        }
        break;
        case A_stringExp:{
            return expTy(NULL,Ty_String());
        }
        break;
        case A_callExp:{
            E_enventry e=S_look(venv,a->u.call.func);
            if(e && e->kind==E_funEntry){
                Ty_tyList p_actual=e->u.func.formals;
                A_expList p=a->u.call.args;
                while(p && p_actual){
                    struct expty argty = transExp(venv,tenv,p->head);

                    if(argty.ty->kind == p_actual->head->kind){
                    }else{
                        EM_error(p->head->pos,"func args not match");
                    }
                    p       =p->       tail;
                    p_actual=p_actual->tail;
                }

                if(p){
                    	EM_error(a->pos,"arg number too much");
                }else if(p_actual){
			        EM_error(a->pos,"arg number not enough");
		        }
                return expTy(NULL,e->u.func.ty);
            }else{
                EM_error(a->pos,"just not function");
            }


        }
        break;
        case A_recordExp:{
               Ty_ty e=S_look(tenv,a->u.record.typ);
               if(e && e->kind==Ty_record){
                   A_efieldList p=a->u.record.fields;
                   while(p){
                       struct expty field_e    = transExp(venv,tenv,p->head->exp);
                       Ty_ty name_ty=NULL;
                       int match =0 ;
                       for(Ty_fieldList i=e->u.record;i;i=i->tail){
                           if(i->head->name==p->head->name){
                               match = 1;
                               name_ty = i->head->ty;
                               break;
                           }
                       }

                       if(match == 0){
                           EM_error(a->pos,"recordexp has no such field");
                       }

                       if(field_e.ty->kind!=name_ty->kind){
                            EM_error(p->head->exp->pos,"field and value not match");
                       }
                       p=p->tail;
                   }
                   return expTy(NULL,e);
               }else{
                   EM_error(a->pos,"unknown type or not recordtype");
               }
        }
        break;
        case A_assignExp:{
            A_exp e= a->u.assign.exp;
            A_var v= a->u.assign.var;
            struct expty ety=transExp(venv,tenv,e);
            struct expty vty=transVar(venv,tenv,v);
            if(ety.ty->kind!=vty.ty->kind && ety.ty->kind!=Ty_nil){
                EM_error(a->pos,"can't assign");
            }
            return expTy(NULL,Ty_Void());
        }
        break;
        case A_ifExp:{
            A_exp testt=a->u.iff.test;
            A_exp thenn=a->u.iff.then;
            A_exp elsee=a->u.iff.elsee;

            struct expty testty = transExp(venv,tenv,testt);
            struct expty thenty = transExp(venv,tenv,thenn);
            struct expty elsety = transExp(venv,tenv,elsee);

            if(testty.ty->kind!=Ty_int){
                EM_error(testt->pos,"test not int");
            }
            if(thenty.ty->kind!=elsety.ty->kind){
                EM_error(thenn->pos,"not compatible ");
            }
            return  expTy(NULL,elsety.ty);
        }
        break;
        case A_whileExp:{
            A_exp testt=a->u.whilee.test;
            A_exp bodyy=a->u.whilee.body;

            struct expty testty = transExp(venv,tenv,testt);
            struct expty bodyty = transExp(venv,tenv,bodyy);
            if(testty.ty->kind!=Ty_int){
                EM_error(testt->pos,"while test not int");
            }

            if(bodyty.ty->kind!=Ty_void){
                EM_error(bodyy->pos,"while body can not have value");
            }

            return expTy(NULL,Ty_Void());
        }
        break;
        case A_forExp:{
           S_symbol count = a->u.forr.var;
           A_exp lo    = a->u.forr.lo;
           A_exp hi    = a->u.forr.hi;
           A_exp body  = a->u.forr.body;
            S_beginScope(tenv);
            S_beginScope(venv);
            S_enter(venv,count,E_VarEntry(Ty_Int()));
            /*
           E_enventry e= S_look(venv,count);

           if(!e ||  e->kind!=E_varEntry ){
                EM_error(a->pos,"counter undefined varible");
           }else if(e->u.var.ty->kind!=Ty_int){
                EM_error(a->pos,"couter not int");
           }
           */
           struct expty loty=transExp(venv,tenv,lo);
           struct expty hity=transExp(venv,tenv ,hi);
           struct expty bodyty=transExp(venv,tenv,body);

           if(loty.ty->kind!=Ty_int || hity.ty->kind!=Ty_int){
                EM_error(lo->pos,"lo or hi has not int value");

           }

           if(bodyty.ty->kind!=Ty_void){
              EM_error(body->pos,"for body has value");
           }
            S_endScope(tenv);
            S_endScope(venv);

           return expTy(NULL,Ty_Void());
        }
        break;
        case A_letExp:{
              S_beginScope(venv);
              S_beginScope(tenv);

              A_decList decs= a->u.let.decs;
              A_exp body =a->u.let.body;

              while(decs){
                  transDec(venv,tenv,decs->head);
                  decs=decs->tail;
              }
              struct expty bodyty = transExp(venv,tenv,body);

              S_endScope(venv);
              S_endScope(tenv);
              

              return expTy(NULL,bodyty.ty);
        }
        break;

            // asdsrray[34] of 123
        case A_arrayExp:{
              S_symbol asym=a->u.array.typ;
              A_exp    initexp=a->u.array.init;
              A_exp    sizeexp=a->u.array.size;

              Ty_ty syme      =S_look(tenv,asym);
              // 这里的name

              struct expty initty =transExp(venv,tenv,initexp);
              struct expty sizety= transExp(venv,tenv,sizeexp);

              if(!syme ||  syme->u.array!=initty.ty){
                    EM_error(a->pos,"value and init not match for arrayexp");
              }

              if(sizety.ty->kind!=Ty_int){
                EM_error(a->pos,"size not int");

              }
              return expTy(NULL,syme);
        }
        break;
        default:
            EM_error(a->pos,"exp unknown type");
    }
}





