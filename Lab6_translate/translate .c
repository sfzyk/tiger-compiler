//
// Created by sf on 18-9-24.
//
#include "absyn.h"
#include "translate .h"
#include "petiumframe.h"
#include "frame.h"
#include "temp.h"
#include "tree.h"
#include "util.h"
#include "error.h"
typedef struct patchList_ patchList;
/*
 * 需要填充标号的地点 链表
 */
struct patchList_{
    Temp_label* head; // Temp_label 就是S_symbol的别名
    PatchList tail;
};
/*
 *  这里的head 就是我们说的需要填充的 地址标号
 */
static patchList patchList(Temp_label * haed ,patchList tail){
    patchList pl= checked_malloc(sizeof(struct patchList_));
    pl->head=head;
    pl->tail=tail;
    return pl;
}
void doPatch(patchList tList ,Temp_label label){
    for(;tList ; tList=tList.tail){
        *(tList.head) = label;
    }
    return ;
}
patchList joinPatch(patchList first,patchList second){
    if(!first){
        return second;
    }
    for(;first->tail;first=first->tail);// go to end list
    first.tail=second;
    return first;// ?? 这样不就返回了一个链表的中间部分
}


struct Tr_access_ {
	Tr_level level ; F_access access;
};

struct Tr_level_{
    Tr_level parent;
    F_frame frame;
};
static Tr_level outer=NULL;

struct Tr_access_{
    Tr_level level;
    F_access access;
};

struct Tr_expList_ {
    Tr_exp head;
    Tr_expList tail;
};

Tr_expList Tr_ExpList(Tr_exp head,Tr_expList tail){
    Tr_expList Tr_l =checked_malloc(sizeof(*Tr_l));
    Tr_l->head=head;
    Tr_l->tail=tail;
    return Tr_l;
}


Tr_access Tr_allocLocal(Tr_level lev,bool esc){
	F_frame frame = lev->frame;
	F_access access =F_allocLocal(frame,esc);
	Tr_access tr_access=checked_malloc(sizeof(struct Tr_access));
	tr_access->access=access;
	tr_access->level=lev;
	return tr_access;
}

Tr_level Tr_newLevel(Tr_level parent ,temp_Label name ,U_boolList formals){
	F_frame frame =F_newFrame(name,formals);
	Tr_level lev  =checked_malloc(sizeof(struct Tr_level_));
	lev->frame=frame;
	lev->parent=parent;
	return lev;
}

/*
 * singleton
 */
Tr_level Tr_outermost(){
    if(!outer){
        outer=Tr_newLevel(NULL,Temp_name(),NULL);
    }
    return outer;
}

struct Cx{patchList trues;patchList falses;T_stm stm;};
struct Tr_exp_{
    enum {Tr_ex,Tr_nx,Tr_cx}kind;
    union{T_exp ex;T_stm nx;struct Cx cx;}u;
};
/*
 * 一下三个构造函数只在translate 中可见 ，是私用的
 */
static Tr_exp Tr_Ex(T_exp ex){
    Tr_exp Tr_e = checked_malloc(sizeof(struct Tr_exp_));
    Tr_e->kind=Tr_ex;
    Tr_e->u.ex=ex;
    return Tr_e;
}

static Tr_exp Tr_Nx(T_stm nx){
    Tr_exp Tr_e =checked_malloc(sizeof(struct Tr_exp_));
    Tr_e->kind =Tr_nx;
    Tr_e->u.nx=nx;
    return Tr_e;
}

static Tr_exp Tr_Cx(patchList trues,patchList falses ,T_stm stm){
    Tr_exp Tr_e=checked_malloc(sizeof(struct Tr_exp_));
    Tr_e->kind=Tr_cx;
    Tr_e->u.cx.stm=stm;
    Tr_e->u.cx.falses=falses;
    Tr_e->u.cx.trues=trues;
    return Tr_e;
}

