#include "error.h"
#include"lex_analyze.h"
#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>

FILE* file_in = NULL;												// 输入文件指针
char buffer[LENGTH_BUFFER + 1];									// 程序缓存区
int forward = 0;													// 当前识别位置在缓存区的指针
char buffer_string[LENGTH_BUFFER_CONST + 1];					// 字符串缓存区
long long buffer_int = 0;													// 数字缓存区
char buffer_char;													// 字符缓存区
int line_counter = 1;													// 行计数器
int char_counter = 1;												// 行字符计数器

int to_file_end = 0;													// 文件读取结束标志

// 词法分析输出
LEX_LOG lex_log_list[LENGTH_BUFFER_SYM] = {NULL, NULL, NULL};		// 词法分析日志
LEX_LOG lex_log = NULL;													// 当前词素信息
int index = 0;																// 当前词素信息在日志中的索引值

//symbol sym;															// 词法单元种类
//int num;																// 词素数值
//char ch;																// 词素字符
//char* str = buffer_const;												// 词素字符串值
//int to_file_end = 0;													// 源程序读取结束标志

/*
	词法单元输出集合。
*/
char symbol_output[][15] = {
	"ident",
	"plus", "minus", "multiply", "divide",
	"left_parent", "right_parent", "left_bracket", "right_bracket", "left_brace", "right_brace",
	"comma", "semicolon", "colon", "quote", "double_quote",
	"less", "less_equal", "greater", "greater_equal", "equal", "not_equal",
	"become",
	"int", "char", "const", "void", "if", "while", "switch", "case", "default", "return",
	"main", "printf", "scanf",
	"const_int", "const_char", "const_string"
};

/*
	词法单元识别集合。
*/
char symbol_in_str[][15] = {
	"ident",
	"+", "-", "*", "/",
	"(", ")", "[", "]", "{", "}",
	",", ";", ":", "\'", "\"",
	"<", "<=", ">", ">=", "==", "!=",
	"==",
	"int", "char", "const", "void", "if", "while", "switch", "case", "default", "return",
	"main", "printf", "scanf",
	"const_int", "const_char", "const_string"
};

symbol str2simple(char ch) {
	int i;
	char str[] = { ch , '\0'};
	for (i = 1; i <= 13; i++) {
		if (strcmp(str, symbol_in_str[i]) == 0) {
			return (symbol)i;
		}
	}
	return 0;
}

symbol str2symbol(char* str) {
	int i;
	for (i = 0; i < SUM_SUMBOL_LIST; i++) {
		if (strcmp(str, symbol_in_str[i]) == 0) {
			return (symbol)i;
		}
	}
	return identifier;
}

/*
	更新缓存区。
*/
int flush_buffer() {
	if (fgets(buffer, LENGTH_BUFFER, file_in) == NULL) {
		to_file_end = 1;
		buffer[0] = EOF;
		buffer[1] = '\n';
	}
	else {
		int len = (int)strlen(buffer); 
		buffer[len] = ' ';
		buffer[len + 1] = '\0';
	}
	return 0;
}

/*
	将 forward 后移一位，若 forward 指向某一缓冲区尾部，则将缓冲区更新并指向其起始。
*/
void move() {
	if (to_file_end == 1) {
		buffer[forward] = EOF;
	}
	else if (forward == LENGTH_BUFFER - 2 || buffer[forward] == '\n' || buffer[forward] == '\0') {		// 到达缓冲区尾部，更新缓冲区。
		flush_buffer();
		forward = 0;
	}
	else {
		forward++;		// 移动。
	}
	char_counter++;
}

/*
	设置输入文件。
*/
int set_input_file(char* file_path) {
	if (fopen_s(&file_in, file_path, "r") != 0) {
		error_stand(FILE_NOT_FOUND);
		return -1;											// 无法打开文件，结束运行。
	}
	flush_buffer();															// 初始化缓冲区。
	return 0;
}

/*
	扩展字符串缓冲区。

void extend_char_buffer(char** buffer_pre, int* length_pre) {
	int length_new = *length_pre * 2;
	char* buffer_new = (char*)malloc(sizeof(char) * length_new);
	strcpy_s(buffer_new, length_new, *buffer_pre);
	*buffer_pre = buffer_new;
	*length_pre = length_new;
	free(buffer_new);
}
*/

