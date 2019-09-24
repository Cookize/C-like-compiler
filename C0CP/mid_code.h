#ifndef __MID_H_
#define __MID_H_
#define _CRT_SECURE_NO_WARNINGS
#define SIZE_MID_CODE				100000
#define SUM_TEMP_VAR				1000
#define TEMP_VAR(x)					"tx"
#define SYM2COMPARE(x)				(OP_FOUR_CODE)(x - 9)



typedef enum four_code_op {

				// standard operation
	GEI,		// GET a b c ----- a = b[c]
	AEI,		// AEI a b c ----- a[b] = c
	ADD,		// ADD a b c ----- a = b + c
	SUB,		// SUB a b c ----- a = b - c
	MUL,		// MUL a b c ----- a = b * c
	DIV,		// DIV a b c ----- a = b / c
	ASN,		// ASN a b ------- a = b
	

				// compare operation
	LES,		// LES a b ------- a < b
	LEQ,		// LEQ a b ------- a <= b
	GTR,		// GTR a b ------- a > b
	GEQ,		// GTE a b ------- a >= b
	EQL,		// EQL a b ------- a == b
	NEQ,		// NEQ a b ------- a != b

				// jump operation
	LABEL,		// LABEL LABEL_1 - LABEL_1:
	GOTO,		// GOTO LABEL_1 -- jump to LABEL_1
	BZ,			// BZ LABEL_1 --- if condition is false then jump to LABEL_1

				// function operation
	FUNC,		// FUNC type name - declare a function named "name" returning a value in "type"
	PARA,		// PARA type a ---- declare a parameter "a" in "type"	
	PUSH,		// PUSH a --------- transmit value para
	CALL,		// CALL a --------- call a function named "a"
	RET,		// RET a ---------- return (a)

				// declare operation
	CONST,		// CONST INT a 10 - declare a constant and assign to 10
	VAR,		// VAR INT a ------ declare a variable
	ARRAY,		// ARRAY INT a ---- declare a array of integer

				// I/O operation
	READ,		// READ a INT ----- read a in type of integer
	PRINT,		// PRINT a INT ---- print a in type of integer

	END,		// END ------------ end of program
	NOP,		// NOP ------------ nop

} OP_FOUR_CODE;

/*
	基本块标记
*/
typedef enum BLKMARK {
	IGNORE,					// 忽略		
	BLKEND,					// 基本块结尾
	FUNCSTART,				// 函数块开头
	NORMAL,					// 识别语
	DEFPARA,
} BLK_MARK;

typedef struct FOUR_CODE {
	OP_FOUR_CODE op;		// 操作符
	char* a;
	char* b;
	char* c;
	BLK_MARK is_break_point;		// 基本块标记位：true---基本块分割点
} *four_code, four_code_obj;


extern int gen(OP_FOUR_CODE op, char* a, char* b, char* c);
extern int alloc_label(char** temp);
extern void init_mid_code();
extern void reset_mid_code();
extern void print_all_mid_code();
extern void insert_break_point(int);
extern void print_four_code_insert(void* out, four_code code);
extern void print_mid_code_to(char* addr);
extern int mark_mid_code();
extern void copy_marked_mid_code(int start, int end);
extern void reverse_top_compare();
extern four_code mid_code[SIZE_MID_CODE];
extern int length_mid_code;
#endif // !__MID_H_

