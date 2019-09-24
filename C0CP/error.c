#include<stdio.h>
#include"error.h"

/*
	标准错误
*/
void error_stand(int error_code) {
	if (error_code == END_FILE) {
		return;
	}
	printf_s("ERROR: ");
	switch (error_code) {
	case FILE_NOT_FOUND:
		printf_s("FILE_NOT_FOUND \n");
		return;
	default:
		printf_s("UNKNOWN_STANDARD_ERROR \n");
	}
}

/*
	语法错误
*/
void commit_error_gramma(int error_code, int line) {
	printf_s("ERROR: ");
	switch (error_code) {
	case RECOVER_LEFT:
		printf_s("RECOVER_LEFT ");
		break;
	case RECOVER_RIGHT:
		printf_s("RECOVER_RIGHT ");
		break;
	case MISSMATCH_CONST_INT:
		printf_s("MISSMATCH_CONST_INT ");
		break;
	case MISSMATCH_IDENTIFIER:
		printf_s("MISSMATCH_IDENTIFIER ");
		break;
	case MISSMATCH_INT:
		printf_s("MISSMATCH_INT ");
		break;
	case MISSMATCH_CONST_CHAR:
		printf_s("MISSMATCH_CONST_CHAR ");
		break;
	case MISSMATCH_TYPE_DEF:
		printf_s("MISSMATCH_TYPE_DEF ");
		break;
	case MISSMATCH_VOID:
		printf_s("MISSMATCH_VOID ");
		break;
	case MISSMATCH_KEY_CONST:
		printf_s("MISSMATCH_KEY_CONST ");
		break;
	case MISSMATCH_MAIN:
		printf_s("MISSMATCH_MAIN ");
		break;
	case MISSMATCH_IF:
		printf_s("MISSMATCH_IF ");
		break;
	case MISSMATCH_SCANF:
		printf_s("MISSMATCH_SCANF ");
		break;
	case MISSMATCH_PRINTF:
		printf_s("MISSMATCH_PRINTF ");
		break;
	case MISSMATCH_SWITCH:
		printf_s("MISSMATCH_SWITCH ");
		break;
	case MISSMATCH_CASE:
		printf_s("MISSMATCH_CASE ");
		break;
	case MISSMATCH_DEFAULT:
		printf_s("MISSMATCH_DEFAULT ");
		break;
	case MISSMATCH_RETURN:
		printf_s("MISSMATCH_RETURN ");
		break;
	case MISSMATCH_SEMICOLON:
		printf_s("MISSMATCH_SEMICOLON ");
		break;
	case MISSMATCH_RIGHT_BRACKET:
		printf_s("MISSMATCH_RIGHT_BRACKET ");
		break;
	case MISSMATCH_LEFT_PARENT:
		printf_s("MISSMATCH_LEFT_PARENT ");
		break;
	case MISSMATCH_RIGHT_PARENT:
		printf_s("MISSMATCH_RIGHT_PARENT ");
		break;
	case MISSMATCH_LEFT_BRACE:
		printf_s("MISSMATCH_LEFT_BRACE ");
		break;
	case MISSMATCH_RIGHT_BRACE:
		printf_s("MISSMATCH_RIGHT_BRACE ");
		break;
	case MISSMATCH_BECOME:
		printf_s("MISSMATCH_BECOME ");
		break;
	case MISSMATCH_COMPARE:
		printf_s("MISSMATCH_COMPARE ");
		break;
	case MISSMATCH_COLON:
		printf_s("MISSMATCH_COLON ");
		break;
	case MISSMATCH_PARALIST_END:
		printf_s("MISSMATCH_PARALIST_END ");
		break;
	case MISS_MAIN:
		printf_s("MISS_MAIN ");
		break;
	case MISSBRANCH_PROGRAM:
		printf_s("MISSBRANCH_PROGRAM ");
		break;
	case MISSBRANCH_SENTENCE:
		printf_s("MISSBRANCH_SENTENCE ");
		break;
	case MISSBRANCH_FACTOR:
		printf_s("MISSBRANCH_FACTOR ");
		break;
	case EXCESSIVE_PROGRAM:
		printf_s("EXCESSIVE_PROGRAM ");
		break;
	case INT_OVERFLOW:
		printf_s("INT_OVERFLOW ");
		break;
	default:
		printf_s("UNKONWN_ERROR_GRAMMA ");
		break;
	}
	printf_s("at line %d \n", line);
}

