/*
*   遍历抽象语法树生成中间代码，入口函数是generateLIR
*   说明：在建立抽象语法树的时候，顺便记录了归约所用的规则，保存在结点的rule属性中，在遍历抽象语法树的
*        时候，根据结点的rule值，就可以判断是用那条规则归约的，然后就知道该结点的子节点有那些，方便遍历。
*/

#include "Semantic.c"

//操作类型
enum op_kind { 
	OP_VARIABLE, OP_TEMP, OP_CONSTANT, 
	OP_LABEL, OP_FUNC, OP_RELOP,
   	OP_SIZE, OP_ADDR, OP_STAR};

//代码类型
enum ic_kind { 
	CODE_LABEL, CODE_FUNC, CODE_ASSIGN, 
	CODE_ADD, CODE_SUB, CODE_MUL, CODE_DIV,
   	CODE_ADDR, CODE_GOTO, CODE_IFGT, 
   	CODE_RETURN, CODE_DEC, CODE_ARG, CODE_CALL,
   	CODE_PARAM, CODE_READ, CODE_WRITE};

typedef struct Vari Vari,*pVari;    //变量数据结构
typedef struct Temps Temps,*pTemps; //临时变量数据结构
typedef struct Operand Operand,*pOperand;   //操作数数据结构
typedef struct InterCode InterCode,*pInterCode; //单条中间代码数据结构
typedef struct InterCodes InterCodes,*pInterCodes; //中间代码数据结构

struct Vari
{
    int vari_no;    //变量的编号
    char str[32];   //变量的名字
    pVari next;     //指向下一个变量
};

struct Temps
{
    int temp_no;    //临时变量的编号
    pTemps next;    //指向下一个临时变量
};

struct Operand
{
	enum op_kind kind; //操作的类型
    int val_no;        //操作的编号
	union 
	{
		int value;     //保存操作数的整形值
		char str[32];  //保存操作数的字符串
	};
};

struct InterCode
{
	enum ic_kind kind; //中间代码的类型
	pOperand op1,op2,op3,relop; //操作数
};

struct InterCodes
{
	pInterCode code;   //中间代码
	pInterCodes prev,next; //指向下一条中间代码
};

int vari_no = 0;       //记录变量的编号
int temp_no = 0;       //记录临时变量的编号
int cons_no = 0;       //记录常量的编号
int label_no = 0;      //记录标签的编号
char func_name[32];    //记录函数的名字
pTemps tempsHead = NULL;    //保存临时变量的链表的头指针
pVari varsHead = NULL;      //保存变量的链表的头指针
pInterCodes codeHead = NULL;//保存中间代码的链表的头指针
pInterCodes codeLast = NULL;//保存中间代码的链表的尾指针

pOperand newTemp();         //创建一个临时变量
pOperand newLabel();        //创建一个标签
pOperand newOp(enum op_kind);//创建一个操作数
pInterCode newCode(enum ic_kind);//创建一个中间代码

void addCode(pInterCode);   //添加一条中间代码
void addLabel(pOperand);    //添加一个标签
void addGoto(pOperand);     //添加一个跳转语句
void addVar(pOperand);      //添加一个变量

void parse(pnode);          //遍历抽象语法树，一次根据不同结点选择不同的遍历函数，生成对应的中间代码
pOperand parseVarDec(pnode);
void parseFunDec(pnode);    
void parseVarList(pnode);   
void parseCompSt(pnode);    
void parseDefList(pnode);
void parseDecList(pnode);
void parseDec(pnode);
void parseStmtList(pnode);
pOperand parseExp(pnode);
void parseStmt(pnode);
void parseCond(pnode, pOperand, pOperand);
void parseArgs(pnode);
void parseR_AND_W();

void optimize();        //中间代码的优化

void displayOperand(pOperand); //输出生成的中间代码
void displayInterCode(pInterCode);
void displayInterCodes();

void generateLIR(pnode root);   //中间代码生成的入口函数

/*创建一个临时变量*/
pOperand newTemp()
{
	temp_no++;
	pOperand temp = (pOperand)malloc(sizeof(Operand));
	temp->kind = OP_TEMP;
	temp->val_no = temp_no;
    pTemps tp = (pTemps)malloc(sizeof(Temps));
    tp->temp_no = temp_no;
    tp->next = NULL;
    if(tempsHead == NULL)
    {
        tempsHead = tp;
    }
    else
    {
        pTemps p = tempsHead;
        while(p->next!=NULL)
        {
            p = p->next;
        }
        p->next = tp;
    }
	return temp;
}

