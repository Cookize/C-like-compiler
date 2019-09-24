#include"lex_analyze.h"
#include"grammatical_analyze.h"
#include"mips_generator.h"
#include"optimize.h"
#include<stdio.h>

// 测试
FILE * file;

int main() {
	char path_file_in[129]= "text.txt";
	printf_s("Please input path of program: \n");
	scanf_s("%s", path_file_in, 128);
	if (set_input_file(path_file_in) != 0) { 
		return -1;
	}
	
	analyze_program();					// 词法分析、语法分析、语义分析、中间代码生成
	start_generator();					// 目标代码生成
	optimize();							// 优化
	printf_s("END\n");
}