/*
*	遍历抽象语法树，建立符号表，并进行语义分析
*	说明：在建立抽象语法树的时候，顺便记录了归约所用的规则，保存在结点的rule属性中，在遍历抽象语法树的
*		 时候，根据结点的rule值，就可以判断是用那条规则归约的，然后就知道该结点的子节点有那些，方便遍历。
*/

#include "Semantic.h"

void Program(pnode p)
{
	assert(p!=NULL);  	//p为空时错误，退出
	ExtDefList(p->child);
	checkFuncDef();    //检测错误18,是否有未定义的函数
}

void ExtDefList(pnode p)
{
	assert(p!=NULL);  //p为空时错误，退出
	if(p->rule == 2)  //遇到规则2，ExtDefList->ExtDef ExtDefList
	{
		ExtDef(p->child);
		ExtDefList(p->child->brother);
	}
	else 			  //遇到规则3, 空规则	
	{
		return;
	}
}

void ExtDef(pnode p)
{
	assert(p!=NULL);  		//p为空时错误，退出
	VarNode* type = Specifier(p->child);  //确定类型	Specifier
	if(p->rule == 4) 		//遇到规则4, ExtDef->Specifier ExtDecList SEMI;	全局变量
	{
		ExtDecList(p->child->brother, type);
	}
	else if(p->rule == 5) 	//遇到规则5, ExtDef->Specifier SEMI;	结构体
	{
		return;
	}
	else					//n指向FunDec，进入规则：Specifier FunDec CompSt或Specifier FunDec SEMI
	{  
		IntoScope();  //进入一个作用域，Scope+1
		FuncNode* f = FunDec(p->child->brother, type);//type传入返回类型
		if(p->rule == 6) 		//遇到规则6, ExtDef->Specifier FunDec CompSt	表示此时为函数定义，将Def_flag置为1
		{
			f->Def_flag = TRUE;
			CompSt(p->child->brother->brother, type);
		}
		else if(p->rule == 7)	//遇到规则6, ExtDef->Specifier FunDec SEMI 表示此时为函数声明，未定义，将Def_flag置为0				
		{
			f->Def_flag = FALSE;
		}
		OutScope();  //退出一个作用域 Scope-1
		int flag = insertFuncNode(f);
		switch(flag){
			case REDEFINE_ERROR:  					//错误类型4：重复定义
				printError(f->lineno,f->name,4);
				break;
			case INCONSISTENT_DELARATION_ERROR:  	//错误类型19：函数声明冲突，或者声明与定义之间互相冲突
				printError(f->lineno,f->name,19);
				break;
		}
	}
}

void ExtDecList(pnode p, VarNode* var)
{
	assert(p!=NULL);  //p为空时错误，退出
	VarDec(p->child, var, EXT_VAR);	
	if(p->rule == 9)					//遇到规则9, ExtDecList->VarDec COMMA ExtDecList
	{
		VarNode* varc = (VarNode*)malloc(sizeof(VarNode));
		memcpy(varc, var, sizeof(VarNode));
		ExtDecList(p->child->brother->brother, varc);
	}
}

VarNode* Specifier(pnode p)
{
	assert(p!=NULL);  	//p为空时错误，退出
	VarNode* type = NULL;
	if(p->rule == 10)	//遇到规则10, Specifier->TYPE 变量类型
	{ 
		type = (VarNode*)malloc(sizeof(VarNode));
		memset(type, 0, sizeof(VarNode));
		type->kind = BASIC_T;	//int或者float
		if(p->child->kind==TYPE_INT)  		//int
		{
			type->detail.basic = 284;
		}
		else if(p->child->kind==TYPE_FLOAT)  //float
		{
			type->detail.basic = 285;
		}
	}
	else				//遇到规则11, Specifier->StructSpecifier 结构体类型
	{  
		type = StructSpecifier(p->child);
	}
	return type;
}

