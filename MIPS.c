/*
*
*	中间代码生成目标代码，入口函数是translate
*	生成的目标代码存放在exp4.2.s文件中
*
*/

#include "LIR.c"

struct Tbl 						//寄存器使用映射表，变量到寄存器组s0～s7的映射
{
	int usd[8];					//使用位，值为1表示对应下标的寄存器已被使用
	int lock[8];				//占用位，值为1表示对应下标的寄存器不可以被淘汰
	int kind[8];				//对应下标的寄存器存放的变量的类型
	int val_no[8];				//对应下标的寄存器存放的变量的编号
	int changed[8];				//修改位，值为1表示对应下标的寄存器已经被修改过，置换该寄存器对应的变量时，要将变量写回内存
};

FILE *fp = NULL;				//文件指针
int argPtr = 0;					//参数计数，用于接受参数的时候的计数，最多能接受四个参数，从a0~a3寄存器中取出参数
int paramPtr = 0;				//参数计数，用于传递参数的时候的计数，最多能传四个参数，使用a0~a3寄存器传递参数
int param_no[4];				//每个参数的编号
int param_kind[4];				//每个参数的类型
struct Tbl tbl;

void translate();				//中间代码翻译成目标代码的入口函数
void translateIR();				//将中间代码翻译成目标代码
void load(pOperand,int);		//从数据段加载变量到对应的寄存器上
void store(pOperand,int);		//将寄存器上已经修改过的变量保存到数据段上
int getRegNo(pOperand);			//获取操作数中对应的变量，如果该变量已经跟某个寄存器关联，则将该寄存器的编号返回
void unlock();					//解除对寄存器使用的限制，比如在一个加法，三个操作变量关联的寄存器都是不能被更改关联的变量的，
								//这时候，就要将这三个变量关联的寄存器锁起，等满足一定条件后，执行unlock函数可以解除对寄存器的占用。

/*中间代码翻译成目标代码的入口函数*/
void translate()
{
	if((fp=fopen("./exp4.2.s","w"))==NULL)		//打开文件，供目标代码输出
	{
		perror("fopen");
		exit(0);
	}
	for(int i=0; i<8; i++)						//初始化寄存器使用映射表
	{
		tbl.usd[i]=0;
		tbl.lock[i]=0;
		tbl.kind[i]=-1;
		tbl.val_no[i]=-1;
		tbl.changed[i]=0;
	}
	fprintf(fp, "%s\n", ".data");				//输出目标代码开头部分，包括数据段，read和write函数
	fprintf(fp, "%s\n", "_prompt: .asciiz \"Enter an integer:\"");
	fprintf(fp, "%s\n", "_ret: .asciiz \"\\n\"");
	for(pVari p=varsHead; p!=NULL; p=p->next)	//将中间代码中的变量保存在数据段中			
	{
		fprintf(fp, "v%d: .word 0\n", p->vari_no);
	}
	for(pTemps p=tempsHead; p!=NULL; p=p->next)
	{
		fprintf(fp, "t%d: .word 0\n", p->temp_no);
	}
	fprintf(fp, "%s\n", ".globl main");
	fprintf(fp, "%s\n", ".text");
	fprintf(fp, "%s\n", "read:");				//read和write
	fprintf(fp, "%s\n", "	li $v0, 4");
	fprintf(fp, "%s\n", "	la $a0, _prompt");
	fprintf(fp, "%s\n", "	syscall");
	fprintf(fp, "%s\n", "	li $v0, 5");
	fprintf(fp, "%s\n", "	syscall");
	fprintf(fp, "%s\n\n", "	jr $ra");
	fprintf(fp, "%s\n", "write:");
	fprintf(fp, "%s\n", "	li $v0, 1");
	fprintf(fp, "%s\n", "	syscall");
	fprintf(fp, "%s\n", "	li $v0, 4");
	fprintf(fp, "%s\n", "	la $a0, _ret");
	fprintf(fp, "%s\n", "	syscall");
	fprintf(fp, "%s\n", "	move $v0, $0");
	fprintf(fp, "%s\n\n", "	jr $ra");
	translateIR();								//将中间代码翻译成目标代码
	fclose(fp);
}

