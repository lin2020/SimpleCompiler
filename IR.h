#include "syntax.h"

#define ARRAY_ARGS 10

enum op_kind { VARIABLE, CONSTANT, ADDRESS, 
               OP_LABEL, TEMP, OP_FUNC, OP_PARAM };

enum code_kind { 
        CODE_LABEL, CODE_FUNC, CODE_ASSIGN, CODE_ADD, CODE_SUB, CODE_MUL, CODE_DIV, 
        CODE_ADDR, CODE_FROM_ADDR, CODE_TO_ADDR, 
        CODE_GOTO, CODE_RETURN, CODE_DEC, CODE_ARG, CODE_CALL, CODE_PARAM, READ, WRITE, 
        IF_GT, IF_GE, IF_LT, IF_LE, IF_EQ, IF_NE
    };

int count_var = 0;
int count_temp = 0;
int count_label = 0;

typedef struct Operand_* Operand;
typedef struct InterCode_* InterCode;
typedef struct InterCodeline_* InterCodeline;
struct Operand_ {
    enum op_kind kind;

    int var_no;     //临时变量temp， 记录是第几个 //标号label，记录是第几个
    int value;      //常量constant，记录值
    float fvalue;   //常量constant，记录值
    FieldList var;  //符号，记录符号表中的node

    Operand prev;
    Operand next;
};

struct InterCode_ {
    enum code_kind kind;
    Operand result, op1, op2;
};

struct InterCodeline_ {
    InterCode code;
    struct InterCodeline_ *prev;
    InterCodeline next;
};

/***************************************************************************/
FieldList findVarNoArea(struct FieldList_ *head, char *name, char *name_func, Type type);
void displayInterCode(InterCodeline codehead, FieldList symhead);
void displayOperand(Operand op);
InterCodeline BuildInterCodeline(enum code_kind operator_kind, 
                                 Operand one, 
                                 Operand two, 
                                 Operand three);
struct InterCodeline_ *AddToInterCodelinelist(InterCodeline head, 
                                              InterCodeline node);
struct Operand_ *findTemp(InterCodeline head, Operand node);
struct Operand_ *findVariable(InterCodeline head, Operand node);
enum code_kind RelopToKind(struct TREE_EXP *treehead);
InterCodeline translate_Cond(struct TREE_EXP *treehead,
                             InterCodeline codehead,
                             FieldList symhead,
                             Operand label1,
                             Operand label2, 
                             char *name_func
                             );
InterCodeline translate_BoolExp(struct TREE_EXP *treehead, 
                                InterCodeline codehead, 
                                FieldList symhead, 
                                Operand op, 
                                char *name_func
                                );
Operand *AddToArgList(Operand *argList, Operand temp);
InterCodeline translate_Args(struct TREE_EXP *treehead, 
                             InterCodeline codehead, 
                             FieldList symhead, 
                             Operand argList[ARRAY_ARGS],
                             char *name_func
                             );
InterCodeline translate_Exp(struct TREE_EXP* treehead,
                            InterCodeline codehead,
                            FieldList symhead,
                            Operand op,
                            char *name_func
                            );
InterCodeline translate_StmtList(struct TREE_EXP* treehead,
                                 InterCodeline codehead,
                                 FieldList symhead,
                                 char *name_func
                                 );
InterCodeline translate_CompSt(struct TREE_EXP* treehead,
                               InterCodeline codehead,
                               FieldList symhead,
                               char *name_func
                               );
InterCodeline translate_VarList(struct TREE_EXP* treehead,
                                InterCodeline codehead,
                                FieldList symhead,
                                char *name_func
                                );
InterCodeline translate_FunDec(struct TREE_EXP *treehead,
                               InterCodeline codehead,
                               FieldList symhead,
                               char *name_func
                               );
InterCodeline translate_Stmt(struct TREE_EXP *treehead, //fixed
                             InterCodeline codehead, //change-able
                             FieldList symhead,  //fixed
                             char *name_func    //fixed
                             );
Operand getop(enum op_kind kind, int number, int value, FieldList var);
InterCodeline BuildInterCode(struct TREE_EXP *treehead, 
                        InterCodeline codehead, 
                        FieldList symhead,
                        char *name_func
                        );
/***************************************************************************/


InterCodeline BuildInterCode(struct TREE_EXP *treehead, 
                        InterCodeline codehead, 
                        FieldList symhead,
                        char *name_func
                        )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    //printf("............... %s\n", treehead->key);
    //InterCodeline temp;
    if (!strcmp(treehead->key, "Stmt")) {
        //printf("func1!\n");
        codehead = translate_Stmt(treehead, codehead, symhead, name_func);
        //if (codehead == NULL) printf("stmt failed!\n");
        //else printf("stmt?\n");
        return codehead;
    }
    else if (!strcmp(treehead->key, "FunDec")) {
        //printf("func2!\n");
        codehead = translate_FunDec(treehead, codehead, symhead, name_func);
        //if (codehead == NULL) printf("funcdec failed!\n");
        //else printf("funcdec\n");
        return codehead;
    }
    codehead = BuildInterCode(treehead->c1, codehead, symhead, name_func);
    codehead = BuildInterCode(treehead->c2, codehead, symhead, name_func);
    codehead = BuildInterCode(treehead->c3, codehead, symhead, name_func);
    //if (codehead == NULL) printf("zong failed!\n");
    //else printf("%s : kind : %d\n", treehead->key, codehead->code->kind);
    return codehead;
}

