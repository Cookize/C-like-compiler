#ifndef __ERROR_H_
#define __ERROR_H_

/*
	error code
*/
/*
	standard error
*/
#define	ILLEGAL_NUM				1					// 函数传入参数错误
#define FILE_NOT_FOUND			2					// 文件打开失败
#define END_FILE				3					// 读取至文件尾部，结束程序
#define OVERFLOW_MID_CODE		4					// 中间代码超限

/*
	lex error
*/
#define LEX_ERROR				1					// 词法错误
#define TOO_LARGE_INT			2					// 输入数字溢出
#define ILLEGAL_INT				3					// 非法数字格式
#define TOO_LONG_STRING			4					// 输入字符串溢出
#define MISSMATCH_SYM_QUOTE			5		// 单引号匹配错误
#define MISSMATCH_SYM_DOUBLE_QUOTE	6		// 双引号匹配错误
#define MISSMATCH_SYM_NOT_EQUAL		7		// 不等号匹配错误
#define MISSMATCH_CHAR			8					// 字符匹配错误
#define ILLEGAL_CHAR			9					// 非法标识符
#define EXCEPTIONAL_END			10					// 异常结束

/*
	gramma error
*/
#define MISSMATCH_CONST_INT				1		// 未匹配到整型
#define MISSMATCH_IDENTIFIER			2		// 未匹配到标识符
#define MISS_MAIN						3		// 未匹配到主函数
#define MISSMATCH_INT					4		// 未匹配到无符号整数
#define MISSMATCH_CONST_CHAR			5		// 未匹配到字符常量
#define MISSMATCH_TYPE_DEF				6		// 未匹配到类型声明保留字“int”“char”
#define MISSMATCH_VOID					7		// 未匹配到“void”
#define	MISSMATCH_KEY_CONST				8		// 未匹配到“const”
#define MISSMATCH_MAIN					9		// 未匹配到“main”
#define MISSMATCH_IF					10		// 未匹配到“if”
#define MISSMATCH_SCANF					11		// 未匹配到“scanf”
#define MISSMATCH_PRINTF				12		// 未匹配到“printf”
#define MISSMATCH_SWITCH				13		// 未匹配到“switch”
#define MISSMATCH_CASE					14		// 未匹配到“case”
#define MISSMATCH_DEFAULT				15		// 未匹配到“default”
#define MISSMATCH_RETURN				16		// 未匹配到“return”
#define MISSMATCH_SEMICOLON				17		// 未匹配到分号
#define MISSMATCH_RIGHT_BRACKET			18		// 未匹配到右中括号
#define MISSMATCH_LEFT_PARENT			19		// 未匹配到左括号
#define MISSMATCH_RIGHT_PARENT			20		// 未匹配到右括号
#define MISSMATCH_LEFT_BRACE			21		// 未匹配到左大括号
#define MISSMATCH_RIGHT_BRACE			22		// 未匹配到右大括号
#define MISSMATCH_BECOME				23		// 未匹配到赋值号
#define MISSMATCH_COMPARE				24		// 未匹配到关系运算符
#define MISSMATCH_COLON					25		// 未匹配到冒号
#define MISSMATCH_PARALIST_END			26		// 参数表结束符匹配错误
#define	MISSMATCH_COMMA					27		// 未匹配到逗号
#define INT_OVERFLOW					28		// 整型越界

#define MISSBRANCH_PROGRAM				101		// <程序>中分支错误
#define MISSBRANCH_SENTENCE				102		// <语句>中分支错误
#define MISSBRANCH_FACTOR				103		// <因子>中分支错误

#define EXCESSIVE_PROGRAM				110		// 源程序超出<程序>
#define RECOVER_RIGHT					140		// 右括号恢复。
#define RECOVER_LEFT					141		// 左括号恢复。


/*
	sema error
*/
#define	DUPLICATE						1		// 重复定义
#define WRONG_TYPE						2		// 引用时类型错误
#define ILLEGAL_COMPARE_TYPE			3		// 比较式类型错误
#define UNDEFINED						4		// 未定义
#define MISTAKE_SUM_VALUE_PARA			5		// 函数调用值参数表数量错误
#define ILLEGAL_FUNC_CALL				6		// 无函数调用
#define ILLEGAL_ARRAY_SIZE				7		// 数组长度定义错误
#define INDEX_OVERFLOW					8		// 数组下标越界
#define ILLEGAL_FUNC_CALL_RET			9		// 非法调用无返回值函数
#define WRONG_ASSIGN_TYPE				10		// 赋值类型不匹配
#define WRONG_KIND						11		// 非法种类
#define WRONG_INDEX_TYPE				12		// 非法数组下标类型
#define NO_RETURN						13		// 函数无有效返回语句
#define WRONG_RETURN_TYPE				14		// 返回值类型错误

/*
	block error
*/
#define ILLEGAL_BLOCK					1		// 分块错误

/*
	mips generate error
*/
#define MISSMATCH_VAR					1		// 查表错误
extern void commit_error_lex(int, int, int);
extern void commit_error_gramma(int, int);
extern void commit_error_sema(int, int, char*);
extern void error_stand(int);
extern void commit_jump_info(int line, char *id);
extern void commit_error_mips_gen(int error_code, char* info_1, char* info_2);
extern void commit_error_block(int, int);

#endif // !__ERROR_H_
