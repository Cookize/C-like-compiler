#include"mips_generator.h"
#include"mips_generator_opt.h"
#include"blocks.h"
#include"symbol_table.h"
#include"error.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define SUM_TEMP_REG		8		
#define TEMPREG(x)			temp_regs_opt[x]
#define SUM_GLOBAL_REG		11
#define GLOBALREG(x)		global_regs_opt[x]

extern tbl table;
extern stbl str_table;
extern four_code mid_code[SIZE_MID_CODE];
extern int length_mid_code;

int mid_code_pointer_opt = 0;										// 目标代码指针
FILE *mips_out_opt;													// 输出文件
tbl_field current_field_opt;										// 当前的域

int lru_sum_opt = 0;
lru_item lru_list_opt[SUM_TEMP_REG] = { NULL };						// LRU链
char temp_regs_opt[SUM_TEMP_REG + 1][5] = {							// 临时寄存器池
	REG_t0, REG_t1, REG_t2, REG_t3, REG_t4, REG_t5, REG_t6, REG_t7, REG_v0
};
int temp_regs_opt_usage[SUM_TEMP_REG] = {							// 临时寄存器使用情况：0---未使用；1---驻留；2---临时使用；
	0, 0, 0, 0, 0, 0, 0, 0
};

char global_regs_opt[SUM_GLOBAL_REG][5] = {							// 全局寄存器池
	REG_s0, REG_s1, REG_s2, REG_s3, REG_s4, REG_s5, REG_s6, REG_s7, REG_a1, REG_a2, REG_a3
};
char* var_list_opt[SIZE_TBL];										// 分配到全局寄存器的变量名
int assign_opt[SIZE_TBL] = { 0 };									// 分配到的全局寄存器编号
int assign_opt_sum = 0;												// 分配到全局寄存器的变量数

int size_global = 0;											// 使用的全局寄存器数
int size_part = 0;												// 局部变量区大小


/*
	初始化
*/
void init_generator_opt() {
	mid_code_pointer_opt = 0;
	if (fopen_s(&mips_out_opt, "mips_opt.txt", "w") != 0) {
		// 无法打开文件，结束运行。
	}
}

/*
	释放资源
*/
void reset_generator_opt() {
	fclose(mips_out_opt);
}

void print_LRU_opt() {
	int i;
	printf("%d:\t", mid_code_pointer_opt);
	for (i = 0; i < SUM_TEMP_REG; i++) {
		printf("%d", temp_regs_opt_usage[i]);
	}
	printf("\n");
	return;
}

/*
	检查数字是否为2的幂次
*/
int is_lucky_num(int num) {
	int times = 0;
	while (num > 1) {
		times++;
		num /= 2;
	}
	if (num == 0) {
		return times;
	}
	return -1;
}

/*
	生成MPIS汇编代码
	TODO:修改输出格式
*/
void gen_mips_opt(char* op, char* reg_1, char* reg_2, char* reg_3) {
	if (strcmp(op, MIPS_LABEL) == 0) {
		fprintf(mips_out_opt, "%s:\n", reg_1);
		return;
	}
	else if (strcmp(op, MIPS_SW) == 0
		|| strcmp(op, MIPS_LW) == 0) {
		fprintf(mips_out_opt, "%s %s, %s(%s)\n", op, reg_1, reg_2, reg_3);
		return;
	}
	else if (strcmp(op, MIPS_ADD) == 0
		|| strcmp(op, MIPS_ADDI) == 0
		|| strcmp(op, MIPS_ADDU) == 0
		|| strcmp(op, MIPS_SUB) == 0
		|| strcmp(op, MIPS_SUBU) == 0
		|| strcmp(op, MIPS_SLL) == 0
		|| strcmp(op, MIPS_SRL) == 0
		|| strcmp(op, MIPS_MUL) == 0
		|| strcmp(op, MIPS_BEQ) == 0
		|| strcmp(op, MIPS_BGT) == 0
		|| strcmp(op, MIPS_BGE) == 0
		|| strcmp(op, MIPS_BLE) == 0
		|| strcmp(op, MIPS_BLT) == 0
		|| strcmp(op, MIPS_BNE) == 0) {
		fprintf(mips_out_opt, "%s %s, %s, %s\n", op, reg_1, reg_2, reg_3);
		return;
	}
	else if (strcmp(op, MIPS_DIV) == 0
		|| strcmp(op, MIPS_LA) == 0
		|| strcmp(op, MIPS_BGTZ) == 0
		|| strcmp(op, MIPS_BLEZ) == 0
		|| strcmp(op, MIPS_BGEZ) == 0
		|| strcmp(op, MIPS_BLTZ) == 0) {
		fprintf(mips_out_opt, "%s %s, %s\n", op, reg_1, reg_2);
		return;
	}
	else if (strcmp(op, MIPS_J) == 0
		|| strcmp(op, MIPS_JR) == 0
		|| strcmp(op, MIPS_JAL) == 0
		|| strcmp(op, MIPS_JALR) == 0
		|| strcmp(op, MIPS_MFLO) == 0) {
		fprintf(mips_out_opt, "%s %s\n", op, reg_1);
		return;
	}
	else if (strcmp(op, MIPS_CALL) == 0
		|| strcmp(op, MIPS_DATA) == 0
		|| strcmp(op, MIPS_TEXT) == 0) {
		fprintf(mips_out_opt, "%s\n", op);
		return;
	}
}

