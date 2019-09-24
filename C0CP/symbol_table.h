#ifndef __TABLE_H_
#define __TABLE_H_

#define _CRT_SECURE_NO_WARNINGS 

#define SIZE_TBL		(10000)
#define	SIZE_STR_TBL	(1000)

/*
	符号类型
*/
enum Kind {
	NOKIND,		// 占位
	Const,		// 常量
	Var,		// 变量
	Func,		// 函数
	Para		// 参数
};
typedef enum Kind Kind;

/*
	符号值类型
*/
enum Type {
	NOTYPE,		//
	INT,		// 整型
	CHAR,		// 字符型
};
typedef enum Type Type;

/*
	函数类型
*/
enum Func_type {
	NOTFUNC,	//
	REVOID,		// 无返回值
	REINT,		// 返回整型
	RECHAR		// 返回字符型
};
typedef enum Func_type Func_type;

/*
	符号表项
*/
typedef struct TABLE_ITEM {
	char* id;							// 名称
	Kind kind;							// 类型		
	Type type;							// 值类型
	Func_type return_type;				// 函数返回值类型
	int const_value;					// 常量值
	int array_length;					// 数组长度
	int addr;							// 地址
	int field_num;						// 所属域编号
} *tbl_item, tbl_item_obj;

/*
	符号表域信息
*/
typedef struct TABLE_FIELD {
	char* func_name;					// 作用域名称
	int first_item_loca;				// 首个标识符在符号表中的索引
	int	last_item_loca;					// 末尾标识符在符号表中的索引
	int para_sum;						// 参数数量
	int temp_sum;						// 临时变量数量
	int var_sum;						// 局部变量数量
}*tbl_field, tbl_field_obj;

/*
	符号表
*/
typedef struct TABLE {
	tbl_item* item_list;				// 域符号表
	tbl_field* field_list;				// 域信息表
	int sum_item;						// 域符号数量
	int sum_field;						// 域数量
} *tbl, tbl_obj;

/*
	字符串常量表
*/
typedef struct STR_TABLE {
	char** strings;
	int sum;
} *stbl, stbl_obj;

extern void init_tbl();
extern void enter_const(char*, Type, int, int);
extern void enter_array(char* id, Type ele_type, int length, int);
extern void enter_var(char* id, Type type, int);
extern void enter_para(char* id, Type type, int);
extern void enter_func(char* id, Func_type return_type, int);
extern tbl_item find_id(char*);
extern tbl_item get_id(int index);
extern tbl_field find_field(char* func_name);
extern void print_table();
extern int enter_str(char* str, char** const_name);
extern int find_str(char* const_name, char** str_return);
extern void reset_table();
extern void print_str_table();
extern int alloc_temp(char** temp);
extern int alloc_temp_in(char** temp, char* func_name);
extern int is_global(char *id);


#endif // !__TABLE_H_

