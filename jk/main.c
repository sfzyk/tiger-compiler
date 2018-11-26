#include <stdio.h>
#include <malloc.h>

typedef enum TYPE{add,mul,lp,rp,id,eof}type;
type class;
int token;
void nextsym(){

}
void error(){
    printf("error\n");
}

int * new(){
    return (int *)malloc(sizeof(int));
}



void out(char ch,int* a,int* b ,int* c){
    printf("%c %llx %llx %llx\n",ch,a,b,c);
}


int main() {
    nextsym();
    int *x;
    proc_E(x);
    if(class !=eof){
        error();
    }
    return 0;
}

void proc_E(int *x){
    int *y,*z;
    x = new();
    proc_T(y);
    proc_EE(z);
    *x= *y+ *z;
    out("+",y,z,x);
    nextsym();
}

void proc_EE(int *x){
    int *y,*z;
    x = new();
    if(class==add){
        proc_T(y);
        proc_EE(z);
        *x=*y+*z;
        out('+',y,z,x);
        nextsym();
        return;
    }else{
        *x=0;
        return;
    }
}

void proc_T(int *x){
    x =new();
    int *y,*z;
    proc_F(y);
    proc_TT(z);
    *x=*y*(*z);
    out('*',y,z,x);
    nextsym();
}
void proc_F(int* x){
    x = new();
    if(class==lp){
        proc_E(x);

        if(class==rp){
            nextsym();
            return ;
        }else{
            error();
        }

    }
    else if(class==id){
        int *a=new();
        *a= token;
        nextsym();
        return ;
    }
}
void proc_TT(int *x){
    int *y,*z;
    x = new();
    if(class==mul){
        proc_F(y);
        proc_TT(z);
        *x=*y+*z;
        out('*',y,z,x);
        nextsym();
        return;
    }else{
        *x=0;
        return;
    }
}
