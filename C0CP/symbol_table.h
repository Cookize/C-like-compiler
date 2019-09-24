#ifndef __TABLE_H_
#define __TABLE_H_

#define _CRT_SECURE_NO_WARNINGS 

#define SIZE_TBL		(10000)
#define	SIZE_STR_TBL	(1000)

/*
	��������
*/
enum Kind {
	NOKIND,		// ռλ
	Const,		// ����
	Var,		// ����
	Func,		// ����
	Para		// ����
};
typedef enum Kind Kind;

/*
	����ֵ����
*/
enum Type {
	NOTYPE,		//
	INT,		// ����
	CHAR,		// �ַ���
};
typedef enum Type Type;

/*
	��������
*/
enum Func_type {
	NOTFUNC,	//
	REVOID,		// �޷���ֵ
	REINT,		// ��������
	RECHAR		// �����ַ���
};
typedef enum Func_type Func_type;

/*
	���ű���
*/
typedef struct TABLE_ITEM {
	char* id;							// ����
	Kind kind;							// ����		
	Type type;							// ֵ����
	Func_type return_type;				// ��������ֵ����
	int const_value;					// ����ֵ
	int array_length;					// ���鳤��
	int addr;							// ��ַ
	int field_num;						// ��������
} *tbl_item, tbl_item_obj;

/*
	���ű�����Ϣ
*/
typedef struct TABLE_FIELD {
	char* func_name;					// ����������
	int first_item_loca;				// �׸���ʶ���ڷ��ű��е�����
	int	last_item_loca;					// ĩβ��ʶ���ڷ��ű��е�����
	int para_sum;						// ��������
	int temp_sum;						// ��ʱ��������
	int var_sum;						// �ֲ���������
}*tbl_field, tbl_field_obj;

/*
	���ű�
*/
typedef struct TABLE {
	tbl_item* item_list;				// ����ű�
	tbl_field* field_list;				// ����Ϣ��
	int sum_item;						// ���������
	int sum_field;						// ������
} *tbl, tbl_obj;

/*
	�ַ���������
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

