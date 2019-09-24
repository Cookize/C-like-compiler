#include"mid_code.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"error.h"

FILE* out;

four_code mid_code[SIZE_MID_CODE];					// 中间代码表
int length_mid_code = 0;							// 中间代码长度
int temp_var_num = 0;								// 临时变量计数
int	label_num = 0;									// 跳转标识计数


void init_mid_code() {
	if (fopen_s(&out, "mid_code.txt", "w") != 0) {
		error_stand(FILE_NOT_FOUND);
													// 无法打开文件，结束运行。
	}
}

void reset_mid_code() {
	fclose(out);
}

void print_four_code_alt (four_code code) {
	switch (code->op){
	case ADD: 
		fprintf_s(out, "CODE:	ADD %s %s %s\n", code->a, code->b, code->c);
		break;
	case SUB:
		fprintf_s(out, "CODE:	SUB %s %s %s\n", code->a, code->b, code->c);
		break;
	case MUL:
		fprintf_s(out, "CODE:	MUL %s %s %s\n", code->a, code->b, code->c);
		break;
	case DIV:
		fprintf_s(out, "CODE:	DIV %s %s %s\n", code->a, code->b, code->c);
		break;
	case GEI:
		fprintf_s(out, "CODE:	GEI %s %s %s\n", code->a, code->b, code->c);
		break;
	case AEI:
		fprintf_s(out, "CODE:	AEI %s %s %s\n", code->a, code->b, code->c);
		break;
	case ASN:
		fprintf_s(out, "CODE:	ASN %s %s\n", code->a, code->b);
		break;
	case LES:
		fprintf_s(out, "CODE:	LES %s %s\n", code->a, code->b);
		break;
	case LEQ:
		fprintf_s(out, "CODE:	LEQ %s %s\n", code->a, code->b);
		break;
	case GTR:
		fprintf_s(out, "CODE:	GTR %s %s\n", code->a, code->b);
		break;
	case GEQ:
		fprintf_s(out, "CODE:	GEQ %s %s\n", code->a, code->b);
		break;
	case EQL:
		fprintf_s(out, "CODE:	EQL %s %s\n", code->a, code->b);
		break;
	case NEQ:
		fprintf_s(out, "CODE:	NEQ %s %s\n", code->a, code->b);
		break;
	case LABEL:
		fprintf_s(out, "CODE:	LABEL %s\n", code->a);
		break;
	case GOTO:
		fprintf_s(out, "CODE:	GOTO %s\n", code->a);
		break;
	case BZ:
		fprintf_s(out, "CODE:	BZ %s\n", code->a);
		break;
	case FUNC:
		fprintf_s(out, "CODE:	FUNC %s %s\n", code->a, code->b);
		break;
	case PARA:
		fprintf_s(out, "CODE:	PARA %s %s\n", code->a, code->b);
		break;
	case PUSH:
		fprintf_s(out, "CODE:	PUSH %s\n", code->a);
		break;
	case CALL:
		fprintf_s(out, "CODE:	CALL %s\n", code->a);
		break;
	case RET:
		if(code->a != NULL) fprintf_s(out, "CODE:	RET %s\n",code->a);
		else fprintf_s(out, "CODE:	RET\n");
		break;
	case CONST:
		fprintf_s(out, "CODE:	CONST %s %s %s\n", code->a, code->b, code->c);
		break;
	case VAR:
		fprintf_s(out, "CODE:	VAR %s %s\n", code->a, code->b);
		break;
	case ARRAY:
		fprintf_s(out, "CODE:	ARRAY %s %s\n", code->a, code->b);
		break;
	case READ:
		fprintf_s(out, "CODE:	READ %s\n", code->a);
		break;
	case PRINT:
		fprintf_s(out, "CODE:	PRINT %s %s\n", code->a, code->b);
		break;
	default:
		break;
	}
}

