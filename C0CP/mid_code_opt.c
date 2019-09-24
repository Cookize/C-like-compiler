#include"mid_code_opt.h"
#include"mid_code.h"
#include"symbol_table.h"
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<ctype.h>

four_code mark_code[SIZE_MID_CODE];		// 标记中间代码
int mark_line[SIZE_MID_CODE];			// 标记行数
int is_deletable[SIZE_MID_CODE];		// 是否能删除
int sum_mark;							// 标记总数

/*
	删除标记
*/
int delete_mark(char* name) {
	int i, j;
	for (i = 0; i < sum_mark; i++) {
		if (strcmp(mark_code[i]->a, name) == 0 || strcmp(mark_code[i]->b, name) == 0) {
			sum_mark--;
			for (j = i; j < sum_mark; j++) {
				mark_code[j] = mark_code[j + 1];
				mark_line[j] = mark_line[j + 1];
				is_deletable[j] = is_deletable[j + 1];
			}
			i--;
		}
	}
	return 0;
}

/*
	更新标记集合
*/
int mark(four_code code, int line) {
	int i;
	for (i = 0; i < sum_mark; i++) {
		if (strcmp(code->a, mark_code[i]->a) == 0) {	// 覆盖已有记录
			mark_code[i] = code;
			mark_line[i] = line;
			is_deletable[i] = 0;
			//printf_s("MARK:\t%s\n", code->a);
			return 0;
		}
	}
	if (sum_mark < SIZE_MID_CODE) {						// 新建记录
		delete_mark(code->a);
		mark_code[sum_mark] = code;
		mark_line[sum_mark] = line;
		is_deletable[sum_mark] = 0;
		//printf_s("MARK:\t%s\n", code->a);
		sum_mark++;
		return 0;
	}
	return -1;											// 无法新建
}

/*
	删除全局变量标记
*/
int delete_global_mark() {
	int i, j;
	for (i = 0; i < sum_mark; i++) {
		if (is_global(mark_code[i]->a) || is_global(mark_code[i]->b) || strcmp("$RET", mark_code[i]->b) == 0) {
			sum_mark--;
			for (j = i; j < sum_mark; j++) {
				mark_code[j] = mark_code[j + 1];
				mark_line[j] = mark_line[j + 1];
				is_deletable[j] = is_deletable[j + 1];
			}
			i--;
		}
	}
	return 0;
}

/*
	删除全部标记
*/
int delete_all() {
	sum_mark = 0;
	return 0;
}

/*
	检查是否能被复写
*/
int check_rewrite(char** name) {
	int i;
	for (i = 0; i < sum_mark; i++) {
		if (strcmp(mark_code[i]->a, *name) == 0) {
			printf_s("CHANGE:\t%s\tto\t%s\n", *name, mark_code[i]->b);
			*name = (char*)malloc(sizeof(char) * strlen(mark_code[i]->b));
			strcpy(*name, mark_code[i]->b);
			return 0;
		}
	}
	return -1;
}

/*
	整理中间代码
*/
void arrange_mid_code() {
	int i, j;
	for (i = 0, j = 0; i < length_mid_code; i++) {
		if (mid_code[i]->op != NOP) {
			mid_code[j++] = mid_code[i];
		}
	}
	length_mid_code = j;
	return;
}

/*
	跨基本块复写传播、死代码删除
*/
int rewrite() {
	int mid_code_pointer;
	sum_mark = 0;
	for (mid_code_pointer = 0; mid_code_pointer < length_mid_code; mid_code_pointer++) {
		four_code code = mid_code[mid_code_pointer];
		switch (code->op) {
		case GEI:
			check_rewrite(&(code->c));
			break;
		case AEI:
			check_rewrite(&(code->b));
			delete_mark(code->a);
			break;
		case ADD:
		case SUB:
			check_rewrite(&(code->b));
			check_rewrite(&(code->c));
			if ((isdigit(code->b[0]) || code->b[0] == '-')
				&& (isdigit(code->c[0]) || code->c[0] == '-')) {
				sprintf(code->b, "%d", code->op == ADD ? atoi(code->b) + atoi(code->c) : atoi(code->b) - atoi(code->c));
				code->op = ASN;
				code->c = NULL;
				mark(code, mid_code_pointer);
			}
			else if (strcmp(code->c, "0") == 0) {
				mark(code, mid_code_pointer);
			}
			else {
				delete_mark(code->a);
			}
			break;
		case MUL:
		case DIV:
			check_rewrite(&(code->b));
			check_rewrite(&(code->c));
			if ((isdigit(code->b[0]) || code->b[0] == '-')
				&& (isdigit(code->c[0]) || code->c[0] == '-')) {
				sprintf(code->b, "%d", code->op == MUL ? atoi(code->b) * atoi(code->c) : atoi(code->b) / atoi(code->c));
				code->op = ASN;
				code->c = NULL;
				mark(code, mid_code_pointer);
			}
			else if (strcmp(code->c, "1") == 0) {
				mark(code, mid_code_pointer);
			}
			else {
				delete_mark(code->a);
			}
			break;
		case ASN:
			check_rewrite(&(code->b));
			mark(code, mid_code_pointer);
			break;
		case LES:
		case LEQ:
		case GTR:
		case GEQ:
		case EQL:
		case NEQ:
			check_rewrite(&(code->a));
			check_rewrite(&(code->b));
			break;
		case CALL:
			delete_global_mark();
			break;
		case RET:
			if (code->a != NULL) {
				check_rewrite(&(code->a));
			}
			delete_all();
			break;
		case PUSH:
			check_rewrite(&(code->a));
			break;
		case READ:
			delete_mark(code->a);
			delete_mark("$RET");
			break;
		case PRINT:
			delete_mark("$RET");
			check_rewrite(&(code->a));
			break;
		case GOTO:
		case FUNC:
		case LABEL:
		case BZ:
			delete_all();
			break;
		default:
			break;
		}
	}
	return 0;
}

