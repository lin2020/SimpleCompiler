#include "Semantic.c"

enum op_kind { 
	OP_VARIABLE, OP_TEMP, OP_CONSTANT, 
	OP_LABEL, OP_FUNC, OP_RELOP,
   	OP_SIZE, OP_ADDR, OP_STAR};

enum ic_kind { 
	CODE_LABEL, CODE_FUNC, CODE_ASSIGN, 
	CODE_ADD, CODE_SUB, CODE_MUL, CODE_DIV,
   	CODE_ADDR, CODE_GOTO, CODE_IFGT, 
   	CODE_RETURN, CODE_DEC, CODE_ARG, CODE_CALL,
   	CODE_PARAM, CODE_READ, CODE_WRITE};

typedef struct Operand Operand,*pOperand;
typedef struct InterCode InterCode,*pInterCode;
typedef struct InterCodes InterCodes,*pInterCodes;

struct Operand
{
	enum op_kind kind;
	union 
	{
		int var_no;
		int value;
		char str[32];
	}
};

struct InterCode
{
	enum ic_kind kind;
	pOperand op1,op2,op3,relop;
};

struct InterCodes
{
	pInterCode code;
	pInterCodes prev,next;
};

int temp_no = 0;
int label_no = 0;
pInterCodes codeHead = NULL;
pInterCodes codeLast = NULL;

pOperand newTemp();
pOperand newLabel();
pOperand newOp(enum op_kind);
pInterCode newCode(enum ic_kind);

void addCode(pInterCode);
void addLabel(pOperand);
void addGoto(pOperand);

void translate(pnode);
pOperand translateVarDec(pnode);
void translateFunDec(pnode);
void translateVarList(pnode);
void translateCompSt(pnode);
void translateDefList(pnode);
void translateDecList(pnode);
void translateDec(pnode);
void translateStmtList(pnode);
pOperand translateExp(pnode);
void translateStmt(pnode);
void translateCond(pnode, pOperand, pOperand);
void translateArgs(pnode);

//void optimize();

void displayOperand(pOperand);
void displayInterCode(pInterCode);
void displayInterCodes();

void generateLIR(pnode root);

pOperand newTemp()
{
	temp_no++;
	pOperand temp = (pOperand)malloc(sizeof(Operand));
	temp->kind = OP_TEMP;
	temp->var_no = temp_no;
	return temp;
}

pOperand newLabel()
{
	label_no++;
    pOperand label = (pOperand)malloc(sizeof(Operand));
    label->kind = OP_LABEL;
    label->var_no = label_no;
    return label;
}

pOperand newOp(enum op_kind kind) 
{
    pOperand op = (pOperand)malloc(sizeof(Operand));
    new->kind = kind;
    new->value = 0;
    return op;
}

pInterCode newCode(enum ic_kind kind) 
{
    pInterCode code = (pInterCode)malloc(sizeof(InterCode));
    code->kind = kind;
    code->op1 = NULL;
    code->op2 = NULL;
    code->op3 = NULL;
    code->relop = NULL;
    return code;
}

void addCode(pInterCode code) 
{
    pInterCodes codes = (pInterCodes)malloc(sizeof(InterCodes));    
    codes->code = code;
    codes->prev = NULL;
    codes->next = NULL;
    if(codeHead==NULL)
    {
    	codeHead = codes;
    	codeLast = codes;
    }
    else
    {
    	codeLast->next = codes;
    	codes->prev = codeLast;
    	codeLast = codes;
    }
}

void addLabel(pOperand label) 
{
    pInterCode code = newCode(CODE_LABEL);
    code->op1 = label;
    addCode(code);
}

void addGoto(pOperand label) 
{
    pInterCode code = newCode(CODE_GOTO);
    code->op1 = label;
    addCode(code);
}

void translate(pnode root)
{
    if(root == NULL) return;
    if(root->rule == 6) 
    {
        translateFunDec(root->child->brother);
        translateCompSt(root->child->brother->brother);
    }
    else
    {
    	pnode node = root->child;
    	for (; node!=NULL; node = node->brother)
        	translate(node);
    }
}