/*
	从缓冲区获取标识符字符串，长度限制在128个字符，完成后将识别到的字符串缓存至缓存区。
	require:	forward 指向该字符串起始位置。
	effect：	forward 指向匹配后字符串末尾的下一个字符，将获取到的字符串存至 str。
*/
int fetch_ident() {
	int loca = 0;
	do {
		if (loca == LENGTH_BUFFER_CONST) {				// 缓冲区满。
			buffer_string[loca] = '\0';
			while (isalnum(buffer[forward]) || buffer[forward] == '_') {
				commit_error_lex(TOO_LONG_STRING, line_counter, char_counter);
				move();
			}
			return -1;
		}
		buffer_string[loca++] = buffer[forward];
		move();
	} while (isalnum(buffer[forward]) || buffer[forward] == '_');
	buffer_string[loca] = '\0';
	return 0;
}

/*
	从缓存区获取数字常量，完成后将获取到的数字缓存到缓存区。
	识别数字超出整型范围时，终止识别并输出已识别的部分。
	require:	forward 指向数字字符串起始位置。
	effect:	forward 指向匹配后字符串末尾的下一个字符，将获取到的数字存至 num，并检查数字范围。
*/
int fetch_int() {
	long long num_new = 0;
	buffer_int = 0;
	if (buffer[forward] == '0') {										// 排除前缀零数字。
		move();
		while (isdigit(buffer[forward])) {
			commit_error_lex(ILLEGAL_INT, line_counter, char_counter);
			move();
		}
		return -2;
	}
	do {
		if (num_new * 10 + buffer[forward] - '0' > (long long)INT_MAX + 1) {		// 排除溢出数字。
			buffer_int = num_new;
			while (isdigit(buffer[forward])) {
				commit_error_lex(TOO_LARGE_INT, line_counter, char_counter);
				move();
			}
			return -1;
		}
		num_new = num_new * 10 + buffer[forward] - '0';
		move();
	} while (isdigit(buffer[forward]));
	buffer_int = num_new;
	return 0;
}

/*
	从缓存区获取字符常量，完成后将字符缓存到缓存区。
	前置引号后第一个字符为非法字符时，放弃识别并报错。
	无法识别后置引号时，自动补全并报错。
	require:	forward 指向字符常量前置引号。
	effect:	forward 指向匹配后字符常量后置引号的下一个字符，将获取到的字符存至 ch，并检查字符范围。
*/
int fetch_char() {
	move();
	if (!(buffer[forward] == '+' || buffer[forward] == '-' || buffer[forward] == '*' || buffer[forward] == '/'
		|| isalnum(buffer[forward]) || buffer[forward] == '_')) {
		// 报错。
		commit_error_lex(MISSMATCH_CHAR, line_counter, char_counter);
		return -1;
	}
	buffer_char = buffer[forward];
	move();
	if (buffer[forward] != '\'') {
		commit_error_lex(MISSMATCH_SYM_QUOTE, line_counter, char_counter);
		return -2;
	}
	move();
	return 0;
}

/*
	从缓存区获取字符串常量，长度限制在128个字符。
	require:	forward 指向字符串常量前置双引号。
	effect:	forward 指向匹配后字符串常量后置双引号的下一个字符，将获取到的字符存至 str，并检查字符范围。
*/
int fetch_string() {
	int loca = 0;
	move();
	while (buffer[forward] != 34 && buffer[forward] >= 32 && buffer[forward] <= 126) {
		if (loca == LENGTH_BUFFER_CONST) {				// 缓冲区满。
			buffer[loca] = '\0';
			while (buffer[forward] != 34 && buffer[forward] >= 32 && buffer[forward] <= 126) {
				commit_error_lex(TOO_LONG_STRING, line_counter, char_counter);
				move();
			}
			if (buffer[forward] != '\"') {
				commit_error_lex(MISSMATCH_SYM_DOUBLE_QUOTE, line_counter, char_counter);
				return -1;
			}
			move();
			return -2;
		}
		buffer_string[loca++] = buffer[forward];
		if (buffer[forward] == '\\') {
			buffer_string[loca++] = buffer[forward];
		}
		move();
	}
	//buffer_string[loca++] = '\\';
	//buffer_string[loca++] = 'n';
	buffer_string[loca++] = '\0';
	if (buffer[forward] != '\"') {
		commit_error_lex(MISSMATCH_SYM_DOUBLE_QUOTE, line_counter, char_counter);
		return -1;
	}
	move();
	return 0;
}