Operand getop(enum op_kind kind, int number, int value, FieldList var)
{
    Operand op = (Operand)malloc(sizeof(struct Operand_));
    op->kind = kind;
    if (kind == VARIABLE || kind == ADDRESS || kind == OP_FUNC || kind == OP_PARAM) {
        op->var = var;
    }
    else {
        op->var = NULL;
    }
    if (kind == CONSTANT) {
        op->var_no = 0;
    }
    else {
        op->var_no = number;
    }
    if (kind == CONSTANT || kind == TEMP) {
        op->value = value;
    }
    else {
        op->value = 0;
    }
    return op;
}

InterCodeline translate_Stmt(struct TREE_EXP *treehead, //fixed
                             InterCodeline codehead, //change-able
                             FieldList symhead,  //fixed
                             char *name_func    //fixed
                             )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    printf("self: %s. father: %s. child: %s\n", treehead->key, treehead->father->key, treehead->c1->key);
    if (treehead->kind == NT) { //Exp SEMI
        //printf("this is hello 6\n");
        codehead = translate_Exp(treehead->c1, codehead, symhead, NULL, name_func);
        printf("%s this is hello 7\n", treehead->key);
        return codehead;
    }
    else if (treehead->kind == N) { //CompSt
        printf("mark1\n");
        codehead = translate_CompSt(treehead->c1, codehead, symhead, name_func);
        printf("mark2\n");
        //printf("self: %s. father: %s. child: %s\n", treehead->key, treehead->father->key, treehead->c1->key);
        return codehead;
    }
    else if (treehead->kind == TNT) {   //RETURN Exp SEMI
        //printf("000000000\n");
        Operand optemp = getop(TEMP, count_temp++, 0, NULL);
        //printf("1111111111\n");
        codehead = translate_Exp(treehead->c1, codehead, symhead, optemp, name_func);
        //printf("222222222\n");
        InterCodeline intercodeline = BuildInterCodeline(CODE_RETURN, optemp, NULL, NULL);
        //printf("333333333\n");
        codehead = AddToInterCodelinelist(codehead, intercodeline);
        return codehead;
    }
    else if (treehead->kind == TTNTN) {
        if (!strcmp(treehead->term_name0, "IF")) {//IF LP Exp RP Stmt
            Operand label1 = getop(OP_LABEL, count_label++, 0, NULL);
            Operand label2 = getop(OP_LABEL, count_label++, 0, NULL);
            codehead = translate_Cond(treehead->c1, codehead, symhead, label1, label2, name_func);
            InterCodeline intercodeline1 = BuildInterCodeline(CODE_LABEL, label1, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline1);
            printf("here1?\n");
            codehead = translate_Stmt(treehead->c2, codehead, symhead, name_func);
            printf("here2!\n");
            InterCodeline intercodeline2 = BuildInterCodeline(CODE_LABEL, label2, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline2);
            return codehead;
        }
        else if (!strcmp(treehead->term_name0, "WHILE")) {//WHILE LP Exp RP Stmt
            Operand label1 = getop(OP_LABEL, count_label++, 0, NULL);
            Operand label2 = getop(OP_LABEL, count_label++, 0, NULL);
            Operand label3 = getop(OP_LABEL, count_label++, 0, NULL);
            InterCodeline intercodeline1 = BuildInterCodeline(CODE_LABEL, label1, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline1);
            codehead = translate_Cond(treehead->c1, codehead, symhead, label2, label3, name_func);
            InterCodeline intercodeline2 = BuildInterCodeline(CODE_LABEL, label2, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline2);
            codehead = translate_Stmt(treehead->c2, codehead, symhead, name_func);
            InterCodeline intercodeline3 = BuildInterCodeline(CODE_GOTO, label1, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline3);
            InterCodeline intercodeline4 = BuildInterCodeline(CODE_LABEL, label3, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline4);
            return codehead;
        }
    }
    else if (treehead->kind == TTNTNTN) {   //IF LP Exp RP Stmt ELSE Stmt
        Operand label1 = getop(OP_LABEL, count_label++, 0, NULL);
        Operand label2 = getop(OP_LABEL, count_label++, 0, NULL);
        Operand label3 = getop(OP_LABEL, count_label++, 0, NULL);
        codehead = translate_Cond(treehead->c1, codehead, symhead, label1, label2, name_func);
        InterCodeline intercodeline1 = BuildInterCodeline(CODE_LABEL, label1, NULL, NULL);
        codehead = AddToInterCodelinelist(codehead, intercodeline1);
        codehead = translate_Stmt(treehead->c2, codehead, symhead, name_func);
        InterCodeline intercodeline2 = BuildInterCodeline(CODE_GOTO, label3, NULL, NULL);
        codehead = AddToInterCodelinelist(codehead, intercodeline2);
        InterCodeline intercodeline3 = BuildInterCodeline(CODE_LABEL, label2, NULL, NULL);
        codehead = AddToInterCodelinelist(codehead, intercodeline3);
        codehead = translate_Stmt(treehead->c3, codehead, symhead, name_func);
        InterCodeline intercodeline4 = BuildInterCodeline(CODE_LABEL, label3, NULL, NULL);
        codehead = AddToInterCodelinelist(codehead, intercodeline4);
        return codehead;
    }
    return codehead;
}

InterCodeline translate_FunDec(struct TREE_EXP *treehead,
                               InterCodeline codehead,
                               FieldList symhead,
                               char *name_func
                               )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    FieldList symtemp = findFunc(symhead, treehead->term_name0);
    //printf("hehe\n");
    Operand opfunc = getop(OP_FUNC, 0, 0, symtemp);
    //printf("hehe\n");
    InterCodeline intercodeline = BuildInterCodeline(CODE_FUNC, opfunc, NULL, NULL);
    //if (intercodeline == NULL) printf("xixilei!\n");
    //else printf("iiiiii\n");
    //if (codehead == NULL) printf("yep supposed to be\n");
    codehead = AddToInterCodelinelist(codehead, intercodeline);
    //printf("kind : %d\n", codehead->code->kind);
    if (treehead->kind == TTNT) {
        codehead = translate_VarList(treehead->c1, codehead, symhead, name_func);
    }
    else if (treehead->kind == TTT) {
        ;
    }
    //if (codehead == NULL) printf("xixilei!\n");
    //else printf("iiiiii\n");
    return codehead;
}