/*创建一个标签*/
pOperand newLabel()
{
	label_no++;
    pOperand label = (pOperand)malloc(sizeof(Operand));
    label->kind = OP_LABEL;
    label->val_no = label_no;
    return label;
}

/*创建一个操作数*/
pOperand newOp(enum op_kind kind) 
{
    pOperand op = (pOperand)malloc(sizeof(Operand));
    if(kind == OP_CONSTANT)
    {
        cons_no++;
        op->val_no = cons_no;
    }
    op->kind = kind;
    op->value = 0;
    return op;
}

/*创建一条中间代码*/
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

/*添加一条中间代码*/
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

/*添加一个标签*/
void addLabel(pOperand label) 
{
    pInterCode code = newCode(CODE_LABEL);
    code->op1 = label;
    addCode(code);
}

/*添加一条转移语句*/
void addGoto(pOperand label) 
{
    pInterCode code = newCode(CODE_GOTO);
    code->op1 = label;
    addCode(code);
}

/*添加一个变量*/
void addVar(pOperand op) 
{
    for(pVari p=varsHead; p!=NULL; p=p->next)
    {
        if(strcmp(p->str,op->str)==0)
        {
            op->val_no = p->vari_no;
            return;
        }
    }
    vari_no++;
    op->val_no = vari_no;
    pVari v = (pVari)malloc(sizeof(Vari));
    v->vari_no = vari_no;
    v->next = NULL;
    strcpy(v->str, op->str);
    if(varsHead == NULL)
    {
        varsHead = v;
    }
    else
    {
        pVari p = varsHead;
        while(p->next!=NULL)
        {
            p=p->next;
        }
        p->next = v;
    }
}

/*遍历抽象语法树的根结点*/
void parse(pnode root)
{
    if(root == NULL) return;
    if(root->rule == 6) 
    {
        parseFunDec(root->child->brother);
        parseCompSt(root->child->brother->brother);
    }
    else
    {
    	pnode node = root->child;
    	for (; node!=NULL; node = node->brother)
        	parse(node);
    }
}

/*遍历FunDec结点*/
void parseFunDec(pnode  root) 
{
    if (root==NULL) 
    {
    	return;
    }
    pInterCode code = newCode(CODE_FUNC);
    code->op1 = newOp(OP_FUNC);
    strcpy(code->op1->str,root->child->c);
    strcpy(func_name,root->child->c);
    addCode(code);
    if (root->rule == 19)  
    { 
        parseVarList(root->child->brother->brother);
    }
}

/*遍历CompSt结点*/
void parseCompSt(pnode  root) 
{
    if(root==NULL) 
    {
    	return;
    }
    parseDefList(root->child->brother);
    parseStmtList(root->child->brother->brother);
}

/*遍历VarList结点*/
void parseVarList(pnode  root) 
{
    if(root==NULL) 
    {
    	return;
    }
    pInterCode code = newCode(CODE_PARAM);
    code->op1 = newOp(OP_VARIABLE);
    strcpy(code->op1->str,root->child->child->brother->child->c);
    addVar(code->op1);
    addCode(code);
    if (root->rule == 21)
    {
        parseVarList(root->child->brother->brother);
    }
}

/*遍历DefList结点*/
void parseDefList(pnode root) 
{
    if(!root || root->rule == 34) 
    {
    	return;
    }
    parseDecList(root->child->child->brother);
    parseDefList(root->child->brother);
}

/*遍历DecList结点*/
void parseDecList(pnode root) 
{
    if (root==NULL) 
    {
    	return;
    }
    parseDec(root->child);
    if (root->rule == 37) 
    {
    	parseDecList(root->child->brother->brother);
    }
}

/*遍历StmtList结点*/
void parseStmtList(pnode  root) 
{
    if (!root || root->rule == 26) 
    {
    	return;
    }
    parseStmt(root->child);
    parseStmtList(root->child->brother);
}