void translateFunDec(pnode  root) 
{
    if (root==NULL) 
    {
    	return;
    }
    pInterCode code = newCode(CODE_FUNC);
    code->op1 = newOp(OP_FUNC);
    strcpy(code->op1->str,root->child->c);
    addCode(code);
    if (root->rule == 19)  
    { // FunDec -> ID LP VarList RP
        translateVarList(root->child->brother->brother);
    }
}

void translateCompSt(pnode  root) 
{
    if(root==NULL) 
    {
    	return;
    }
    translateDefList(root->child->brother);
    translateStmtList(root->child->brother->brother);
}

void translateVarList(pnode  root) 
{
    if(root==NULL) 
    {
    	return;
    }
    pInterCode code = newCode(CODE_PARAM);
    code->op1 = newOp(OP_VARIABLE);
    strcpy(ode->op1->str,root->child->child->brother->child->c);
    addCode(code);
    if (root->rule == 21)
    {
        translateVarList(root->child->brother->brother);
    }
}

void translateDefList(pnode root) 
{
    if(!root || root->rule == 34) 
    {
    	return;
    }
    translateDecList(root->child->child->brother);
    translateDefList(root->child->brother);
}

void translateDecList(pnode root) 
{
    if (root==NULL) 
    {
    	return;
    }
    translateDec(root->child);
    if (root->rule == 37) 
    {
    	translateDecList(root->child->brother->brother);
    }
}

void translateStmtList(pnode  root) 
{
    if (!root || root->rule == 26) 
    {
    	return;
    }
    translateStmt(root->child);
    translateStmtList(root->child->brother);
}

void translateStmt(pnode  root) 
{
    if (!root) 
    {
    	return;
    }
    pInterCode code = NULL;
    pOperand label1=NULL;
    pOperand label2=NULL;
    pOperand label3=NULL;
    switch (root->rule) 
    {
        case 27:    // Stmt -> Exp SEMI
            translateExp(root->child);
            break;
        case 28:    // Stmt -> CompSt
            translateCompSt(root->child);
            break;
        case 29:    // Stmt -> RETURN Exp SEMI
            code = newCode(CODE_RETURN);
            code->op1 = translateExp(root->child->brother);
            addCode(code);
            break;
        case 30:    // Stmt -> IF LP Exp RP Stmt
            label1 = newLabel();
            label2 = newLabel();
            translateCond(root->child->brother->brother, label1, label2);
            addLabel(label1);
            translateStmt(root->child->brother->brother->brother->brother);
            addLabel(label2);
            break;
        case 31:    // Stmt -> IF LP Exp RP Stmt ELSE Stmt
            label1 = newLabel();
            label2 = newLabel();
            label3 = newLabel();
            translateCond(root->child->brother->brother, label1, label2);
            addLabel(label1);
            translateStmt(root->child->brother->brother->brother->brother);
            addGoto(label3);
            addLabel(label2);
            translateStmt(root->child->brother->brother->brother->brother->brother->brother);
            addLabel(label3);
            break;
        case 32:    // Stmt -> WHILE LP Exp RP Stmt
            label1 = newLabel();
            label2 = newLabel();
            label3 = newLabel();
            addLabel(label1);
            translateCond(root->child->brother->brother, label2, label3);
            addLabel(label2);
            translateStmt(root->child->brother->brother->brother->brother);
            addGoto(label1);
            addLabel(label3);
            break;
    }
}


void translateDec(pnode root) 
{
    if(!root)
    {
     	return;
    }
    if (root->rule == 38) 
    {
        translateVarDec(root->child);
    } 
    else if (root->rule == 39) 
    {
        pInterCode code = newCode(CODE_ASSIGN);
        code->op1 = translateVarDec(root->child);
        code->op2 = translateExp(root->child->brother->brother);
        addCode(code);
    }
}

pOperand translateVarDec(pnode root) 
{
    if (root==NULL) 
    {
    	return NULL;
    }
    pOperand op = NULL;
    pInterCode code = NULL;
    if (root->rule == 17)
    {
        op = newOp(OP_VARIABLE);
        strcpy(op->str,root->child->c);
    } 
    else if (root->rule == 18) 
    {
        code = newCode(CODE_DEC);
        code->op1 = translateVarDec(root->child);
        op = newOp(OP_SIZE);
        op->value = root->child->brother->brother->i;
        code->op2 = op;
        addCode(code);
    }
    return op;
}