InterCodeline translate_VarList(struct TREE_EXP* treehead,
                                InterCodeline codehead,
                                FieldList symhead,
                                char *name_func
                                )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    InterCodeline intercodeline = NULL;
    if (treehead->c1->c2->kind == T) {
        FieldList symtemp = findVarNoArea(symhead, treehead->c1->c2->term_name0, name_func, NULL);
        Operand optemp = getop(OP_PARAM, 0, 0, symtemp);
        intercodeline = BuildInterCodeline(CODE_PARAM, optemp, NULL, NULL);
        codehead = AddToInterCodelinelist(codehead, intercodeline);
    }
    if (treehead->kind == NTN) {
        codehead = translate_VarList(treehead->c2, codehead, symhead, name_func);
    }
    return codehead;
}

InterCodeline translate_CompSt(struct TREE_EXP* treehead,
                               InterCodeline codehead,
                               FieldList symhead,
                               char *name_func
                               )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    printf("%s\n", treehead->c2->key);
    printf("mark3\n");
    codehead = translate_StmtList(treehead->c2, codehead, symhead, name_func);
    printf("mark4\n");
    return codehead;
}

InterCodeline translate_StmtList(struct TREE_EXP* treehead,
                                 InterCodeline codehead,
                                 FieldList symhead,
                                 char *name_func
                                 )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    codehead = translate_Stmt(treehead->c1, codehead, symhead, name_func);
    codehead = translate_StmtList(treehead->c2, codehead, symhead, name_func);
    if (treehead->c2 == NULL) printf("stmtlist is null\n");
    return codehead;
}

