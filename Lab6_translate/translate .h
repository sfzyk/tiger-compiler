/
// Created by sf on 18-9-24.
//

#ifndef TRANSLATE_H
#define TRANSLATE_H
typedef struct Tr_access_ *Tr_access;
typedef struct Tr_accessList_ * Tr_accessList;


Tr_accessList Tr_AccessList(Tr_accessList head,Tr_accessList tail);
Tr_level Tr_outermost(void);


#endif //LAB5_SEMANTIC_TRANSLATE_H
