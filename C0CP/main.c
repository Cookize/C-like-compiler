#include"lex_analyze.h"
#include"grammatical_analyze.h"
#include"mips_generator.h"
#include"optimize.h"
#include<stdio.h>

// ����
FILE * file;

int main() {
	char path_file_in[129]= "text.txt";
	printf_s("Please input path of program: \n");
	scanf_s("%s", path_file_in, 128);
	if (set_input_file(path_file_in) != 0) { 
		return -1;
	}
	
	analyze_program();					// �ʷ��������﷨����������������м��������
	start_generator();					// Ŀ���������
	optimize();							// �Ż�
	printf_s("END\n");
}