InterCodeline translate_Exp(struct TREE_EXP* treehead,
                            InterCodeline codehead,
                            FieldList symhead,
                            Operand op,
                            char *name_func
                            )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    if (op == NULL) op = getop(TEMP, count_temp++, 0, NULL);
    //printf("this is hello 4 %s\n", treehead->key);
    switch (treehead->kind) {
        case NTN :
            //printf("this is hello 5\n");
            //printf("%s\n", treehead->term_name0);
            if (!strcmp(treehead->term_name0, "ASSIGNOP")) {
                //Exp = Exp
                //printf("this is hello 3\n");
                codehead = translate_Exp(treehead->c1, codehead, symhead, op, name_func);
                Operand optemp = getop(TEMP, count_temp++, 0, NULL);
                codehead = translate_Exp(treehead->c2, codehead, symhead, optemp, name_func);
                op->value = optemp->value;
                //printf("this is hello 1\n");
                InterCodeline intercodeline = BuildInterCodeline(CODE_ASSIGN, op, optemp, NULL);
                //printf("this is hello 2\n");
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                return codehead;
                /*if (treehead->c1->tkind[0] == _ID) {
                    symtemp1 = findVarNoArea(sym_table, treehead->c1->term_name0, name_func, NULL);
                    headlist = translate_Exp(treehead->c2, sym_table, &optemp2, name_func, headlist);
                    if (symtemp->type->kind == BASIC && symtemp->type->basic == 1) {
                        optemp1 = BuildOperand(1, 11, optemp2->u.value, 0, symtemp1);
                    }
                    else if (symtemp->type->kind == BASIC && symtemp->type->basic == 2) {
                        optemp1 = BuildOperand(1, 21, 0, optemp2->u.fvalue, symtemp1);
                    }
                    ICLans_temp = BuildInterCodeline("CODE_ASSIGN", optemp1, optemp2, NULL);
                    headlist = AddToInterCodelinelist(headlist, ICLans_temp);
                    *ophead = optemp1;
                    return headlist;
                }*/
            }
            else if (!strcmp(treehead->term_name0, "AND")) {
                codehead = translate_BoolExp(treehead, codehead, symhead, op, name_func);
                return codehead;
            }
            else if (!strcmp(treehead->term_name0, "OR")) {
                codehead = translate_BoolExp(treehead, codehead, symhead, op, name_func);
                return codehead;
            }
            else if (!strcmp(treehead->term_name0, "RELOP")) {
                codehead = translate_BoolExp(treehead, codehead, symhead, op, name_func);
                return codehead;
            }
            else if (!strcmp(treehead->term_name0, "PLUS")) {
                //Exp = Exp + Exp
                Operand optemp1 = getop(TEMP, count_temp++, 0, NULL);
                Operand optemp2 = getop(TEMP, count_temp++, 0, NULL);
                codehead = translate_Exp(treehead->c1, codehead, symhead, optemp1, name_func);
                codehead = translate_Exp(treehead->c2, codehead, symhead, optemp2, name_func);
                op->value = optemp1->value + optemp2->value;
                InterCodeline intercodeline = BuildInterCodeline(CODE_ADD, op, optemp1, optemp2);
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                return codehead;
                /*
                headlist = translate_Exp(treehead->c1, sym_table, &optemp2, name_func, headlist);
                headlist = translate_Exp(treehead->c2, sym_table, &optemp3, name_func, headlist);

                if ((optemp2->u.var_type == 10 || optemp2->u.var_type == 11) && 
                    (optemp3->u.var_type == 10 || optemp3->u.var_type == 11)) {
                    optemp1 = BuildOperand(1, 10, optemp2->u.value+optemp3->u.value, 0, NULL);
                }
                else if ((optemp2->u.var_type == 10 || optemp2->u.var_type == 11) && 
                         (optemp3->u.var_type == 20 || optemp3->u.var_type == 21)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.value+optemp3->u.fvalue, NULL);
                }
                else if ((optemp2->u.var_type == 20 || optemp2->u.var_type == 21) && 
                         (optemp3->u.var_type == 10 || optemp3->u.var_type == 11)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.fvalue+optemp3->u.value, NULL);
                }
                else if ((optemp2->u.var_type == 20 || optemp2->u.var_type == 21) && 
                         (optemp3->u.var_type == 20 || optemp3->u.var_type == 21)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.fvalue+optemp3->u.fvalue, NULL);
                }

                ICLans_temp = BuildInterCodeline("PLUS", optemp1, optemp2, optemp3);
                headlist = AddToInterCodelinelist(headlist, ICLans_temp);
                *ophead = optemp1;
                return headlist;*/
            }
            else if (!strcmp(treehead->term_name0, "MINUS")) {
                //Exp = Exp - Exp
                Operand optemp1 = getop(TEMP, count_temp++, 0, NULL);
                Operand optemp2 = getop(TEMP, count_temp++, 0, NULL);
                codehead = translate_Exp(treehead->c1, codehead, symhead, optemp1, name_func);
                codehead = translate_Exp(treehead->c2, codehead, symhead, optemp2, name_func);
                op->value = optemp1->value - optemp2->value;
                InterCodeline intercodeline = BuildInterCodeline(CODE_SUB, op, optemp1, optemp2);
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                return codehead;
                /*
                headlist = translate_Exp(treehead->c1, sym_table, &optemp2, name_func, headlist);
                headlist = translate_Exp(treehead->c2, sym_table, &optemp3, name_func, headlist);
                
                if ((optemp2->u.var_type == 10 || optemp2->u.var_type == 11) && 
                    (optemp3->u.var_type == 10 || optemp3->u.var_type == 11)) {
                    optemp1 = BuildOperand(1, 10, optemp2->u.value-optemp3->u.value, 0, NULL);
                }
                else if ((optemp2->u.var_type == 10 || optemp2->u.var_type == 11) && 
                         (optemp3->u.var_type == 20 || optemp3->u.var_type == 21)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.value-optemp3->u.fvalue, NULL);
                }
                else if ((optemp2->u.var_type == 20 || optemp2->u.var_type == 21) && 
                         (optemp3->u.var_type == 10 || optemp3->u.var_type == 11)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.fvalue-optemp3->u.value, NULL);
                }
                else if ((optemp2->u.var_type == 20 || optemp2->u.var_type == 21) && 
                         (optemp3->u.var_type == 20 || optemp3->u.var_type == 21)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.fvalue-optemp3->u.fvalue, NULL);
                }

                ICLans_temp = BuildInterCodeline("MINUS", optemp1, optemp2, optemp3);
                headlist = AddToInterCodelinelist(headlist, ICLans_temp);
                *ophead = optemp1;
                return headlist;*/
            }
            else if (!strcmp(treehead->term_name0, "STAR")) {
                //Exp = Exp * Exp
                Operand optemp1 = getop(TEMP, count_temp++, 0, NULL);
                Operand optemp2 = getop(TEMP, count_temp++, 0, NULL);
                codehead = translate_Exp(treehead->c1, codehead, symhead, optemp1, name_func);
                codehead = translate_Exp(treehead->c2, codehead, symhead, optemp2, name_func);
                op->value = optemp1->value * optemp2->value;
                InterCodeline intercodeline = BuildInterCodeline(CODE_MUL, op, optemp1, optemp2);
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                return codehead;
                /*
                headlist = translate_Exp(treehead->c1, sym_table, &optemp2, name_func, headlist);
                headlist = translate_Exp(treehead->c2, sym_table, &optemp3, name_func, headlist);
                
                if ((optemp2->u.var_type == 10 || optemp2->u.var_type == 11) && 
                    (optemp3->u.var_type == 10 || optemp3->u.var_type == 11)) {
                    optemp1 = BuildOperand(1, 10, optemp2->u.value*optemp3->u.value, 0, NULL);
                }
                else if ((optemp2->u.var_type == 10 || optemp2->u.var_type == 11) && 
                         (optemp3->u.var_type == 20 || optemp3->u.var_type == 21)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.value*optemp3->u.fvalue, NULL);
                }
                else if ((optemp2->u.var_type == 20 || optemp2->u.var_type == 21) && 
                         (optemp3->u.var_type == 10 || optemp3->u.var_type == 11)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.fvalue*optemp3->u.value, NULL);
                }
                else if ((optemp2->u.var_type == 20 || optemp2->u.var_type == 21) && 
                         (optemp3->u.var_type == 20 || optemp3->u.var_type == 21)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.fvalue*optemp3->u.fvalue, NULL);
                }

                ICLans_temp = BuildInterCodeline("CODE_MUL", optemp1, optemp2, optemp3);
                headlist = AddToInterCodelinelist(headlist, ICLans_temp);
                *ophead = optemp1;
                return headlist;*/
            }
            else if (!strcmp(treehead->term_name0, "DIV")) {
                //Exp = Exp / Exp
                Operand optemp1 = getop(TEMP, count_temp++, 0, NULL);
                Operand optemp2 = getop(TEMP, count_temp++, 0, NULL);
                codehead = translate_Exp(treehead->c1, codehead, symhead, optemp1, name_func);
                codehead = translate_Exp(treehead->c2, codehead, symhead, optemp2, name_func);
                op->value = optemp1->value / optemp2->value;
                InterCodeline intercodeline = BuildInterCodeline(CODE_DIV, op, optemp1, optemp2);
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                return codehead;
                /*
                headlist = translate_Exp(treehead->c1, sym_table, &optemp2, name_func, headlist);
                headlist = translate_Exp(treehead->c2, sym_table, &optemp3, name_func, headlist);
                
                if ((optemp2->u.var_type == 10 || optemp2->u.var_type == 11) && 
                    (optemp3->u.var_type == 10 || optemp3->u.var_type == 11)) {
                    optemp1 = BuildOperand(1, 10, optemp2->u.value/optemp3->u.value, 0, NULL);
                }
                else if ((optemp2->u.var_type == 10 || optemp2->u.var_type == 11) && 
                         (optemp3->u.var_type == 20 || optemp3->u.var_type == 21)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.value/optemp3->u.fvalue, NULL);
                }
                else if ((optemp2->u.var_type == 20 || optemp2->u.var_type == 21) && 
                         (optemp3->u.var_type == 10 || optemp3->u.var_type == 11)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.fvalue/optemp3->u.value, NULL);
                }
                else if ((optemp2->u.var_type == 20 || optemp2->u.var_type == 21) && 
                         (optemp3->u.var_type == 20 || optemp3->u.var_type == 21)) {
                    optemp1 = BuildOperand(1, 20, 0, optemp2->u.fvalue/optemp3->u.fvalue, NULL);
                }

                ICLans_temp = BuildInterCodeline("CODE_DIV", optemp1, optemp2, optemp3);
                headlist = AddToInterCodelinelist(headlist, ICLans_temp);
                *ophead = optemp1;
                return headlist;*/
            }
            break;
        case TNT :
            //( Exp )
            codehead = translate_Exp(treehead->c1, codehead, symhead, op, name_func);
            return codehead;
            break;
        case TN :
            if (!strcmp(treehead->term_name0, "MINUS")) {
                //- Exp
                Operand optemp = getop(TEMP, count_temp++, 0, NULL);
                codehead = translate_Exp(treehead->c1, codehead, symhead, optemp, name_func);
                Operand opconst = getop(CONSTANT, 0, 0, NULL);
                op->value = 0 - optemp->value;
                InterCodeline intercodeline = BuildInterCodeline(CODE_SUB, op, opconst, optemp);
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                return codehead;
            }
            else if (!strcmp(treehead->term_name0, "NOT")) {
                codehead = translate_BoolExp(treehead, codehead, symhead, op, name_func);
                return codehead;
            }
            break;
        case TTNT :
            //ID ( Args )
            if (1) {
                FieldList functemp = findFunc(symhead, treehead->term_name0);
                Operand opfunc = getop(OP_FUNC, 0, 0, functemp);
                Operand argList[ARRAY_ARGS];
                int i;
                for (i = 0; i < ARRAY_ARGS; i++) {
                    argList[i] = NULL;
                }
                codehead = translate_Args(treehead->c1, codehead, symhead, argList, name_func);
                if (!strcmp(treehead->term_name0, "write")) {
                    InterCodeline intercodeline = BuildInterCodeline(WRITE, argList[0], NULL, NULL);
                    codehead = AddToInterCodelinelist(codehead, intercodeline);
                    return codehead;
                }
                else {
                    int j;
                    for (j = 0; j < ARRAY_ARGS; j ++) {
                        if (argList[j] == NULL) break;
                        InterCodeline intercodeline = BuildInterCodeline(CODE_ARG, argList[j], NULL, NULL);
                        codehead = AddToInterCodelinelist(codehead, intercodeline);
                    }
                    InterCodeline another = BuildInterCodeline(CODE_CALL, op, opfunc, NULL);
                    codehead = AddToInterCodelinelist(codehead, another);
                    return codehead;
                }
            }
            break;
        case TTT :
            //ID ()
            if (!strcmp(treehead->term_name0, "read")) {
                InterCodeline intercodeline = BuildInterCodeline(READ, op, NULL, NULL);
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                return codehead;
            }
            else {
                FieldList functemp = findFunc(symhead, treehead->term_name0);
                Operand opfunc = getop(OP_FUNC, 0, 0, functemp);
                InterCodeline intercodeline = BuildInterCodeline(CODE_CALL, op, opfunc, NULL);
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                return codehead;
            }
            break;
        case NTNT :
            return codehead;
            break;
        case NTT :
            return codehead;
            break;
        case T :
            //printf("hello??\n");
            if (treehead->tkind[0] == _IN) {
                //INT
                Operand optemp;
                Operand op2 = getop(CONSTANT, 0, treehead->term_int0, NULL);
                op->value = op2->value;
                printf("%d\n", treehead->term_int0);
                /*
                if ((optemp = findTemp(codehead, op)) != NULL) {
                    op = optemp;
                    count_temp --;
                }*/
                //printf("4444444444\n");
                InterCodeline intercodeline = BuildInterCodeline(CODE_ASSIGN, op, op2, NULL);
                //printf("555555\n");
                //displayInterCode(codehead, symhead);
                codehead = AddToInterCodelinelist(codehead, intercodeline);
                //printf("666666666666\n");
                return codehead;
            }
            else if (treehead->tkind[0] == _ID) {
                //ID
                FieldList var = findVarNoArea(symhead, treehead->term_name0, name_func, NULL);
                //op = getop(VARIABLE, count_var++, 0, 0, var);
                op->kind = VARIABLE;
                op->var_no = count_var++;
                op->value = 0;
                op->var = var;
                printf("%s\n", treehead->term_name0);
                //printf("?????????? !!!!!!!!!!!!!!!!%s\n", treehead->key);
                //printf("%s : kind : %d\n", treehead->key, codehead->code->kind);
                return codehead;
            }
            break;
    }
    return codehead;
}