/*遍历Stmt结点*/
void parseStmt(pnode  root) 
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
            parseExp(root->child);
            break;
        case 28:    // Stmt -> CompSt
            parseCompSt(root->child);
            break;
        case 29:    // Stmt -> RETURN Exp SEMI
            code = newCode(CODE_RETURN);
            code->op1 = parseExp(root->child->brother);
            addCode(code);
            break;
        case 30:    // Stmt -> IF LP Exp RP Stmt
            label1 = newLabel();
            label2 = newLabel();
            parseCond(root->child->brother->brother, label1, label2);
            addLabel(label1);
            parseStmt(root->child->brother->brother->brother->brother);
            addLabel(label2);
            break;
        case 31:    // Stmt -> IF LP Exp RP Stmt ELSE Stmt
            label1 = newLabel();
            label2 = newLabel();
            label3 = newLabel();
            parseCond(root->child->brother->brother, label1, label2);
            addLabel(label1);
            parseStmt(root->child->brother->brother->brother->brother);
            addGoto(label3);
            addLabel(label2);
            parseStmt(root->child->brother->brother->brother->brother->brother->brother);
            addLabel(label3);
            break;
        case 32:    // Stmt -> WHILE LP Exp RP Stmt
            label1 = newLabel();
            label2 = newLabel();
            label3 = newLabel();
            addLabel(label1);
            parseCond(root->child->brother->brother, label2, label3);
            addLabel(label2);
            parseStmt(root->child->brother->brother->brother->brother);
            addGoto(label1);
            addLabel(label3);
            break;
    }
}

/*遍历Dec结点*/
void parseDec(pnode root) 
{
    if(!root)
    {
     	return;
    }
    if (root->rule == 38) 
    {
        parseVarDec(root->child);
    } 
    else if (root->rule == 39) 
    {
        pInterCode code = newCode(CODE_ASSIGN);
        code->op1 = parseVarDec(root->child);
        code->op2 = parseExp(root->child->brother->brother);
        addCode(code);
    }
}

/*遍历VarDec结点*/
pOperand parseVarDec(pnode root) 
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
        addVar(op);
    } 
    else if (root->rule == 18) 
    {
        code = newCode(CODE_DEC);
        code->op1 = parseVarDec(root->child);
        op = newOp(OP_SIZE);
        op->value = root->child->brother->brother->i;
        code->op2 = op;
        addCode(code);
    }
    return op;
}

/*遍历Exp结点*/
pOperand parseExp(pnode  root) 
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
            code->op1 = parseExp(root->child);
            code->op2 = parseExp(root->child->brother->brother);
            break;
        case 44:    // Exp -> Exp PLUS Exp
            code = newCode(CODE_ADD);
            code->op2 = parseExp(root->child);
            code->op3 = parseExp(root->child->brother->brother);
            code->op1 = newTemp();
            break;
        case 45:    // Exp -> Exp MINUS Exp
            code = newCode(CODE_SUB);
            code->op2 = parseExp(root->child);
            code->op3 = parseExp(root->child->brother->brother);
            code->op1 = newTemp();
            break;
        case 46:    // Exp -> Exp STAR Exp
            code = newCode(CODE_MUL);
            code->op2 = parseExp(root->child);
            code->op3 = parseExp(root->child->brother->brother);
            code->op1 = newTemp();
            break;
        case 47:    // Exp -> Exp DIV Exp
            code = newCode(CODE_DIV);
            code->op2 = parseExp(root->child);
            code->op3 = parseExp(root->child->brother->brother);
            code->op1 = newTemp();
            break;
        case 48:    // Exp -> LP Exp RP
            return parseExp(root->child->brother);
            break;
        case 49:    // Exp -> MINUS Exp
            code = newCode(CODE_SUB);
            code->op2 = newOp(OP_CONSTANT);
            code->op2->value = 0;
            code->op3 = parseExp(root->child->brother);
            code->op1 = newTemp();
            break;
        case 51:    // Exp -> ID LP Args RP
            code = newCode(CODE_CALL);
            parseArgs(root->child->brother->brother);
            code->op2 = newOp(OP_FUNC);
            strcpy(code->op2->str, root->child->c);
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
            code->op2 = parseExp(root->child->brother->brother);
            code->op3 = newOp(OP_CONSTANT);
            code->op3->value = 4;
            op = newTemp();
            code->op1 = op;
            addCode(code);
            code = newCode(CODE_ADD);
            code->op3 = op;
            op = newOp(OP_ADDR);
            strcpy(op->str, parseExp(root->child)->str);
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
            addVar(op);
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

/*遍历Args结点*/
void parseArgs(pnode  root)
{
    if (!root) 
    {
    	return;
    }
    pInterCode code = newCode(CODE_ARG);
    code->op1 = parseExp(root->child);
    if (root->rule == 58) 
    {
    	parseArgs(root->child->brother->brother);
    }
    addCode(code);
}