static T_exp unEx(Tr_exp e){
    switch (e->kind){
        case Tr_ex:
            return e->u.ex;
        case Tr_cx:
            Temp_temp r=Temp_newtemp();
            Temp_label t=Temp_newlabel(), f= Temp_newlabel();
            doPatch(e->u.cx.trues,t);
            doPatch(e->u.cx.falses,f);
            return T_Eseq(T_Move(T_Temp(r),T_Const(1)),
                          T_Eseq(e->u.cx.stm,
                                 T_Eseq(T_Label(f),
                                    T_Eseq(T_Move(T_Temp(r),T_Const(0)),
                                        T_Eseq(T_Label(t),T_Temp(r))
                                    )
                                 )
                          )
                    );
        case Tr_nx:
            return T_Eseq(e->u.nx,T_Const(0));
    }
    assert(0);
}

static T_exp unCx(Tr_exp e){
    switch(e->kind){
        case Tr_ex:

            T_stm stm= T_Cjump(T_ne,T_Const(0),e->u.ex,NULL,NULL);
            patchList trues =patchList(&stm->u.CJUMP.true,NULL);
            patchList falses=patchList(&stm->u.CJUMP.false,NULL);
            Tr_exp tc=Tr_Cx(trues,falses,stm);
            return tc->u.cx;
        case Tr_cx:
            return e->u.cx;
        case Tr_nx:
            EM_error(0,"nx cannot be cx");
            struct Cx cx;
            return cx;
    }
}

static T_exp unNx(Tr_exp e) {
    switch (e->kind) {
        case Tr_ex:
            return T_Exp(e->u.ex);
        case Tr_cx:
            Temp_temp r = Temp_newtemp();
            Temp_label t = Temp_newlabel();
            Temp_label f = Temp_newlabel();
            doPatch(e->u.cx.falses, f);
            doPatch(e->u.cx.trues, t);
            // 结果是true 就返回1 否则就是 0
            return T_Seq(T_Move(T_Temp(r), Tr_const(1)),
                         T_Seq(e->u.cx,
                               T_Seq(Temp_labelstring(f),
                                     T_Seq(T_Move(T_Temp(r), T_Const(0)),
                                           T_seq(T_Label(f), T_Temp(r))
                                     )
                               )
                         )
            );
        case Tr_nx:
            return e->u.nx;
    }
}


/*
 *  下面都是将 Tree IR 转换成 Tr_exp
 *
 */
Tr_exp Tr_nullEx(){
    return Tr_Ex(T_Const(0));
}
Tr_exp Tr_nullCx(){
    return Tr_Cx(NULL,NULL,T_Const(0));
}
Tr_exp Tr_nullNx(){
    return Tr_Nx(T_exp(T_Const(0)));
}

Tr_exp Tr_nilExp(){
    return Tr_exp(T_const(0));
}

Tr_exp Tr_intExp(A_exp e){
    assert(e->kind==A_intExp);
    return Tr_exp(T_const(e->u.intt));
}

Tr_exp Tr_stringExp(string str){
    Temp_label strpos=Temp_newlabel();
    F_frag frag =F_string(strpos,str);
    fragList = F_FragList(frag,fragList);
    return Tr_Ex(T_name(strpos));
}

/*
 *  a 和包含其的层次在a  内
 *  lev 是指访问a 时的层次
 */
Tr_exp Tr_simpleVar(Tr_access a,Tr_level lev){
    T_exp staticLinkExp = Temp(F_FP());
    while(a->level!=lev){
        staticLinkExp = F_FPExp(staticLinkExp); // 向上查找栈帧 静态链就是栈帧指针
        lev=lev->parent;
    }
    return Tr_Ex(F_exp(a->access,staticLinkExp));
}


Tr_exp Tr_fieldVar(Tr_exp var, int fieldIndex , Tr_level lev){
    return Tr_Ex(T_MEM(
            T_Binop(T_plus,unEx(var),
                    T_Binop(T_mul,T_Const(fieldIndex),T_Const(F_wordSize)))));
}
Tr_exp Tr_subscriptVar(Tr_exp var,Tr_exp sub , Tr_level){
    return Tr_Ex(T_Mem(T_Binop(T_plus,unEx(var),T_Binop(T_mul,
                                                        unEx(sub),T_Const(F_wordSize)
    ))));
}