/*
	获取未被使用的寄存器
*/
int get_unused_temp_reg_opt() {
	int i;
	for (i = 0; i < SUM_TEMP_REG; i++) {
		if (temp_regs_opt_usage[i] == 0)	return i;
	}
	return -1;
}

/*
	获取变量和参量相对偏移
	获取数组首元素的相对偏移
	name -------- 名称
	index ------- 数组下标
	offset ------ 计算出的偏移
	reg --------- 相对的寄存器
*/
void get_offset_opt(char *name, char** offset, char** reg) {
	int index, counter = 0;
	char num[20];
	*offset = (char*)malloc(sizeof(char) * 20);
	*reg = (char*)malloc(sizeof(char) * 5);
	for (index = current_field_opt->first_item_loca; index <= current_field_opt->last_item_loca; index++) {		// 搜索局部符号表
		if (strcmp(table->item_list[index]->id, name) == 0) {											// 在局部域找到
			if (table->item_list[index]->kind == Para) {												// 参量
				sprintf(*offset, "%d", (current_field_opt->para_sum - table->item_list[index]->addr) * 4);	// 计算相对$sp的偏移
				strcpy(*reg, "$sp");
				return;
			}
			else if (table->item_list[index]->kind == Var) {										// 变量
				sprintf(*offset, "%d", -(counter) * 4);											// 计算相对$sp的偏移
				strcpy(*reg, "$sp");
				return;
			}
		}
		if (table->item_list[index]->kind == Var) {													// 数组
			table->item_list[index]->array_length > 0 ? counter += table->item_list[index]->array_length :
				counter++;
		}
	}
	if (name[0] == '$') {																			// 临时变量
		strcpy(num, name);
		num[0] = '0';
		num[1] = '0';
		sprintf(*offset, "%d", -(atoi(num) + counter) * 4);										// 计算相对$fp偏移
		strcpy(*reg, "$sp");
		return;
	}
	for (index = counter = 0; index <= table->field_list[0]->last_item_loca; index++) {
		if (strcmp(table->item_list[index]->id, name) == 0) {										// 在全局域找到
			if (table->item_list[index]->kind == Var) {												// 变量
				sprintf(*offset, "%d", (counter) * 4);												// 计算相对$gp的偏移
				strcpy(*reg, "$gp");
				return;
			}
		}
		if (table->item_list[index]->kind == Var) {													// 数组
			table->item_list[index]->array_length > 0 ? counter += table->item_list[index]->array_length :
				counter++;
		}
	}
	commit_error_mips_gen(MISSMATCH_VAR, current_field_opt->func_name, name);										// 查表失败错误
}

/*
	申请全局寄存器
*/
int alloc_reg_global_opt(char* name, char** reg) {
	int i;
	for (i = 0; i < assign_opt_sum; i++) {
		if (name, var_list_opt[i] != NULL && strcmp(name, var_list_opt[i]) == 0) {
			strcpy(*reg, GLOBALREG(assign_opt[i]));
			return 0;
		}
	}
	return -1;
}