/*将中间代码翻译成目标代码*/
void translateIR()
{
	pInterCodes codes = codeHead;					//中间代码头指针，用于遍历中间代码
	int reg,reg1,reg2;								//操作数1～3对应的寄存器编号
	for(; codes!=NULL; codes=codes->next)
	{
		unlock();	//满足一定条件的时候解除对寄存器的使用限制
		switch(codes->code->kind)	//根据中间代码的类型，翻译成对应的目标代码，选择的指令的操作数基本都是寄存器s0~s7，
		{							//通过getRegNo取出中间代码中变量对应的寄存器的编号。
			case CODE_LABEL:
				fprintf(fp, "label%d:\n", codes->code->op1->val_no); 
				break;
			case CODE_FUNC: 
				fprintf(fp, "%s:\n", codes->code->op1->str); 
				break;
			case CODE_ASSIGN:
				reg = getRegNo(codes->code->op1);
				reg1 = getRegNo(codes->code->op2);
				fprintf(fp, "	move $s%d, $s%d\n",reg,reg1);
				tbl.changed[reg] = 1;		//寄存器reg已经被修改，将对应的修改位置为1,表示更改该寄存器关联的变量的时候，
				break;						//要先把原来的寄存器关联的变量写回数据段，以下是同样的道理，不再注释。
			case CODE_ADD:
				reg = getRegNo(codes->code->op1);
				reg1 = getRegNo(codes->code->op2);
				reg2 = getRegNo(codes->code->op3);
				fprintf(fp, "	add $s%d, $s%d, $s%d\n",reg,reg1,reg2);
				tbl.changed[reg] = 1;
				break;
			case CODE_SUB: 
				reg = getRegNo(codes->code->op1);
				reg1 = getRegNo(codes->code->op2);
				reg2 = getRegNo(codes->code->op3);
				fprintf(fp, "	sub $s%d, $s%d, $s%d\n", reg, reg1, reg2);
				tbl.changed[reg] = 1;
				break;
			case CODE_MUL:
				reg = getRegNo(codes->code->op1);
				reg1 = getRegNo(codes->code->op2);
				reg2 = getRegNo(codes->code->op3);
				fprintf(fp, "	mul $s%d, $s%d, $s%d\n", reg, reg1, reg2);
				tbl.changed[reg] = 1;
				break;
			case CODE_DIV:
				reg = getRegNo(codes->code->op1);
				reg1 = getRegNo(codes->code->op2);
				reg2 = getRegNo(codes->code->op3);
				fprintf(fp, "	div $s%d, $s%d\n", reg1, reg2);
				fprintf(fp, "	mflo $s%d\n", reg);
				tbl.changed[reg] = 1;
				break;
   			case CODE_GOTO:
   				fprintf(fp, "	j label%d\n", codes->code->op1->val_no);
   				break;
   			case CODE_IFGT:
   			   	reg = getRegNo(codes->code->op1);
				reg1 = getRegNo(codes->code->op2);
   				if(strcmp(codes->code->relop->str, "==")==0)
   				{
   					fprintf(fp, "	beq $s%d, $s%d, label%d\n",reg,reg1,codes->code->op3->val_no);
   				}
   				else if(strcmp(codes->code->relop->str, "!=")==0)
   				{
   					fprintf(fp, "	bne $s%d, $s%d, label%d\n",reg,reg1,codes->code->op3->val_no);
   				}
   				else if(strcmp(codes->code->relop->str, ">")==0)
   				{
   					fprintf(fp, "	bgt $s%d, $s%d, label%d\n",reg,reg1,codes->code->op3->val_no);
   				}
   				else if(strcmp(codes->code->relop->str, "<")==0)
   				{
   					fprintf(fp, "	blt $s%d, $s%d, label%d\n",reg,reg1,codes->code->op3->val_no);
   				}
   				else if(strcmp(codes->code->relop->str, ">=")==0)
   				{
   					fprintf(fp, "	bge $s%d, $s%d, label%d\n",reg,reg1,codes->code->op3->val_no);
   				}
   				else if(strcmp(codes->code->relop->str, "<=")==0)
   				{
   					fprintf(fp, "	ble $s%d, $s%d, label%d\n",reg,reg1,codes->code->op3->val_no);
   				}
   				break; 
   			case CODE_RETURN:
				reg = getRegNo(codes->code->op1);
				fprintf(fp, "	move $v0, $s%d\n",reg);
				fprintf(fp, "	jr $ra\n");
   				break; 
   			case CODE_ARG:							//传递参数，将参数送到a0~a3寄存器中，并将相关的变量压栈保护起来
   				reg = getRegNo(codes->code->op1);
   				if(codes->code->op1->kind==OP_TEMP)	//如果是临时变量，则认为要保护的变量在这条语句之前，查找前一条中间语句
   				{
   					param_no[paramPtr]=codes->prev->code->op2->val_no;
   					param_kind[paramPtr]=codes->prev->code->op2->kind;
   					fprintf(fp, "	move $a%d, $s%d\n", paramPtr, reg);	//传递参数
   					reg1 = getRegNo(codes->prev->code->op2);
   					fprintf(fp, "	sw $s%d, -%d($sp)\n",reg1, (paramPtr+1)*4);	//压栈保护
   					paramPtr++;
   				}
   				else if(codes->code->op1->kind==OP_VARIABLE)//如果是变量，将其压栈保护
   				{
   					param_no[paramPtr]=codes->code->op1->val_no;
   					param_kind[paramPtr]=codes->code->op1->kind;
   					fprintf(fp, "	move $a%d, $s%d\n", paramPtr, reg);
   					fprintf(fp, "	sw $s%d, -%d($sp)\n",reg1, (paramPtr+1)*4);
   					paramPtr++;
   				}
   				else if(codes->code->op1->kind==OP_CONSTANT)//如果是常量，不保护，只传递参数
   				{
   					param_no[paramPtr]=codes->code->op1->val_no;
   					param_kind[paramPtr]=codes->code->op1->kind;
   					fprintf(fp, "	move $a%d, $s%d\n", paramPtr, reg);
   					paramPtr++;
   				}
   				break; 
   			case CODE_CALL:					//调用函数
   				fprintf(fp, "	addi $sp, $sp, -%d\n",(paramPtr+1)*4);	//先分配栈空间
   				fprintf(fp, "	sw $ra, 0($sp)\n");
   				int p = paramPtr;
   				if(codes->code->op2!=NULL)	//调用函数指令
   				{
   					fprintf(fp, "	jal %s\n", codes->code->op2->str);
   				}
   				else
   				{
   					fprintf(fp, "	jal %s\n", codes->code->op1->str);
   				}
   				fprintf(fp, "	lw $ra, 0($sp)\n");
   				int i = 0;
   				if(i < paramPtr)			//调用函数结束后，出栈恢复保护的变量
   				{
   					pOperand temp = (pOperand)malloc(sizeof(Operand));
   					temp->kind = param_kind[paramPtr-i-1];
   					temp->val_no = param_no[paramPtr-i-1];
   					if(temp->kind == OP_VARIABLE)	//只有类型为变量的才需要恢复
   					{
   						reg = getRegNo(temp);
   						fprintf(fp, "	lw $s%d, %d($sp)\n", reg, (i+1)*4);
   						tbl.changed[reg] = 1;
   					}
   					i++;
   				}
   				fprintf(fp, "	addi $sp, $sp, %d\n", p*4+4);
   				if(codes->code->op2!=NULL)		//设置返回值
   				{
   					reg1 = getRegNo(codes->code->op1);
   					fprintf(fp, "	move $s%d, $v0\n", reg1);
   					tbl.changed[reg1] = 1;
   				}
   				paramPtr = 0;
   				argPtr=0;
   				break;
   			case CODE_PARAM: 				//获取参数
   				reg = getRegNo(codes->code->op1);
   				fprintf(fp, "	move $s%d, $a%d\n", reg, argPtr);
   				tbl.changed[reg] = 1;
   				argPtr++;
   				break;
   			case CODE_READ: 
   				fprintf(fp, "	addi $sp, $sp, -4\n");
   				fprintf(fp, "	sw $ra, 0($sp)\n");
   				fprintf(fp, "	jal read\n");
   				fprintf(fp, "	lw $ra, 0($sp)\n");
   				fprintf(fp, "	addi $sp, $sp, 4\n");
   				reg = getRegNo(codes->code->op1);
   				fprintf(fp, "	move $s%d, $v0\n", reg);
   				tbl.changed[reg] = 1;
   				break;
   			case CODE_WRITE:
   				reg = getRegNo(codes->code->op1);
   				fprintf(fp, "	move $a0, $s%d\n",reg);
   				fprintf(fp, "	addi $sp, $sp, -4\n");
   				fprintf(fp, "	sw $ra, 0($sp)\n");
   				fprintf(fp, "	jal write\n");
   				fprintf(fp, "	lw $ra,0($sp)\n");
   				fprintf(fp, "	addi $sp, $sp, 4\n");
   				break;
		}
	}
}