InterCodeline translate_Args(struct TREE_EXP *treehead, 
                             InterCodeline codehead, 
                             FieldList symhead, 
                             Operand argList[ARRAY_ARGS],
                             char *name_func
                             )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    Operand optemp = getop(TEMP, count_temp++, 0, NULL);
    codehead = translate_Exp(treehead->c1, codehead, symhead, optemp, name_func);
    argList = AddToArgList(argList, optemp);
    if (treehead->kind == NTN) {
        codehead = translate_Args(treehead->c2, codehead, symhead, argList, name_func);
    }
    return codehead;
}

Operand *AddToArgList(Operand *argList, Operand temp)
{
    int i;
    for (i = 0; i < ARRAY_ARGS; i ++) {
        if (argList[i] == NULL) {
            argList[i] = temp;
            break;
        }
    }
    return argList;
}

InterCodeline translate_BoolExp(struct TREE_EXP *treehead, 
                                InterCodeline codehead, 
                                FieldList symhead, 
                                Operand op, 
                                char *name_func
                                )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    if (op == NULL) op = getop(TEMP, count_temp++, 0, NULL);
    Operand label1 = getop(OP_LABEL, count_label++, 0, NULL);
    Operand label2 = getop(OP_LABEL, count_label++, 0, NULL);
    //printf("here comes the count_temp!!! %d\n\n\n\n\n\n\n", count_temp);
    Operand boolop0 = getop(CONSTANT, 0, 0, NULL);
    Operand boolop1 = getop(CONSTANT, 0, 1, NULL);
    InterCodeline intercodeline1 = BuildInterCodeline(CODE_ASSIGN, op, boolop0, NULL);
    codehead = AddToInterCodelinelist(codehead, intercodeline1);
    codehead = translate_Cond(treehead, codehead, symhead, label1, label2, name_func);
    InterCodeline intercodeline2 = BuildInterCodeline(CODE_LABEL, label1, NULL, NULL);
    codehead = AddToInterCodelinelist(codehead, intercodeline2);
    InterCodeline intercodeline3 = BuildInterCodeline(CODE_ASSIGN, op, boolop1, NULL);
    codehead = AddToInterCodelinelist(codehead, intercodeline3);
    InterCodeline intercodeline4 = BuildInterCodeline(CODE_LABEL, label2, NULL, NULL);
    codehead = AddToInterCodelinelist(codehead, intercodeline4);
    return codehead;
}

