#include"symbol_table.h"
#include<stdlib.h>
#include<string.h>
#include<stdio.h>
#include"error.h"


tbl	table;						// 符号表
stbl str_table;					// 字符串常量表
int global_offset = 0;			// 全局地址
int func_offset = 0;			// 局部地址
FILE *table_out;						// 输出文件

char type_out[][5] = {
	"", "INT", "CHAR"
};
char kind_out[][6] = {
	"", "CONST", "VAR", "FUNC", "PARA"
};
char func_type_out[][10] = {
	"", "REVOID", "REINT", "RECHAR"
};

int is_enterable(tbl_item item);

/*
	新建表项
*/
tbl_item create_table_item (char* id, Kind kind, Type type, Func_type func_type, int const_value, int array_length) {
	tbl_item new_item = (tbl_item)malloc(sizeof(tbl_item_obj));
	new_item->id = (char*)malloc(sizeof(char) * strlen(id));			// id
	strcpy(new_item->id, id);
	new_item->kind = kind;												// kind
	new_item->type = type;												// type
	new_item->return_type = func_type;									// ret type
	new_item->const_value = const_value;								// const_int
	new_item->array_length = array_length;								// array_length
	return new_item;
}

/*
	新建域
*/
int create_table_field (char* name) {
	if (table->sum_field >= SIZE_TBL)	return -1;
	func_offset = 0;																	// reset func offset
	tbl_field new_field = (tbl_field)malloc(sizeof(tbl_field_obj));						
	new_field->func_name = (char*)malloc(sizeof(char) * strlen(name));
	strcpy(new_field->func_name, name);													// field name
	new_field->first_item_loca = table->sum_item;										// first item loca
	new_field->last_item_loca = table->sum_item - 1;									// last item loca
	new_field->para_sum = 0;															// count para
	new_field->temp_sum = 0;															// count temp
	new_field->var_sum = 0;																// count var
	table->field_list[table->sum_field++] = new_field;									// append new field
	return 1;
}

/*
	初始化字符串表
*/
void init_stbl() {
	str_table = (stbl)malloc(sizeof(stbl_obj));
	str_table->strings = (char**)malloc(sizeof(char*) * SIZE_STR_TBL);
	str_table->sum = 0;
}

/*
	初始化符号表
*/
void init_tbl() {
	table = (tbl)malloc(sizeof(tbl_obj));
	table->item_list = (tbl_item*)malloc(sizeof(tbl_item) * (SIZE_TBL + 1));			// create list
	table->field_list = (tbl_field*)malloc(sizeof(tbl_field) * (SIZE_TBL + 1));
	table->sum_field = 0;																// reset sum
	table->sum_item = 0;
	global_offset = 0;																	// reset offset
	func_offset = 0;
	create_table_field("GLOBAL");														// create global field

	init_stbl();
	if (fopen_s(&table_out, "table.txt", "w") != 0) {
		error_stand(FILE_NOT_FOUND);
													// 无法打开文件，结束运行。
	}
	return;
}

/*
	关闭输出文件
*/
void reset_table() {
	fclose(table_out);
}


/*
	加入字符串常量
*/
int enter_str(char* str, char** const_name) {
	if (str_table->sum >= SIZE_STR_TBL)	return -1;
	char num[5];
	sprintf(num, "%d", str_table->sum);
	*const_name = (char*)malloc(sizeof(char) * 7);
	strcpy(*const_name, "$s");
	strcat(*const_name, num);
	str_table->strings[str_table->sum] = (char*)malloc(sizeof(char) * strlen(str));
	strcpy(str_table->strings[str_table->sum], str);
	str_table->sum++;
	return 0;
}

/*
	获取字符串常量
*/
int find_str(char* const_name, char** str_return) {
	if (const_name[0] != '$' || const_name[1] != 's')	return -1;
	char num[6];
	int i;
	for (i = 2; i < (int)strlen(const_name); i++) {
		num[i - 2] = const_name[i];
	}
	num[i] = '\0';
	i = atoi(num);
	*str_return = (char*)malloc(sizeof(char) * strlen(str_table->strings[i]));
	strcpy(*str_return, str_table->strings[i]);
	return 0;
}