/*
TODO：
	申请临时寄存器
	驻留策略：
		对于非数组变量和参量，申请临时寄存器时，会将其加入LRU驻留队列，在使用完成后无需释放；
		对于数组元素，以非驻留形式申请临时寄存器，使用完成后需要另外写回并释放寄存器；
		对于计算中间因子，以非驻留形式申请临时寄存器，使用完成后需要另外写回并释放寄存器；
*/
/*
	申请驻留寄存器（reg_ret返回寄存器名）：
		1.遍历LRU链，检查是否已经驻留
		2.若驻留，将其提前至链首
		3.若未驻留，且还有剩余寄存器，记录驻留情况，加入至链首
		3.若未驻留，且无生于寄存器，取链为记录，写回数据并记录新驻留情况，加入至链首
*/
void alloc_reg_resident_opt(int dirty, char* name, char** reg_ret) {
	lru_item temp;
	char *offset, *reg;
	int i, j;
	*reg_ret = (char*)malloc(sizeof(char) * 5);
	if (strcmp(name, "$RET") == 0) {
		strcpy(*reg_ret, TEMPREG(SUM_TEMP_REG));
		return;
	}
	if (alloc_reg_global_opt(name, reg_ret) == 0) return;
	for (i = 0; i < lru_sum_opt; i++) {											// 在LRU链中查找
		if (strcmp(lru_list_opt[i]->name, name) == 0) {							// 在LRU链中找到
			temp = lru_list_opt[i];
			for (j = i; j > 0; j--) {										// 提前至链首
				lru_list_opt[j] = lru_list_opt[j - 1];
			}
			lru_list_opt[0] = temp;
			strcpy(*reg_ret, TEMPREG(temp->reg));
			temp->dirty |= dirty;											// 更新脏位
			return;
		}
	}
	if ((i = get_unused_temp_reg_opt()) != -1) {								// 还有寄存器未分配
		temp = (lru_item)malloc(sizeof(lru_item_obj));
		temp->name = (char*)malloc(sizeof(char) * strlen(name));
		strcpy(temp->name, name);											// 设置name
		temp->reg = i;														// 设置reg
		temp->dirty = 0;
		temp_regs_opt_usage[temp->reg] = 1;										// 更新临时寄存器使用状态
		for (j = lru_sum_opt; j > 0; j--) {										// 加入链首
			lru_list_opt[j] = lru_list_opt[j - 1];
		}
		lru_list_opt[0] = temp;
		lru_sum_opt++;
	}
	else {																	// 无剩余寄存器
		temp = lru_list_opt[lru_sum_opt - 1];
		for (j = lru_sum_opt - 1; j > 0; j--) {									// 插入链首
			lru_list_opt[j] = lru_list_opt[j - 1];
		}
		lru_list_opt[0] = temp;
		if (temp->dirty == 1) {
			get_offset_opt(temp->name, &offset, &reg);
			gen_mips_opt(MIPS_SW, TEMPREG(temp->reg), offset, reg);					// 脏位写回
		}
		temp->name = (char*)malloc(sizeof(char) * strlen(name));			// 更新使用的变量名
		temp->dirty = 0;
		strcpy(temp->name, name);
	}
	get_offset_opt(temp->name, &offset, &reg);
	if (dirty != 1) {
		gen_mips_opt(MIPS_LW, TEMPREG(temp->reg), offset, reg);					// 加载
	}
	temp->dirty |= dirty;													// 设置脏位
	strcpy(*reg_ret, TEMPREG(temp->reg));
	return;
}

void print_assign() {
	int i; 
	printf_s("ASSIGN:\n");
	for (i = 0; i < assign_opt_sum; i++) {
		printf_s("\t%s\t%d\n", var_list_opt[i], assign_opt[i]);
	}
}

/*
	分配临时寄存器（着色算法）
*/
int assign_opt_global_reg_opt(char *func_name) {
	int remain_active, i, j, max_index, min_index;
	int div[SIZE_CONFLICT] = { 0 };
	int conflict_index[SIZE_TBL] = { -1 };
	gen_conflict_in(func_name);
	assign_opt_sum = 0;
	remain_active = size_conflict;
	for (i = 0; i < size_conflict; i++) {							// 统计
		for (j = 0; j < size_conflict; j++) {
			div[i] += conflict_matrix[i][j];
		}
	}
	while (remain_active > 0) {
		max_index = -1;
		min_index = -1;
		for (i = 0; i < size_conflict; i++) {
			if (div[i] == -1) {
				continue;
			}
			if (max_index == -1 || div[i] > div[max_index]) {
				max_index = i;
			}
			if (min_index == -1 || div[i] < div[min_index]) {
				min_index = i;
			}
		}
		if (div[min_index] < SUM_GLOBAL_REG) {													// 压入分配栈
			var_list_opt[assign_opt_sum] = (char*)malloc(sizeof(char) * strlen(name_list[min_index]));
			strcpy(var_list_opt[assign_opt_sum], name_list[min_index]);
			assign_opt[assign_opt_sum] = -1;
			conflict_index[assign_opt_sum] = min_index;
			assign_opt_sum++;
			printf_s("IN:\t%s\t%d\n", name_list[min_index], div[min_index]);
			div[min_index] = -1;
		}
		else {																				// 无结点压入分配栈
			printf_s("DLELTE:\t%s\t%d\n", name_list[max_index], div[max_index]);
			div[max_index] = -1;
			for (j = 0; j < assign_opt_sum; j++) {
				if (div[j] == -1)continue;
				if (conflict_matrix[j][max_index] == 1) {
					div[j] -= 1;
				}
			}
		}
		remain_active--;
	}
	for (i = assign_opt_sum - 1; i >= 0; i--) {													// 分配寄存器
		int reg_num = 0, is_conflict;
		 do {
			is_conflict = 0;
			for (j = i + 1; j < assign_opt_sum; j++) {												// 检查是否与已分配的冲突
				if (var_list_opt[j] == NULL)	continue;
				if (conflict_matrix[conflict_index[i]][conflict_index[j]] == 1 && assign_opt[j] == reg_num) {
					reg_num++;
					is_conflict = 1;
				}
			}
		 } while (is_conflict && reg_num < SUM_GLOBAL_REG);
		if (reg_num < SUM_GLOBAL_REG) {														// 有全局寄存器分配
			assign_opt[i] = reg_num;
			printf_s("ASSIGN:\t%s\t%d\n", var_list_opt[i], reg_num);
		}
		else {																				// 无全局寄存器分配
			printf_s("ABONDON:\t%s\n", var_list_opt[i]);
			var_list_opt[i] = NULL;
		}
	}
	//print_assign();
	return 0;
}

