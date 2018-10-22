//
// Created by sf on 18-9-24.
//

#include "frame.h"


// 可以存放在栈中或者寄存器中的 形式参数或者是局部变量
struct F_access_ {
    enum{inFrame,inReg}kind;
    union{
        int offset; /* inframe */
        Temp_temp reg; /*inreg*/
    }u;
};