#ifndef __BLOCKS_H_
#define __BLOCKS_H_
#include"mid_code.h"

#define SIZE_BLOCK	100000
#define SIZE_CONFLICT	10000

/*
	活跃变量分析记录块
*/
typedef struct LIVE_ANALYSIS {
	char** use;
	char** def;
	char** in;
	char** out;
	int sum_use;
	int sum_def;
	int sum_in;
	int sum_out;
} *live_var_info, live_var_info_obj;

/*
	基本块
*/
typedef struct BLOCK {
	char* label;			// 标记
	int first_index;		// 首个中间代码下标
	int last_index;			// 最后中间代码下标
//	four_code* code;		// 基本块内中间代码
//	int length;				// 代码长度

	int *prev;				// 前驱基本块索引
	int sum_prev;			// 前驱基本块数量
	int *next;				// 后继基本块索引
	int sum_next;			// 后继基本块数量

	live_var_info live_var;	// 活跃变量分析记录

} *block, block_obj;

/*
	函数块
*/
typedef struct FUNC_BLOCK {
	char* func_name;		// 函数名
	block *block_list;		// 函数内基本块列表
	int sum_block;			// 基本块数量
	
	int first_info_index;	// 函数信息段开始下标
	int last_info_index;	// 函数信息段结束下标

} *func_blk, func_blk_obj;

extern int build_blocks();
extern int live_variable_analysis();
extern void print_block();
extern int gen_conflict_in(char* func_name);
extern int is_out(char *name, char* func_name);
extern func_blk func_blk_list[SIZE_BLOCK];
extern int length_func_blk;
extern char* name_list[SIZE_CONFLICT + 1];
extern int conflict_matrix[SIZE_CONFLICT + 1][SIZE_CONFLICT + 1];
extern int size_conflict;
#endif __BLOCKS_H_
