#define FALSE 0 
#define TRUE 1
#include "syntax.tab.h"
typedef struct NODE 			//抽象语法树的结点数据结构
{
	int kind;		//结点类型
	int lineno;		//结点所在行号
	int rule;		//归约所用规则
	union			//结点的具体内容
	{
		char c[32];	//标识符或关键字
		int i;		//int变量
		float f;	//float变量
	};
	struct NODE *child;		//左孩子
	struct NODE *brother;	//右兄弟
}node,*pnode,**ppnode;

void init_c(ppnode pp,int rule,int kind,int lineno,char* c) //创建结点，其中保存的具体内容为标识符或关键字
{
	pnode p = (pnode)malloc(sizeof(node));
	p->kind = kind;
	p->rule = rule;
	p->lineno = lineno;
	strcpy(p->c,c);
	p->child = NULL;
	p->brother = NULL;
	*pp = p;
}
 
void init_i(ppnode pp,int rule,int kind,int lineno,int i)  //创建结点，其中保存的具体内容为int变量
{
	pnode p = (pnode)malloc(sizeof(node));
	p->kind = kind;
	p->rule = rule;
	p->lineno = lineno;
	p->i = i;
	p->child = NULL;
	p->brother = NULL;
	*pp = p;
}

void init_f(ppnode pp,int rule,int kind,int lineno,float f) //创建结点，其中保存的具体内容为float变量
{
	pnode p = (pnode)malloc(sizeof(node));
	p->kind = kind;
	p->rule = rule;
	p->lineno = lineno;
	p->f = f;
	p->child = NULL;
	p->brother = NULL;
	*pp = p;
}

void display(pnode root,int indent) 		//遍历抽象语法树
{
	if(root != NULL)
	{
		switch(root->kind)				//访问父结点
		{
			case NT:if(indent==0) printf("%s (%d)\n",root->c,root->lineno); else printf("%*c%s (%d)\n",indent,' ',root->c,root->lineno); break;
			case T: printf("%*c%s\n",indent,' ',root->c); break;
			case ID: printf("%*cID:%s\n",indent,' ',root->c); break;
			case INT: printf("%*cINT:%d\n",indent,' ',root->i); break;
			case FLOAT: printf("%*cFLOAT:%f\n",indent,' ',root->f); break;
			case TYPE_INT: printf("%*cTYPE:int\n",indent,' '); break;
			case TYPE_FLOAT: printf("%*cTYPE:float\n",indent,' '); break;
		}	
		display(root->child,indent+2);	//访问子结点
		display(root->brother,indent);  //访问兄弟结点
	}
}