InterCodeline translate_Cond(struct TREE_EXP *treehead,
                             InterCodeline codehead,
                             FieldList symhead,
                             Operand label1,
                             Operand label2, 
                             char *name_func
                             )
{
    if (treehead == NULL || symhead == NULL) return codehead;
    /*
    if (!strcmp(treehead->c1->key, "Exp")) {
        //Exp
        Operand optemp = getop(TEMP, count_temp++, 0, NULL);
        codehead = translate_Exp(treehead->c1, codehead, symhead, optemp, name_func);
        Operand opconst = getop(CONSTANT, 0, 0, NULL);
        InterCodeline intercodeline1 = BuildInterCodeline(IF_NE, optemp, opconst, label1);
        codehead = AddToInterCodelinelist(codehead, intercodeline1);
        InterCodeline intercodeline2 = BuildInterCodeline(CODE_GOTO, label2, NULL, NULL);
        codehead = AddToInterCodelinelist(codehead, intercodeline2);
        return codehead;
    }
    else*/ if (treehead->kind == TN) {
        //NOT Exp
        codehead = translate_Cond(treehead->c1, codehead, symhead, label2, label1, name_func);
        return codehead;
    }
    else if (treehead->kind == NTN) {
        if (!strcmp(treehead->term_name0, "RELOP")) {
            //Exp RELOP Exp
            Operand optemp1 = getop(TEMP, count_temp++, 0, NULL);
            Operand optemp2 = getop(TEMP, count_temp++, 0, NULL);
            //printf("here_comes_the_count_temp!!! %d\n\n\n\n\n\n\n", count_temp);
            codehead = translate_Exp(treehead->c1, codehead, symhead, optemp1, name_func);
            codehead = translate_Exp(treehead->c2, codehead, symhead, optemp2, name_func);
            InterCodeline intercodeline1 = BuildInterCodeline(RelopToKind(treehead), optemp1, optemp2, label1);
            codehead = AddToInterCodelinelist(codehead, intercodeline1);
            InterCodeline intercodeline2 = BuildInterCodeline(CODE_GOTO, label2, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline2);
            return codehead;
        }
        else if (!strcmp(treehead->term_name0, "AND")) {
            //Exp && Exp
            Operand labeltemp = getop(OP_LABEL, count_label++, 0, NULL);
            codehead = translate_Cond(treehead->c1, codehead, symhead, labeltemp, label2, name_func);
            InterCodeline intercodeline1 = BuildInterCodeline(CODE_LABEL, labeltemp, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline1);
            codehead = translate_Cond(treehead->c2, codehead, symhead, label1, label2, name_func);
            return codehead;
        }
        else if (!strcmp(treehead->term_name0, "OR")) {
            //Exp || Exp
            Operand labeltemp = getop(OP_LABEL, count_label++, 0, NULL);
            codehead = translate_Cond(treehead->c1, codehead, symhead, label1, labeltemp, name_func);
            InterCodeline intercodeline1 = BuildInterCodeline(CODE_LABEL, labeltemp, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline1);
            codehead = translate_Cond(treehead->c2, codehead, symhead, label1, label2, name_func);
            return codehead;
        }
        else {
            Operand optemp = getop(TEMP, count_temp++, 0, NULL);
            codehead = translate_Exp(treehead, codehead, symhead, optemp, name_func);
            Operand opconst = getop(CONSTANT, 0, 0, NULL);
            InterCodeline intercodeline1 = BuildInterCodeline(IF_NE, optemp, opconst, label1);
            codehead = AddToInterCodelinelist(codehead, intercodeline1);
            InterCodeline intercodeline2 = BuildInterCodeline(CODE_GOTO, label2, NULL, NULL);
            codehead = AddToInterCodelinelist(codehead, intercodeline2);
            return codehead;
        }
    }
    return codehead;
}

