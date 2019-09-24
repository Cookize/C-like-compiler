#ifndef __LEX_H_
#define __LEX_H_
#define _CRT_SECURE_NO_WARNINGS

#define SUM_SUMBOL_LIST				(39)						// �ʷ���Ԫ����
#define LENGTH_BUFFER				(4096)						// ���򻺳�����С
#define LENGTH_BUFFER_CONST			(128)						// �ַ�����������С
#define LENGTH_BUFFER_SYM			(10)						// ���ػ�����

/*
	�ʷ���Ԫö���ࡣ����ʶ����Ϊһ��һ�ࡣ
*/
enum SYMBOL {
	/* ��ʶ�� */
	identifier,
	/* ���ţ���������ĸ�����֡��»��ߣ� */
	sym_plus, sym_minus, sym_multiply, sym_divide,
	sym_left_parent, sym_right_parent, sym_left_bracket, sym_right_bracket, sym_left_brace, sym_right_brace,
	sym_comma, sym_semicolon, sym_colon, sym_quote, sym_double_quote,
	sym_less, sym_less_equal, sym_greater, sym_greater_equal, sym_equal, sym_not_equal,
	sym_become,
	/* �ؼ��� */
	key_int, key_char, key_const, key_void,
	key_if, key_while, key_switch, key_case, key_default, key_return,
	/* ����������׼����������� */
	func_main, func_printf, func_scanf,
	/* ���� */
	const_int, const_char, const_string
};
typedef enum SYMBOL symbol;


struct LEX_LOG {
	symbol sym;
	long long num;
	char ch;
	char* str;
	int line_counter;
};
typedef struct LEX_LOG LEX_LOG_DATA;
typedef struct LEX_LOG *LEX_LOG;

extern char symbol_output[][15];
extern LEX_LOG lex_log;
extern int to_file_end;
extern int set_input_file(char*);
extern int next_sym(int);
extern void bk_sym(int);
extern int get_bk_line();

#endif // __LEX_H_

