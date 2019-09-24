#ifndef __BLOCKS_H_
#define __BLOCKS_H_
#include"mid_code.h"

#define SIZE_BLOCK	100000
#define SIZE_CONFLICT	10000

/*
	��Ծ����������¼��
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
	������
*/
typedef struct BLOCK {
	char* label;			// ���
	int first_index;		// �׸��м�����±�
	int last_index;			// ����м�����±�
//	four_code* code;		// ���������м����
//	int length;				// ���볤��

	int *prev;				// ǰ������������
	int sum_prev;			// ǰ������������
	int *next;				// ��̻���������
	int sum_next;			// ��̻���������

	live_var_info live_var;	// ��Ծ����������¼

} *block, block_obj;

/*
	������
*/
typedef struct FUNC_BLOCK {
	char* func_name;		// ������
	block *block_list;		// �����ڻ������б�
	int sum_block;			// ����������
	
	int first_info_index;	// ������Ϣ�ο�ʼ�±�
	int last_info_index;	// ������Ϣ�ν����±�

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