/*
	生成.data段
*/
void gen_data_opt() {
	int i;
	gen_mips_opt(MIPS_DATA, NULL, NULL, NULL);
	for (i = 0; i < str_table->sum; i++) {
		fprintf(mips_out_opt, "s$%d : .asciiz \"%s\"\n", i, str_table->strings[i]);
	}
}

/*
	初始化栈空间:
		1.置-4($sp)为$ra
		2.置-8($sp)为$fp
		3.置$fp为新栈帧起始地址
		4.保护需要使用的全局寄存器中的值
		4.计算局部变量、临时变量空间大小，置$sp为栈顶
*/
void gen_init_stack_opt() {
	char frame_size[20];
	int counter = 0, i, j, temp;
	for (i = current_field_opt->first_item_loca; i <= current_field_opt->last_item_loca; i++) {			// 计算局部变量大小
		if (table->item_list[i]->kind == Var) {
			table->item_list[i]->array_length > 0 ? counter += table->item_list[i]->array_length : counter++;
		}
	}
	size_part = counter;
	temp = -(counter + current_field_opt->temp_sum) * 4;											// 缓存$sp
	for (i = 0, counter = 0; i < SUM_GLOBAL_REG; i++) {												// 保存全局寄存器
		for (j = 0; j < assign_opt_sum; j++) {
			if (var_list_opt[j] != NULL && assign_opt[j] == i) {
				sprintf(frame_size, "%d", -4 * counter + temp);
				gen_mips_opt(MIPS_SW, GLOBALREG(i), frame_size, REG_sp);
				printf_s("SAVE:\t%d\n", i);
				counter++;
				break;
			}
		}
	}
	size_global = counter;
	for (i = 0; i < assign_opt_sum; i++) {
		if (var_list_opt[i][0] != '$') {
			char * offset, *reg, *aim;
			get_offset_opt(var_list_opt[i], &offset, &reg);
			if (atoi(offset) > 0) {
				alloc_reg_resident_opt(0, var_list_opt[i], &aim);
				gen_mips_opt(MIPS_LW, aim, offset, reg);					// 加载
			}
		}
	}
	return;
}

/*
	保存环境：
		1.扫描所有驻留的寄存器
		2.按照分配情况写回至相应的地址
*/
void gen_save_env_opt(int is_call) {
	int i;
	char *offset, *reg;
	//printf_s("WB:\t");
	for (i = 0; i < lru_sum_opt; i++) {
		if (lru_list_opt[i]->dirty == 1 && (is_call || is_out(lru_list_opt[i]->name, current_field_opt->func_name) || is_global(lru_list_opt[i]->name))) {
			get_offset_opt(lru_list_opt[i]->name, &offset, &reg);
			gen_mips_opt(MIPS_SW, TEMPREG(lru_list_opt[i]->reg), offset, reg);		// 脏位写回
			//printf_s("%d", lru_list_opt[i]->reg);
		}
		//free(lru_list_opt[i]);
	}
	//printf_s("\n");
	lru_sum_opt = 0;
	for (i = 0; i < SUM_TEMP_REG; i++) {
		temp_regs_opt_usage[i] = 0;
	}
	return;
}

/*
	函数声明：
		1.根据函数名设置当前域
		2.生成跳转标签
		3.main函数置$fp等于$sp
		4.main函数初始化栈帧操作
*/
void gen_func_def_opt(char *name) {
	int i;
	current_field_opt = find_field(name);															// 设置当前域
	printf_s("FUNC:\t%s\n", name);
	lru_sum_opt = 0;
	for (i = 0; i < SUM_TEMP_REG; i++) {
		temp_regs_opt_usage[i] = 0;
	}
	assign_opt_global_reg_opt(name);
	gen_mips_opt(MIPS_LABEL, name, NULL, NULL);														// 生成函数标签
	if (strcmp(name, "main") == 0) {																// main函数
//		gen_mips_opt(MIPS_ADDU, REG_fp, REG_sp, REG_0);												// 初始化$fp
	}
	gen_init_stack_opt();																			// 初始化栈帧
	return;
}

/*
	函数调用：
		1.保存现场
		2.初始化被调用函数的栈空间
*/
void gen_func_call_opt(char* name) {																			// 保存环境
	gen_mips_opt(MIPS_JAL, name, NULL, NULL);														// jal name
	return;
}