VarNode* StructSpecifier(pnode p)
{
	assert(p!=NULL);  //p为空时错误，退出
	StrucNode* struc=NULL;
	VarNode* var=NULL;
	if(p->rule == 12)	//遇到规则12, StructSpecifier->STRUCT OptTag LC DefList RC 结构体定义
	{  
		var = (VarNode*)malloc(sizeof(VarNode));
		memset(var, 0, sizeof(VarNode));		
		struc = (StrucNode*)malloc(sizeof(StrucNode));
		memset(struc, 0, sizeof(StrucNode));
		var->detail.struc = struc;	//保存结构体中变量
		var->kind = STRUCT_T;
		if(p->child->brother->child!=NULL)			//OptTag->sons为ID类型	
		{
			strcpy(struc->name, p->child->brother->child->c);//保存结构体名
		}
		if(insertStrucNode(struc)==REDEFINE_ERROR)	//错误类型16：结构体名与之前的结构体名或变量名重复
		{  
			printError(p->lineno, struc->name, 16);
			return var;
		}
		IntoScope();
		struc->VarInStruc = DefList(p->child->brother->brother->brother, STRUCT_VAR);  //指向结构体的作用域
		OutScope();
	}
	else 				//遇到规则13, StructSpecifier->STRUCT Tag 结构体声明
	{  
		struc = findStrucNode(p->child->brother->child->c);//查找结构体 
		if(struc==NULL)//未找到已定义的该结构体，错误类型17：直接使用未定义过的结构体来定义变量
		{  
			printError(p->lineno,p->child->brother->child->c,17);
			return var;
		}
		var = (VarNode*)malloc(sizeof(VarNode));
		memset(var, 0, sizeof(VarNode));
		var->detail.struc = struc;//保存结构体中变量
		var->kind = STRUCT_T;
	}
	return var;
}

VarNode* VarDec(pnode p, VarNode* var, int flag)
{
	assert(p!=NULL);  //p为空时错误，退出
	if(var == NULL)
	{
		return NULL;
	}
	if(p->rule == 17) //遇到规则17, VarDec->ID 变量定义
	{ 
		strcpy(var->name, p->child->c);				//保存变量名
		if(insertVarNode(var) == REDEFINE_ERROR )	//重复定义
		{
			if(flag!=STRUCT_VAR)//错误类型3：变量重复定义
			{
				printError(p->lineno,var->name,3);
			}
			else  				//错误类型15：同一结构体中变量重复定义
			{
				printError(p->lineno,var->name,15);
			}
			return NULL;
		}
		else
		{
			return var;
		}
	}
	else 			//遇到规则18, VarDec->VarDec LB INT RB 定义数组
	{  
		VarNode* array = (VarNode*)malloc(sizeof(VarNode));
		memcpy(array, var, sizeof(VarNode));
		array->kind = ARRAY_T;
		array->detail.array.type = var;					//保存数组类型
		array->detail.array.size = p->child->brother->brother->i;		//保存数组大小
		return VarDec(p->child, array, flag);			//递归调用VarDec，获取数组名，添加数组结点
	}
}

FuncNode* FunDec(pnode p, VarNode* retType)
{
	assert(p!=NULL);  	//p为空时错误，退出
	FuncNode* func=(FuncNode*)malloc(sizeof(FuncNode));
	memset(func, 0, sizeof(FuncNode));
	strcpy(func->name, p->child->c);
	func->lineno = p->child->lineno;
	func->retType = retType;
	if(p->rule == 19)	//遇到规则19, FunDec->ID LP VarList RP 函数定义
	{	
		func->paraList = VarList(p->child->brother->brother);//获取函数参数信息
	}
	return func;
}

VarNode*  VarList(pnode p)
{
	assert(p!=NULL);  	//p为空时错误，退出
	VarNode* varlist = ParamDec(p->child);//第一个参数作为参数链的头结点
	if(p->rule == 21)	//遇到规则19, VarList->ParamDec COMMA VarList 多参数	
	{ 	
		VarNode* q=varlist;
		if(q!=NULL)
		{
			while(q->nextInList!=NULL)
				q = q->nextInList;
			q->nextInList = VarList(p->child->brother->brother);
		}
		else
		{
			varlist = VarList(p->child->brother->brother);
		}
	}
	return varlist;		//返回参数链
}

VarNode* ParamDec(pnode p)
{
	assert(p!=NULL);  //p为空时错误，退出
	VarNode* type = Specifier(p->child);//获取类型
	VarNode* para = VarDec(p->child->brother, type, PARAM_VAR);//VARDEC_IN_PARAM函数参数作用域
	return para;
}

void CompSt(pnode p, VarNode* retType)
{
	assert(p!=NULL);  //p为空时错误，退出
	DefList(p->child->brother, LOCAL_VAR);
	StmtList(p->child->brother->brother, retType);
}

