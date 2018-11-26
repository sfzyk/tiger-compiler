//
// Created by sf on 18-9-24.
//
#include "util.h"
#include "tree.h"
#include "frame.h"
#include "temp.h"



// 可以存放在栈中或者寄存器中的 形式参数或者是局部变量
static int MAX_REG_SIZE= 6;
const int F_WordSize = 4;

struct F_frame_{
    F_accessList f_accessList; // 参数列表
    F_accessList f_lcoal;       //局部变量访问列表
    int local_size;            //局部变量个数
    int arg_size;              //参数个数
    Temp_temp name;
};


struct F_access_ {
    enum{inFrame,inReg}kind;
    union{
        int offset; /* inframe */
        Temp_temp reg; /*inreg*/
    }u;
};

F_accessList F_accessList(F_access f_a,F_accessList f_aList){
    F_accessList ret_f_aList = checked_malloc(sizeof(struct F_accssList_));
    ret_f_aList->head=f_a;
    ret_f_aList->tail=f_aList;
    return ret_f_aList;
}

static F_access InFrame(int offset){
	F_access f_a = checked_malloc(sizeof(struct F_access));
	f_a -> knid = inFrame;
	f_a ->u.offser = offser;
	return f_a;
}

static F_access InReg(Temp_temp reg){
	F_access f_a = checked_malloc(sizeof(struct F_access));
	f_a -> kind=inReg;
	f_a -> u.reg = reg;
	return f_a;
}
/*
 * 使用给定的标号和 参数的逃逸情况申请栈帧
 *
 */
F_frame F_newFrame(Temp_label name,U_boolList formals){
	F_frame frame = checked_malloc(sizeof(struct F_frame));
	frame->name = name ;
	frame->local_size=0;
	frame->f_accessList = MakeFormalList(frame,formals,&(frame->arg_size)); //设置好参数和静态连
	return frame;
}

/*
 *  在这个栈内分配一个局部变量
 *  同时返回这个访问
 */
F_access F_allocLocal(F_Frame frame,bool escape){
    F_access  newacc;
    int offset = (- frame->local_size)*F_wordSize;

    if(escape){
        newacc=InFrame(offset);
        frame->local_size++;
    }else{
        newacc=InReg(Temp_newtemp());
    }
    frame->f_local = F_accessList(newacc,frame->f_local);
    return newacc;
}

/*  x86 栈帧布局
 *    参数区  （前6个会被分配到寄存器中，并且第一个参数是静态链)
 *    返回地址
 *    saved RBP    <------- RBP
 *    局部变量
 */
F_accessList MakeFormalList(F_frame frame,U_boolList ub_l,int * argnum){
	int count =0 ;              // 使用的寄存器数量
	int offset = 2*F_wordSize ; //相对于栈帧的偏移(个是saved RBP 返回地址)

	F_access access;
	F_accessList ret=NULL;
	for (bool escape = ub_l.head ; ub_l !=NULL;ub_l=ub_l->tail){

		if(!escape && count < MAX_REG_SIZE){
			access = InReg(Temp_newtemp());
			count = count + 1;
		}
		else{
			access = InFrame(offset);
			offset=offset + F_wordSize;
		}
		ret = F_accessList(access,ret);
		argnum++ ;
	}
	return ret;
}

/*
 * singleton
 */
static Temp_temp fp=NULL;
Temp_temp F_FP(void){
	if(fp==NULL){
		return fp;
	}
	else{
		fp=Temp_newtemp();
		return fp;
	}
}

static int F_accessOffset(F_access f){
	return f->u.offset;
}

T_exp F_exp(F_access acc,T_exp frameptr){
	if(acc->kind==inFrame){
		return T_Mem(T_Binop(T_plus,frameptr,T_Const(F_accessOffset(acc))));

	}else if(acc->kind==inReg){

		return T_Temp(acc->u.reg);
	}
}

T_exp F_FPExp(T_exp fp){
	return T_MEM(fp);
}
/*
 * 对外部引用的调用
 */
T_exp F_externalCall(string s,T_expList args){
	return T_Call(Temp_namedlabel(s),args);
}
// 标签和字符串
F_frag F_stringFrag(Temp_label label,string str){
    F_frag f = checked_malloc(sizeof(*f));
    f->kind = F_stringFrag;
    f->u.stringg.label=label;
    f->u.stringg=str;
    return f;
}
//栈帧和中间表示树
F_frag F_procFrag(T_stm body,F_frame frame){
	F_frag  f=checked_malloc(sizeof(*f));
	f->kind=F_procFrag;
	f->u.proc.body=body;
	f->u.proc.frame=frame;
	return f;
}

F_fragList F_FragList(F_frag head,F_fragList tail) {
	F_fragList fl = checked_malloc(sizeof(*fl));
	fl->tail = tail;
	fl->head = head;
	return fl;
}
static Temp_temp rv=NULL;

Temp_temp F_RV(void){
	if(rv==NULL){
		rv=Temp_newtemp();
	}
	return rv;
}

T_stm F_procEntryExit1(F_frame f ,T_stm stm){// 一种推荐的虚拟实现
	return  stm;
}