/*
	函数返回：
		1.有返回值函数将返回值传值$v0
		2.置$ra为返回地址
		3.置$sp为上一栈帧顶地址
		4.置$fp为上一栈帧起始地址
*/
void gen_return_opt(char* ret) {
	char num[10], *reg;
	int counter = 0, i, j;
	if (ret != NULL) {																				// 返回值传值
		if (isdigit(ret[0]) || ret[0] == '-') {
			gen_mips_opt(MIPS_ADDI, REG_v0, REG_0, ret);											// 返回常量值
		}
		else {
			alloc_reg_resident_opt(0, ret, &reg);
			gen_mips_opt(MIPS_ADDU, REG_v0, reg, REG_0);											// 返回名
		}
	}
	gen_save_env_opt(0);																				// 写回临时寄存器
	for (i = 0, counter = 0; i < SUM_GLOBAL_REG; i++) {												// 保存全局寄存器
		for (j = 0; j < assign_opt_sum; j++) {
			if (var_list_opt[j] != NULL && assign_opt[j] == i) {
				sprintf(num, "%d", -4 * (counter + size_part + current_field_opt->temp_sum));
				gen_mips_opt(MIPS_LW, GLOBALREG(i), num, REG_sp);
				printf_s("LOAD:\t%d\n", i);
				counter++;
				break;
			}
		}
	}
	gen_mips_opt(MIPS_JR, REG_ra, NULL, NULL);														// jr $ra
	return;
}

/*
	选择跳转：
		1.分别加载比较因子
		1.根据比较类型生成不满足跳转语句
*/
void gen_B_jump_opt(OP_FOUR_CODE compare, char* name_1, char* name_2, char* label) {
	char *reg_1, *reg_2;
	char num[20];
	int is_num = 1;
	if ((isdigit(name_1[0]) || name_1[0] == '-') && (isdigit(name_2[0]) || name_2[0] == '-')) {
		sprintf(num, "%d", atoi(name_2) - atoi(name_1));
		reg_1 = (char*)malloc(sizeof(char) * 5);
		reg_2 = (char*)malloc(sizeof(char) * 5);
		strcpy(reg_1, REG_0);
		strcpy(reg_2, num);
	}
	else if (isdigit(name_1[0]) || name_1[0] == '-') {
		alloc_reg_resident_opt(0, name_2, &reg_1);
		reg_2 = (char*)malloc(sizeof(char) * 5);
		strcpy(reg_2, name_1);
		if (compare == LES) {
			compare = GTR;
		}
		else if (compare == LEQ) {
			compare = GEQ;
		}
		else if (compare == GTR) {
			compare = LES;
		}
		else if(compare == GEQ) {
			compare = LEQ;
		}
	}
	else if (isdigit(name_2[0]) || name_2[0] == '-') {
		alloc_reg_resident_opt(0, name_1, &reg_1);
		reg_2 = (char*)malloc(sizeof(char) * 5);
		strcpy(reg_2, name_2);
	}
	else {
		alloc_reg_resident_opt(0, name_1, &reg_1);
		alloc_reg_resident_opt(0, name_2, &reg_2);
		is_num = 0;
	}
	gen_save_env_opt(0);
	switch (compare) {
	case LES:
		gen_mips_opt(MIPS_BGE, reg_1, reg_2, label);		// bge name_1 name_2 label
		break;
	case LEQ:
//		if (is_num) {
//			sprintf(reg_2, "%d", -atoi(reg_2));
//			gen_mips_opt(MIPS_ADDI, REG_t8, reg_1, reg_2);
//		}
//		else {
//			gen_mips_opt(MIPS_SUB, REG_t8, reg_1, reg_2);
//		}
//		gen_mips_opt(MIPS_BGTZ, REG_t8, label, NULL);
		gen_mips_opt(MIPS_BGT, reg_1, reg_2, label);		// bgt name_1 name_2 label
		break;
	case GTR:
//		if (is_num) {
//			sprintf(reg_2, "%d", -atoi(reg_2));
//			gen_mips_opt(MIPS_ADDI, REG_t8, reg_1, reg_2);
//		}
//		else {
//			gen_mips_opt(MIPS_SUB, REG_t8, reg_1, reg_2);
//		}
//		gen_mips_opt(MIPS_BLEZ, REG_t8, label, NULL);
		gen_mips_opt(MIPS_BLE, reg_1, reg_2, label);		// ble name_1 name_2 label
		break;
	case GEQ:
		gen_mips_opt(MIPS_BLT, reg_1, reg_2, label);		// blt name_1 name_2 label
		break;
	case EQL:
		gen_mips_opt(MIPS_BNE, reg_1, reg_2, label);		// bne name_1 name_2 label
		break;
	case NEQ:
		gen_mips_opt(MIPS_BEQ, reg_1, reg_2, label);		// beq name_1 name_2 label
		break;
	}
	return;
}