// 数字的运算符
Tr_exp Tr_arOpExp(A_oper o,Tr_exp left, Tr_exp right){
    T_binOp op = T_plus;
    switch(o){
        case A_plusOp : op=T_plus ;break;
        case A_minusOp: op=T_minus ;break;
        case A_timesOp: op=T_mul; break;
        case A_divideOp:e op=T_div;break;
        default:
            assert(0);
    }
    return Tr_Ex(T_Binop(op,unEx(left),unEx(right)));
}
Tr_exp Tr_condOpExp(A_oper o,Tr_exp left,Tr_exp right ){
    T_binOp op = T_plus;
    T_stm s;
    switch (o){
        case A_eqOp : op =T_eq;break;
        case A_neqOp: op =T_ne;break;
        case A_ltOp: op=T_lt;  break;
        case A_leOp: op=T_le; break;
        case A_gtOp: op=T_gt ; break;
        case A_geOp: op=T_ge; break;
    }

    T_stm s=T_Cjump(o,unEx(left),unEx(right),NULL,NULL);

    patchList tures =patchList(&s->u.CJUMP.false,NULL);
    patchList falses =patchList(&s->u.CJUMP.true,NULL);

    return Tr_Cx(trues.falses,s);
}


Tr_exp Tr_strOpExp(A_oper o,Tr_exp left,Tr_exp right){
    T_binOp t=T_plus;
    T_stm s;
    switch (o){
        case A_eqOp:  op=T_eq;break;
        case A_neqOp: op=T_ne;break;
        case A_leOp: op=T_le;break;
        case A_ltOp: op=T_lt;break;
        case A_geOp: op=T_ge;break;
        case A_gtOp: op=T_gt;break;
    }

    if(op==T_eq || op==T_ne){
        T_exp t = F_externalCall("stringEqual",T_ExpList(unEx(left), T_ExpList(unEx(right),NULL)));
        s =T_Cjump(op,t,T_Const(1),NULL,NULL);
    }else{
        //??? to do
        Temp_temp t=Temp_newtemp();
        Temp_label fin=Temp_newlabel();
        //???
        T_exp e=F_externalCall("stringCompare",T_ExpList(unEx(left),T_ExpList(unEx(right),NULL)));
        s= T_Cjump(op,e,NULL,NULL);
    }

    patchList trues=patchList(&s->u.CJUMP.true,NULL);
    patchList falses=patchList(&s->u.CJUMP.false,NULL);

    return Tr_Cx(trues,falses,s);
}

Tr_exp Tr_assignExp(Tr_exp var,Tr_exp exp){
    return Tr_nx(T_Move(unEx(var),unEx(exp)));
}

/*
 * p  Tmep_temp 是寄存器资源
 * p  Temp_label 是标号
 * if 就是预留了一个寄存器资源 R
 * 通过控制转移将表达式转移到寄存器资源上
 */
Tr_exp Tr_ifExp(Tr_exp test,Tr_exp then,Tr_exp elsee,Ty_ty iftty) {
    Temp_label t = Temp_newlabel();
    Temp_label f = Temp_newlabel();
    Temp_label m = Temp_newlabel();
    if (test->kind == Tr_ex) {
        struct Cx testcx = unCx(test);
        test = Tr_Cx(testcx.trues, testcx.falses, testcx.stm);
    } else if (test->kind == Tr_nx) {
        EM_error(0, "test exp cannot be nx");
    }

    doPatch(test->u.cx.trues, t);
    doPatch(test->u.cx.falses, f);

    if (iftty.kind != Ty_void) { //有返回值的情况
        Temp_temp r = Temp_newlabel();
        T_stm stm = T_Seq(unCx(test).stm,
                          T_Seq(T_Label(t),
                                T_Seq(T_Move(T_Temp(r), unEx(then)),
                                      T_Seq(T_Jump(T_Name(m), Temp_LabelList(m, NULL)),
                                            T_Seq(T_Label(f),
                                                  T_Seq(T_Move(T_Temp(r), unEx(elsee)), T_Temp(m)
                                                  )
                                            )
                                      )
                                )
                          )
        );
        if (then->kind == Tr_nx) {
            EM_error(0, "then exp cannot be nx");
        }

        if (elsee->kind == Tr_nx) {
            EM_error(0, "else exp cannot be nx");
        }

        return Tr_Ex(T_Eseq(stm, T_Temp(r)));
    } else {
        T_stm stm = T_Seq(unCx(test).stm,
                          T_Seq(T_Label(t),
                                T_Seq(T_Move(T_Temp(r), unEx(then)),
                                      T_Seq(T_Jump(T_Name(m), Temp_LabelList(m, NULL)),
                                            T_Seq(T_Label(f),
                                                  T_Seq(T_Move(T_Temp(r), unEx(elsee)), T_Temp(m)
                                                  )
                                            )
                                      )
                                )
                          )
        );
        return Tr_Nx(stm);
    }
    /*
     * breakbl
     */
}
Tr_exp Tr_whileExp(Tr_exp testexp,Tr_exp body,Temp_label breakbl){
    Temp_label test = Temp_newlabel();
    Temp_label done = breakbl;
    Temp_label loopstart =Temp_newlabel();
    T_stm s=T_Seq(T_label(test),
                  T_Seq(T_Cjump(T_ne,unEx(testexp),T_Const(0),loopstart,done),
                        T_Seq(T_Label(loopstart),
                              T_Seq(unEx(body),
                                    T_Seq(T_Jump(T_Name(test),Temp_LabelList(test,NULL)),T_Label(done))))));
    return Tr_nx(s);
}