void StmtList(pnode p, VarNode* retType)
{
	assert(p!=NULL);  //p为空时错误，退出
	if(p->rule == 25)	//遇到规则25, StmtList->Stmt StmtList	
	{
		Stmt(p->child, retType);
		StmtList(p->child->brother, retType);
	}
	else 				//遇到规则26, 空规则
	{
		return;
	}
}

void Stmt(pnode p, VarNode* retType)
{
	assert(p!=NULL);  	//p为空时错误，退出
	if(p->rule == 27)	//遇到规则27, Stmt->Exp SEMI
	{
		Exp(p->child);
	}
	else if(p->rule == 28)	//遇到规则28, Stmt->CompSt
	{ 
		IntoScope();  	//进入作用域，Scope+1
		CompSt(p->child, retType);
		OutScope();   	//退出作用域，Scope-1
	}
	else if(p->rule == 29)	//遇到规则29, Stmt->RETURN Exp SEMI 返回语句
	{ 
		VarNode* ret = Exp(p->child->brother);
		if( retType==NULL && ret==NULL )
			return;
		else if( retType==NULL || ret==NULL || VarNodeCmp(retType, ret)==FALSE) //错误类型8：return语句的返回类型与函数定义类型不一致
		{  
			printError(p->lineno,"null",8);
		}
		return;
	}
	else if(p->rule == 32)	//遇到规则32, Stmt->WHILE LP Exp RP Stmt 循环体
	{ 
		VarNode* whileExp = Exp(p->child->brother->brother);
		Stmt(p->child->brother->brother->brother->brother, retType);
	}
	else  					//遇到规则30或规则31
	{  
		VarNode* whileExp = Exp(p->child->brother->brother);
		Stmt(p->child->brother->brother->brother->brother, retType);
		if(p->rule == 31)	//遇到规则31, Stmt->IF LP Exp RP Stmt ELSE Stmt
		{
			Stmt(p->child->brother->brother->brother->brother->brother->brother, retType);
		}
	}
}

VarNode* DefList(pnode p, int flag )
{
	assert(p!=NULL);  		//p为空时错误，退出
	if(p->rule == 34)		//遇到规则34, 空规则
	{
		return NULL;
	}
	VarNode* var = Def(p->child, flag);
	VarNode* varlist = DefList(p->child->brother, flag);
	VarNode* q = var;
	if(q!=NULL)
	{
		while(q->nextInList!=NULL)  //仅保存第一个节点，后续用链表链接
			q = q->nextInList;  	//指向后续DecList
		q->nextInList = varlist;	//将声明的其他变量链接到链表末尾
	}
	else
	{
		var = varlist;
	}
	return var;
}

VarNode* Def(pnode p, int flag)
{
	assert(p!=NULL);  //p为空时错误，退出
	VarNode * type = Specifier(p->child);
	return DecList(p->child->brother, type, flag);
}

VarNode* DecList(pnode p, VarNode* type, int flag)
{
	assert(p!=NULL);  //p为空时错误，退出
	VarNode* var = NULL;
	VarNode* varlist = NULL;
	if(p->rule == 36)	//遇到规则36, DecList->Dec 
	{
		var = Dec(p->child, type, flag);
	}
	else 				//遇到规则37, DecList->Dec COMMA DecList 
	{
		VarNode* typeC = (VarNode*)malloc(sizeof(VarNode));
		memcpy(typeC, type, sizeof(VarNode));
		var = Dec(p->child, type, flag);
		varlist = DecList(p->child->brother->brother, typeC, flag);//保存连续声明的变量
	}
	if(var==NULL)
	{
		var = varlist;
	}
	else
	{
		var->nextInList = varlist;
	}
	return var;//返回当前已保存变量链表
}

VarNode* Dec(pnode p, VarNode* type, int flag)
{
	assert(p!=NULL);  //p为空时错误，退出
	VarNode* var = VarDec(p->child, type, flag);
	if(p->rule == 39) //遇到规则39, DecList->VarDec ASSIGNOP Exp 声明并赋值
	{
		if(flag==STRUCT_VAR)	//错误类型15：同一结构体中域名重复定义或或在定义时对域名初始化
		{  
			printError(p->lineno, var->name, 15);
			return var;
		}
		VarNode* varType = Exp(p->child->brother->brother);	//保存表达式，并获取其类型
		if(var!=NULL && varType!=NULL)
		{  
			if(VarNodeCmp(var, varType)==FALSE)				//错误类型5：赋值号两边的表达式类型不匹配
			{
				printError(p->child->lineno,"null",5);
			}
		}
	}
	return var;
}

