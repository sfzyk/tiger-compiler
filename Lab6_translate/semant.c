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
#include "translate.h"
#include "find_escape.h"

struct expty{Tr_exp exp;Ty_ty ty;};

struct expty expTy(Tr_exp exp,Ty_ty ty){
    struct expty e;
    e.exp=exp;
    e.ty=ty;
    return e;
}

void SEM_transProg(A_exp exp){

    Tr_level lev = Tr_outermost();
    S_table tenv=E_base_tenv();
    S_table venv=E_base_venv();

    Esc_findEscape(exp);//buggy

    struct expty prog = transExp(lev,venv,tenv,exp);

}

Ty_ty actual_ty(Ty_ty p){
    while(p && p->kind==Ty_name) {
        p=p->u.name.ty;
    }
    return p;
}

/*
 *  自然的倒置实现
 */
U_boolList makeFormalBoolList(A_fieldList f){
    U_boolList u_b=NULL;
    for(;f;f=f->tail) {
        u_b =U_BoolList(f->head->escape,u_b);
    }

    return u_b;
}

/*    将语义分析得到的  A_fieldList
 *    转换成类型分析所需要的 Ty_tyList
 *    实现上有一个非常ugly�因为传入的param 是向后链接的单链�
 *    输出的也是向后链接的单链��自然的实现会把链表倒置
 *    只能使用这种实现方式�
 */