/*
	从文件中获取一个新的词素，存至 lex_log。
*/
int get_sym() {
	int end_flag;
	do {
		end_flag = 1;
		// 开始讨论。
		if (buffer[forward] == ' ' ||  buffer[forward] == '\t' || buffer[forward] == '\0'){							// 忽略字符。
			move();
			end_flag = 0;
		}
		else if (isalpha(buffer[forward]) || buffer[forward] == '_') {												// 识别字母，讨论情况：关键字、标识符。
			fetch_ident();
			lex_log->sym = str2symbol(buffer_string);
			lex_log->str = buffer_string;
		}
		else if (isdigit(buffer[forward])) {											// 识别数字，获取数字值，判断有效性。
			fetch_int();
			lex_log->sym = const_int;
			lex_log->num = buffer_int;
		}
		else if (buffer[forward] == '\'') {											// 识别引号，获取字符。
			if (fetch_char() != -1) {
				lex_log->sym = const_char;
				lex_log->ch = buffer_char;
			}
			else {
				end_flag = 0;
			}
		}
		else if (buffer[forward] == '\"') {											// 识别双引号，获取字符串。
			if (fetch_string() == 0) {
				lex_log->sym = const_string;
				lex_log->str = buffer_string;
			}
			else {
				end_flag = 0;
			}
		}
		else if (buffer[forward] == '<') {
			move();
			if (buffer[forward] == '=') {												// 识别“<=”
				move();
				lex_log->sym = sym_less_equal;
			}
			else {																	// 识别“<”	
				lex_log->sym = sym_less;
			}
		}
		else if (buffer[forward] == '>') {
			move();
			if (buffer[forward] == '=') {												// 识别“>=”
				move();
				lex_log->sym = sym_greater_equal;
			}
			else {
				lex_log->sym = sym_greater;
			}
		}
		else if (buffer[forward] == '=') {
			move();
			if (buffer[forward] == '=') {												// 识别“==”
				move();
				lex_log->sym = sym_equal;
			}
			else {																	// 识别“=”
				lex_log->sym = sym_become;
			}
		}
		else if (buffer[forward] == '!') {
			move();
			int temp_line = line_counter, temp_loca = char_counter;
			if (buffer[forward] == '=') {												// 识别“==”
				move();
				lex_log->sym = sym_not_equal;
			}
			else {																	// 识别“=”
				commit_error_lex(MISSMATCH_SYM_NOT_EQUAL, temp_line, temp_loca);
				end_flag = 0;
			}
		}
		else if (lex_log->sym = str2simple(buffer[forward])) {
			move();
		}
		else if (buffer[forward] == '\n') {
			line_counter++;
			move();
			char_counter = 1;
			end_flag = 0;
		}
		else {
			commit_error_lex(ILLEGAL_CHAR, line_counter, char_counter);
			move();
			end_flag = 0;
		}
	} while (!end_flag && !to_file_end);															// 忽略无法识别的字符。
	lex_log->line_counter = line_counter;
	if (to_file_end) {																				// 文件结束
		return -1;
	}
	return 0;
}

int next_sym(int isEnd) {
	if (index == 0) {								// 插入新日志。
		int i;
		lex_log = (LEX_LOG)malloc(sizeof(LEX_LOG_DATA));
		if (get_sym() != 0) {
			error_stand(END_FILE);
			if (to_file_end && !isEnd) {
				commit_error_lex(EXCEPTIONAL_END, line_counter,char_counter);
				exit(0);
			}
			return -1;
		}
		free(lex_log_list[LENGTH_BUFFER_SYM - 1]);
		for (i = LENGTH_BUFFER_SYM - 1; i >= 1; i--) {
			lex_log_list[i] = lex_log_list[i - 1];
		}
		lex_log_list[0] = lex_log;
	}
	else if (index > 0) {
		index--;
		lex_log = lex_log_list[index];
	}
	else {
		error_stand(ILLEGAL_NUM);
	}
	return 0;
}

void bk_sym(int num) {
	if (index + num < LENGTH_BUFFER_SYM && index + num >= 1) {
		index = index + num;
		lex_log = lex_log_list[index];
	}
	else {
		error_stand(ILLEGAL_NUM);
	}
}

int get_bk_line() {
	return lex_log_list[1]->line_counter;
}