/*
	申请临时变量
*/
int alloc_temp(char** temp) {
	char num[10];
	sprintf(num, "%d", table->field_list[table->sum_field - 1]->temp_sum++);
	*temp = (char*)malloc(sizeof(char) * (strlen(num) + 2));
	strcpy(*temp, "$t");
	strcat(*temp, num);
	return 0;
}
int alloc_temp_in (char** temp, char* func_name) {
	char num[10];
	int i;
	for (i = 0; i < table->sum_field; i++) {
		if (strcmp(func_name, table->field_list[i]->func_name) == 0) {
			sprintf(num, "%d", table->field_list[i]->temp_sum++);
			*temp = (char*)malloc(sizeof(char) * (strlen(num) + 2));
			strcpy(*temp, "$t");
			strcat(*temp, num);
			return 0;
		}
	}
	return -1;
}

/*
	加入表项
*/
int enter_item(tbl_item item) {
	if (table->sum_item >= SIZE_TBL)	return -1;
	if (item->kind == Func) {													// enter function
		table->field_list[table->sum_field - 1]->last_item_loca = table->sum_item - 1;
		create_table_field(item->id);											// create field of function
		table->item_list[table->sum_item++] = item;								// append func item
		item->field_num = 0;													// global
		item->addr = global_offset++;											// set addr
	}
	else {																		// enter other items
		table->item_list[table->sum_item++] = item;								// append new item
		item->field_num = table->sum_field - 1;									// set field number
		if(table->sum_field == 1) item->addr = item->kind != Const ? global_offset++ : -1;
		else item->addr = item->kind != Const ? func_offset++ : -1;					// set offset
		item->kind == Para ? table->field_list[table->sum_field - 1]->para_sum++ : 0;	// count para
	}
	item->kind == Var ? table->field_list[table->sum_field - 1]->var_sum++ : 0;	// count var
	table->field_list[table->sum_field - 1]->last_item_loca++;
	return 0;
}

/*
	加入字符常量
*/
void enter_const(char* id, Type type, int const_value, int line) {
	tbl_item item = create_table_item(id, Const, type, 0, const_value, 0);
	if (is_enterable(item) != 0) {
		commit_error_sema(DUPLICATE, line, id);
		item->id = " ";
	}
	enter_item(item);
}

/*
	加入数组
*/
void enter_array(char* id, Type ele_type, int length, int line) {
	tbl_item item = create_table_item(id, Var, ele_type, 0, 0, length);
	if (is_enterable(item) != 0) {
		commit_error_sema(DUPLICATE, line, id);
		item->id = " ";
	}
	enter_item(item);
}

/*
	加入变量
*/
void enter_var(char* id, Type type, int line) {
	tbl_item item = create_table_item(id, Var, type, 0, 0, 0);
	if (is_enterable(item) != 0) {
		commit_error_sema(DUPLICATE, line, id);
		item->id = " ";
	}
	enter_item(item);
}

/*
	加入参量
*/
void enter_para(char* id, Type type, int line) {
	tbl_item item = create_table_item(id, Para, type, 0, 0, 0);
	if (is_enterable(item) != 0) {
		commit_error_sema(DUPLICATE, line, id);
		item->id = " ";
	}
	enter_item(item);
}

/*
	加入函数
*/
void enter_func(char* id, Func_type return_type, int line) {
	tbl_item item = create_table_item(id, Func, 0, return_type, 0, 0);
	if (is_enterable(item) != 0) {
		commit_error_sema(DUPLICATE, line, id);
		item->id = " ";
		table->field_list[table->sum_field - 1]->func_name = " ";
	}
	enter_item(item);
}

/*
	检查是否能加入
	mode = 0	仅查找全局符号表
	mode = 1	查找局部符号表，以及全局符号表的函数部分
*/
int is_enterable(tbl_item item) {
	int i;
	if (item->kind == Func) {											// function
		for (i = 0; i <= table->field_list[0]->last_item_loca; i++) {	// check global const and var
			if (strcmp(item->id, table->item_list[i]->id) == 0) return -DUPLICATE;
		}
		for (i = 1; i < table->sum_field; i++) {						// check global functions
			if (strcmp(item->id, table->field_list[i]->func_name) == 0) return -DUPLICATE;
		}
	}
	else {																// not function
		for (i = table->field_list[table->sum_field - 1]->first_item_loca; i <= table->field_list[table->sum_field - 1]->last_item_loca; i++) {	// check field const and var
			if (strcmp(item->id, table->item_list[i]->id) == 0) return -DUPLICATE;
		}
	}
	return 0;
}