/*
	计算语句：
		1.根据计算种类生成不同代码
	TODO：数组越界检查
*/
void gen_cul_opt(OP_FOUR_CODE op, char* dst, char* rec_1, char* rec_2) {
	char *temp_reg_1, *temp_reg_2, *temp_reg_3;
	char *offset_1;
	char *reg_1;
	char num_1[20], num_2[20];
	switch (op) {
	case GEI:
		get_offset_opt(rec_1, &offset_1, &reg_1);															// 获取数组首元素的偏移
		if (isdigit(rec_2[0]) || rec_2[0] == '-') {
			sprintf(num_1, "%d", strcmp(reg_1, REG_sp) == 0 ? -atoi(rec_2) * 4 : atoi(rec_2) * 4);
			gen_mips_opt(MIPS_ADDI, REG_t8, reg_1, num_1);
		}
		else {
			alloc_reg_resident_opt(0, rec_2, &temp_reg_2);													// 加载数组下标
			gen_mips_opt(MIPS_SLL, REG_t9, temp_reg_2, "2");										// 计算数组内偏移
			gen_mips_opt(strcmp(reg_1, REG_sp) == 0 ? MIPS_SUBU : MIPS_ADDU, REG_t8, reg_1, REG_t9);
		}
		alloc_reg_resident_opt(1, dst, &temp_reg_3);
		gen_mips_opt(MIPS_LW, temp_reg_3, offset_1, REG_t8);										// 加载数组元素值
		break;
	case AEI:
		get_offset_opt(dst, &offset_1, &reg_1);																// 获取数组首元素的偏移
		if (isdigit(rec_1[0]) || rec_1[0] == '-') {
			sprintf(num_1, "%d", strcmp(reg_1, REG_sp) == 0 ? -atoi(rec_1) * 4 : atoi(rec_1) * 4);
			gen_mips_opt(MIPS_ADDI, REG_t8, reg_1, num_1);									// 计算偏移
		}
		else {
			alloc_reg_resident_opt(0, rec_1, &temp_reg_3);
			gen_mips_opt(MIPS_SLL, REG_t9, temp_reg_3, "2");
			gen_mips_opt(strcmp(reg_1, REG_sp) == 0 ? MIPS_SUBU : MIPS_ADDU, REG_t8, reg_1, REG_t9);													// 计算偏移
		}
		if (isdigit(rec_2[0]) || rec_2[0] == '-') {
			gen_mips_opt(MIPS_ADDI, REG_t9, REG_0, rec_2);													// 计算值
			gen_mips_opt(MIPS_SW, REG_t9, offset_1, REG_t8);
		}
		else {
			alloc_reg_resident_opt(0, rec_2, &temp_reg_2);														// 计算值
			gen_mips_opt(MIPS_SW, temp_reg_2, offset_1, REG_t8);
		}
		break;
	case ASN:
		if (isdigit(rec_1[0]) || rec_1[0] == '-') {
			alloc_reg_resident_opt(1, dst, &temp_reg_1);
			gen_mips_opt(MIPS_ADDI, temp_reg_1, REG_0, rec_1);
		}
		else {
			alloc_reg_resident_opt(0, rec_1, &temp_reg_2);
			alloc_reg_resident_opt(1, dst, &temp_reg_1);
			gen_mips_opt(MIPS_ADDU, temp_reg_1, temp_reg_2, REG_0);
		}
		break;
	default:
		if ((isdigit(rec_1[0]) || rec_1[0] == '-') && (isdigit(rec_2[0]) || rec_2[0] == '-')) {
			alloc_reg_resident_opt(1, dst, &temp_reg_1);
			if (op == ADD) {
				sprintf(num_1, "%d", atoi(rec_1) + atoi(rec_2));
			}
			else if (op == SUB) {
				sprintf(num_1, "%d", atoi(rec_1) - atoi(rec_2));
			}
			else if (op == MUL) {
				sprintf(num_1, "%d", atoi(rec_1) * atoi(rec_2));
			}
			else {
				sprintf(num_1, "%d", atoi(rec_1) / atoi(rec_2));
			}
			gen_mips_opt(MIPS_ADDI, temp_reg_1, REG_0, num_1);
		}
		else if (isdigit(rec_2[0]) || rec_2[0] == '-') {
			int num2bit;
			alloc_reg_resident_opt(0, rec_1, &temp_reg_2);
			alloc_reg_resident_opt(1, dst, &temp_reg_1);
			sprintf(num_1, "%d", atoi(rec_2));
			sprintf(num_2, "%d", -atoi(rec_2));
			if (op == ADD) {
				gen_mips_opt(MIPS_ADDI, temp_reg_1, temp_reg_2, num_1);
			}
			else if (op == SUB) {
				gen_mips_opt(MIPS_ADDI, temp_reg_1, temp_reg_2, num_2);
			}
			else if (op == MUL) {
				if ((num2bit = is_lucky_num(atoi(num_1))) >= 0) {
					sprintf(num_1, "%d", num2bit);
					gen_mips_opt(MIPS_SLL, temp_reg_1, temp_reg_2, num_1);
				}
				else {
					gen_mips_opt(MIPS_MUL, temp_reg_1, temp_reg_2, num_1);
				}
			}
			else {
				if ((num2bit = is_lucky_num(atoi(num_1))) >= 0) {
					sprintf(num_1, "%d", num2bit);
					gen_mips_opt(MIPS_SRL, temp_reg_1, temp_reg_2, num_1);
				}
				else {
					gen_mips_opt(MIPS_ADDI, REG_t9, REG_0, num_1);
					gen_mips_opt(MIPS_DIV, temp_reg_2, REG_t9, NULL);
					gen_mips_opt(MIPS_MFLO, temp_reg_1, NULL, NULL);
				}
			}
		}
		else if (isdigit(rec_1[0]) || rec_1[0] == '-') {
			int num2bit;
			alloc_reg_resident_opt(0, rec_2, &temp_reg_2);
			alloc_reg_resident_opt(1, dst, &temp_reg_1);
			sprintf(num_1, "%d", atoi(rec_1));
			sprintf(num_2, "%d", -atoi(rec_1));
			if (op == ADD) {
				gen_mips_opt(MIPS_ADDI, temp_reg_1, temp_reg_2, num_1);
			}
			else if (op == SUB) {
				gen_mips_opt(MIPS_SUB, REG_t8, REG_0, temp_reg_2);
				gen_mips_opt(MIPS_ADDI, temp_reg_1, REG_t8, num_1);
			}
			else if (op == MUL) {
				if ((num2bit = is_lucky_num(atoi(num_1))) >= 0) {
					sprintf(num_1, "%d", num2bit);
					gen_mips_opt(MIPS_SLL, temp_reg_1, temp_reg_2, num_1);
				}
				else {
					gen_mips_opt(MIPS_MUL, temp_reg_1, temp_reg_2, num_1);
				}
			}
			else {
				gen_mips_opt(MIPS_ADDI, REG_t8, REG_0, num_1);
				gen_mips_opt(MIPS_DIV, REG_t8, temp_reg_2, NULL);
				gen_mips_opt(MIPS_MFLO, temp_reg_1, NULL, NULL);
			}
		}
		else {
			alloc_reg_resident_opt(0, rec_1, &temp_reg_2);
			alloc_reg_resident_opt(0, rec_2, &temp_reg_3);
			alloc_reg_resident_opt(1, dst, &temp_reg_1);
			switch (op) {
			case ADD:
				gen_mips_opt(MIPS_ADDU, temp_reg_1, temp_reg_2, temp_reg_3);
				break;
			case SUB:
				gen_mips_opt(MIPS_SUB, temp_reg_1, temp_reg_2, temp_reg_3);
				break;
			case MUL:
				gen_mips_opt(MIPS_MUL, temp_reg_1, temp_reg_2, temp_reg_3);
				break;
			case DIV:
				gen_mips_opt(MIPS_DIV, temp_reg_2, temp_reg_3, NULL);
				gen_mips_opt(MIPS_MFLO, temp_reg_1, NULL, NULL);
				break;
			}
		}
	}
	return;
}

