#include "Table.c"
#include <assert.h>

typedef enum {LOCAL_VAR,EXT_VAR,STRUCT_VAR,PARAM_VAR} _VAR;

void Program(pnode p );
void ExtDefList(pnode p );
void ExtDef(pnode p );
void ExtDecList(pnode p, VarNode* type );

VarNode* Specifier(pnode p );
VarNode* StructSpecifier(pnode p );

VarNode* VarDec(pnode p, VarNode* type, int context );
FuncNode* FunDec(pnode p, VarNode* retType );
VarNode*  VarList(pnode p );
VarNode* ParamDec(pnode p );

void CompSt(pnode p, VarNode* retType );
void StmtList(pnode p, VarNode* retType );
void Stmt(pnode p, VarNode* retType );

VarNode* DefList(pnode p, int context );
VarNode* Def(pnode p, int context );
VarNode* DecList(pnode p, VarNode* type, int context );
VarNode* Dec(pnode p, VarNode* type, int context );

VarNode* Exp(pnode p );
int Args(pnode p, VarNode* para );
void printArgs(pnode p );
