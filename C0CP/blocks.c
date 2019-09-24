#include"blocks.h"
#include"error.h"
#include"symbol_table.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define ENTER_DEF	0
#define ENTER_USE	1

FILE* out;

/*
	数据流分析
*/
func_blk func_blk_list[SIZE_BLOCK];			// 函数块列表
int length_func_blk;						// 函数块数量
func_blk current_func_blk = NULL;			// 当前函数块

/*
	冲突图
*/
char* name_list[SIZE_CONFLICT + 1];							// 冲突图变量名索引
int conflict_matrix[SIZE_CONFLICT + 1][SIZE_CONFLICT + 1];	// 冲突图邻接矩阵
int size_conflict = 0;										// 冲突图大小

void print_four_code_in_block(four_code code) {
	switch (code->op) {
	case ADD:
		fprintf_s(out, "CODE:%d\t%s = %s + %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case SUB:
		fprintf_s(out, "CODE:%d\t%s = %s - %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case MUL:
		fprintf_s(out, "CODE:%d\t%s = %s * %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case DIV:
		fprintf_s(out, "CODE:%d\t%s = %s / %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case GEI:
		fprintf_s(out, "CODE:%d\t%s = %s [ %s ]\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case AEI:
		fprintf_s(out, "CODE:%d\t%s [%s] = %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case ASN:
		fprintf_s(out, "CODE:%d\t%s = %s\n", code->is_break_point, code->a, code->b);
		break;
	case LES:
		fprintf_s(out, "CODE:%d\t%s < %s\n", code->is_break_point, code->a, code->b);
		break;
	case LEQ:
		fprintf_s(out, "CODE:%d\t%s <= %s\n", code->is_break_point, code->a, code->b);
		break;
	case GTR:
		fprintf_s(out, "CODE:%d\t%s > %s\n", code->is_break_point, code->a, code->b);
		break;
	case GEQ:
		fprintf_s(out, "CODE:%d\t%s >= %s\n", code->is_break_point, code->a, code->b);
		break;
	case EQL:
		fprintf_s(out, "CODE:%d\t%s == %s\n", code->is_break_point, code->a, code->b);
		break;
	case NEQ:
		fprintf_s(out, "CODE:%d\t%s != %s\n", code->is_break_point, code->a, code->b);
		break;
	case LABEL:
		fprintf_s(out, "CODE:%d\t%s :\n", code->is_break_point, code->a);
		break;
	case GOTO:
		fprintf_s(out, "CODE:%d\tGOTO %s\n", code->is_break_point, code->a);
		break;
	case BZ:
		fprintf_s(out, "CODE:%d\tBZ %s\n", code->is_break_point, code->a);
		break;
	case FUNC:
		fprintf_s(out, "CODE:%d\tFUNC %s %s\n", code->is_break_point, code->a, code->b);
		break;
	case PARA:
		fprintf_s(out, "CODE:%d\tPARA %s %s\n", code->is_break_point, code->a, code->b);
		break;
	case PUSH:
		fprintf_s(out, "CODE:%d\tPUSH %s\n", code->is_break_point, code->a);
		break;
	case CALL:
		fprintf_s(out, "CODE:%d\tCALL %s\n", code->is_break_point, code->a);
		break;
	case RET:
		if (code->a != NULL) fprintf_s(out, "CODE:%d\tRET %s\n", code->is_break_point, code->a);
		else fprintf_s(out, "CODE:%d\tRET\n", code->is_break_point);
		break;
	case CONST:
		fprintf_s(out, "CODE:%d\tCONST %s %s %s\n", code->is_break_point, code->a, code->b, code->c);
		break;
	case VAR:
		fprintf_s(out, "CODE:%d\tVAR %s %s\n", code->is_break_point, code->a, code->b);
		break;
	case ARRAY:
		fprintf_s(out, "CODE:%d\tARRAY %s %s\n", code->is_break_point, code->a, code->b);
		break;
	case READ:
		fprintf_s(out, "CODE:%d\tREAD %s\n", code->is_break_point, code->a);
		break;
	case PRINT:
		fprintf_s(out, "CODE:%d\tPRINT %s %s\n", code->is_break_point, code->a, code->b);
		break;
	default:
		break;
	}
}

/*
	打印分块信息
*/
void print_block() {
	if (fopen_s(&out, "blocks.txt", "w") != 0) {
		error_stand(FILE_NOT_FOUND);
		// 无法打开文件，结束运行。
	}
	int func_pointer;
	for (func_pointer = 0; func_pointer < length_func_blk; func_pointer++) {
		fprintf_s(out, "FUNC:\t%s\n", func_blk_list[func_pointer]->func_name);
		int blk_pointer;
		for (blk_pointer = 0; blk_pointer < func_blk_list[func_pointer]->sum_block; blk_pointer++) {
			fprintf_s(out, "BLK:\t%d\n", blk_pointer);
			block blk = func_blk_list[func_pointer]->block_list[blk_pointer];
			int pointer;
			fprintf_s(out, "PREV:\t");
			for (pointer = 0; pointer < blk->sum_prev; pointer++) {
				fprintf_s(out, "%d ", blk->prev[pointer]);
			}
			fprintf_s(out, "\n");
			fprintf_s(out, "NEXT:\t");
			for (pointer = 0; pointer < blk->sum_next; pointer++) {
				fprintf_s(out, "%d ", blk->next[pointer]);
			}
			fprintf_s(out, "\n");
			fprintf_s(out, "DEF:\n");
			for (pointer = 0; pointer < blk->live_var->sum_def; pointer++) {
				fprintf_s(out, "\t%d:\t%s\n", pointer + 1, blk->live_var->def[pointer]);
			}
			fprintf_s(out, "USE:\n");
			for (pointer = 0; pointer < blk->live_var->sum_use; pointer++) {
				fprintf_s(out, "\t%d:\t%s\n", pointer + 1, blk->live_var->use[pointer]);
			}
			fprintf_s(out, "IN:\n");
			for (pointer = 0; pointer < blk->live_var->sum_in; pointer++) {
				fprintf_s(out, "\t%d:\t%s\n", pointer + 1, blk->live_var->in[pointer]);
			}
			fprintf_s(out, "OUT:\n");
			for (pointer = 0; pointer < blk->live_var->sum_out; pointer++) {
				fprintf_s(out, "\t%d:\t%s\n", pointer + 1, blk->live_var->out[pointer]);
			}
			for (pointer = blk->first_index;
				pointer <= blk->last_index;
				pointer++) {
				print_four_code_in_block(mid_code[pointer]);
			}
			fprintf_s(out, "*----------------------------------------------------------*\n");
		}
		fprintf_s(out, "*************************************************************\n");
	}
	fclose(out);
}

/*
	初始化基本块
*/
int create_block(block *new_blk_ptr) {
	block new_blk = (block)malloc(sizeof(block_obj));
	new_blk->label = (char*)malloc(sizeof(char) * 10);
	new_blk->first_index = 0;
	new_blk->last_index = 0;
	new_blk->prev = NULL;
	new_blk->next = NULL;
	new_blk->sum_prev = 0;
	new_blk->sum_next = 0;
	// 初始化活跃变量记录
	new_blk->live_var = (live_var_info)malloc(sizeof(live_var_info_obj));
	new_blk->live_var->def = (char**)malloc(sizeof(char*) * 10000);
	new_blk->live_var->use = (char**)malloc(sizeof(char*) * 10000);
	new_blk->live_var->in = (char**)malloc(sizeof(char*) * 10000);
	new_blk->live_var->out = (char**)malloc(sizeof(char*) * 10000);
	new_blk->live_var->sum_def = 0;
	new_blk->live_var->sum_use = 0;
	new_blk->live_var->sum_in = 0;
	new_blk->live_var->sum_out = 0;
	*new_blk_ptr = new_blk;
	return 0;
}

/*
	初始化函数块
*/
int create_func_block(char *name, func_blk *new_func_blk_ptr) {
	func_blk new_func_blk = (func_blk)malloc(sizeof(func_blk_obj));
	new_func_blk->func_name = (char*)malloc(sizeof(char )* strlen(name));
	strcpy(new_func_blk->func_name, name);
	new_func_blk->block_list = (block*)malloc(sizeof(block_obj) * SIZE_BLOCK);
	create_block(&(new_func_blk->block_list[0]));							// 入口基本块
	strcpy(new_func_blk->block_list[0]->label, "Entrance");
	new_func_blk->block_list[0]->first_index = -1;
	new_func_blk->block_list[0]->last_index = -2;
	create_block(&(new_func_blk->block_list[1]));							// 出口基本块
	strcpy(new_func_blk->block_list[1]->label, "Exit");
	new_func_blk->block_list[1]->first_index = -1;
	new_func_blk->block_list[1]->last_index = -2;
	new_func_blk->sum_block = 2;
	*new_func_blk_ptr = new_func_blk;
	return 0;
}

/*
	将中间代码转换为基本块
*/
int create_blocks() {
	int mid_code_pointer;
	int have_start = -1;
	int have_label = 0;
	char label[10];
	// 遍历中间代码
	for (mid_code_pointer = 0; mid_code_pointer < length_mid_code; mid_code_pointer++) {
		if (mid_code[mid_code_pointer]->is_break_point == IGNORE) {								// 忽略中间代码
			continue;
		}
		else if (mid_code[mid_code_pointer]->is_break_point == FUNCSTART) {						// 进入新函数块
			func_blk new_func_blk;
			create_func_block(mid_code[mid_code_pointer]->b, &new_func_blk);
			current_func_blk = new_func_blk;
			func_blk_list[length_func_blk++] = new_func_blk;
			if (mid_code[mid_code_pointer + 1]->is_break_point == DEFPARA) {
				new_func_blk->block_list[0]->first_index = mid_code_pointer + 1;
				while (mid_code[mid_code_pointer + 1]->is_break_point == DEFPARA) {				// 添加入口块定义点
					mid_code_pointer++;
				}
				new_func_blk->block_list[0]->last_index = mid_code_pointer;
			}
		}
		else if (mid_code[mid_code_pointer]->is_break_point == NORMAL && have_start == -1) {	// 识别基本块起始代码
			have_start = mid_code_pointer;
			if (mid_code[mid_code_pointer]->op == LABEL) {
				have_label = 1;
				strcpy(label, mid_code[mid_code_pointer]->a);
			}
			continue;
		}
		else if (mid_code[mid_code_pointer]->is_break_point == BLKEND && have_start != -1) {	// 识别基本块结尾，生成基本块
			block new_blk;
			create_block(&new_blk);
			new_blk->first_index = have_start;
			new_blk->last_index = mid_code_pointer;
			strcpy(new_blk->label, have_label ? label : "");									// 若果有label，使用label标记基本块
			have_label = 0;
			have_start = -1;
			current_func_blk->block_list[current_func_blk->sum_block++] = new_blk;				// 将基本块添加至当前函数块
		}
		else if (mid_code[mid_code_pointer]->is_break_point == BLKEND && have_start == -1) {
			block new_blk;
			create_block(&new_blk);
			new_blk->first_index = mid_code_pointer;
			new_blk->last_index = mid_code_pointer;
			strcpy(new_blk->label, "");									
			have_label = 0;
			have_start = -1;
			current_func_blk->block_list[current_func_blk->sum_block++] = new_blk;				// 将基本块添加至当前函数块
		}
	}
	return 0;
}
/*
	搜索后继基本块
*/
int search_next(func_blk func, int index) {
	block prev = func->block_list[index];
	four_code code = mid_code[prev->last_index];
	int pointer;
	if (index == 2) {										// 首个基本块前驱入口基本块
		func->block_list[0]
			->next[func->block_list[0]->sum_next++] = index;
		prev->prev[prev->sum_prev++] = 0;
	}
	if (code->op == GOTO) {
		// 无条件跳转语句
	}
	else if (code->op == BZ) {		// 选择跳转添加后一基本块为后继基本块
		if (index < func->sum_block - 1) {					
			prev->next[prev->sum_next++] = index + 1;
			func->block_list[index + 1]
				->prev[func->block_list[index + 1]->sum_prev++] = index;
		}
	}
	else if (code->op == RET) {		// 返回语句后继出口基本块	
		func->block_list[1]
			->prev[func->block_list[1]->sum_prev++] = index;
		prev->next[prev->sum_next++] = 1;
		return 0;
	}
	else {
		if (index < func->sum_block - 1) {					// 普通语句后继基本块只有后一基本块
			prev->next[prev->sum_next++] = index + 1;
			func->block_list[index + 1]
				->prev[func->block_list[index + 1]->sum_prev++] = index;
		}
		return 0;
	}
	// 遍历函数内基本块，搜索跳转目标基本块
	for (pointer = 2; pointer < func->sum_block; pointer++) {
		if (mid_code[func->block_list[pointer]->first_index]->op == LABEL
			&& strcmp(mid_code[func->block_list[pointer]->first_index]->a,
				mid_code[prev->last_index]->a) == 0) {
			if (mid_code[prev->last_index]->op != GOTO && pointer == index + 1) {
				continue;
			}
			prev->next[prev->sum_next++] = pointer;
			func->block_list[pointer]
				->prev[func->block_list[pointer]->sum_prev++] = index;
		}
	}
	return 0;
}

/*
	标记基本块前驱后继
*/
int mark_blocks() {
	int func_pointer;
	// 遍历函数块
	for (func_pointer = 0; func_pointer < length_func_blk; func_pointer++) {
		int blk_pointer;
		for (blk_pointer = 0; blk_pointer < func_blk_list[func_pointer]->sum_block; blk_pointer++) {
			block current_block = func_blk_list[func_pointer]->block_list[blk_pointer];
			current_block->prev = (int*)malloc(sizeof(int) * func_blk_list[func_pointer]->sum_block);
			current_block->next = (int*)malloc(sizeof(int) * func_blk_list[func_pointer]->sum_block);
		}
		if (func_blk_list[func_pointer]->sum_block == 2) {							// 无基本块
			func_blk_list[func_pointer]->block_list[0]->next[func_blk_list[func_pointer]->block_list[0]->sum_next++] = 1;
			func_blk_list[func_pointer]->block_list[1]->prev[func_blk_list[func_pointer]->block_list[0]->sum_prev++] = 0;
			continue;
		}
		for (blk_pointer = 2; blk_pointer < func_blk_list[func_pointer]->sum_block; blk_pointer++) {
			search_next(func_blk_list[func_pointer], blk_pointer);
		}
	}
	return 0;
}

/*
	分块
*/
int build_blocks() {
	//TODO:分块
	int i;
	for (i = 0; i < length_mid_code; i++) {
		insert_break_point(i);
	}
	length_func_blk = 0;
	current_func_blk = NULL;
	create_blocks();
	mark_blocks();
	return 0;
}

/*
	添加至use集合或def集合
	0 ------ def集合
	1 ------ use集合
*/
int enter_use_def(int mode, char* new_var, live_var_info live_var) {
	int use_pointer;
	int def_pointer;
	if ((new_var[0] == 's' && new_var[1] == '$') || isdigit(new_var[0]) || new_var[0] == '-') {		// 忽略临时变量和常数
		return -1;
	}
	for (use_pointer = 0; use_pointer < live_var->sum_use; use_pointer++) {		// 检查是否以存在于use集合
		if (strcmp(new_var, live_var->use[use_pointer]) == 0) {
			return -1;
		}
	}
	for (def_pointer = 0; def_pointer < live_var->sum_def; def_pointer++) {		// 检查是否以存在于def集合
		if (strcmp(new_var, live_var->def[def_pointer]) == 0) {
			return -1;
		}
	}
	if (mode) {																	// 加入至use集合
		live_var->use[live_var->sum_use] = (char*)malloc(sizeof(char) * (strlen(new_var) + 1));
		strcpy(live_var->use[live_var->sum_use], new_var);
		live_var->sum_use++;
	}
	else {																		// 加入至def集合
		live_var->def[live_var->sum_def] = (char*)malloc(sizeof(char) * (strlen(new_var) + 1));
		strcpy(live_var->def[live_var->sum_def], new_var);
		live_var->sum_def++;
	}
	return 0;
}

/*
	生成use集合、def集合
*/
int gen_use_def() {
	int func_pointer;
	for (func_pointer = 0; func_pointer < length_func_blk; func_pointer++) {
		int blk_pointer;
		for (blk_pointer = 0; blk_pointer < func_blk_list[func_pointer]->sum_block; blk_pointer++) {
			block blk = func_blk_list[func_pointer]->block_list[blk_pointer];
			int mid_code_pointer;
			for (mid_code_pointer = blk->first_index; mid_code_pointer <= blk->last_index; mid_code_pointer++) {	// 遍历块内代码
				four_code code = mid_code[mid_code_pointer];
				switch (code->op) {
				case GEI:
					enter_use_def(ENTER_DEF, code->a, blk->live_var);
					enter_use_def(ENTER_USE, code->c, blk->live_var);
					break;
				case AEI:
					enter_use_def(ENTER_USE, code->b, blk->live_var);
					enter_use_def(ENTER_USE, code->c, blk->live_var);
					break;
				case ADD:
				case SUB:
				case MUL:
				case DIV:
					enter_use_def(ENTER_DEF, code->a, blk->live_var);
					enter_use_def(ENTER_USE, code->b, blk->live_var);
					enter_use_def(ENTER_USE, code->c, blk->live_var);
					break;
				case ASN:
					enter_use_def(ENTER_DEF, code->a, blk->live_var);
					enter_use_def(ENTER_USE, code->b, blk->live_var);
					break;
				case LES:
				case LEQ:
				case GTR:
				case GEQ:
				case EQL:
				case NEQ:
					enter_use_def(ENTER_USE, code->a, blk->live_var);
					enter_use_def(ENTER_USE, code->b, blk->live_var);
					break;
				case CALL:
					enter_use_def(ENTER_DEF, "$RET", blk->live_var);
					break;
				case RET:
					if (code->a != NULL) {
						enter_use_def(ENTER_USE, code->a, blk->live_var);
					}
					break;
				case PUSH:
				case PRINT:
					enter_use_def(ENTER_USE, code->a, blk->live_var);
					break;
				case READ:
					enter_use_def(ENTER_DEF, code->a, blk->live_var);
					break;
				case PARA:
					enter_use_def(ENTER_DEF, code->b, blk->live_var);
					break;
				case LABEL:
				case GOTO:
				case BZ:
				default:
					break;
				}
			}
			int pointer;
			for (pointer = 0; pointer < blk->live_var->sum_use; pointer++) {										// 将use集合加入in集合
				blk->live_var->in[pointer] = (char*)malloc(sizeof(char) * strlen(blk->live_var->use[pointer]));
				strcpy(blk->live_var->in[pointer], blk->live_var->use[pointer]);
				blk->live_var->sum_in++;
			}
		}
	}
	return 0;
}

/*
	计算活跃变量in集合和out集合
	ret:
		0 --- 无改变
		1 --- 存在改变
*/
int refresh_live(func_blk func) {
	int is_change = 0;
	int index;
	for (index = 0; index < func->sum_block; index++) {
		block blk = func->block_list[index];
		// 刷新out集合
		int blk_pointer;
		for (blk_pointer = 0; blk_pointer < blk->sum_next; blk_pointer++) {		// 遍历后继基本块
			int i;
			block next_blk = func->block_list[blk->next[blk_pointer]];
			for (i = 0; i < next_blk->live_var->sum_in; i++) {					// 遍历后继基本块的in集合
				int is_in = 0;
				int j;
				for (j = 0; j < blk->live_var->sum_out; j++) {					// 遍历本基本块的out集合
					if (strcmp(blk->live_var->out[j], next_blk->live_var->in[i]) == 0) {	// 已存在
						is_in = 1;
						break;
					}
				}
				if (!is_in) {													// 添加至out集
					is_change = 1;
					blk->live_var->out[blk->live_var->sum_out] = (char*)malloc(sizeof(char) * strlen(next_blk->live_var->in[i]));
					strcpy(blk->live_var->out[blk->live_var->sum_out], next_blk->live_var->in[i]);
					blk->live_var->sum_out++;
				}
			}
		}
		// 计算in集合
		int i;
		for (i = 0; i < blk->live_var->sum_out; i++) {							// 遍历out集合
			int j;
			int is_in_def = 0;
			for (j = 0; j < blk->live_var->sum_def; j++) {						// 遍历def集合
				if (strcmp(blk->live_var->out[i], blk->live_var->def[j]) == 0) {	// 在def集合中存在
					is_in_def = 1;
					break;
				}
			}
			if (!is_in_def) {
				int is_in = 0;
				for (j = 0; j < blk->live_var->sum_in; j++) {					// 遍历in集合
					if (strcmp(blk->live_var->out[i], blk->live_var->in[j]) == 0) {	// 在in集合中存在
						is_in = 1;
					}
				}
				if (!is_in) {													// 添加至in集合
					is_change = 1;
					blk->live_var->in[blk->live_var->sum_in] = (char*)malloc(sizeof(char) * strlen(blk->live_var->out[i]));
					strcpy(blk->live_var->in[blk->live_var->sum_in], blk->live_var->out[i]);
					blk->live_var->sum_in++;
				}
			}
		}
	}
	return is_change;
}

int gen_live() {
	int func_pointer;
	for (func_pointer = 0; func_pointer < length_func_blk; func_pointer++) {
		while(refresh_live(func_blk_list[func_pointer])){
			//printf_s("!\n");
		}
		//printf_s("FINISH - %s\n", func_blk_list[func_pointer]->func_name);
	}
	return 0;
}

/*
	活跃变量分析
*/
int live_variable_analysis() {
	gen_use_def();
	gen_live();
	return 0;
}

/*
	初始化冲突图
*/
void init_conflict() {
	int i, j;
	for (i = 0; i < size_conflict; i++) {
		name_list[i] = NULL;
		for (j = 0; j < size_conflict; j++) {
			conflict_matrix[i][j] = 0;
		}
	}
	size_conflict = 0;
}

/*
	增加冲突
*/
int add_conflict(char* var_1, char* var_2, char* func_name) {
	int index_1 = -1, index_2 = -1;
	int i;
	if (isdigit(var_1[0]) || isdigit(var_2[0]) || var_1[0] == '-' || var_2[0] == '-'
		|| strcmp("$RET", var_1) == 0
		|| strcmp("$RET", var_2) == 0
		|| (strlen(var_1) > 1 && var_1[0] == 's' && var_1[1] == '$')
		|| (strlen(var_2) > 1 && var_2[0] == 's' && var_2[1] == '$')
		|| !is_out(var_1, func_name)
		|| !is_out(var_2, func_name)
		|| is_global(var_1)
		|| is_global(var_2)) {
		return 0;
	}
	for (i = 0; i < size_conflict; i++) {									// 搜索变量索引值
		if (index_1 == -1 && strcmp(var_1, name_list[i]) == 0) {
			index_1 = i;
		}
		if (index_2 == -1 && strcmp(var_2, name_list[i]) == 0) {
			index_2 = i;
		}
	}
	if (index_1 == -1) {													// 增加新结点
		index_1 = size_conflict;
		name_list[size_conflict] = (char*)malloc(sizeof(char) * strlen(var_1));
		strcpy(name_list[size_conflict], var_1);
		for (i = 0; i < size_conflict; i++) {
			conflict_matrix[i][size_conflict] = 0;
			conflict_matrix[size_conflict][i] = 0;
		}
		conflict_matrix[size_conflict][size_conflict] = 0;
		size_conflict++;
	}
	if (index_2 == -1) {
		index_2 = size_conflict;
		name_list[size_conflict] = (char*)malloc(sizeof(char) * strlen(var_2));
		strcpy(name_list[size_conflict], var_2);
		for (i = 0; i < size_conflict; i++) {
			conflict_matrix[i][size_conflict] = 0;
			conflict_matrix[size_conflict][i] = 0;
		}
		conflict_matrix[size_conflict][size_conflict] = 0;
		size_conflict++;
	}
	conflict_matrix[index_1][index_2] = 1;									// 增加新边
	conflict_matrix[index_2][index_1] = 1;
	printf_s("ADD:\t%s-----%s\n", var_1, var_2);
	return 0;
}

/*
	消除冲突
*/
int delete_conflict(char* var_1, char* var_2) {
	int index_1 = -1, index_2 = -1;
	int i;
	for (i = 0; i < size_conflict; i++) {									// 搜索变量索引值
		if (index_1 == -1 && strcmp(var_1, name_list[i]) == 0) {
			index_1 = i;
		}
		if (index_2 == -1 && strcmp(var_2, name_list[i]) == 0) {
			index_2 = i;
		}
	}
	if (index_1 == -1 || index_2 == -1) {
		return 0;
	}
	conflict_matrix[index_1][index_2] = 0;									// 删除已有边
	conflict_matrix[index_2][index_1] = 0;
	return 0;
}

/*
	检查是否为跨块变量
*/
int is_out(char *name, char* func_name) {
	int i, j;
	func_blk func = NULL;
	for (i = 0; i < length_func_blk; i++) {
		if (strcmp(func_name, func_blk_list[i]->func_name) == 0) {
			func = func_blk_list[i];
			break;
		}
	}
	for (i = 0; func != NULL &&  i < func->sum_block; i++) {
		block blk = func->block_list[i];
		for (j = 0; j < blk->live_var->sum_out; j++) {
			if (strcmp(name, blk->live_var->out[j]) == 0) {
				return 1;
			}
		}
	}
	
	return 0;
}

int is_def(char *name, char* func_name) {
	int i, j;
	func_blk func = NULL;
	for (i = 0; i < length_func_blk; i++) {
		if (strcmp(func_name, func_blk_list[i]->func_name) == 0) {
			func = func_blk_list[i];
			break;
		}
	}
	for (i = 0; func != NULL && i < func->sum_block; i++) {
		block blk = func->block_list[i];
		for (j = 0; j < blk->live_var->sum_out; j++) {
			if (strcmp(name, blk->live_var->out[j]) == 0) {
				return 1;
			}
		}
	}

	return 0;
}

void print_conflict() {
	int i, j;
	printf_s("CONFLICT_NODE:\n");
	for (i = 0; i < size_conflict; i++) {
		printf_s("\t%s\n", name_list[i]);
	}
	printf_s("CONFLICT:\n");
	for (i = 0; i < size_conflict; i++) {
		for (j = 0; j < size_conflict; j++) {
			if (conflict_matrix[i][j] == 1 && i < j) {
				printf_s("\t%s\t%s\n", name_list[i], name_list[j]);
			}
		}
	}
}

/*
	识别冲突
*/
int gen_conflict_in(char* func_name) {
	int mid_code_pointer, i, j, sum_var = 0, blk_pointer;
	func_blk func = NULL;
	char* var_list[SIZE_CONFLICT];
	int is_active[SIZE_CONFLICT] = { 0 };
	init_conflict();
	for (i = 0; i < length_func_blk; i++) {																		// 获取目标函数快
		if (strcmp(func_name, func_blk_list[i]->func_name) == 0) {
			func = func_blk_list[i];
			break;
		}
	}
	if (func->sum_block <= 3) {
		return 0;
	}
	for (i = 0; i < func->block_list[0]->live_var->sum_out - 1; i++) {
		for (j = i + 1; j < func->block_list[0]->live_var->sum_out; j++) {
			add_conflict(func->block_list[0]->live_var->out[i], func->block_list[0]->live_var->out[j], func_name);
		}
	}
	for (blk_pointer = 2; func != NULL && blk_pointer < func->sum_block; blk_pointer++) {						// 遍历函数快内的基本块
		block blk = func->block_list[blk_pointer];
		sum_var = 0;
		for (i = 0; i < blk->live_var->sum_out; i++) {															// 设置out集合活跃变量初值
			var_list[sum_var] = (char*)malloc(sizeof(char) * strlen(blk->live_var->out[i]));
			strcpy(var_list[sum_var], blk->live_var->out[i]);
			is_active[sum_var] = 1;
			sum_var++;
		}
		for (mid_code_pointer = blk->last_index; mid_code_pointer >= blk->first_index; mid_code_pointer--) {	// 反向遍历基本块代码
			four_code code = mid_code[mid_code_pointer];
			int exist_1 = 0, exist_2 = 0;
			switch (code->op) {																					// 更新冲突
			case READ:
			case GEI:
			case ADD:
			case SUB:
			case MUL:
			case DIV:
			case ASN:
				if (is_out(code->a, func->func_name)) {																		// 检查定义点变量是否跨块
					for (i = 0; i < sum_var; i++) {																// 将当前活跃变量加入冲突图
						if (is_active[i] == 1 && strcmp(code->a, var_list[i]) != 0) {
							add_conflict(code->a, var_list[i], func_name);
						}
					}
				}
				for (i = 0; i < sum_var; i++) {																	// 更新活跃情况
					if (strcmp(code->a, var_list[i]) == 0) {
						is_active[i] = 0;
						break;
					}
				}
			}
			switch (code->op) {																					// 更新活跃情况
			case AEI:
			case ADD:
			case SUB:
			case MUL:
			case DIV:
				for (i = 0; i < sum_var; i++) {
					if (strcmp(code->b, var_list[i]) == 0) {
						is_active[i] = 1;
						exist_1 = 1;
					}
					if (strcmp(code->c, var_list[i]) == 0) {
						is_active[i] = 1;
						exist_2 = 1;
					}
				}
				if (!exist_1) {
					var_list[sum_var] = (char*)malloc(sizeof(char) * strlen(code->b));
					strcpy(var_list[sum_var], code->b);
					is_active[sum_var] = 1;
					sum_var++;
				}
				if (!exist_2) {
					var_list[sum_var] = (char*)malloc(sizeof(char) * strlen(code->c));
					strcpy(var_list[sum_var], code->c);
					is_active[sum_var] = 1;
					sum_var++;
				}
				break;
			case ASN:
				for (i = 0; i < sum_var; i++) {
					if (strcmp(code->b, var_list[i]) == 0) {
						is_active[i] = 1;
						exist_1 = 1;
						break;
					}
				}
				if (!exist_1) {
					var_list[sum_var] = (char*)malloc(sizeof(char) * strlen(code->b));
					strcpy(var_list[sum_var], code->b);
					is_active[sum_var] = 1;
					sum_var++;
				}
				break;
			case LES:
			case LEQ:
			case GTR:
			case GEQ:
			case EQL:
			case NEQ:
				for (i = 0; i < sum_var; i++) {
					if (strcmp(code->a, var_list[i]) == 0) {
						is_active[i] = 1;
						exist_1 = 1;
					}
					if (strcmp(code->b, var_list[i]) == 0) {
						is_active[i] = 1;
						exist_2 = 1;
					}
				}
				if (!exist_1) {
					var_list[sum_var] = (char*)malloc(sizeof(char) * strlen(code->a));
					strcpy(var_list[sum_var], code->a);
					is_active[sum_var] = 1;
					sum_var++;
				}
				if (!exist_2) {
					var_list[sum_var] = (char*)malloc(sizeof(char) * strlen(code->b));
					strcpy(var_list[sum_var], code->b);
					is_active[sum_var] = 1;
					sum_var++;
				}
				break;
			case PUSH:
			case PRINT:
			case RET:
				if (code->a == NULL) {
					break;
				}
				for (i = 0; i < sum_var; i++) {
					if (strcmp(code->a, var_list[i]) == 0) {
						is_active[i] = 1;
						exist_1 = 1;
						break;
					}
				}
				if (!exist_1) {
					var_list[sum_var] = (char*)malloc(sizeof(char) * strlen(code->a));
					strcpy(var_list[sum_var], code->a);
					is_active[sum_var] = 1;
					sum_var++;
				}
				break;
			case GEI:
				for (i = 0; i < sum_var; i++) {
					if (strcmp(code->c, var_list[i]) == 0) {
						is_active[i] = 1;
						exist_1 = 1;
						break;
					}
				}
				if (!exist_1) {
					var_list[sum_var] = (char*)malloc(sizeof(char) * strlen(code->c));
					strcpy(var_list[sum_var], code->c);
					is_active[sum_var] = 1;
					sum_var++;
				}
				break;
			}
		}
	}
	for (i = 0; i < size_conflict; i++) {
		conflict_matrix[i][i] = 0;
	}
	print_conflict();
	return 0;
}