/*
	输入语句:
	TODO：输入检查
*/
void gen_scanf_opt(char* dst, char* type) {
	char *reg;
	alloc_reg_resident_opt(1, dst, &reg);
	if (strcmp(type, "INT") == 0) {
		gen_mips_opt(MIPS_ADDI, REG_v0, REG_0, "5");
	}
	else {
		gen_mips_opt(MIPS_ADDI, REG_v0, REG_0, "12");
	}
	gen_mips_opt(MIPS_CALL, NULL, NULL, NULL);
	gen_mips_opt(MIPS_ADDU, reg, REG_v0, REG_0);
	return;
}

/*
	输出语句
*/
void gen_printf_opt(char* rec, char* type) {
	char* reg;
	if (strcmp(type, "STR") == 0) {								// 输出字符串
		rec[0] = 's';
		rec[1] = '$';
		gen_mips_opt(MIPS_LA, REG_a0, rec, NULL);
		gen_mips_opt(MIPS_ADDI, REG_v0, REG_0, "4");
		gen_mips_opt(MIPS_CALL, NULL, NULL, NULL);
	}
	else if (strcmp(type, "INT") == 0) {						// 输出整数
		gen_mips_opt(MIPS_ADDI, REG_v0, REG_0, "1");
		if (isdigit(rec[0]) || rec[0] == '-') {
			gen_mips_opt(MIPS_ADDI, REG_a0, REG_0, rec);
		}
		else {
			alloc_reg_resident_opt(0, rec, &reg);
			gen_mips_opt(MIPS_ADDU, REG_a0, reg, REG_0);
		}
		gen_mips_opt(MIPS_CALL, NULL, NULL, NULL);
	}
	else if (strcmp(type, "CHAR") == 0) {						// 输出字符
		gen_mips_opt(MIPS_ADDI, REG_v0, REG_0, "11");
		if (isdigit(rec[0]) || rec[0] == '-') {
			gen_mips_opt(MIPS_ADDI, REG_a0, REG_0, rec);
		}
		else {
			alloc_reg_resident_opt(0, rec, &reg);
			gen_mips_opt(MIPS_ADDU, REG_a0, reg, REG_0);
		}
		gen_mips_opt(MIPS_CALL, NULL, NULL, NULL);
	}
	return;
}