pOperand translateExp(pnode  root) 
{
    if (!root) 
    {
    	return NULL;
    }
    pOperand op = NULL;
    pInterCode code = NULL;
    switch (root->rule) 
    {
        case 40:    // Exp -> Exp ASSIGNOP Exp
            code = newCode(CODE_ASSIGN);
            code->op1 = translateExp(root->child);
            code->op2 = translateExp(root->child->brother->brother);
            break;
        case 44:    // Exp -> Exp PLUS Exp
            code = newCode(CODE_ADD);
            code->op2 = translateExp(root->child);
            code->op3 = translateExp(root->child->brother->brother);
            code->op1 = newTemp();
            break;
        case 45:    // Exp -> Exp MINUS Exp
            code = newCode(CODE_SUB);
            code->op2 = translateExp(root->child);
            code->op3 = translateExp(root->child->brother->brother);
            code->op1 = newTemp();
            break;
        case 46:    // Exp -> Exp STAR Exp
            code = newCode(CODE_MUL);
            code->op2 = translateExp(root->child);
            code->op3 = translateExp(root->child->brother->brother);
            code->op1 = newTemp();
            break;
        case 47:    // Exp -> Exp DIV Exp
            code = newCode(CODE_DIV);
            code->op2 = translateExp(root->child);
            code->op3 = translateExp(root->child->brother->brother);
            code->op1 = newTemp();
            break;
        case 48:    // Exp -> LP Exp RP
            return translateExp(root->child->brother);
            break;
        case 49:    // Exp -> MINUS Exp
            code = newCode(CODE_SUB);
            code->op2 = newOp(OP_CONSTANT);
            code->op2->value = 0;
            code->op3 = translateExp(root->child->brother);
            code->op1 = newTemp();
            break;
        case 51:    // Exp -> ID LP Args RP
            code = newCode(CODE_CALL);
            translateArgs(root->child->brother->brother);
            code->op2 = newOp(OP_FUNC);
            code->op2->str = root->child->type_id;
            code->op1 = newTemp();
            break;
        case 52:    // Exp -> ID LP RP
            code = newCode(CODE_CALL);
            code->op2 = newOp(OP_FUNC);
            strcpy(code->op2->str, root->child->c);
            code->op1 = newTemp();
            break;
        case 53:    // Exp -> Exp LB Exp RB
            code = newCode(CODE_MUL);
            code->op2 = translateExp(root->child->brother->brother);
            code->op3 = newOp(OP_CONSTANT);
            code->op3->value = 4;
            op = newTemp();
            code->op1 = op;
            addCode(code);
            
            code = newCode(CODE_ADD);
            code->op3 = op;
            op = newOp(OP_ADDREES);
            strcpy(op->str, translateExp(root->child)->str);
            code->op2 = op;
            code->op1 = newTemp();
            addCode(code);
            op = newOp(OP_STAR);
            strcpy(op->str, code->op1->str);
            return op;
            break;
        case 55:    // Exp -> ID
            op = newOp(OP_VARIABLE);
            strcpy(op->str, root->child->c);
            return op;
            break;
        case 56:    // Exp -> INT
            op = newOp(OP_CONSTANT);
            op->value = root->child->i;
            return op;
            break;
        default:
            break;
    }
    addCode(code);
    return code->op1;
}

void translateArgs(pnode  root)
{
    if (!root) 
    {
    	return;
    }
    pInterCode code = newCode(CODE_ARG);
    code->op1 = translateExp(root->child);
    if (root->rule == 58) 
    {
    	translateArgs(root->child->brother->brother);
    }
    addCode(code);
}