/*获取操作数中对应的变量，如果该变量已经跟某个寄存器关联，则将该寄存器的编号返回*/
int getRegNo(pOperand op)
{
	int i;
	for(i=0; i<8; i++)		//遍历关联表，查找是否已经将变量关联到某一个寄存器上
	{
		if(op->kind==tbl.kind[i] && op->val_no==tbl.val_no[i])
		{
			break;
		}
	}
	if(i!=8)				//如果找到，将该寄存器的编号返回
	{
		return i;
	}
	else					//否者，要进一步考察
	{
		for(i=0; i<8 && tbl.usd[i]==1; i++)	//查看关联表中是否要寄存器没有关联到变量
			;
		if(i!=8)							//如果有，这将该寄存器关联到目前的变量
		{
			if(op->kind == OP_CONSTANT)		//如果是常量，要先加载到寄存器上，变量和临时变量不需要
			{
				load(op,i);
			}				
			tbl.usd[i]=1;					//将使用位和占用位置为1
			tbl.lock[i]=1;
			tbl.kind[i]=op->kind;			//保存关联变量的类型和编号
			tbl.val_no[i]=op->val_no;
			return i;						//返回关联到的寄存器的编号
		}
		else				//如果所有的寄存器都已经关联到某一变量上，需要淘汰其中一个，腾出空寄存器用来加载新的变量
		{
			for(i=7; i>=0 && tbl.lock[i]==1; i--)	//如果占用位为1,表示不可以被淘汰，淘汰策略是后进先出
				;
			if(i!=-1)		//如果找到可淘汰的
			{
				pOperand op1 = (pOperand)malloc(sizeof(Operand));
				op1->kind = tbl.kind[i]; 
				op1->val_no = tbl.val_no[i]; 
				if(tbl.changed[i]==1)	//查看原来关联的变量是否修改过，如果修改过,则要重新写回数据段
				{
					store(op1,i);
				}
				load(op,i);				//加载新的变量
				tbl.usd[i]=1;
				tbl.lock[i]=1;
				tbl.kind[i]=op->kind;
				tbl.val_no[i]=op->val_no;
				tbl.changed[i]=0;
				return i;
			}
			return -1;					//失败，所有寄存其都正在使用，且不可淘汰，返回-1；
		}
	}
}