Ty_tyList makeFormalTyList(S_table tenv, A_fieldList param){
    Ty_tyList p=NULL;           //前一个参�
    Ty_tyList n=NULL;           //当前参数
    Ty_tyList first=NULL;       //第一个参�
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
 *  接受的类型定义是 A_Ty 类型�
 *  A_Ty 类型也只会在语义解析中的 dec 中出�
 *  进入环境中需要的是新定义�"类型"类型
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


void transDec(S_table venv,S_table tenv ,A_dec dec,Tr_level lev){

    switch (dec->kind) {
        case A_typeDec: {
            A_nametyList p = dec->u.type;

            /* 第一�扫描 加入类型的头 */
            while (p) {
                // 只进行后向查冲即�
                for(A_nametyList  i= p->tail;i;i=i->tail) {
                    if(!i)break ;
                    if(i->head->name == p->head->name){
                        EM_error(dec->pos,"type dec not allow same name");
                    }
                }

                S_enter(tenv, p->head->name, Ty_Name(p->head->name, NULL));
                p = p->tail;
            }
            /* 第二�扫描 加入全部的内�*/
            p = dec->u.type;
            while (p) {
                Ty_ty wait = S_look(tenv, p->head->name);
                /* 去除第一遍的值重新改�*/
				Ty_ty temp = transTy(tenv, p->head->ty);
				
                wait->u.name.ty = temp;
                /* 这里使用 actual_ty 是为了一般的 Namety 深入 不是为了嵌套
                 * 更一般的 使用两次扫描�模式也是 为了�actual_ty 的时候正确的返回
                 * */
                Ty_ty wait_true = actual_ty(wait);
                if(wait_true) {   //  所有的�Namety 类型  actual_ty 的结果就是他自身 没有意义
                                  //  NameTy  类型执行actul_ty 不良的定义会导致这里�wait_true 为空
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

                Tr_access  Tr_a = Tr_allocLocal(lev,dec->u.var.escape);
                S_enter(venv,dec->u.var.var,E_VarEntry(mark_varty));

            }else{
                if(varty->kind==Ty_nil){
                    EM_error(dec->pos,"short term can't be Nil");
                }

                Tr_access tr_a= Tr_allocLocal(lev,dec->u.var.escape);
                S_enter(venv,dec->u.var.var,E_VarEntry(tr_a,varty));
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
                Temp_label t_l = Temp_newlabel();
                U_BoolList esc_l =makeFormalBoolList(f->head->params);
                Ty_tyList formals=makeFormalTyList(tenv,f->head->params);
                Tr_level  newlevel = Tr_newLevel(lev,t_l,esc_l);

                Ty_ty        retty;
		        if(f->head->result)
         		    retty= S_look(tenv,f->head->result);
		        else
			        retty= Ty_Void();

                if(!retty) {
                    EM_error(f->head->pos, "undefined return type");
                }

                E_enventry funenv=E_FunEntry(formals,retty,t_l,newlevel);
                S_enter(venv,f->head->name,funenv);
                f=f->tail;
            }
		
	    //第二编扫描解决expbody的问�
            f= dec->u.function;
	        while(f){
                E_enventry funwait = S_look(venv, f->head->name );
                Tr_level  newlevel =funwait->u.func.level;
                Temp_label func_label=funwait->u.func.label;

                A_fieldList  params = f->head->params;
                S_beginScope(venv);
                S_beginScope(tenv);

                while(params){
                    Ty_ty argty = S_look(tenv,params->head->typ);
                    Tr_access varacc=Tr_allocLocal(lev,params->head->escape);

                    E_enventry ent=E_VarEntry(varacc,argty);

                    S_enter(venv,params->head->name,ent);

                    params=params->tail;
                }

                struct expty truety =transExp(lev,venv,tenv,f->head->body);

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

struct expty transVar(Tr_level  lev, S_table venv,S_table tenv ,A_var a,Temp_label breakbl){
    switch(a->kind) {
        case A_simpleVar: {
            E_enventry x = S_look(venv, a->u.simple);
            Tr_access Tr_a =x->u.var.access;

            if (x && x->kind == E_varEntry) {
                return expTy(Tr_simpleVar(Tr_a,lev), actual_ty(x->u.var.ty));

            } else {
                EM_error(a->pos, "undefined variable", S_name(a->u.simple));
                return expTy(Tr_nullEx(), Ty_Int());
            }
        }
        break;
        case A_fieldVar:{
            struct expty e=transVar(lev,venv,tenv,a->u.field.var,breakbl);

            if(e.ty->kind==Ty_record) {
                Ty_fieldList p = e.ty->u.record;

                int fieldindex =0;
                while (p && p->head->name != a->u.field.sym) {
                    p = p->tail;
                    fieldindex++;
                }

                if (p) {
                    return expTy(Tr_fieldVar(e.exp,fieldindex,lev), p->head->ty);
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
            struct expty var=transVar(lev,venv,tenv,a->u.subscript.var,breakbl);
            struct expty sub=transExp(lev,venv,tenv,a->u.subscript.exp,breakbl);

            if(var.ty->kind==Ty_array ){
                if(sub.ty->kind==Ty_int){
                    return expTy(Tr_subscriptVar(var.exp,sub.exp,lev),var.ty->u.array);
                }else{
                    EM_error(a->pos,"subscript not int");
                }
            }else{
                EM_error(a->pos,"not arrat type");
            }
            return expTy(Tr_nullNx(),Ty_Void());
        }
        break;
        default:
            EM_error(a->pos,"unknown type");
    }
}

struct expty transExp(Tr_level  lev,S_table venv, S_table tenv,A_exp a ,Temp_label breakbl){
    switch(a->kind){
        /*
         *  约定 + - * / 只有int类型是接受的
         *  = != 只要相同类型就可以
         *  < > <= >= 则支持str 和 int 类型
         */
        case A_opExp:{
            A_oper oper= a->u.op.oper ;
            struct expty left  = transExp(lev,venv,tenv,a->u.op.left,breakbl);
            struct expty right = transExp(lev,venv,tenv,a->u.op.right,breakbl);

            if (oper==A_plusOp | oper==A_minusOp | oper==A_timesOp | oper==A_divideOp){
                /* A_plusOp, A_minusOp, A_timesOp, A_divideOp,
                        A_eqOp, A_neqOp, A_ltOp, A_leOp, A_gtOp, A_geOp */
                    if (left.ty->kind != Ty_int) {
                        EM_error(a->u.op.left->pos, "i need integer");
                    }
                    if(right.ty->kind != Ty_int){
                        EM_error(a->u.op.left->pos,"i need interger");
                    }
                    return expTy(Tr_arOpExp(oper,left,right),Ty_Int());
            }
            else if(oper==A_eqOp|oper==A_neqOp){
                    if(left.ty->kind!=right.ty->kind && left.ty->kind!=Ty_nil && right.ty->kind!=Ty_nil){
                        EM_error(a->u.op.right->pos,"value type not match");
                        return expTy(Tr_nullCx(),Ty_int());
                    }

                    if(left.ty->kind==Ty_string){
                        return expTy(Tr_strOpExp(oper,left,right),Ty_Int());
                    }
                    else{
                        return expTy(Tr_condOpExp(oper,left,right),Ty_Int());
                    }

            }else if(oper==A_leOp| oper==A_geOp | oper==A_gtOp | oper==A_ltOp){
                if(left.ty->kind!=right.ty->kind && left.ty->kind!=Ty_nil && right.ty->kind!=Ty_nil){
                    EM_error(a->u.op.right->pos,"value type not match");
                    return expTy(Tr_nullCx(),Ty_int());
                }
                if(left.ty->kind!=Ty_int && left.ty->kind !=Ty_string){
                    EM_error(a->u.op.left->pos,"only int and string are allow for comapare");
                    return expTy(Tr_nullCx(),Ty_int());
                }
                if(left.ty->kind==Ty_int){
                    return expTy(Tr_condOpExp(op,left,right),Ty_Int());

                }else if(left.ty->kind==Ty_string){
                    return expTy(Tr_strOpExp(op,left,right),Ty_Int());
                }
            }
            return expTy(Tr_nullEx(),Ty_Int());
        }
        break;
        case A_varExp:{
            struct expty e=transVar(lev,venv,tenv,a->u.var,breaklbl); //查找这个变量的定义时访问
            return expTy(Tr_simpleVar(e.exp,lev),e.ty); //生成这个变量的访问
        }
        break;

        case A_seqExp:{
           A_expList p=a->u.seq;

           while(p && p->tail){
               transExp(lev,venv,tenv,p->head,breaklbl);
               p=p->tail;
           }

           struct expty e;

           if(p) e = transExp(lev,venv,tenv,p->head,breaklbl);
                else  e = expTy(NULL,Ty_Void());

           return e;
        }
        break;
        case A_breakExp:{
            return expTy(Tr_breakExp(breakbl),Ty_Void());
        }
        break;
        case A_nilExp:{
            return expTy(Tr_nilExp(),Ty_Nil());
        }
        break;
		case A_intExp:{ // int 和 string 的调用约定稍有不同
            return expTy(Tr_intExp(a),Ty_Int());
        }
        break;
        case A_stringExp:{
            return expTy(Tr_stringExp(a->u.stringg),Ty_String());
        }
        break;
        case A_callExp:{
            E_enventry e=S_look(venv,a->u.call.func);
            Temp_label func_label=e->u.func.label;

            if(e && e->kind==E_funEntry){
                Ty_tyList p_actual=e->u.func.formals;
                A_expList p=a->u.call.args;
                Tr_expList explist=NULL;
                while(p && p_actual){
                    struct expty argty = transExp(lev,venv,tenv,p->head,breakbl);
                    explist=Tr_ExpList(argty.exp,explist);

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

                return expTy(Tr_callExp(func_label,explist),e->u.func.ty);
            }else{
                EM_error(a->pos,"just not function");
            }
        }
        break;
        // to do translate
        case A_recordExp:{

               Ty_ty e=S_look(tenv,a->u.record.typ);
               if(e && e->kind==Ty_record){
                   A_efieldList p=a->u.record.fields;
                   while(p){
                       struct expty field_e    = transExp(lev,venv,tenv,p->head->exp,breakbl);
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
            struct expty ety=transExp(lev,venv,tenv,e,breakbl);
            struct expty vty=transVar(lev,venv,tenv,v,breakbl);

            if(ety.ty->kind!=vty.ty->kind && ety.ty->kind!=Ty_nil){
                EM_error(a->pos,"can't assign");
            }

            return expTy(Tr_assignExp(vty.exp,ety.exp),Ty_Void());
        }
        break;
        case A_ifExp:{
            A_exp testt=a->u.iff.test;
            A_exp thenn=a->u.iff.then;
            A_exp elsee=a->u.iff.elsee;

            struct expty testty = transExp(lev,venv,tenv,testt,breakbl);
            struct expty thenty = transExp(lev,venv,tenv,thenn,breakbl);
            struct expty elsety = transExp(lev,venv,tenv,elsee,breakbl);// else could be null
            if(testty.ty->kind!=Ty_int){
                EM_error(testt->pos,"test not int");
            }

            if(thenty.ty->kind!=elsety.ty->kind){
                EM_error(thenn->pos,"not compatible ");
            }

            return  expTy(Tr_ifExp(testty.exp,thenty.exp,testty.exp,thenty.ty),thenty.ty);
        }
        break;
        case A_whileExp:{

            A_exp testt=a->u.whilee.test;
            A_exp bodyy=a->u.whilee.body;

            struct expty testty = transExp(lev,venv,tenv,testt,breakbl);
            Temp_label new_break=Temp_newlabel();
            struct expty bodyty = transExp(lev,venv,tenv,bodyy,new_break);

            if(testty.ty->kind!=Ty_int){
                EM_error(testt->pos,"while test not int");
            }

            if(bodyty.ty->kind!=Ty_void){
                EM_error(bodyy->pos,"while body can not have value");
            }

            return expTy(Tr_whileExp(testty.exp,bodyty.exp,new_break),Ty_Void());
        }
        break;
            // 只有函数的定义是申请了新的栈帧，即使for exp 和 while 中又一些局部变量的信息，但是这并不是一个 栈帧特征
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
            Temp_label  new_break =Temp_newlabel();

           struct expty loty=transExp(lev,venv,tenv,lo,breakbl);
           struct expty hity=transExp(lev,venv,tenv ,hi,breakbl);
           struct expty bodyty=transExp(lev,venv,tenv,body,new_break);

           if(loty.ty->kind!=Ty_int || hity.ty->kind!=Ty_int){
                EM_error(lo->pos,"lo or hi has not int value");

           }

           if(bodyty.ty->kind!=Ty_void){
              EM_error(body->pos,"for body has value");
           }
            S_endScope(tenv);
            S_endScope(venv);

           return expTy(Tr_forExp(loty.exp,hity.exp,bodyty.exp,new_break),Ty_Void());
        }
        break;
        case A_letExp:{
              S_beginScope(venv);
              S_beginScope(tenv);

              A_decList decs= a->u.let.decs;
              A_exp body =a->u.let.body;

              while(decs){
                  transDec(lev,venv,tenv,decs->head,breakbl);
                  decs=decs->tail;
              }

              struct expty bodyty = transExp(lev,venv,tenv,body,breakbl);

              S_endScope(venv);
              S_endScope(tenv);
              

              return bodyty;
        }
        break;

            // asdsrray[34] of 123
            // 没有判断是否是一个数组类型
        case A_arrayExp:{
              S_symbol asym=a->u.array.typ;
              A_exp    initexp=a->u.array.init;
              A_exp    sizeexp=a->u.array.size;

              Ty_ty syme      =S_look(tenv,asym);
              // 这里的name

              struct expty initty =transExp(lev,venv,tenv,initexp,breakbl);
              struct expty sizety= transExp(lev,venv,tenv,sizeexp,breakbl);


              if(!syme ||  syme->u.array!=initty.ty){
                    EM_error(a->pos,"value and init not match for arrayexp");
              }

              if(sizety.ty->kind!=Ty_int){
                EM_error(a->pos,"size not int");

              }
              return expTy(Tr_arrayExp(initty.exp,sizety.exp),syme);
        }
        break;
        default:
            EM_error(a->pos,"exp unknown type");
    }
}