/*
	搜索标识符
*/
tbl_item find_id(char* id) {
	int i;
	for (i = table->field_list[table->sum_field - 1]->first_item_loca; i <= table->field_list[table->sum_field - 1]->last_item_loca; i++) {	// search field const and var
		if (strcmp(id, table->item_list[i]->id) == 0) return table->item_list[i];
	}
	for (i = 1; i < table->sum_field; i++) {																				// check global functions
		if (strcmp(id, table->field_list[i]->func_name) == 0) return table->item_list[table->field_list[i]->first_item_loca];
	}
	for (i = 0; i <= table->field_list[0]->last_item_loca; i++) {															// check global const and var
		if (strcmp(id, table->item_list[i]->id) == 0) return table->item_list[i];
	}
	return NULL;
}

/*
	检查是否为全局变量
*/
int is_global(char *id) {
	int i;
	for (i = table->field_list[table->sum_field - 1]->first_item_loca; i <= table->field_list[table->sum_field - 1]->last_item_loca; i++) {	// search field const and var
		if (strcmp(id, table->item_list[i]->id) == 0) return 0;
	}
	for (i = 1; i < table->sum_field; i++) {																				// check global functions
		if (strcmp(id, table->field_list[i]->func_name) == 0) return 0;
	}
	for (i = 0; i <= table->field_list[0]->last_item_loca; i++) {															// check global const and var
		if (strcmp(id, table->item_list[i]->id) == 0) return 1;
	}
	return 0;
}

/*
	直接获取标识符
*/
tbl_item get_id(int index) {
	if (index >= table->sum_item) return NULL;
	return table->item_list[index];
}

/*
	获取域信息
*/
tbl_field find_field(char* func_name) {
	int i;
	for (i = 0; i < table->sum_field; i++) {
		if (strcmp(func_name, table->field_list[i]->func_name) == 0)
			return table->field_list[i];
	}
	return NULL;
}

/*
	打印符号表
*/
void print_table() {
	int i, j;
	fprintf_s(table_out, "********************************************************\n");
	fprintf_s(table_out, "%s:\n", table->field_list[0]->func_name);
	for (j = table->field_list[0]->first_item_loca + 1; j <= table->field_list[0]->last_item_loca; j++) {
		fprintf(table_out, "%s\t%s\t%s\t%s\n",  
			kind_out[table->item_list[j]->kind],
			type_out[table->item_list[j]->type],
			func_type_out[table->item_list[j]->return_type],
			table->item_list[j]->id
			);
	}
	for (j = 1; j < table->sum_field; j++) {
		fprintf(table_out, "%s\t%s\t%s\t%s\n",
			kind_out[table->item_list[table->field_list[j]->first_item_loca]->kind],
			type_out[table->item_list[table->field_list[j]->first_item_loca]->type],
			func_type_out[table->item_list[table->field_list[j]->first_item_loca]->return_type],
			table->item_list[table->field_list[j]->first_item_loca]->id);
	}
	for (i = 1; i < table->sum_field; i++) {
		fprintf_s(table_out, "********************************************************\n");
		fprintf_s(table_out, "%s:\n", table->field_list[i]->func_name);
		for (j = table->field_list[i]->first_item_loca + 1; j <= table->field_list[i]->last_item_loca; j++) {
			fprintf(table_out, "%s\t%s\t%s\t%s\n",
				kind_out[table->item_list[j]->kind],
				type_out[table->item_list[j]->type],
				func_type_out[table->item_list[j]->return_type],
				table->item_list[j]->id
			);
		}
		fprintf_s(table_out, "\n");
	}
	fprintf_s(table_out, "********************************************************\n");
}

/*
	打印字符串表
*/
void print_str_table() {
	int i;
	fprintf_s(table_out, "********************************************************\n");
	fprintf_s(table_out, "String table:\n");
	for (i = 0; i < str_table->sum; i++) {
		fprintf(table_out, "$s%d\t\"%s\"\n", i, str_table->strings[i]);
	}
	fprintf_s(table_out, "********************************************************\n");
}
