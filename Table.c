#include "Table.h"

/*初始化：符号表以及作用域数组申请空间*/
void initTable(){
	varHead = NULL;
	strucHead = NULL;
	funcHead = NULL;
	Scope = 0;  //全局变量的作用域值为0
	memset(varScope, 0, 100*sizeof(VarNode*));//申请变量作用域数组空间
	memset(strucScope, 0, 100*sizeof(StrucNode*));//申请结构体作用域数组空间
}

/*插入变量数组结点*/
int insertVarNode(VarNode* p)
{
	if( p->name == NULL )
		return -1;
	p->Scope = Scope;
	for(VarNode* q=varHead; q!=NULL; q=q->next)
	{
		if(strcmp(p->name,q->name)==0&&p->Scope==q->Scope)
			return REDEFINE_ERROR;
	}
	if(varHead != NULL)
	{
		p->next = varHead;
		varHead->pre = p;
	}
	varHead= p;
	p->nextInScope = varScope[Scope];
	varScope[Scope] = p;
	return 0;
}

/*插入结构体结点*/
int insertStrucNode(StrucNode* p){
	if( p->name == NULL )
		return -1;
	p->Scope = Scope;
	for(StrucNode* q=strucHead; q!=NULL; q=q->next)
	{
		if(strcmp(p->name,q->name)==0&&p->Scope==q->Scope)
			return REDEFINE_ERROR;
	}
	if(strucHead != NULL)
	{
		p->next = strucHead;
		strucHead->pre = p;
	}
	strucHead = p;
	p->nextInScope = strucScope[Scope];  //插入结点到当前作用域链表中
	strucScope[Scope] = p;
	return 0;
}

/*插入函数结点*/
int insertFuncNode(FuncNode* p){
	if(p->name == NULL)
	{
		return -1;
	}
	for(FuncNode* q=funcHead; q!=NULL; q=q->next)
	{
		if(strcmp(p->name,q->name)==0)
		{
			if(q->Def_flag==TRUE && p->Def_flag==TRUE)
			{
				return REDEFINE_ERROR;
			}
			else
			{
				if(VarNodeCmp(p->retType, q->retType)==FALSE || VarNodeCmp(p->paraList, q->paraList)==FALSE)
				{
					return INCONSISTENT_DELARATION_ERROR;
				}
				if(p->Def_flag==TRUE)
				{
					q->Def_flag = TRUE;//标记该函数已定义
				}
				return 0;
			}
		}
	}
	if(funcHead != NULL)
	{
		p->next = funcHead;
	}
	funcHead = p;
	return 0; 
}

/*查找变量或数组*/
VarNode* findVarNode(char *name){
	VarNode* q;
	for(q=varHead; q!=NULL; q=q->next)
	{
		if(strcmp(q->name,name) == 0)
			break;
	}
	return q;
}

/*查找结构体*/
StrucNode* findStrucNode(char* name){
	StrucNode* q;
	for(q=strucHead; q!=NULL; q=q->next)
	{
		if(strcmp(q->name,name) == 0)
			break;
	}
	return q;
}

/*查找函数*/
FuncNode* findFuncNode(char* name){
	FuncNode* q;
	for(q=funcHead; q!=NULL; q=q->next)
	{
		if(strcmp(q->name,name) == 0)
			break;
	}
	return q;
}

/*检测错误18，函数声明但未定义*/
void checkFuncDef(){
	for(FuncNode *q=funcHead; q!=NULL; q=q->next)
	{
		if(q->Def_flag == FALSE)
			printError(q->lineno,q->name,18);
	}
}

/*符号结点比较函数*/
int VarNodeCmp(VarNode* a, VarNode* b)
{
	if(a->kind!=b->kind)  			//类型不同
	{
		return FALSE;
	}
	else
	{
		VarNode* p;
		VarNode* q;
		switch(a->kind){
			case BASIC_T:  //变量类型不同
				if(a->detail.basic!=b->detail.basic)
				{
					return FALSE;
				}
				break;
			case ARRAY_T:  //若为数组则继续比较其元素类型等
				return VarNodeCmp(a->detail.array.type, b->detail.array.type);
				break;
			case STRUCT_T:  //若为结构体则继续比较其成员
				p = a->detail.struc->VarInStruc;
				q = b->detail.struc->VarInStruc;
				if((p==NULL&&q!=NULL)
					||(p!=NULL&&q==NULL))
				{
					return FALSE;
				}
				else if(p==NULL&&q==NULL)
				{
					return TRUE;
				}
				else
				{
					p=p->nextInList,q=q->nextInList;
					if((p==NULL&&q!=NULL)
					||(p!=NULL&&q==NULL))
					{
						return FALSE;
					}
					else if(p==NULL&&q==NULL)
					{
						return TRUE;
					}
					else
					{
						return VarNodeCmp(p,q);
					}
				}
				break;
			default: return FALSE;
		}
	}
	return TRUE;
}