/*遍历Cond结点*/
void parseCond(pnode root, pOperand label_true, pOperand label_false) 
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
            parseCond(root->child, label1, label_false);
            addLabel(label1);
            parseCond(root->child->brother->brother, label_true, label_false);
            break;
        case 42:    // Exp -> Exp OR Exp
            label1 = newLabel();
            parseCond(root->child, label_true, label1);
            addLabel(label1);
            parseCond(root->child->brother->brother, label_true, label_false);
            break;
        case 43:    // Exp -> Exp RELOP Exp
            temp1 = parseExp(root->child);
            temp2 = parseExp(root->child->brother->brother);
            code = newCode(CODE_IFGT);
            code->op1 = temp1;
            code->op2 = temp2;
            code->op3 = label_true;
            code->relop = newOp(OP_RELOP);
            strcpy(code->relop->str, root->child->brother->c);
            addCode(code);
            addGoto(label_false);
            break;
        case 50:    // Exp -> NOT Exp
            parseCond(root->child->brother, label_false, label_true);
            break;
        default:
            temp1 = parseExp(root);
            code = newCode(CODE_IFGT);
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

/*翻译read和write*/
void parseR_AND_W()
{
    pInterCodes ics = codeHead;
    for(; ics!=NULL; ics=ics->next)
    {
        if(ics->code->kind == CODE_CALL)
        {
            if(strcmp(ics->code->op2->str,"read")==0)
            {
                ics->code->kind = CODE_READ;
                if(ics->next->code->op3==NULL)
                {
                    ics->code->op1 = ics->next->code->op1;
                    ics->next->next->prev = ics;
                    ics->next = ics->next->next;
                }
            }
            else if(strcmp(ics->code->op2->str,"write")==0)
            {
                ics->code->kind = CODE_WRITE;
                ics->code->op1 = ics->prev->code->op1;
                ics->prev->prev->next = ics;
                ics->prev = ics->prev->prev;
            }
        }
    }
}

/*中间代码的优化*/
void optimize()
{
    pInterCodes ics = codeHead;
    if(ics==NULL)
    {
        return;
    }
    for(; ics->next!=NULL; ics=ics->next)
    {
        switch(ics->code->kind)
        {
            case CODE_RETURN:
                if(ics->next->code->kind==CODE_GOTO)
                {
                    ics->next->next->prev = ics;
                    ics->next = ics->next->next;
                }
                break;
            case CODE_ADD:
                if(ics->next->code->kind==CODE_ASSIGN &&
                    ics->code->op1->kind==ics->next->code->op2->kind &&
                    ics->code->op1->val_no==ics->next->code->op2->val_no)
                {
                    ics->code->op1 = ics->next->code->op1;
                    ics->next->next->prev = ics;
                    ics->next = ics->next->next;
                }
                break;
        }
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
    		case OP_TEMP:		printf("t%d", op->val_no);break;
    		case OP_VARIABLE:	printf("v%d", op->val_no);break;
    		case OP_ADDR:	printf("&%s",op->str);break;
    		case OP_LABEL:	printf("label%d", op->val_no);break;
    		case OP_FUNC:	printf("%s", op->str);break;
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
            printf("LABEL ");
            displayOperand(ic->op1);
            printf(" :");
            break;
        case CODE_FUNC :
            printf("FUNCTION ");
            displayOperand(ic->op1);
            printf(" :");
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
        case CODE_IFGT:
            printf("IF ");
            displayOperand(ic->op1);
            printf(" %s ", ic->relop->str);
            displayOperand(ic->op2);
            printf(" GOTO ");
            displayOperand(ic->op3);
            break;
        case CODE_RETURN:
            printf("RETURN ");
            displayOperand(ic->op1);
            break;
        case CODE_DEC:
            printf("DEC ");
            displayOperand(ic->op1);
            putchar(' ');
            displayOperand(ic->op2);
            break;
        case CODE_ARG:
            printf("ARG ");
            displayOperand(ic->op1);
            break;
        case CODE_CALL:
            displayOperand(ic->op1);
            printf(" := ");
            printf("CALL ");
            displayOperand(ic->op2);
            break;
        case CODE_PARAM:
        	printf("PARAM ");
            displayOperand(ic->op1);
            break;
        case CODE_READ:
            printf("READ ");
            displayOperand(ic->op1);
            break;
        case CODE_WRITE:
            printf("WRITE ");
            displayOperand(ic->op1);
            break;
        default:
            break;
    }
}

void displayInterCodes()
{
    parseR_AND_W();
	if(codeHead==NULL)
	{
		printf("No InterCode!\n");
        return;
	}
	else
	{
        int i = 1;
		pInterCodes tics = NULL;
		for(tics=codeHead; tics!=NULL; tics=tics->next)
		{
            printf("%.2d ",i++);
			displayInterCode(tics->code);
			printf("\n");
		}
	}
}

/*入口函数，分别调用解释函数，优化函数以及中间代码输出函数*/
void generateLIR(pnode root)
{
	 parse(root);
     optimize();
	 displayInterCodes();
}