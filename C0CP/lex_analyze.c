#include "error.h"
#include"lex_analyze.h"
#include<stdio.h>
#include<ctype.h>
#include<stdlib.h>
#include<string.h>
#include<limits.h>

FILE* file_in = NULL;												// �����ļ�ָ��
char buffer[LENGTH_BUFFER + 1];									// ���򻺴���
int forward = 0;													// ��ǰʶ��λ���ڻ�������ָ��
char buffer_string[LENGTH_BUFFER_CONST + 1];					// �ַ���������
long long buffer_int = 0;													// ���ֻ�����
char buffer_char;													// �ַ�������
int line_counter = 1;													// �м�����
int char_counter = 1;												// ���ַ�������

int to_file_end = 0;													// �ļ���ȡ������־

// �ʷ��������
LEX_LOG lex_log_list[LENGTH_BUFFER_SYM] = {NULL, NULL, NULL};		// �ʷ�������־
LEX_LOG lex_log = NULL;													// ��ǰ������Ϣ
int index = 0;																// ��ǰ������Ϣ����־�е�����ֵ

//symbol sym;															// �ʷ���Ԫ����
//int num;																// ������ֵ
//char ch;																// �����ַ�
//char* str = buffer_const;												// �����ַ���ֵ
//int to_file_end = 0;													// Դ�����ȡ������־

/*
	�ʷ���Ԫ������ϡ�
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
	�ʷ���Ԫʶ�𼯺ϡ�
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
	���»�������
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
	�� forward ����һλ���� forward ָ��ĳһ������β�����򽫻��������²�ָ������ʼ��
*/
void move() {
	if (to_file_end == 1) {
		buffer[forward] = EOF;
	}
	else if (forward == LENGTH_BUFFER - 2 || buffer[forward] == '\n' || buffer[forward] == '\0') {		// ���ﻺ����β�������»�������
		flush_buffer();
		forward = 0;
	}
	else {
		forward++;		// �ƶ���
	}
	char_counter++;
}

/*
	���������ļ���
*/
int set_input_file(char* file_path) {
	if (fopen_s(&file_in, file_path, "r") != 0) {
		error_stand(FILE_NOT_FOUND);
		return -1;											// �޷����ļ����������С�
	}
	flush_buffer();															// ��ʼ����������
	return 0;
}

/*
	��չ�ַ�����������

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
	�ӻ�������ȡ��ʶ���ַ���������������128���ַ�����ɺ�ʶ�𵽵��ַ�����������������
	require:	forward ָ����ַ�����ʼλ�á�
	effect��	forward ָ��ƥ����ַ���ĩβ����һ���ַ�������ȡ�����ַ������� str��
*/
int fetch_ident() {
	int loca = 0;
	do {
		if (loca == LENGTH_BUFFER_CONST) {				// ����������
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
	�ӻ�������ȡ���ֳ�������ɺ󽫻�ȡ�������ֻ��浽��������
	ʶ�����ֳ������ͷ�Χʱ����ֹʶ�������ʶ��Ĳ��֡�
	require:	forward ָ�������ַ�����ʼλ�á�
	effect:	forward ָ��ƥ����ַ���ĩβ����һ���ַ�������ȡ�������ִ��� num����������ַ�Χ��
*/
int fetch_int() {
	long long num_new = 0;
	buffer_int = 0;
	if (buffer[forward] == '0') {										// �ų�ǰ׺�����֡�
		move();
		while (isdigit(buffer[forward])) {
			commit_error_lex(ILLEGAL_INT, line_counter, char_counter);
			move();
		}
		return -2;
	}
	do {
		if (num_new * 10 + buffer[forward] - '0' > (long long)INT_MAX + 1) {		// �ų�������֡�
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
	�ӻ�������ȡ�ַ���������ɺ��ַ����浽��������
	ǰ�����ź��һ���ַ�Ϊ�Ƿ��ַ�ʱ������ʶ�𲢱���
	�޷�ʶ���������ʱ���Զ���ȫ������
	require:	forward ָ���ַ�����ǰ�����š�
	effect:	forward ָ��ƥ����ַ������������ŵ���һ���ַ�������ȡ�����ַ����� ch��������ַ���Χ��
*/
int fetch_char() {
	move();
	if (!(buffer[forward] == '+' || buffer[forward] == '-' || buffer[forward] == '*' || buffer[forward] == '/'
		|| isalnum(buffer[forward]) || buffer[forward] == '_')) {
		// ����
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
	�ӻ�������ȡ�ַ�������������������128���ַ���
	require:	forward ָ���ַ�������ǰ��˫���š�
	effect:	forward ָ��ƥ����ַ�����������˫���ŵ���һ���ַ�������ȡ�����ַ����� str��������ַ���Χ��
*/
int fetch_string() {
	int loca = 0;
	move();
	while (buffer[forward] != 34 && buffer[forward] >= 32 && buffer[forward] <= 126) {
		if (loca == LENGTH_BUFFER_CONST) {				// ����������
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
	���ļ��л�ȡһ���µĴ��أ����� lex_log��
*/
int get_sym() {
	int end_flag;
	do {
		end_flag = 1;
		// ��ʼ���ۡ�
		if (buffer[forward] == ' ' ||  buffer[forward] == '\t' || buffer[forward] == '\0'){							// �����ַ���
			move();
			end_flag = 0;
		}
		else if (isalpha(buffer[forward]) || buffer[forward] == '_') {												// ʶ����ĸ������������ؼ��֡���ʶ����
			fetch_ident();
			lex_log->sym = str2symbol(buffer_string);
			lex_log->str = buffer_string;
		}
		else if (isdigit(buffer[forward])) {											// ʶ�����֣���ȡ����ֵ���ж���Ч�ԡ�
			fetch_int();
			lex_log->sym = const_int;
			lex_log->num = buffer_int;
		}
		else if (buffer[forward] == '\'') {											// ʶ�����ţ���ȡ�ַ���
			if (fetch_char() != -1) {
				lex_log->sym = const_char;
				lex_log->ch = buffer_char;
			}
			else {
				end_flag = 0;
			}
		}
		else if (buffer[forward] == '\"') {											// ʶ��˫���ţ���ȡ�ַ�����
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
			if (buffer[forward] == '=') {												// ʶ��<=��
				move();
				lex_log->sym = sym_less_equal;
			}
			else {																	// ʶ��<��	
				lex_log->sym = sym_less;
			}
		}
		else if (buffer[forward] == '>') {
			move();
			if (buffer[forward] == '=') {												// ʶ��>=��
				move();
				lex_log->sym = sym_greater_equal;
			}
			else {
				lex_log->sym = sym_greater;
			}
		}
		else if (buffer[forward] == '=') {
			move();
			if (buffer[forward] == '=') {												// ʶ��==��
				move();
				lex_log->sym = sym_equal;
			}
			else {																	// ʶ��=��
				lex_log->sym = sym_become;
			}
		}
		else if (buffer[forward] == '!') {
			move();
			int temp_line = line_counter, temp_loca = char_counter;
			if (buffer[forward] == '=') {												// ʶ��==��
				move();
				lex_log->sym = sym_not_equal;
			}
			else {																	// ʶ��=��
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
	} while (!end_flag && !to_file_end);															// �����޷�ʶ����ַ���
	lex_log->line_counter = line_counter;
	if (to_file_end) {																				// �ļ�����
		return -1;
	}
	return 0;
}

int next_sym(int isEnd) {
	if (index == 0) {								// ��������־��
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