Tr_exp Tr_forExp(Tr_exp explo,Tr_exp exphi,Tr_exp body,Temp_label breakbl){
    Temp_label  test=Temp_newlabel();
    Temp_label  done=breakbl;
    Temp_label  loopstart=Temp_newlabel();

    /*
     * 为了i 和 limit 申请的寄存器空间
     */
    Temp_temp i=Temp_newtemp();
    Temp_temp limit =Temp_newtemp();

    T_stm s= T_Seq(T_Move(T_Temp(i),unEx(explo)),
                T_Seq(T_Move(T_Temp(limit),unEx(exphi)),
                      T_Seq(T_Label(test),
                         T_Seq(T_Cjump(T_le,T_Temp(i),T_Temp(limit),loopstart,done),
                               T_Seq(T_Label(loopstart),
                                   T_Seq(unEx(body),
                                       T_Seq(T_Move(T_Temp(i),T_Binop(T_plus,T_Temp(i),T_Const(1))),
                                           T_Seq(T_Jump(test,Temp_LabelList(test,NLL)),T_Label(done)))))))));
    return Tr_nx(s);
}

Tr_exp Tr_breakExp(Temp_label breakbl){
    return Tr_Nx(T_Jump(breakbl,Temp_LabelList(breakbl,NULL)));
}


Tr_exp Tr_arrayExp(Tr_expList el ,Tr_exp size){
    return Tr_Ex(F_externalCall("initarry"),T_ExpList(unEx(size),T_ExpList(size,NULL)));
}

Tr_exp Tr_recordExp(Tr_expList el,int fieldCount){
    // for single field record can be buggy
    Temp_temp r=Temp_newtemp();
    T_stm alloc=T_stm(T_Move(T_Temp(r),F_externalCall("malloc",T_ExpList(T_const(F_wordSize*size),NULL))));

    T_stm init=NULL,current =NULL;
    int fieldIndex=0;
    for(;el;el=el->tail,++fieldIndex){
        if(init==NULL){
            init=current = T_Seq(T_Move(T_MEM(T_Binop(T_plus,T_Temp(r),T_Const((fieldCount -1- fieldIndex)*F_wordSize
                    ))),unEx(el->head)),T_Exp(T_Const(0)));


        }else{
            current->u.SEQ.right=T_Seq(T_Move(T_MEM(T_Binop(T_plus,T_Temp(r),T_Const((fieldCount -1- fieldIndex)*F_wordSize
            ))),unEx(el->head)),T_Exp(T_Const(0)));

            current =current->u.SEQ.right;
        }

    }
    return Tr_Ex(T_Eseq(T_Seq(alloc,init)),T_Temp(r));
}

Tr_exp Tr_callExp(Temp_label name,Tr_expList rawel){
    T_expList el=NULL;
    for(; rawel; rawel=rawel->tail){
        el=T_ExpList(unEx(rawel->head),el);
    }

    // to do fetch static link
    T_exp staticlink=T_const(0);
    el=T_ExpList(staticlink,el);

    return Tr_Ex(T_Call(T_Name(name),el));
}










