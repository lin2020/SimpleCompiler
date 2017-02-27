#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Node.h"

#define REDEFINE_ERROR -2  //重复定义错误
#define INCONSISTENT_DELARATION_ERROR -3  //声明不一致错误

//define type
typedef enum {BASIC_T,ARRAY_T,STRUCT_T} _T;
/*符号表结点定义*/
typedef struct VarNode
{
	char name[32];
	struct VarNode* nextInList;  //一次声明多个同一类型变量，只保存头结点信息，如int a，b，c；
	_T kind;  //变量类型：BASIC_KIND or ARRAY_KIND or STRUCT_KIND
	union
	{
		int basic;  //INT_TYPE or FLOAT_TYPE
		struct 		//数组
		{  
			struct VarNode* type;  //数组元素类型
			int size;  //数组元素个数
		} array;
		struct StrucNode* struc;  //结构体类型，如在函数参数列表中存在结构体类型，同样可用此类型结点保存
	} detail;
	int int_val;
	float float_val;
	int Scope;  //变量的作用域，0代表全局
	struct VarNode* nextInScope;  //指向同一作用域中下一个变量
	struct VarNode* pre;
	struct VarNode* next;
}VarNode;

/*结构体符号表结点定义*/
typedef struct StrucNode
{
	char name[32];
	VarNode* VarInStruc;  //保存结构体中变量
	int Scope;  //作用域
	struct StrucNode* nextInScope;//指向同一作用域中下一个结构体
	struct StrucNode* pre;
	struct StrucNode* next;
} StrucNode;

/*函数符号表结点定义*/
typedef struct FuncNode
{
	char name[32];
	int Def_flag;  //为1表示已定义，为0表示未定义
	VarNode* retType;  //保存返回类型
	VarNode* paraList;  //保存参数列表
	int lineno;//行号
	struct FuncNode* next;
} FuncNode;

VarNode* varHead;  		//变量数组符号表
StrucNode* strucHead;  	//结构体符号表
FuncNode* funcHead;  	//函数符号表
int Scope;  			//变量和数组的作用域
VarNode* varScope[100]; 		//变量数组作用域栈
StrucNode* strucScope[100];  	//结构体作用域栈

/***符号表操作函数***/
void initTable();  //初始化
int insertVarNode(VarNode* p);  //插入变量数组
int insertFuncNode(FuncNode* p);  //插入函数
int insertStrucNode(StrucNode* p);  //插入结构体
VarNode* findVarNode(char* name);  //查找变量或数组
FuncNode* findFuncNode(char* name);  //查找函数
StrucNode* findStrucNode(char* name);  //查找结构体
int VarNodeCmp(VarNode* a, VarNode* b);  //符号比较函数
void IntoScope();  //进入新的作用域时，Scope值+1
void OutScope();  //当从当前作用域退出时，清空当前作用域内的元素
void checkFuncDef();  //检测ERROR 18，函数声明但未定义
void printType(VarNode* p);  //打印结点类型
void printTypeList(VarNode* p);  //打印链表结点类型
void printError(int error_line, char *error_str, int error_type);//出错处理程序