/*
	生成代码段:
		顺序遍历中间代码
*/
void gen_text_opt() {
	int push_counter, base;
	char* reg;
	char offset[20];
	gen_mips_opt(MIPS_TEXT, NULL, NULL, NULL);
	gen_mips_opt(MIPS_JAL, "main", NULL, NULL);
	gen_mips_opt(MIPS_ADDI, REG_v0, REG_0, "10");
	gen_mips_opt(MIPS_CALL, NULL, NULL, NULL);
	while (mid_code_pointer_opt < length_mid_code) {
		switch (mid_code[mid_code_pointer_opt]->op) {
		case GEI:
		case AEI:
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case ASN:
			gen_cul_opt(mid_code[mid_code_pointer_opt]->op,
				mid_code[mid_code_pointer_opt]->a,
				mid_code[mid_code_pointer_opt]->b,
				mid_code[mid_code_pointer_opt]->c);
			break;
		case LES:
		case LEQ:
		case GTR:
		case GEQ:
		case EQL:
		case NEQ:
			gen_B_jump_opt(mid_code[mid_code_pointer_opt]->op,
				mid_code[mid_code_pointer_opt]->a,
				mid_code[mid_code_pointer_opt]->b,
				mid_code[mid_code_pointer_opt + 1]->a);
			mid_code_pointer_opt++;
			break;
		case GOTO:
			gen_save_env_opt(0);
			gen_mips_opt(MIPS_J, mid_code[mid_code_pointer_opt]->a, NULL, NULL);
			break;
		case FUNC:
			gen_func_def_opt(mid_code[mid_code_pointer_opt]->b);
			break;
		case CALL:
			base = size_part + size_global + current_field_opt->temp_sum;
			sprintf(offset, "%d", -(base) * 4);
			gen_mips_opt(MIPS_SW, REG_ra, offset, REG_sp);										// 非叶函数保存$ra
			gen_save_env_opt(1);
			sprintf(offset, "%d", -(1 + base) * 4);
			gen_mips_opt(MIPS_ADDI, REG_sp, REG_sp, offset);									// 移进$sp			
			gen_func_call_opt(mid_code[mid_code_pointer_opt]->a);
			sprintf(offset, "%d", (1 + base) * 4);
			gen_mips_opt(MIPS_ADDI, REG_sp, REG_sp, offset);									// 回退$sp
			sprintf(offset, "%d", -(base) * 4);
			gen_mips_opt(MIPS_LW, REG_ra, offset, REG_sp);										// 非叶函数恢复$ra
			break;
		case PUSH:
			base = size_part + size_global + current_field_opt->temp_sum;
			push_counter = 0;
			sprintf(offset, "%d", -(base) * 4);
			gen_mips_opt(MIPS_SW, REG_ra, offset, REG_sp);										// 非叶函数保存$ra
			push_counter++;
			do {																				// 压入参数
				sprintf(offset, "%d", -(push_counter + base) * 4);
				if (isdigit(mid_code[mid_code_pointer_opt]->a[0]) || mid_code[mid_code_pointer_opt]->a[0] == '-') {
					gen_mips_opt(MIPS_ADDI, REG_t9, REG_0, mid_code[mid_code_pointer_opt]->a);
					gen_mips_opt(MIPS_SW, REG_t9, offset, REG_sp);
				}
				else {
					alloc_reg_resident_opt(0, mid_code[mid_code_pointer_opt]->a, &reg);
					gen_mips_opt(MIPS_SW, reg, offset, REG_sp);
				}
				push_counter++;
				mid_code_pointer_opt++;
			} while (mid_code[mid_code_pointer_opt]->op == PUSH);
			gen_save_env_opt(1);
			sprintf(offset, "%d", -(push_counter + base) * 4);
			gen_mips_opt(MIPS_ADDI, REG_sp, REG_sp, offset);									// 移进$sp												
			gen_func_call_opt(mid_code[mid_code_pointer_opt]->a);								// call
			sprintf(offset, "%d", (push_counter + base) * 4);
			gen_mips_opt(MIPS_ADDI, REG_sp, REG_sp, offset);									// 回退$sp
			sprintf(offset, "%d", -(base) * 4);
			gen_mips_opt(MIPS_LW, REG_ra, offset, REG_sp);										// 非叶函数恢复$ra
			break;
		case RET:
			gen_return_opt(mid_code[mid_code_pointer_opt]->a);
			break;
		case READ:
			gen_scanf_opt(mid_code[mid_code_pointer_opt]->a, mid_code[mid_code_pointer_opt]->b);
			break;
		case PRINT:
			gen_printf_opt(mid_code[mid_code_pointer_opt]->a, mid_code[mid_code_pointer_opt]->b);
			break;
		case LABEL:
			gen_save_env_opt(0);
			gen_mips_opt(MIPS_LABEL, mid_code[mid_code_pointer_opt]->a, NULL, NULL);
			break;
		}
		mid_code_pointer_opt++;
		//print_LRU_opt();
	}
}

/*
	代码生成程序启动函数
*/
void start_generator_opt() {
	init_generator_opt();
	gen_data_opt();
	gen_text_opt();
	reset_generator_opt();
}