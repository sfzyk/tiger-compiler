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


#endif //LAB5_SEMANTIC_FRAME_H