/*
	词法错误
*/
void commit_error_lex(int error_code, int line, int loca) {
	if (error_code == 0) {
		return;
	}
	printf_s("ERROR: ");
	switch (error_code) {
	case LEX_ERROR:
		printf_s("LEX_ERROR ");
		break;
	case TOO_LARGE_INT:
		printf_s("TO_LARGE_INT ");
		break;
	case TOO_LONG_STRING:
		printf_s("TO_LONG_STRING ");
		break;
	case MISSMATCH_SYM_QUOTE:
		printf_s("MISSMATCH_SYM_QUOTE ");
		break;
	case MISSMATCH_SYM_DOUBLE_QUOTE:
		printf_s("MISSMATCH_SYM_DOUBLE_QUOTE ");
		break;
	case MISSMATCH_SYM_NOT_EQUAL:
		printf_s("MISSMATCH_SYM_NOT_EQUAL ");
		break;
	case MISSMATCH_CHAR:
		printf_s("MISSMATCH_CHAR ");
		break;
	case ILLEGAL_CHAR:
		printf_s("ILLEGAL_CHAR ");
		break;
	case EXCEPTIONAL_END:
		printf_s("EXCEPTIONAL_END ");
		break;
	default:
		printf_s("UNKNOWN_ERROR_LEX ");
	}
	printf_s("at line %d, char %d\n", line, loca);
}

/*
	语义错误
*/
void commit_error_sema(int error_code, int line, char* id) {
	if (error_code == NO_RETURN) {
		printf_s("WARNING: NO_RETURN at line %d \n", line);
		return;
	}
	printf_s("ERROR: ");
	switch (error_code) {
	case DUPLICATE:
		printf_s("DUPLICATE ");
		break;
	case WRONG_TYPE:
		printf_s("WRONG_TYPE ");
		break;
	case WRONG_ASSIGN_TYPE:
		printf_s("WRONG_ASSIGN_TYPE ");
		break;
	case ILLEGAL_COMPARE_TYPE:
		printf_s("ILLEGAL_COMPARE_TYPE ");
		break;
	case UNDEFINED:
		printf_s("UNDEFINED ");
		break;
	case MISTAKE_SUM_VALUE_PARA:
		printf_s("MISTAKE_SUM_VALUE_PARA ");
		break;
	case ILLEGAL_FUNC_CALL:
		printf_s("ILLEGAL_FUNC_CALL ");
		break;
	case ILLEGAL_ARRAY_SIZE:
		printf_s("ILLEGAL_ARRAY_SIZE ");
		break;
	case INDEX_OVERFLOW:
		printf_s("INDEX_OVERFLOW ");
		break;
	case ILLEGAL_FUNC_CALL_RET:
		printf_s("ILLEGAL_FUNC_CALL_RET ");
		break;
	case WRONG_KIND:
		printf_s("WRONG_KIND ");
		break;
	case WRONG_INDEX_TYPE:
		printf_s("WRONG_INDEX_TYPE ");
		break;
	case WRONG_RETURN_TYPE:
		printf_s("WRONG_RETURN_TYPE ");
		break;
	default:
		printf_s("UNKNOWN_ERROR_SEMA ");
	}
	printf_s("at line%d\t %s\n", line, id);
}

/*
	跳转信息
*/
void commit_jump_info(int line, char *id) {
	printf("IGNORE: line %d\t %s\n", line, id == NULL? "":id);
}

/*
	目标代码生成错误
*/
void commit_error_mips_gen(int error_code, char* info_1, char* info_2) {
	printf_s("ERROR: ");
	switch (error_code){
	case MISSMATCH_VAR:
		printf_s("MISSMATCH_VAR ");
		break;
	}
	return;
}

/*
	分块错误
*/
void commit_error_block(int error_code, int mid_code_line) {
	printf_s("ERROR: ");
	switch (error_code) {
	case ILLEGAL_BLOCK:
		printf_s("ILLEGAL_BLOCK ");
		break;
	}
	printf_s("at mid code line%d\n", mid_code_line);
	return;
}