void translateCond(pnode root, pOperand label_true, pOperand label_false) 
{
    if (!root)
    {
     	return;
    }
    pInterCode code;
    pOperand label1;
    pOperand temp1, temp2;
    switch (root->rule) {
        case 41:    // Exp -> Exp AND Exp
            label1 = newLabel();
            translateCond(root->child, label1, label_false);
            addLabel(label1);
            translateCond(root->child->brother->brother, label_true, label_false);
            break;
        case 42:    // Exp -> Exp OR Exp
            label1 = newLabel();
            translateCond(root->child, label_true, label1);
            addLabel(label1);
            translateCond(root->child->brother->brother, label_true, label_false);
            break;
        case 43:    // Exp -> Exp RELOP Exp
            temp1 = translateExp(root->child);
            temp2 = translateExp(root->child->brother->brother);
            code = newCode(IF_GT);
            code->op1 = temp1;
            code->op2 = temp2;
            code->op3 = label_true;
            code->relop = newOp(OP_RELOP);
            strcpy(code->relop->str, root->child->brother->c);
            addCode(code);
            addGoto(label_false);
            break;
        case 50:    // Exp -> NOT Exp
            translateCond(root->child->brother, label_false, label_true);
            break;
        default:
            temp1 = translateExp(root);
            code = newCode(IF_GT);
            code->op1 = temp1;
            code->op2 = newOp(OP_CONSTANT);
            code->op2->value = 0;
            code->op3 = label_true;
            code->relop = newOp(OP_RELOP);
            strcpy(code->relop->str, "!=");
            addCode(code);
            addGoto(label_false);
            break;
    }
}

void displayOperand(pOperand op)
{
    if(op == NULL)
    {
    	return;
    }
    else
    {
    	switch(op->kind)
    	{
    		case OP_CONSTANT:	printf("#%d", op->value);break;
    		case OP_TEMP:		printf("t%d", op->var_no);break;
    		case OP_VARIABLE:	printf("%s", op->str);break;
    		case OP_ADDREES:	printf("&%s",op->str);break;
    		case OP_LABEL:	printf("LABEL L%d", op->var_no);break;
    		case OP_FUNC:	printf("FUNCTION %s", op->str);break;
    		case OP_RELOP:	printf("%s", op->str);break;
    		case OP_STAR:	printf("*%s", op->str);break;
    		default:
    			break;
    	}
    }
}

void displayInterCode(pInterCode ic)
{
    switch (ic->kind) 
    {
        case CODE_LABEL :
            displayOperand(ic->op1);
            break;
        case CODE_FUNC :
            displayOperand(ic->op1);
            break;
        case CODE_ASSIGN:
            displayOperand(ic->op1);
            printf(" := ");
            displayOperand(ic->op2);
            break;
        case CODE_ADD:
            displayOperand(ic->op1);
            printf(" := ");
            displayOperand(ic->op2);
            printf(" + ");
            displayOperand(ic->op3);
            break;
        case CODE_SUB:
            displayOperand(ic->op1);
            printf(" := ");
            displayOperand(ic->op2);
            printf(" - ");
            displayOperand(ic->op3);
            break;
        case CODE_MUL:
            displayOperand(ic->op1);
            printf(" := ");
            displayOperand(ic->op2);
            printf(" * ");
            displayOperand(ic->op3);
            break;
        case CODE_DIV:
            displayOperand(ic->op1);
            printf(" := ");
            displayOperand(ic->op2);
            printf(" / ");
            displayOperand(ic->op3);
            break;
        case CODE_ADDR:
            displayOperand(ic->op1);
            printf(" := &");
            displayOperand(ic->op2);
            break;
        case CODE_GOTO:
            printf("GOTO ");
            displayOperand(ic->op1);
            break;
        case IF_GT:
            printf("if ");
            displayOperand(ic->op1);
            printf(" > ");
            displayOperand(ic->op2);
            printf(" GOTO ");
            displayOperand(ic->op3);
            break;
        case CODE_RETURN:
            printf("return ");
            displayOperand(ic->op1);
            break;
        case CODE_DEC:
            break;
        case CODE_ARG:
            printf("Arg ");
            displayOperand(ic->op1);
            break;
        case CODE_CALL:
            displayOperand(ic->op1);
            printf(" := ");
            printf("Call ");
            displayOperand(ic->op2);
            break;
        case CODE_PARAM:
        	printf("Param ");
            displayOperand(ic->op1);
            break;
        case READ:
            printf("Read ");
            displayOperand(ic->op1);
            break;
        case WRITE:
            printf("Write ");
            displayOperand(ic->op1);
            break;
        default:
            break;
    }
}

void displayInterCodes()
{
	if(codeHead==NULL)
	{
		printf("No InterCode!\n");
        return;
	}
	else
	{
		pInterCodes tics = codeHead;
		for(tics=ics; tics!=NULL; tics=tics->next)
		{
			displayInterCode(tics->code);
			printf("\n");
		}
	}
}

void generateLIR(pnode root)
{
	 translate(root);
	 displayInterCodes();
}