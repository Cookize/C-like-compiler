#ifndef __LEX_H_
#define __LEX_H_
#define _CRT_SECURE_NO_WARNINGS

#define SUM_SUMBOL_LIST				(39)						// 词法单元数量
#define LENGTH_BUFFER				(4096)						// 程序缓冲区大小
#define LENGTH_BUFFER_CONST			(128)						// 字符串缓冲区大小
#define LENGTH_BUFFER_SYM			(10)						// 词素缓冲区

/*
	词法单元枚举类。除标识符外为一符一类。
*/
enum SYMBOL {
	/* 标识符 */
	identifier,
	/* 符号（不包括字母、数字、下划线） */
	sym_plus, sym_minus, sym_multiply, sym_divide,
	sym_left_parent, sym_right_parent, sym_left_bracket, sym_right_bracket, sym_left_brace, sym_right_brace,
	sym_comma, sym_semicolon, sym_colon, sym_quote, sym_double_quote,
	sym_less, sym_less_equal, sym_greater, sym_greater_equal, sym_equal, sym_not_equal,
	sym_become,
	/* 关键字 */
	key_int, key_char, key_const, key_void,
	key_if, key_while, key_switch, key_case, key_default, key_return,
	/* 主函数及标准输入输出函数 */
	func_main, func_printf, func_scanf,
	/* 常量 */
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