enum code_kind RelopToKind(struct TREE_EXP *treehead)
{
    if (strcmp(treehead->term_text0, ">") == 0) return IF_GT;
    else if (strcmp(treehead->term_text0, ">=") == 0) return IF_GE;
    else if (strcmp(treehead->term_text0, "<") == 0) return IF_LT;
    else if (strcmp(treehead->term_text0, "<=") == 0) return IF_LE;
    else if (strcmp(treehead->term_text0, "==") == 0) return IF_EQ;
    else if (strcmp(treehead->term_text0, "!=") == 0) return IF_NE;
}

/*
struct Operand_ *AddToVlist(Operand head, Operand node)
{
    if (head == NULL) {
        head = node;
        head->u.var_no = 1;
        head->prev = NULL;
        head->next = NULL;
        return head;
    }
    else {
        Operand temp = head;
        int i = head->u.var_no;
        while (temp->next != NULL) {
            i ++;
            temp = temp0->next;
        }
        i ++;
        temp->next = node;
        node->u.var_no = i;
        node->prev = temp;
        node->next = NULL;
        return head;
    }
}

struct Operand_ *AddToTlist(Operand head, Operand node)
{
    if (head == NULL) {
        head = node;
        head->u.var_no = 1;
        head->prev = NULL;
        head->next = NULL;
        return head;
    }
    else {
        Operand temp = head;
        int i = head->u.var_no;
        while (temp->next != NULL) {
            i ++;
            temp = temp0->next;
        }
        i ++;
        temp->next = node;
        node->u.var_no = i;
        node->prev = temp;
        node->next = NULL;
        return head;
    }
}

struct Operand_ *AddToLabellist(Operand head, Operand node)
{
    if (head == NULL) {
        head = node;
        head->u.var_no = 1;
        head->prev = NULL;
        head->next = NULL;
        return head;
    }
    else {
        Operand temp = head;
        int i = head->u.var_no;
        while (temp->next != NULL) {
            i ++;
            temp = temp0->next;
        }
        i ++;
        temp->next = node;
        node->u.var_no = i;
        node->prev = temp;
        node->next = NULL;
        return head;
    }
}
*/
struct Operand_ *findVariable(InterCodeline head, Operand node)
{
    InterCodeline temp = NULL;
    temp = head;
    Operand t;
    if (head == NULL) return NULL;
    while (temp != NULL) {
        t = temp->code->result;
        if (t->kind == node->kind && t->var == node->var) {
            node->var_no = t->var_no;
            return t;
        }
        t = temp->code->op1;
        if (t->kind == node->kind && t->var == node->var) {
            node->var_no = t->var_no;
            return t;
        }
        t = temp->code->op2;
        if (t->kind == node->kind && t->var == node->var) {
            node->var_no = t->var_no;
            return t;
        }
        temp = temp->next;
    }
    return NULL;
}

struct Operand_ *findTemp(InterCodeline head, Operand node)
{
    InterCodeline temp = NULL;
    temp = head;
    Operand t;
    if (head == NULL) return NULL;
    while (temp != NULL) {
        t = temp->code->result;
        if (t->kind == node->kind && t->value == node->value) {
            node->var_no = t->var_no;
            return t;
        }
        t = temp->code->op1;
        if (t->kind == node->kind && t->value == node->value) {
            node->var_no = t->var_no;
            return t;
        }
        t = temp->code->op2;
        if (t->kind == node->kind && t->value == node->value) {
            node->var_no = t->var_no;
            return t;
        }
        temp = temp->next;
    }
    return NULL;
}
/*
struct Operand_ *findLabel(Operand head, Operand node)
{
    Operand temp = NULL;
    temp = head;
    if (head == NULL) return NULL;
    while (temp != NULL) {
        if (temp->var == node->var) {
            node->var_no = temp->var_no;
            return temp;
        }
    }
    return NULL;
}
*/
struct InterCodeline_ *AddToInterCodelinelist(InterCodeline head, 
                                              InterCodeline node)
{
    //printf("7777777\n");
    if (head == NULL) {
        head = node;
        head->prev = NULL;
        head->next = NULL;
        //printf("hehe-head\n");
        //printf("%d\n", node->code->kind);
        return head;
    }
    else {
        int i = 0;
        //printf("7777777\n");
        InterCodeline temp = head;
        //printf("7777777\n");
        while (temp->next != NULL) {
            if (temp->next != NULL) {
                //printf("77777778 i = %d\n", i);
            }
            //printf("%d\n", node->code->kind);
            temp = temp->next;
            if (temp->next != NULL) {
                //printf("77777779 i = %d\n", i);
            }
            i ++;
        }
        if (node == NULL) {
            //printf("just 88!\n");
        }
        //printf("7777777\n");
        temp->next = node;
        //printf("7777777\n");
        node->prev = temp;
        //printf("7777777\n");
        node->next = NULL;
        //printf("hehe\n");
        return head;
    }
}

InterCodeline BuildInterCodeline(enum code_kind operator_kind, 
                                 Operand one, 
                                 Operand two, 
                                 Operand three)
{
    //printf("iiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
    InterCodeline node = (InterCodeline)malloc(sizeof(struct InterCodeline_));
    node->prev = NULL;
    node->next = NULL;
    //printf("%d\n", operator_kind);
    node->code = (InterCode)malloc(sizeof(struct InterCode_));
    node->code->kind = operator_kind;
    //printf("iiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
    node->code->result = one;
    //printf("iiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
    node->code->op1 = two;
    //printf("iiiiiiiiiiiiiiiiiiiiiiiiiiii\n");
    node->code->op2 = three;
    //printf("in BuildInterCodeline!\n");
    return node;
}