void print_four_code_insert (void* out, four_code code) {
	switch (code->op) {
	case ADD:
		fprintf_s((FILE*)out, "CODE:%d\t%s = %s + %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case SUB:
		fprintf_s((FILE*)out, "CODE:%d\t%s = %s - %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case MUL:
		fprintf_s((FILE*)out, "CODE:%d\t%s = %s * %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case DIV:
		fprintf_s((FILE*)out, "CODE:%d\t%s = %s / %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case GEI:
		fprintf_s((FILE*)out, "CODE:%d\t%s = %s [ %s ]\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case AEI:
		fprintf_s((FILE*)out, "CODE:%d\t%s [%s] = %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case ASN:
		fprintf_s((FILE*)out, "CODE:%d\t%s = %s\n", code->is_break_point, code->a, code->b);
		break;
	case LES:
		fprintf_s((FILE*)out, "CODE:%d\t%s < %s\n", code->is_break_point, code->a, code->b);
		break;
	case LEQ:
		fprintf_s((FILE*)out, "CODE:%d\t%s <= %s\n", code->is_break_point, code->a, code->b);
		break;
	case GTR:
		fprintf_s((FILE*)out, "CODE:%d\t%s > %s\n", code->is_break_point, code->a, code->b);
		break;
	case GEQ:
		fprintf_s((FILE*)out, "CODE:%d\t%s >= %s\n", code->is_break_point, code->a, code->b);
		break;
	case EQL:
		fprintf_s((FILE*)out, "CODE:%d\t%s == %s\n", code->is_break_point, code->a, code->b);
		break;
	case NEQ:
		fprintf_s((FILE*)out, "CODE:%d\t%s != %s\n", code->is_break_point, code->a, code->b);
		break;
	case LABEL:
		fprintf_s((FILE*)out, "CODE:%d\t%s :\n", code->is_break_point, code->a);
		break;
	case GOTO:
		fprintf_s((FILE*)out, "CODE:%d\tGOTO %s\n", code->is_break_point, code->a);
		break;
	case BZ:
		fprintf_s((FILE*)out, "CODE:%d\tBZ %s\n", code->is_break_point, code->a);
		break;
	case FUNC:
		fprintf_s((FILE*)out, "CODE:%d\tFUNC %s %s\n", code->is_break_point, code->a, code->b);
		break;
	case PARA:
		fprintf_s((FILE*)out, "CODE:%d\tPARA %s %s\n", code->is_break_point, code->a, code->b);
		break;
	case PUSH:
		fprintf_s((FILE*)out, "CODE:%d\tPUSH %s\n", code->is_break_point, code->a);
		break;
	case CALL:
		fprintf_s((FILE*)out, "CODE:%d\tCALL %s\n", code->is_break_point, code->a);
		break;
	case RET:
		if (code->a != NULL) fprintf_s((FILE*)out, "CODE:%d\tRET %s\n", code->is_break_point, code->a);
		else fprintf_s((FILE*)out, "CODE:%d\tRET\n", code->is_break_point);
		break;
	case CONST:
		fprintf_s((FILE*)out, "CODE:%d\tCONST %s %s %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case VAR:
		fprintf_s((FILE*)out, "CODE:%d\tVAR %s %s\n", code->is_break_point, code->a, code->b);
		break;
	case ARRAY:
		fprintf_s((FILE*)out, "CODE:%d\tARRAY %s %s\n", code->is_break_point, code->a, code->b);
		break;
	case READ:
		fprintf_s((FILE*)out, "CODE:%d\tREAD %s\n", code->is_break_point, code->a);
		break;
	case PRINT:
		fprintf_s((FILE*)out, "CODE:%d\tPRINT %s %s\n", code->is_break_point, code->a, code->b);
		break;
	default:
		break;
	}
}

void print_four_code (four_code code) {
	switch (code->op) {
	case ADD:
		fprintf_s(out, "CODE:	%s = %s + %s\n", code->a, code->b, code->c);
		break;
	case SUB:
		fprintf_s(out, "CODE:	%s = %s - %s\n", code->a, code->b, code->c);
		break;
	case MUL:
		fprintf_s(out, "CODE:	%s = %s * %s\n", code->a, code->b, code->c);
		break;
	case DIV:
		fprintf_s(out, "CODE:	%s = %s / %s\n", code->a, code->b, code->c);
		break;
	case GEI:
		fprintf_s(out, "CODE:	%s = %s [ %s ]\n", code->a, code->b, code->c);
		break;
	case AEI:
		fprintf_s(out, "CODE:	%s [%s] = %s\n", code->a, code->b, code->c);
		break;
	case ASN:
		fprintf_s(out, "CODE:	%s = %s\n", code->a, code->b);
		break;
	case LES:
		fprintf_s(out, "CODE:	%s < %s\n", code->a, code->b);
		break;
	case LEQ:
		fprintf_s(out, "CODE:	%s <= %s\n", code->a, code->b);
		break;
	case GTR:
		fprintf_s(out, "CODE:	%s > %s\n", code->a, code->b);
		break;
	case GEQ:
		fprintf_s(out, "CODE:	%s >= %s\n", code->a, code->b);
		break;
	case EQL:
		fprintf_s(out, "CODE:	%s == %s\n", code->a, code->b);
		break;
	case NEQ:
		fprintf_s(out, "CODE:	%s != %s\n", code->a, code->b);
		break;
	case LABEL:
		fprintf_s(out, "CODE:	%s :\n", code->a);
		break;
	case GOTO:
		fprintf_s(out, "CODE:	GOTO %s\n", code->a);
		break;
	case BZ:
		fprintf_s(out, "CODE:	BZ %s\n", code->a);
		break;
	case FUNC:
		fprintf_s(out, "CODE:	FUNC %s %s\n", code->a, code->b);
		break;
	case PARA:
		fprintf_s(out, "CODE:	PARA %s %s\n", code->a, code->b);
		break;
	case PUSH:
		fprintf_s(out, "CODE:	PUSH %s\n", code->a);
		break;
	case CALL:
		fprintf_s(out, "CODE:	CALL %s\n", code->a);
		break;
	case RET:
		if (code->a != NULL) fprintf_s(out, "CODE:	RET %s\n", code->a);
		else fprintf_s(out, "CODE:	RET\n");
		break;
	case CONST:
		fprintf_s(out, "CODE:	CONST %s %s %s\n", code->a, code->b, code->c);
		break;
	case VAR:
		fprintf_s(out, "CODE:	VAR %s %s\n", code->a, code->b);
		break;
	case ARRAY:
		fprintf_s(out, "CODE:	ARRAY %s %s\n", code->a, code->b);
		break;
	case READ:
		fprintf_s(out, "CODE:	READ %s\n", code->a);
		break;
	case PRINT:
		fprintf_s(out, "CODE:	PRINT %s %s\n", code->a, code->b);
		break;
	default:
		break;
	}
}

void print_mid_code_to(char* addr) {
	if (fopen_s(&out, addr, "w") != 0) {
		error_stand(FILE_NOT_FOUND);
		// 无法打开文件，结束运行。
	}
	int i;
	for (i = 0; i < length_mid_code; i++) {
		print_four_code_insert(out, mid_code[i]);
	}
	fclose(out);
}

/*
	插入基本块分割点
*/
void insert_break_point(int index) {
	four_code code = mid_code[index];
	if (code->op == LABEL && index > 0) {				// LABEL
		mid_code[index - 1]->is_break_point = BLKEND;
	}
	else if (code->op == GOTO) {
		code->is_break_point = BLKEND;
	}
	else if (code->op == BZ) {
		code->is_break_point = BLKEND;
	}
	else if (code->op == FUNC) {
		if (index > 0 && mid_code[index - 1]->is_break_point == NORMAL) {
			mid_code[index - 1]->is_break_point = BLKEND;
		}
		code->is_break_point = FUNCSTART;
	}
	else if (code->op == CONST
		|| code->op == VAR
		|| code->op == ARRAY) {
		code->is_break_point = IGNORE;
	}
	else if (code->op == RET) {
		code->is_break_point = BLKEND;
	}
	else if (code->op == CALL) {
		code->is_break_point = NORMAL;		// 暂定CALL为基本块分割标准
	}
	else if (code->op == PARA) {
		code->is_break_point = DEFPARA;
	}
}

/*
	加入中间代码
*/
int enter_code(four_code code) {
	if (length_mid_code >= SIZE_MID_CODE) {
		error_stand(OVERFLOW_MID_CODE);
		return -1;
	}
	mid_code[length_mid_code++] = code;
	insert_break_point(length_mid_code - 1);
	return 0;
}

/*
	生成中间代码
*/
int gen(OP_FOUR_CODE op, char* a, char* b, char* c) {
	four_code new_code = (four_code)malloc(sizeof(four_code_obj));
	new_code->op = op;
	if (a != NULL) {
		new_code->a = (char*)malloc(sizeof(char) * strlen(a));
		strcpy(new_code->a, a);
	}
	else new_code->a = NULL;
	if (b != NULL) {
		new_code->b = (char*)malloc(sizeof(char) * strlen(b));
		strcpy(new_code->b, b);
	}
	else new_code->b = NULL;
	if (c != NULL) {
		new_code->c = (char*)malloc(sizeof(char) * strlen(c));
		strcpy(new_code->c, c);
	}
	else new_code->c = NULL;
	new_code->is_break_point = NORMAL;
	//print_four_code(new_code);
	return enter_code(new_code);
}

/*
	申请跳转标识
*/
int alloc_label(char** temp) {
	char num[10];
	sprintf(num, "%d", label_num++);
	*temp = (char*)malloc(sizeof(char) * (strlen(num) + 6));
	strcpy(*temp, "LABEL_");
	strcat(*temp, num);
	return 0;
}

/*
	输出中间代码
*/
void print_all_mid_code() {
	int i;
	for (i = 0; i < length_mid_code; i++) {
		print_four_code_insert(out, mid_code[i]);
	}
}

/*
	标记需要复制的中间代码段
*/
int mark_mid_code() {
	return length_mid_code;
}
/*
	复制被标记的代码段
*/
void copy_marked_mid_code(int start, int end) {
	int i = start;
	while (i >=0 && i < length_mid_code && i < end) {
		gen(mid_code[i]->op, mid_code[i]->a, mid_code[i]->b, mid_code[i]->c);
		i++;
	}
}

/*
	反转判断条件
*/
void reverse_top_compare() {
	four_code code = mid_code[length_mid_code - 1];
	if (code->op == LES) {
		code->op = GEQ;
	}
	else if (code->op == LEQ) {
		code->op = GTR;
	}
	else if (code->op == GTR) {
		code->op = LEQ;
	}
	else if (code->op == GEQ) {
		code->op = LES;
	}
	else if (code->op == EQL) {
		code->op = NEQ;
	}
	else if (code->op == NEQ) {
		code->op = EQL;
	}
}