/*
	死代码删除
*/
void delete_dead() {
	int i, j = 0, k, is_used = 0;
	for (i = 0; i < length_mid_code; i++) {
		four_code code = mid_code[i];
		switch (code->op) {
		case LES:
			if ((isdigit(code->a[0]) || code->a[0] == '-')
				&& (isdigit(code->b[0]) || code->b[0] == '-')) {
				if (atoi(code->a) < atoi(code->b)) {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = NOP;
				}
				else {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = GOTO;
					for (i = i + 2; i < length_mid_code; i++) {
						if (mid_code[i]->op != LABEL
							&& mid_code[i]->op != FUNC) {
							mid_code[i]->op = NOP;
						}
						else
							break;
					}
				}
			}
			break;
		case LEQ:
			if ((isdigit(code->a[0]) || code->a[0] == '-')
				&& (isdigit(code->b[0]) || code->b[0] == '-')) {
				if (atoi(code->a) <= atoi(code->b)) {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = NOP;
				}
				else {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = GOTO;
					for (i = i + 2; i < length_mid_code; i++) {
						if (mid_code[i]->op != LABEL
							&& mid_code[i]->op != FUNC) {
							mid_code[i]->op = NOP;
						}
						else
							break;
					}
				}
			}
			break;
		case GTR:
			if ((isdigit(code->a[0]) || code->a[0] == '-')
				&& (isdigit(code->b[0]) || code->b[0] == '-')) {
				if (atoi(code->a) > atoi(code->b)) {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = NOP;
				}
				else {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = GOTO;
					for (i = i + 2; i < length_mid_code; i++) {
						if (mid_code[i]->op != LABEL
							&& mid_code[i]->op != FUNC) {
							mid_code[i]->op = NOP;
						}
						else
							break;
					}
				}
			}
			break;
		case GEQ:
			if ((isdigit(code->a[0]) || code->a[0] == '-')
				&& (isdigit(code->b[0]) || code->b[0] == '-')) {
				if (atoi(code->a) >= atoi(code->b)) {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = NOP;
				}
				else {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = GOTO;
					for (i = i + 2; i < length_mid_code; i++) {
						if (mid_code[i]->op != LABEL
							&& mid_code[i]->op != FUNC) {
							mid_code[i]->op = NOP;
						}
						else
							break;
					}
				}
			}
			break;
		case EQL:
			if ((isdigit(code->a[0]) || code->a[0] == '-')
				&& (isdigit(code->b[0]) || code->b[0] == '-')) {
				if (atoi(code->a) == atoi(code->b)) {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = NOP;
				}
				else {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = GOTO;
					for (i = i + 2; i < length_mid_code; i++) {
						if (mid_code[i]->op != LABEL
							&& mid_code[i]->op != FUNC) {
							mid_code[i]->op = NOP;
						}
						else
							break;
					}
				}
			}
			break;
		case NEQ:
			if ((isdigit(code->a[0]) || code->a[0] == '-')
				&& (isdigit(code->b[0]) || code->b[0] == '-')) {
				if (atoi(code->a) != atoi(code->b)) {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = NOP;
				}
				else {
					mid_code[i]->op = NOP;
					mid_code[i + 1]->op = GOTO;
					for (i = i + 2; i < length_mid_code; i++) {
						if (mid_code[i]->op != LABEL
							&& mid_code[i]->op != FUNC) {
							mid_code[i]->op = NOP;
						}
						else
							break;
					}
				}
			}
			break;
		}
	}
	for (i = 0; i < length_mid_code; i++) {
		four_code code = mid_code[i];
		switch (code->op) {
		case GEI:
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case ASN:
			if (is_global(code->a)) {
				break;
			}
			is_used = 0;
			for (k = j; k < length_mid_code; k++) {
				if (k == i)	continue;
				if (mid_code[k]->op == FUNC) break;
				switch (mid_code[k]->op) {
				case GEI:
					is_used |= strcmp(mid_code[k]->c, code->a) == 0;
					break;
				case AEI:
					is_used |= strcmp(mid_code[k]->b, code->a) == 0 || strcmp(mid_code[k]->c, code->a) == 0;
					break;
				case ADD:
				case SUB:
				case MUL:
				case DIV:
					is_used |= strcmp(mid_code[k]->b, code->a) == 0 || strcmp(mid_code[k]->c, code->a) == 0;
					break;
				case ASN:
					is_used |= strcmp(mid_code[k]->b, code->a) == 0;
					break;
				case LES:
				case LEQ:
				case GTR:
				case GEQ:
				case EQL:
				case NEQ:
					is_used |= strcmp(mid_code[k]->a, code->a) == 0 || strcmp(mid_code[k]->b, code->a) == 0;
					break;
				case RET:
					if (mid_code[k]->a != NULL) {
						is_used |= strcmp(mid_code[k]->a, code->a) == 0;
					}
					break;
				case PUSH:
				case PRINT:
					is_used |= strcmp(mid_code[k]->a, code->a) == 0;
					break;
				default:
					break;
				}
			}
			if (!is_used) {
				printf_s("DELETE:\t%s %s %s\n", code->a, code->b, code->c);
				code->op = NOP;
			}
			break;
		case FUNC:
			j = i + 1;
			break;
		case PARA:
		case CONST:
		case VAR:
		case ARRAY:
			j++;
			break;
		default:
			break;
		}
	}
	arrange_mid_code();
}

/*
	不跨基本块复写传播
*/
int rewrite_lite() {
	return 0;
}