void displayOperand(Operand op)
{
    if (op == NULL) return;
    if (op->kind == CONSTANT) {
        printf("#%d", op->value);
    }
    else if (op->kind == TEMP) {
        printf("t%d", op->var_no);
    }
    else if (op->kind == VARIABLE) {
        printf("%s", op->var->name);
    }
    else if (op->kind == ADDRESS) {
        ;
    }
    else if (op->kind == OP_LABEL) {
        printf("LABEL L%d", op->var_no);
    }
    else if (op->kind == OP_FUNC) {
        printf("FUNCTION %s", op->var->name);
    }
    else if (op->kind == OP_PARAM) {
        printf("PARAM %s", op->var->name);
    }
    return;
}

void displayInterCode(InterCodeline codehead, FieldList symhead)
{
    if (codehead == NULL || symhead == NULL) {
        printf("No InterCode!\n");
        return;
    }
    InterCodeline codetemp = codehead;
    while (codetemp != NULL) {
        switch (codetemp->code->kind) {
            case CODE_LABEL :
                displayOperand(codetemp->code->result);
                break;
            case CODE_FUNC :
                displayOperand(codetemp->code->result);
                break;
            case CODE_ASSIGN:
                displayOperand(codetemp->code->result);
                printf(" := ");
                displayOperand(codetemp->code->op1);
                break;
            case CODE_ADD:
                displayOperand(codetemp->code->result);
                printf(" := ");
                displayOperand(codetemp->code->op1);
                printf(" + ");
                displayOperand(codetemp->code->op2);
                break;
            case CODE_SUB:
                displayOperand(codetemp->code->result);
                printf(" := ");
                displayOperand(codetemp->code->op1);
                printf(" - ");
                displayOperand(codetemp->code->op2);
                break;
            case CODE_MUL:
                displayOperand(codetemp->code->result);
                printf(" := ");
                displayOperand(codetemp->code->op1);
                printf(" * ");
                displayOperand(codetemp->code->op2);
                break;
            case CODE_DIV:
                displayOperand(codetemp->code->result);
                printf(" := ");
                displayOperand(codetemp->code->op1);
                printf(" / ");
                displayOperand(codetemp->code->op2);
                break;
            case CODE_ADDR:
                displayOperand(codetemp->code->result);
                printf(" := &");
                displayOperand(codetemp->code->op1);
                break;
            case CODE_FROM_ADDR:
                displayOperand(codetemp->code->result);
                printf(" := *");
                displayOperand(codetemp->code->op1);
                break;
            case CODE_TO_ADDR:
                printf("*");
                displayOperand(codetemp->code->result);
                printf(" := ");
                displayOperand(codetemp->code->op1);
                break;
            case CODE_GOTO:
                printf("GOTO ");
                displayOperand(codetemp->code->result);
                break;
            case IF_GT:
                printf("if ");
                displayOperand(codetemp->code->result);
                printf(" > ");
                displayOperand(codetemp->code->op1);
                printf(" GOTO ");
                displayOperand(codetemp->code->op2);
                break;
            case IF_GE:
                printf("if ");
                displayOperand(codetemp->code->result);
                printf(" >= ");
                displayOperand(codetemp->code->op1);
                printf(" GOTO ");
                displayOperand(codetemp->code->op2);
                break;
            case IF_LT:
                printf("if ");
                displayOperand(codetemp->code->result);
                printf(" < ");
                displayOperand(codetemp->code->op1);
                printf(" GOTO ");
                displayOperand(codetemp->code->op2);
                break;
            case IF_LE:
                printf("if ");
                displayOperand(codetemp->code->result);
                printf(" <= ");
                displayOperand(codetemp->code->op1);
                printf(" GOTO ");
                displayOperand(codetemp->code->op2);
                break;
            case IF_EQ:
                printf("if ");
                displayOperand(codetemp->code->result);
                printf(" == ");
                displayOperand(codetemp->code->op1);
                printf(" GOTO ");
                displayOperand(codetemp->code->op2);
                break;
            case IF_NE:
                printf("if ");
                displayOperand(codetemp->code->result);
                printf(" != ");
                displayOperand(codetemp->code->op1);
                printf(" GOTO ");
                displayOperand(codetemp->code->op2);
                break;
            case CODE_RETURN:
                printf("return ");
                displayOperand(codetemp->code->result);
                break;
            case CODE_DEC:
                break;
            case CODE_ARG:
                printf("Arg ");
                displayOperand(codetemp->code->result);
                break;
            case CODE_CALL:
                if (codetemp->code->result->var != NULL) {
                    displayOperand(codetemp->code->result);
                    printf(" := ");
                }
                printf("Call ");
                displayOperand(codetemp->code->op1);
                break;
            case CODE_PARAM:
                displayOperand(codetemp->code->result);
                break;
            case READ:
                printf("Read ");
                displayOperand(codetemp->code->result);
                break;
            case WRITE:
                printf("Write ");
                displayOperand(codetemp->code->result);
                break;
        }
        codetemp = codetemp->next;
        printf("\n");
    }
}

FieldList findVarNoArea(struct FieldList_ *head, char *name, char *name_func, Type type)
{
    if (head != NULL) {
        struct FieldList_ *temp = head;
        struct Type_ *t = NULL;
        int flag = 0;
        while(temp != NULL) {
            if (!strcmp(name, temp->name)) {
                if (!strcmp(name_func, "")) {
                    if (type == NULL || sameType(temp->type, type)) {    
                        flag = 1;
                        break;
                    }
                }
            }
            temp = temp->next;
        }
        return temp;
    }
    return NULL;
}