//解除对寄存器使用的限制，比如在一个加法，三个操作变量关联的寄存器都是不能被更改关联的变量的，
//这时候，就要将这三个变量关联的寄存器锁起，等满足一定条件后，执行unlock函数可以解除对寄存器的占用。
void unlock()
{
	int i;
	for(i=0; i<8 && tbl.lock[i]==1; i++)
		;
	if(i==8)	//当所有寄存器都被占用的时候，将全部解除使用限制
	{
		for(i=0; i<8; i++)
		{
			tbl.lock[i]=0;
		}
	}
}

/*从数据段加载变量到对应的寄存器上，第一个参数是变量，第二个参数是要加载到的寄存器的编号*/
void load(pOperand op,int reg_num)
{
	switch(op->kind)
	{
		case OP_TEMP:
			fprintf(fp, "	lw $s%d, t%d\n", reg_num, op->val_no);
			break;
		case OP_VARIABLE:
			fprintf(fp, "	lw $s%d, v%d\n", reg_num, op->val_no);
			break;
		case OP_CONSTANT:
			fprintf(fp, "	li $s%d, %d\n", reg_num, op->value);
			break;
	}
}

/*将寄存器上已经修改过的变量保存到数据段上，第一个参数是变量，第二个参数是要保存到数据段的寄存器的编号*/
void store(pOperand op,int reg_num)
{
	switch(op->kind)
	{
		case OP_TEMP:
			fprintf(fp, "	sw $s%d, t%d\n", reg_num, op->val_no);
			break;
		case OP_VARIABLE:
			fprintf(fp, "	sw $s%d, v%d\n", reg_num, op->val_no);
			break;
	}
}