VarNode* Exp(pnode p)
{
	assert(p!=NULL);  //p为空时错误，退出
	VarNode* retType=NULL;
	if(strcmp(p->child->c,"Exp")==0)
	{
		if(p->rule == 40)   //遇到规则40, Exp->Exp ASSIGNOP Exp
		{
			VarNode* leftType = NULL;
			if(p->child->child->kind==ID&&p->child->child->brother==NULL) 	//等式左边为变量名ID 进入规则：ID；
			{
				leftType = Exp(p->child);//递归分析表达式，获取左边类型
			}
			else if(strcmp(p->child->child->c,"Exp")==0)
			{
				if(strcmp(p->child->child->brother->c, "LB")==0||strcmp(p->child->child->brother->c, "DOT")==0)
				{
					leftType = Exp(p->child);//递归分析表达式，获取左边类型
				}
			}
			else 		//错误类型6：赋值号左边为右值
			{  
				printError(p->child->lineno,"null",6);
			}
			VarNode* rightType = Exp(p->child->brother->brother);//获取右边类型
			if(leftType==NULL||rightType==NULL)
			{
				return NULL;
			}
			if(VarNodeCmp(leftType, rightType)==TRUE)	//赋值类型匹配，返回最终类型
			{
				return leftType;
			}
			else 		//错误类型5：赋值号两边的表达式类型不匹配
			{  
				printError(p->lineno,"null",5);
				return NULL;
			}
		}
		else if(p->rule == 41 || p->rule == 42 || p->rule == 43 || p->rule == 44 
			|| p->rule == 45 || p->rule == 46 || p->rule == 47) //当前表达式为运算表达式，即右值表达式；
		{
			VarNode* leftType = Exp(p->child);//获取运算符左边类型
			VarNode* rightType = Exp(p->child->brother->brother);//获取运算符右边类型
			if(leftType==NULL || rightType==NULL)
			{
				return NULL;
			}
			if(leftType->kind==BASIC_T && rightType->kind==BASIC_T 
				&& leftType->detail.basic==rightType->detail.basic)
			{
				return leftType;
			}
			else 		//错误类型7：类型不匹配
			{  
				printError(p->lineno,"null",7);
				return NULL;
			}
		}
		else if(p->rule == 53)		 //遇到规则53, Exp->Exp LB Exp RB 数组
		{
			VarNode* arraySym = Exp(p->child);
			if(arraySym==NULL)
			{
				return NULL;
			}
			if(arraySym->kind!=ARRAY_T)		//错误类型10：对非数组型变量使用数组操作符
			{  
				printError(p->lineno,arraySym->name,10);
				return NULL;
			}
			VarNode* arraySize = Exp(p->child->brother->brother);
			if(arraySize==NULL)
			{
				return NULL;
			}
			if(arraySize->kind!=BASIC_T || arraySize->detail.basic!=284)//错误类型12：数组访问操作符中出现非整数
			{  
				printf("Error type 12 at line %d: \"%f\" is not an integer.\n", p->lineno,arraySize->float_val);
				return NULL;
			}
			return arraySym->detail.array.type;//返回数组元素类型
		}
		else if(p->rule == 54)	//遇到规则54, Exp->Exp DOT ID 对结构体的内部变量操作
		{
			VarNode* strucSym = Exp(p->child);
			if(strucSym==NULL)
			{
				return NULL;
			}
			if(strucSym->kind!=STRUCT_T) //错误类型13：对非结构体变量用.操作符
			{  
				printError(p->lineno,"null",13);
				return NULL;
			}
			VarNode* strucVar= strucSym->detail.struc->VarInStruc;
			while(strucVar!=NULL)
			{
				if(!strcmp(strucVar->name,p->child->brother->brother->c)) //若结构体中已定义该变量，则返回该变量信息
				{
					return strucVar;
				}
				strucVar = strucVar->nextInList;
			}
			//错误类型14：访问结构体中未定义过的域
			printError(p->lineno, p->child->brother->brother->c, 14);
			return NULL;
		}
	}
	else if(p->rule == 48) //遇到规则48, Exp->LP Exp RP
	{
		return Exp(p->child->brother);
	}
	else if(p->rule == 49) //遇到规则49, Exp->MINUS Exp 负数表达式
	{
		VarNode* right = Exp(p->child->brother);
		if(right==NULL)
		{
			return NULL;
		}
		if(right->kind!=BASIC_T) //错误类型7：类型不匹配，只能为int或者float
		{  
			printError(p->lineno,"null",7);
			return NULL;
		}
		return right;
	}
	else if(p->rule == 50) //遇到规则49, Exp->NOT Exp 取非运算
	{
		VarNode* right = Exp(p->child->brother);
		if(right==NULL)
		{
			return NULL;
		}
		if(right->kind!=BASIC_T|| right->detail.basic!=284) //错误类型7：类型不匹配，只能为int
		{  
			printError(p->lineno,"null",7);
			return NULL;
		}
		return right;
	}
	else if(p->child->kind==284) //Exp为整数类型
	{
		VarNode* type = (VarNode*)malloc(sizeof(VarNode));
		memset(type, 0, sizeof(VarNode));
		type->kind = BASIC_T;
		type->detail.basic = 284;
		type->int_val = p->child->i;
		return type;
	}
	else if(p->child->kind==285) //Exp为浮点类型
	{
		VarNode* type = (VarNode*)malloc(sizeof(VarNode));
		memset(type, 0, sizeof(VarNode));
		type->kind = BASIC_T;
		type->detail.basic = 285;
		type->float_val = p->child->f;
		return type;
	}
	else if(p->child->kind==ID) //Exp为标识符
	{
		if(p->child->brother!=NULL)
		{
			VarNode* var = findVarNode(p->child->c);
			FuncNode* func = findFuncNode(p->child->c);
			if(func==NULL&&var!=NULL) //该符号为变量，错误类型11：对普通变量用函数操作符
			{  
				printError(p->lineno,p->child->c,11);
				return NULL;
			}
			else if(func==NULL||func->Def_flag==FALSE)
			{
				printError(p->lineno, p->child->c, 2);
				return NULL;
			}
			VarNode* paraList = func->paraList;
			if(p->child->brother->brother->brother==NULL)
			{
				if(func->paraList!=NULL)
				{
					printf( "Error type 9 at line %d: Function \"%s(", p->child->brother->brother->lineno, func->name);
					printTypeList(func->paraList);
					printf( ")\" is not applicable for arguments \"()\".\n" );
				}
			}
			else
			{
				if(Args(p->child->brother->brother,func->paraList)==FALSE) //错误类型9：函数调用实参与形参类型不匹配
				{
					printf( "Error type 9 at line %d: Function \"%s(", p->child->brother->brother->lineno, func->name);
					printTypeList(func->paraList);
					printf( ")\" is not applicable for arguments \"(");
					printArgs(p->child->brother->brother->child);
					printf( ")\".\n" );
				}
			}
			return func->retType;
		}
		else
		{
			VarNode* sym = findVarNode(p->child->c);
			if(sym==NULL)
			{  //错误类型1：变量使用时未定义
				printError(p->lineno, p->child->c, 1);
				return NULL;
			}
			return sym;
		}
	}
	return retType;
}

int Args(pnode p, VarNode* paraList)
{
	assert(p!=NULL);
	if(paraList==NULL)
	{
		return FALSE;
	}
	if(Exp(p->child)==NULL)
	{
		return TRUE;
	}
	if(VarNodeCmp(Exp(p->child), paraList)==FALSE )//形参和实参类型不匹配
	{
		return FALSE;
	}
	if(p->child->brother==NULL && paraList->nextInList==NULL)//比较完毕，返回true
	{
		return TRUE;
	}
	else if(p->child->brother==NULL || paraList->nextInList==NULL)//形参和实参数目不匹配
	{
		return FALSE;
	}
	return Args(p->child->brother->brother, paraList->nextInList );//递归比较下一个形参和实参
}

void printArgs(pnode p)
{//用于报错时打印函数详细信息
	if(p==NULL)
	{
		return;
	}
	printType(Exp(p));
	if(p->brother==NULL)
	{
		return;
	}
	else
	{
		printf(", ");
	}
	printArgs(p->brother->brother->child);
}