/*进入作用域*/
void IntoScope()
{  
	Scope++;
	if( Scope>100)  //当作用域数组超过上限，报错
		printf("ERROR: Scope array overflow!\n");
}

/*离开作用域*/
void OutScope()
{
	if( Scope==0 ){
		printf("ERROR: In the global scope!\n");
		return;
	}
	VarNode* var_temp;
	StrucNode* struc_temp;
	for(var_temp = varScope[Scope]; var_temp!=NULL; var_temp=var_temp->nextInScope)
	{
		if(var_temp->pre!=NULL)
			var_temp->pre->next = var_temp->next;
		else
			varHead = varHead->next;
	}
	varScope[Scope] = NULL;
	for(struc_temp = strucScope[Scope]; struc_temp!=NULL; struc_temp=struc_temp->nextInScope)
	{
		if(struc_temp->pre!=NULL)
			struc_temp->pre->next = struc_temp->next;
		else
			strucHead = strucHead->next;
	}
	strucScope[Scope] = NULL;
	Scope--;//退出作用域之后，域值-1
}

/*打印参数类型*/
void printType(VarNode* p)
{
	switch(p->kind){
		case BASIC_T:
			if(p->detail.basic==INT)
			{
				printf("int");
			}
			else if(p->detail.basic==FLOAT)
			{
				printf("float");
			}
			break;
		case ARRAY_T:
			printType(p->detail.array.type);
			printf("[]");
			break;
		case STRUCT_T:
			printf("struct %s", p->detail.struc->name);
			break;
	}
}

/*打印参数列表*/
void printTypeList(VarNode* p)
{
	for(; p!=NULL; p=p->nextInList)
	{
		printType(p);
		if(p->nextInList!=NULL)
			printf(", ");
	}
}

/*错误管理函数*/
void printError(int error_line, char *error_str, int error_type)
{
	switch(error_type)
	{
		case 1: printf( "Error type 1 at line %d: Undefined variable \"%s\".\n", error_line, error_str);break;
		case 2: printf( "Error type 2 at line %d: Undefined function \"%s\".\n", error_line, error_str);break;
		case 3: printf( "Error type 3 at line %d: Redefined variable \"%s\".\n", error_line, error_str);break;
		case 4: printf( "Error type 4 at line %d: Redefined function \"%s\".\n", error_line, error_str);break;
		case 5: printf( "Error type 5 at line %d: Type mismatched for assignment.\n", error_line);break;
		case 6: printf( "Error type 6 at line %d: The left-hand side of an assignment must be a variable.\n", error_line);break;
		case 7: printf( "Error type 7 at line %d: Operands type mismatched\n", error_line);break;
		case 8: printf("Error type 8 at line %d: Type mismatched for return.\n", error_line);break;
		case 9: printf( "Error type 9 at line %d: Function \"%s(", error_line, error_str);break;
		case 10: printf( "Error type 10 at line %d: \"%s\" is not an array.\n", error_line, error_str);break;
		case 11: printf( "Error type 11 at line %d: \"%s\" is not a function.\n", error_line, error_str);break;
		case 12: break;
		case 13: printf("Error type 13 at line %d: Illegal use of \".\".\n", error_line);break;
		case 14: printf( "Error type 14 at line %d: Non-existent field \"%s\".\n", error_line, error_str);break;
		case 15: printf( "Error type 15 at line %d: Redefined field \"%s\".\n", error_line, error_str);break;
		case 16: printf( "Error type 16 at line %d: Duplicated name \"%s\".\n", error_line, error_str);break;
		case 17: printf( "Error type 17 at line %d: Undefined structure \"%s\".\n", error_line, error_str);break;
		case 18: printf("Error type 18 at line %d: Undefined function \"%s\".\n", error_line, error_str);break;
		case 19: printf( "Error type 19 at line %d: Inconsistent declaration of function \"%s\".\n", error_line, error_str);break;
	}
} 