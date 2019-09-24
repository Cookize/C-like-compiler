#include"symbol_table.h"
#include"mid_code.h"
#include"grammatical_analyze.h"
#include"lex_analyze.h"
#include"error.h"
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<limits.h>

// 输出
#include<stdio.h>
/*
	错误恢复策略中的有界符：（按层次由内到外）
		， --- 1
		） --- 2
		； --- 3
		}  --- 4
*/

symbol jump_semi_rbr[] = { sym_semicolon, sym_right_brace };
symbol jump_comma_semi[] = { sym_comma, sym_semicolon };
symbol jump_comma_semi_rbr[] = { sym_comma, sym_semicolon, sym_right_brace };
symbol jump_comma_rpr_semi_rbr[] = { sym_comma, sym_right_parent, sym_semicolon, sym_right_brace };
symbol jump_rpr_semi_rbr[] = { sym_right_parent, sym_semicolon, sym_right_brace };
symbol jump_case_default_rbr[] = { key_case, key_default, sym_right_brace };
symbol jump_rbr[] = { sym_right_brace };
symbol jump_semi[] = { sym_semicolon };
symbol *jump_list[] = { jump_semi_rbr, jump_comma_semi, jump_comma_semi_rbr, jump_comma_rpr_semi_rbr,
					   jump_rpr_semi_rbr, jump_case_default_rbr, jump_rbr, jump_semi };
int length_jump_list[] = { 2, 2, 3, 4, 3, 3, 1, 1 };

int return_counter = 0;											// 计算函数返回语句。
Func_type flag_func = REVOID;									// 函数返回类型标记。

/*
	跳读符号检查
*/
symbol is_in(int index_jump_list) {
	int i;
	for (i = 0; i < length_jump_list[index_jump_list]; i++) {
		if (lex_log->sym == jump_list[index_jump_list][i]) {
			return jump_list[index_jump_list][i];
		}
	}
	return -1;
}

/*
	左括号容错函数
*/
int recover_left(symbol sym) {
	if (lex_log->sym == sym)	return 0;
	switch (sym) {
	case sym_left_parent:
		commit_error_gramma(MISSMATCH_LEFT_PARENT, lex_log->line_counter);
		break;
	case sym_left_brace:
		commit_error_gramma(MISSMATCH_LEFT_BRACE, lex_log->line_counter);
		break;
	}
	if (lex_log->sym != sym_left_brace && lex_log->sym != sym_left_parent && lex_log->sym != sym_left_bracket) {
		bk_sym(1);
	}
	else commit_error_gramma(RECOVER_LEFT, lex_log->line_counter);
	return -1;
}

/*
	右括号容错
*/
int recover_right(symbol sym) {
	if (lex_log->sym == sym)	return 0;
	switch (sym) {
	case sym_right_brace:
		commit_error_gramma(MISSMATCH_RIGHT_BRACE, lex_log->line_counter);
		break;
	case sym_right_parent:
		commit_error_gramma(MISSMATCH_RIGHT_PARENT, lex_log->line_counter);
		break;
	case sym_right_bracket:
		commit_error_gramma(MISSMATCH_RIGHT_BRACKET, lex_log->line_counter);
		break;
	}
	if (lex_log->sym != sym_right_brace && lex_log->sym != sym_right_parent && lex_log->sym != sym_right_bracket) {
		bk_sym(1);
	}
	else commit_error_gramma(RECOVER_RIGHT, lex_log->line_counter);
	return -1;
}

/*
	分隔符容错
*/
int recover(symbol sym) {
	if (lex_log->sym != sym) {
		bk_sym(1);
		return -1;
	}
	return 0;
}
/*
	容错跳转函数。
	跳转至jump_list集合中的符号的位置。
	返回值为停止时符号对应的枚举类。
*/
symbol jump(int index_jump_list, int if_next_sym) {
	symbol match;
	while ((match = is_in(index_jump_list)) == -1) {
		commit_jump_info(lex_log->line_counter, symbol_output[lex_log->sym]);
		next_sym(0);
	}
	if (if_next_sym)	next_sym(0);
	return match;
}

void check_overflow(char**ans) {
	if (isdigit(*ans[0]) || *ans[0] == '-') {														// 计算时检查越界，越界置零
		if (atoll(*ans) < INT_MIN || atoll(*ans) > INT_MAX) {
			commit_error_gramma(INT_OVERFLOW, lex_log->line_counter);
			*ans = "0";
		}
	}
}

/*
	＜整数＞::=［＋｜－］＜无符号整数＞
	错误恢复策略：
		放弃本次整数的识别，上报错误；
*/
int analyze_int(int *num, char **nums) {
	long long temp = 1;
	*nums = (char*)malloc(sizeof(char) * 20);
	*num = 1;
	if (lex_log->sym == sym_plus) {									// 正整数
		temp = 1;
		next_sym(0);
	}
	else if (lex_log->sym == sym_minus) {							// 负整数
		temp = -1;
		next_sym(0);
	}
	if (lex_log->sym != const_int) {								// 匹配无符号数
		commit_error_gramma(MISSMATCH_INT, lex_log->line_counter);
		return -1;
	}
	temp *= lex_log->num;
	if (temp > INT_MAX || temp < INT_MIN) {							// 整数识别时越界检查
		commit_error_gramma(INT_OVERFLOW, lex_log->line_counter);
		temp = 0;
	}
	*num = (int)temp;
	next_sym(0);
	sprintf(*nums, "%d", *num);
	return 0;
	
}

/*
	＜常量定义＞::= int＜标识符＞＝＜整数＞{,＜标识符＞＝＜整数＞}
                  | char＜标识符＞＝＜字符＞{,＜标识符＞＝＜字符＞}
	错误恢复策略：
		类型标识符匹配错误时，放弃本次常量定义的识别，上报错误；
		标识符匹配错误时，放弃单次常量的声明，跳转右界符：
			若跳转至逗号，接续分析；
			若跳转至分号或右大括号，上报错误；
		整数、字符匹配错误时，同标识符匹配错误的处理；
		“=”缺失时，补全；
	上报错误：
		跳转至分号；

*/
int analyze_const_def() {
	symbol jump_flag;
	symbol type_flag = lex_log->sym;
	char *id, *constant;
	int const_int;
	if (type_flag != key_int && type_flag != key_char) {												// 未匹配到“int”“char”
		commit_error_gramma(MISSMATCH_TYPE_DEF, lex_log->line_counter);
		return jump(JUMP_SEMI, 0);																		// 跳转至分号			
	}
	do {
		jump_flag = 0;
		next_sym(0);
		if (lex_log->sym != identifier) {																// 匹配标识符
			commit_error_gramma(MISSMATCH_IDENTIFIER, lex_log->line_counter);
			jump_flag = jump(JUMP_COMMA_SEMI, 0);
			continue;
		}
		id = (char*)malloc(sizeof(char) * strlen(lex_log->str));
		strcpy(id, lex_log->str);
		next_sym(0);																						// 匹配“=”
		if(recover(sym_become) != 0)	commit_error_gramma(MISSMATCH_BECOME, lex_log->line_counter);
		next_sym(0);
		if (type_flag == key_int) {																		// check <整数>
			if (analyze_int(&const_int, &constant) != 0) {
				jump_flag = jump(JUMP_COMMA_SEMI, 0);
				continue;
			}
			enter_const(id, INT, const_int, lex_log->line_counter);										// enter tbl
			gen(CONST, "INT", id, constant);															// CONST INT id constant
		}
		else {																							// check 字符
			if (lex_log->sym != const_char) {
				commit_error_gramma(MISSMATCH_CONST_CHAR, lex_log->line_counter);
				jump_flag = jump(JUMP_COMMA_SEMI, 0);
				continue;
			}
			enter_const(id, CHAR, (int)lex_log->ch, lex_log->line_counter);								// enter tbl
			char constant[] = { lex_log->ch , '\0'};
			gen(CONST, "CHAR", id, constant);															// CONST CHAR id constant

			next_sym(0);
		}
	} while (lex_log->sym == sym_comma);																// 匹配逗号，循环匹配
	return jump_flag;
}

/*
	＜常量说明＞::= const＜常量定义＞;{ const＜常量定义＞;}
	错误恢复策略：
		缺少分号时，补全分号；
		常量定义错误时，根据跳转结果：
			若跳转到分号，继续分析；
			若跳转到右大括号，上报错误；
		首个“const”无法匹配时，退出常量说明的识别，但不报错；
	不上报错误；
*/
int analyze_const_dec() {
	if (lex_log->sym != key_const)	return 0;															// 匹配“const”
	do {
//		printf_s("This is a const declaration at line %d. \n", lex_log->line_counter);
		next_sym(0);
		analyze_const_def();																			// <常量定义>，跳转至分号，继续分析
		if(recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);// 匹配分号
		next_sym(0);
	} while (lex_log->sym == key_const);
	return 0;
}

/*
	处理<程序>中不满足LL(1)文法的分支情况，返回值为分支选择结果。
*/
int analyze_branch_program() {
	if (lex_log->sym == key_void) {														// <无返回值函数定义>或<主函数>
		next_sym(0);
		if (lex_log->sym == func_main) {								
			bk_sym(1);
			return BRANCH_MAIN;															// <主函数>
		}
		bk_sym(1);
		return BRANCH_FUNC_DEF;															// <无返回值函数定义>
	}
	else if (lex_log->sym == key_char || lex_log->sym == key_int) {						// <有返回值函数定义>或<变量声明>
		next_sym(0);
		if (lex_log->sym == identifier) {
			next_sym(0);
			if (lex_log->sym == sym_left_parent) {
				bk_sym(2);
				return BRANCH_REFUNC_DEF;												// <有返回值函数定义>
			}
			bk_sym(1);
		}
		bk_sym(1);
		return BRANCH_VAR_DEC;															// <变量声明>
	}
	return -1;
}

/*
	＜变量定义＞::=＜类型标识符＞(＜标识符＞|＜标识符＞'['＜无符号整数＞']'){,(＜标识符＞|＜标识符＞'['＜无符号整数＞']' )} 
	注意：＜无符号整数＞表示数组元素的个数，其值需大于0
	错误恢复策略：
		使用括号恢复策略；
		类型标识符匹配错误时，放弃本变量定义的识别，跳转至右界符：
			跳转至分号或右大括号，上报错误；
		标识符或无符号整数匹配错误时，放弃单次变量声明的识别，跳转至变量声明右界符：
			若跳转至逗号，继续分析；
			若跳转至分号或右大括号，上报错误；
	上报错误：
		跳转至分号；
*/
int analyze_var_def() {
	symbol jump_flag;
	Type type;
	char *id;
	int array_length;
//	printf_s("This is a variable declaration at line %d. \n", lex_log->line_counter);
	type = (lex_log->sym == key_int) ? INT : CHAR;
	do {
		jump_flag = 0;
		next_sym(0);
		if (lex_log->sym != identifier) {										// <标识符>
			commit_error_gramma(MISSMATCH_IDENTIFIER, lex_log->line_counter);
			jump_flag = jump(JUMP_COMMA_SEMI, 0);								// 跳转
			continue;
		}
		id = (char*)malloc(sizeof(char) * strlen(lex_log->str));
		strcpy(id, lex_log->str);
		next_sym(0);
		if (lex_log->sym == sym_left_bracket) {									// [ , 数组
			next_sym(0);
			if (lex_log->sym != const_int) {									// <无符号整数>
				commit_error_gramma(MISSMATCH_CONST_INT, lex_log->line_counter);
				jump_flag = jump(JUMP_COMMA_SEMI, 0);							// 跳转
				continue;
			}
			if (lex_log->num <= 0) {											// 检查数组长度
				commit_error_sema(ILLEGAL_ARRAY_SIZE, lex_log->line_counter, id);
				lex_log->num = 1;
			}
			array_length = (int)lex_log->num;
			next_sym(0);
			if (recover(sym_right_bracket) != 0)	commit_error_gramma(MISSMATCH_RIGHT_BRACKET, lex_log->line_counter);	// ]
			enter_array(id, type, array_length, lex_log->line_counter);			// enter tbl	
			gen(ARRAY, type == INT ? "INT" : "CHAR", id, NULL);					// ARRAY INT/CHAR id
			next_sym(0);
		}
		else {
			enter_var(id, type, lex_log->line_counter);							// enter tbl
			gen(VAR, type == INT ? "INT" : "CHAR", id, NULL);					// VAR INT/CHAR id
		}
			
	} while (lex_log->sym == sym_comma);										// ,
	return jump_flag;
}

/*
	＜变量说明＞::=＜变量定义＞;{＜变量定义＞;}
	错误恢复策略：
		缺少分号时，补全分号；
		变量定义出错时，根据跳转结果：
			跳转至分号，继续分析；
			跳转至右大括号，上报错误；
	不上报错误：
*/
int analyze_var_dec() {
	do {
		analyze_var_def();																						// ＜变量定义＞	
		if (recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);	// ;
		next_sym(0);
	} while (analyze_branch_program() == BRANCH_VAR_DEC);
	return 0;
}

/*
	＜参数表＞	::= ＜参数＞{,＜参数＞}
					|＜空>
	＜参数＞	::= ＜类型标识符＞＜标识符＞
	错误恢复策略：
		参数识别出现错误时，跳转至参数的右界符（逗号、右括号）：
			若跳转至逗号，继续分析；
			若跳转至右括号、分号、右大括号，上报错误
	上报错误：
		跳转至右括号、分号、右大括号；
*/
// FINISHED
int analyze_para_list() {
	Type type;
	if (lex_log->sym == sym_right_parent)	return 0;							// <空>
	symbol jump_flag;
	bk_sym(1);
	do {
		jump_flag = 0;
		next_sym(0);
		if (lex_log->sym == key_int)	type = INT;								// int
		else if (lex_log->sym == key_char)	type = CHAR;						// char
		else {
			commit_error_gramma(MISSMATCH_TYPE_DEF, lex_log->line_counter);
			jump_flag = jump(JUMP_COMMA_RPR_SEMI_RBR, 0);
			continue;
		}
		next_sym(0);
		if (lex_log->sym != identifier) {										// <标识符>
			commit_error_gramma(MISSMATCH_IDENTIFIER, lex_log->line_counter);
			jump_flag = jump(JUMP_COMMA_RPR_SEMI_RBR, 0);
			continue;
		}
		enter_para(lex_log->str, type, lex_log->line_counter);					// enter tbl
		gen(PARA,type == INT ? "INT" : "CHAR", lex_log->str, NULL);				// PARA id
		next_sym(0);
	} while (lex_log->sym == sym_comma);										// ,
	return jump_flag;
}

/*
	处理<语句>中不满足LL(1)文法的分支情况，返回值为分支选择结果。
*/
int analyze_branch_sentence() {
	if (lex_log->sym == key_if) {													// < if 语句 >
		return BRANCH_IF;
	}
	else if (lex_log->sym == key_while) {											// < while 语句 >
		return BRANCH_WHILE;
	}
	else if (lex_log->sym == sym_left_brace) {										// {<语句列>}
		return BRANCH_SENTENCE_LIST;
	}
	else if (lex_log->sym == identifier) {											// <函数调用语句>或<赋值语句>
		next_sym(0);
		if (lex_log->sym == sym_left_parent) {										// <函数调用语句>
			bk_sym(1);
			return BRANCH_CALL;
		}
		else if(lex_log->sym == sym_become || lex_log->sym == sym_left_bracket) {
			bk_sym(1);
			return BRANCH_ASSIGN;													// <赋值语句>
		}
	}
	else if (lex_log->sym == func_scanf) {											// <读语句>
		return BRANCH_SCANF;
	}
	else if (lex_log->sym == func_printf) {											// <写语句>
		return BRANCH_PRINTF;				
	}
	else if (lex_log->sym == sym_semicolon) {										// <空语句>
		return BRANCH_NULL_SENTENCE;
	}
	else if (lex_log->sym == key_switch) {											// <情况语句>
		return BRANCH_SWITCH;
	}
	else if (lex_log->sym == key_return) {											// <返回语句>
		return BRANCH_RETURN;
	}
	return -1;
}

// 声明
int analyze_sentence();
int analyze_exp(char**, Type*);

/*	
	＜条件＞::=  ＜表达式＞＜关系运算符＞＜表达式＞
				｜＜表达式＞  // 表达式为0条件为假，否则为真
	错误恢复策略：
		首个表达式匹配错误时，退出条件的识别，并默认条件值为0；
		关系运算符匹配错误时，退出条件的识别，并默认条件值为首个表达式的值；
		次级表达式匹配错误时，退出条件的识别，并默认条件值为首个表达式的值；
		退出后跳转至右界符：
			跳转至右括号、分号或右大括号，上报错误；
	上报错误：
		跳转至右括号、分号、右大括号；
*/
// FINISHED
int analyze_condition() {
	char *exp_1, *exp_2;
	symbol op = sym_equal;
	Type type_1, type_2;
	if (analyze_exp(&exp_1, &type_1) != 0) {								// <表达式>
		return jump(JUMP_RPR_SEMI_RBR, 0);									// 跳转至右括号、分号或右大括号；				
	}
	if (type_1 != INT) {													// 检查比较式类型
		commit_error_sema(ILLEGAL_COMPARE_TYPE, lex_log->line_counter, "");
	}
	if (lex_log->sym == sym_less 
		|| lex_log->sym == sym_less_equal
		|| lex_log->sym == sym_greater
		|| lex_log->sym == sym_greater_equal
		|| lex_log->sym == sym_equal
		|| lex_log->sym == sym_not_equal) {										// 关系运算符
		op = lex_log->sym;
	}
	else if (lex_log->sym == sym_right_parent) {							// 无关系运算符
		gen(NEQ, exp_1, "0", NULL);										// EQL exp_1 0
		return 0;
	}
	else {
		commit_error_gramma(MISSMATCH_COMPARE, lex_log->line_counter);
		bk_sym(1);
	}
	next_sym(0);
	if (analyze_exp(&exp_2, &type_2) != 0) {								// <表达式>
		return jump(JUMP_RPR_SEMI_RBR, 0);									// 跳转至右括号、分号或右大括号；	
	}
	if (type_2 != INT) {
		commit_error_sema(ILLEGAL_COMPARE_TYPE, lex_log->line_counter, "");
	}
	gen(SYM2COMPARE((int)op), exp_1, exp_2, NULL);							// COMPARE exp_1 exp_2
	return 0;
}

/*
	＜条件语句＞::=  if '('＜条件＞')'＜语句＞
	错误恢复策略：
		采用括号恢复策略；
		关键字“if”匹配错误时，放弃条件语句的识别，跳转至分号以上，上报错误；
		条件匹配错误时：
			若跳转到右括号，继续分析；
			若跳转到分号以上，上报错误；
		语句匹配错误时，退出条件语句的识别，根据调准信息上报错误；
	上报错误：
		跳转至分号、右大括号；
*/
// FINISHED
int analyze_if() {
	symbol jump_flag = 0;
	char* label;
	next_sym(0);
	recover_left(sym_left_parent);																	// (
	next_sym(0);
	if ((jump_flag = analyze_condition()) != sym_right_parent && jump_flag != 0) return jump_flag;	// <条件>
	if (jump_flag == 0) {
		recover_right(sym_right_parent);															// )
	}
	alloc_label(&label);
	gen(BZ, label, NULL, NULL);																		// BZ label
	next_sym(0);
	jump_flag = analyze_sentence();
	gen(LABEL, label, NULL, NULL);																	// LABEL label
	return jump_flag;																				// <语句>
}

/*
	＜循环语句＞::=  while '('＜条件＞')'＜语句＞
	错误恢复策略：
		采用括号恢复策略；
		关键字“while”匹配错误时，放弃条件语句的识别，跳转至分号以上，上报错误；
		条件匹配错误时：
			若跳转到右括号，继续分析；
			若跳转到分号以上，上报错误；
		语句匹配错误时，退出条件语句的识别，根据调准信息上报错误；
	上报错误：
		跳转至分号、右大括号；
*/
// FINISHED
int analyze_while() {
	int start, end;
	symbol jump_flag = 0;
	char *label_start, *label_end;
	alloc_label(&label_start);
	alloc_label(&label_end);
	next_sym(0);

	start = mark_mid_code();																	// 开始记录条件判断的代码段
	recover_left(sym_left_parent);																	// (
	next_sym(0);
	if ((jump_flag = analyze_condition()) != sym_right_parent && jump_flag != 0) return jump_flag;	// <条件>
	recover_right(sym_right_parent);																// )
	end = mark_mid_code();																		// 结束记录条件判断的代码段

	gen(BZ, label_end, NULL, NULL);																	// BZ label
	gen(LABEL, label_start, NULL, NULL);															// LABEL label_start
	next_sym(0);
	jump_flag = analyze_sentence();																	// 语句
	copy_marked_mid_code(start, end);																// 复制条件判断代码段
	reverse_top_compare();
	gen(BZ, label_start, NULL, NULL);																// BZ label

	gen(LABEL, label_end, NULL, NULL);																// LABEL label_end
	return jump_flag;
}

/*
	＜值参数表＞::=＜表达式＞{,＜表达式＞}｜＜空＞
	错误恢复策略：
		表达式匹配错误时，根据表达式调准信息：
			若跳转到逗号，继续分析；
			若跳转至右括号以上，上报错误；
		无正确表达式是，值参数表等价于<空>；
	上报错误：
		跳转至右括号、分号、右大括号；
*/
// FINISHD
int analyze_value_list(tbl_field func) {
	char *exp, **para_list;
	Type type;
	int counter = 1;																			// 计数器
	tbl_item para;
	if (lex_log->sym == sym_right_parent)	return 0;											// <空>
	symbol jump_flag = 0;
	bk_sym(1);
	para_list = (char**)malloc(sizeof(char*) * func->para_sum);									// 缓存传入参量
	do {
		jump_flag = 0;
		next_sym(0);
		if (analyze_exp(&exp, &type) != 0) {													// <表达式>
			jump_flag = jump(JUMP_COMMA_RPR_SEMI_RBR, 0);										// 跳转至逗号、右括号、分号、右大括号
			continue;					
		}
		if ((para = get_id(func->first_item_loca + counter)) != NULL && type != para->type) {
			commit_error_sema(WRONG_TYPE, lex_log->line_counter, "");							// 值参量类型检查
		}
		para_list[counter - 1] = exp;
		//gen(PUSH, exp, NULL, NULL);
		counter++;
	} while (lex_log->sym == sym_comma);														// ,
	if (counter != func->para_sum + 1)															// 值参数数量检查
		commit_error_sema(MISTAKE_SUM_VALUE_PARA, lex_log->line_counter, func->func_name);

	else {
		int i;
		for (i = 0; i < func->para_sum; i++) {
			gen(PUSH, para_list[i], NULL, NULL);												// PUSH exp
		}
	}

	return jump_flag;
}

/*
	＜有返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
	＜无返回值函数调用语句＞ ::= ＜标识符＞'('＜值参数表＞')'
	错误恢复策略：
		不采取括号恢复策略；
		标识符匹配错误时，跳转至分号以上，上报错误；
		值参数表匹配错误时，根据跳转信息：
			若跳转至右括号，继续分析；
			若跳转至分号以上，上报错误；
	上报错误：
		跳转至分号、右大括号；
*/
// FINISHED
int analyze_call(Func_type* func_type) {
	symbol jump_flag;
	tbl_item item = find_id(lex_log->str);											// <标识符>
	if (item == NULL || item->kind != Func) {											// 检查是否为函数
		commit_error_sema(ILLEGAL_FUNC_CALL, lex_log->line_counter, lex_log->str);
		return jump(JUMP_SEMI_RBR, 0);
	}
	*func_type = item->return_type;
	tbl_field func = find_field(lex_log->str);
	next_sym(0);																			// (
	next_sym(0);
	if ((jump_flag = analyze_value_list(func)) != sym_right_parent && jump_flag != 0) {	// <值参数表>
		return jump_flag;
	}
	recover_right(sym_right_parent);													// )
	gen(CALL, item->id, NULL, NULL);													// CALL func
	next_sym(0);
	return 0;
}

/*
	＜赋值语句＞::=＜标识符＞＝＜表达式＞
				|＜标识符＞'['＜表达式＞']'=＜表达式＞
	错误恢复策略：
		任何错误均退出赋值语句的识别，跳转至分号以上，上报错误；
	上报错误：
		跳转至分号、右大括号
*/
// FINISHED
int analyze_assign() {
	int array_flag = 0;
	char *exp_1, *exp_2 = "";
	Type type_1, type_2;
	tbl_item id = find_id(lex_log->str);															// 搜索符号表
	if (id == NULL) {																				// 检查符号是否存在
		commit_error_sema(UNDEFINED, lex_log->line_counter, lex_log->str);
		return jump(JUMP_SEMI_RBR, 0);
	}
	else if(!(id->kind == Var || id->kind == Para)){												// 检查符号是否为变量或参量
		commit_error_sema(WRONG_KIND, lex_log->line_counter, lex_log->str);
		return jump(JUMP_SEMI_RBR, 0);
	}
	next_sym(0);
	if (lex_log->sym == sym_left_bracket) {															// [
		if(id->array_length > 0) array_flag = 1;													// 检查符号是否为数组
		next_sym(0);
		if (analyze_exp(&exp_2, &type_2) != 0) return jump(JUMP_SEMI_RBR, 0);						// <表达式>
		if (array_flag == 1 && isdigit(exp_2[0]) && atoi(exp_2) >= id->array_length) {				// 检查索引是否越界
			commit_error_sema(INDEX_OVERFLOW, lex_log->line_counter, id->id);
			exp_2 = "0";
		}
		if (exp_2[0] == '-') {
			commit_error_sema(INDEX_OVERFLOW, lex_log->line_counter, id->id);
			exp_2 = "0";
		}
		recover_right(sym_right_bracket);															// ]
		next_sym(0);
	}
	if (recover(sym_become) != 0)	commit_error_gramma(MISSMATCH_BECOME, lex_log->line_counter);	// =
	next_sym(0);
	if (analyze_exp(&exp_1, &type_1) != 0) return jump(JUMP_SEMI_RBR, 0);							// <表达式>
	if (id->type == INT && type_1 == INT) {
		array_flag ? gen(AEI, id->id, exp_2, exp_1) : gen(ASN, id->id, exp_1, NULL);				// AEI id exp_2 exp_1 / ASN id exp_1 0
		
	}
	else if (id->type == CHAR && type_1 == CHAR) {
		array_flag ? gen(AEI, id->id, exp_2, exp_1) : gen(ASN, id->id, exp_1, NULL);
	}
	else {
		array_flag ? gen(AEI, id->id, exp_2, exp_1) : gen(ASN, id->id, exp_1, NULL);
		commit_error_sema(WRONG_ASSIGN_TYPE, lex_log->line_counter, id->id);
	}
	
	return 0;
}

/*
	＜读语句＞::= scanf '('＜标识符＞{,＜标识符＞}')'
	错误恢复策略：
		采取括号恢复策略；
		标识符识别错误时，跳转至有界符：
			若跳转至分号以上，上报错误；
	上报错误：
		跳转至分号、右大括号；
*/
int analyze_scanf() {
	tbl_item item;
	next_sym(0);																	// scanf
	recover_left(sym_left_parent);												// (
	do {
		next_sym(0);
		if (lex_log->sym != identifier) {										// <标识符>
			commit_error_gramma(MISSMATCH_IDENTIFIER, lex_log->line_counter);
			return jump(JUMP_SEMI_RBR, 0);										// 跳转至分号、右大括号					
		}	
		if ((item = find_id(lex_log->str)) == NULL) {							// 查表
			commit_error_sema(UNDEFINED, lex_log->line_counter, lex_log->str);
		}
		else if (item->kind != Para && item->kind != Var) {
			commit_error_sema(WRONG_KIND, lex_log->line_counter, lex_log->str);
		}
		else {
			gen(READ, item->id, item->type == INT ? "INT" : "CHAR", NULL);
		}
		next_sym(0);
	} while (lex_log->sym == sym_comma);										// ,
	recover_right(sym_right_parent);											// )
	next_sym(0);
	return 0;
}

/*
	＜写语句＞::= printf '(' ＜字符串＞,＜表达式＞ ')'
				| printf '('＜字符串＞ ')'
				| printf '('＜表达式＞')'
	错误恢复策略：
		采取括号恢复策略；
		内部错误时，跳转至右界符：
			若跳转至分号以上，上报错误；
	上报错误：
		跳转至分号、右大括号；
*/
// TODO: 
int analyze_printf() {
	char *exp, *temp;
	Type type;
	next_sym(0);										// printf
	recover_left(sym_left_parent);					// (
	next_sym(0);
	if (lex_log->sym == const_string) {				// <字符串>
		enter_str(lex_log->str, &temp);
		gen(PRINT, temp, "STR", NULL);				// PRINT temp STR
		next_sym(0);
		if (lex_log->sym == sym_right_parent) {		// )
			gen(PRINT, "10", "CHAR", NULL);
			next_sym(0);
			return 0;
		}
		if (recover(sym_comma) != 0)	commit_error_gramma(MISSMATCH_COMMA, lex_log->line_counter);
		next_sym(0);
	}
	if (analyze_exp(&exp, &type) != 0) return jump(JUMP_SEMI_RBR, 0);		// 跳转至分号、右大括号	
	gen(PRINT, exp, type == INT ? "INT" : "CHAR", NULL);
	gen(PRINT, "10", "CHAR", NULL);
	recover_right(sym_right_parent);				// )
	next_sym(0);
	return 0;
}

/*
	＜情况表＞		::=  ＜情况子语句＞{＜情况子语句＞}
	＜情况子语句＞  ::=  case＜常量＞：＜语句＞
	错误恢复策略：
		常量匹配错误时，跳转至右界符：
			若跳转至“case”，继续分析；
			若跳转至“default”，上报错误；
			若跳转至右大括号，上报错误；
		语句识别错误，继续上报错误；
		冒号匹配错误时，补全冒号；
	上报错误：
		跳转到default、右大括号；
*/
// FINISHED
int analyze_case(char* exp, Type type, char* label_end) {
	symbol jump_flag;
	char* label_case;
	int num;
//	printf_s("This is a case sentence at line %d. \n", lex_log->line_counter);
	if (lex_log->sym != key_case) {																// case
		commit_error_gramma(MISSMATCH_CASE, lex_log->line_counter);
		if ((jump_flag = jump(JUMP_CASE_DEFAULT_RBR, 0)) != key_case && jump_flag != 0)	return jump_flag;
	}
	do {
		jump_flag = 0;
		alloc_label(&label_case);
		next_sym(0);
		if (lex_log->sym == const_char) {														// <字符>
			char ch[5];
			sprintf(ch, "%d", (int)lex_log->ch);
			if (type != CHAR) commit_error_sema(WRONG_TYPE, lex_log->line_counter, ch);			// 类型检查
			gen(EQL, exp, ch, NULL);															// EQL exp ch
			gen(BZ, label_case, NULL, NULL);													// BZ label_case
			next_sym(0);
		}
		else {
			char* constant;
			if (analyze_int(&num, &constant) != 0) {											// <整数>
				jump_flag = jump(JUMP_CASE_DEFAULT_RBR, 0);										// 未跳转至case，上报错误；
				continue;
			}
			if (type != INT) commit_error_sema(WRONG_TYPE, lex_log->line_counter, constant);	// 类型检查
			gen(EQL, exp, constant, NULL);														// EQL exp constant
			gen(BZ, label_case, NULL, NULL);													// BZ label_case
		}
		if(recover(sym_colon) != 0)	commit_error_gramma(MISSMATCH_COLON, lex_log->line_counter);// :
		next_sym(0);
		jump_flag = analyze_sentence();															// <语句>
		gen(GOTO, label_end, NULL, NULL);														// GOTO label_end
		gen(LABEL, label_case, NULL, NULL);														// LABEL lebel_case
	} while (lex_log->sym == key_case);															// case
	return 0;
}

/*
	＜缺省＞::=  default : ＜语句＞|＜空＞
	错误恢复策略：
		关键字“default”匹配错误时，放弃缺省的识别，跳转至分号以上，上报错误；
		冒号匹配错误时，补全；
		语句匹配错误时，继续上报错误；
	上报错误：
		跳转至右大括号；
*/
// FINISHED
int analyze_default() {
	if (lex_log->sym != key_default) {															// default
		commit_error_gramma(MISSMATCH_DEFAULT, lex_log->line_counter);
		return jump(JUMP_RBR, 0);
	}
//	printf_s("This is a default sentence at line %d. \n", lex_log->line_counter);
	next_sym(0);
	if (recover(sym_colon) != 0) commit_error_gramma(MISSMATCH_COLON, lex_log->line_counter);	// :
	next_sym(0);
	if (lex_log->sym == sym_right_brace) {														// <空>
		return 0;
	}
	return analyze_sentence();					
}

/*
	＜情况语句＞::= switch '('＜表达式＞')' '{'＜情况表＞＜缺省＞'}'
	错误恢复策略：
		关键字“switch”匹配错误时，放弃情况语句的识别；
		采取括号恢复策略；
		表达式匹配错误时，退出情况语句的识别，跳转至右大括号，继续分析；
		情况表匹配错误时，跳转至右大括号，继续分析；
		缺省匹配错误时，跳转至右大括号，继续分析；
	不上报错误；
*/
// FINISHED
int analyze_switch() {
	symbol jump_flag;
	char *exp, *label;
	Type type;
	alloc_label(&label);
	next_sym(0);										// switch
	recover_left(sym_left_parent);					// (
	next_sym(0);
	if (analyze_exp(&exp, &type) != 0) {			// <表达式>
		if ((jump_flag = jump(JUMP_RPR_SEMI_RBR, 0)) != sym_right_parent && jump_flag != 0)	return jump_flag;	// 未跳转至右括号
	}
	recover_right(sym_right_parent);				// )
	next_sym(0);
	recover_left(sym_left_brace);					// {
	next_sym(0);
	jump_flag = analyze_case(exp, type, label);		// <情况表>
	if (jump_flag == sym_semicolon)
		next_sym(0);
	if (lex_log->sym == key_default) {				// <缺省>
		analyze_default();
	}
	recover_right(sym_right_brace);					// }
	gen(LABEL, label, NULL, NULL);					// LABEL label
	next_sym(0);
	return 0;
}

/*
	＜返回语句＞::= return['('＜表达式＞')'] 
	错误恢复策略：
		所有错误均退出返回语句的识别，跳转至分号及以上，上报错误；
	上报错误：
		跳转至分号、右大括号；
*/
// FINISHED
int analyze_return() {
	char *exp;
	Type type;
	next_sym(0);																		// return
	if (lex_log->sym == identifier) {												// (
		recover_left(sym_left_parent);
	}
	else if (lex_log->sym == sym_semicolon) {
		if (flag_func == REVOID) {
			gen(RET, NULL, NULL, NULL);												// RET
			return_counter++;
		}
		else	commit_error_sema(WRONG_RETURN_TYPE, lex_log->line_counter, "");	// 有返回值函数未返回值
		return 0;
	}
	else if(lex_log->sym != sym_left_parent) {
		commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);
		gen(RET, NULL, NULL, NULL);													// RET
		return 0;
	}
	next_sym(0);
	if (analyze_exp(&exp, &type) != 0)	return jump(JUMP_SEMI_RBR, 0);				// <表达式>
	recover_right(sym_right_parent);												// )
	if (flag_func == REVOID) {														// 无返回值函数返回值
		commit_error_sema(WRONG_RETURN_TYPE, lex_log->line_counter, "");			// 有返回值函数未返回值
		gen(RET, NULL, NULL, NULL);													// RET
	}
	else if ((flag_func == REINT && type == CHAR) || (flag_func == RECHAR && type == INT)) {
		commit_error_sema(WRONG_RETURN_TYPE, lex_log->line_counter, "");			// 有返回值函数未返回值
		gen(RET, exp, NULL, NULL);													// RET exp
	}
	else {
		gen(RET, exp, NULL, NULL);													// RET exp
	}
	return_counter++;
	next_sym(0);
	return 0;
}

/*
	＜语句列＞::=｛＜语句＞｝
	错误恢复策略：
		语句识别错误时，跳转至分号以上，上报错误；
	上报错误：
		跳转到右大括号
*/
int analyze_sentence_list() {
	symbol jump_flag;
	while (lex_log->sym != sym_right_brace) {
		if ((jump_flag = analyze_sentence()) == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
	}
	return 0;
}

/*
	＜语句＞::= ＜条件语句＞｜＜循环语句＞| '{'＜语句列＞'}'| ＜有返回值函数调用语句＞;
			|＜无返回值函数调用语句＞;｜＜赋值语句＞;｜＜读语句＞;｜＜写语句＞;｜＜空＞;|＜情况语句＞｜＜返回语句＞;
	错误分析策略：
		无后缀语句匹配错误时，跳转至分号及以上，上报错误；
		缺少分号时，补充分号；
	上报错误：
		跳转至分号、右大括号；
*/
int analyze_sentence() {
	int v;
	symbol jump_flag;
	Func_type ft;
	v = analyze_branch_sentence();
	switch (v) {
	case BRANCH_IF:																									// ＜条件语句＞
//		printf_s("This is a if sentence at line %d. \n", lex_log->line_counter);
		return analyze_if();
	case BRANCH_WHILE:																								// ＜循环语句＞
//		printf_s("This is a while sentence at line %d. \n", lex_log->line_counter);
		return analyze_while();
	case BRANCH_SENTENCE_LIST:																						// {<语句列>}
//		printf_s("This is a block at line %d. \n", lex_log->line_counter);
		next_sym(0);
		jump_flag = analyze_sentence_list();																		// <语句列>
		recover_right(sym_right_brace);																				// }
		next_sym(0);
		break;
	case BRANCH_CALL:																								// ＜有返回值函数调用语句＞; | ＜无返回值函数调用语句＞;
//		printf_s("This is a function call at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_call(&ft))  == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if(recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);			// ;
		next_sym(0);
		break;
	case BRANCH_ASSIGN:																								// ＜赋值语句＞;
//		printf_s("This is a assign sentence at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_assign()) == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if (recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);		// ;
		next_sym(0);
		break;
	case BRANCH_SCANF:																								// ＜读语句＞;
		//printf_s("This is a scanf sentence at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_scanf()) == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if (recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);		// ;
		next_sym(0);
		break;
	case BRANCH_PRINTF:																								// ＜写语句＞;
		//printf_s("This is a printf sentence at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_printf()) == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if(recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);			// ;
		next_sym(0);
		break;
	case BRANCH_NULL_SENTENCE:																						// ＜空＞;
		//printf_s("This is a null sentence at line %d. \n", lex_log->line_counter);
		next_sym(0);
		break;	
	case BRANCH_SWITCH:																								// <情况语句>
//		printf_s("This is a switch sentence at line %d. \n", lex_log->line_counter);
		analyze_switch();
		break;
	case BRANCH_RETURN:																								// ＜返回语句＞;
//		printf_s("This is a return sentence at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_return()) == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if (recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);		// ;
		next_sym(0);
		break;
	default:
		commit_error_gramma(MISSBRANCH_SENTENCE, lex_log->line_counter);
		return jump(JUMP_SEMI_RBR, 0);
	}
	return 0;
}

/*
	＜复合语句＞::=［＜常量说明＞］［＜变量说明＞］＜语句列＞
	错误恢复策略：
		常量说明匹配错误时，不做操作；
		变量说明匹配错误时，不做操作；
		语句列匹配错误时，不做操作；
*/
int analyze_complex_sentence() {
	if (lex_log->sym == key_const) {								// ［＜常量说明＞］
		analyze_const_dec();
	}
	if (lex_log->sym == key_int || lex_log->sym == key_char) {		// ［＜变量说明＞］
		analyze_var_dec();
	}
	if (analyze_sentence_list() == sym_right_brace) {				// ＜语句列＞
		bk_sym(1);
	}										
	return 0;
}

/*
	＜有返回值函数定义＞  ::= ＜声明头部＞'('＜参数表＞')' '{'＜复合语句＞'}'
	错误恢复策略：
		采取括号恢复策略；
		复合语句错误时，根据错误结果：
			若跳转到分号，继续分析；
			若跳转到右大括号，退出函数定义分析；

*/
int analyze_refunc_def() {
	char *func_name;
	symbol jump_flag;
	Func_type func_type = (lex_log->sym == key_int ? REINT : RECHAR);
	return_counter = 0;															// 重置函数返回语句
	flag_func = func_type;														// 重置函数返回类型标记
	next_sym(0);
	func_name = (char*)malloc(sizeof(char) * strlen(lex_log->str));
	strcpy(func_name, lex_log->str);
	enter_func(lex_log->str, func_type, lex_log->line_counter);
	gen(FUNC, func_type == REINT ? "INT" : "CHAR", lex_log->str, NULL);			// FUNC INT/CHAR function
	next_sym(0);
	recover_left(sym_left_parent);												// (
	next_sym(0);
	jump_flag = analyze_para_list();											// <参数表>
	if (jump_flag == sym_semicolon) {
		jump(JUMP_RBR, 1);
		return 0;
	}
	else if (jump_flag == sym_right_brace)	return 0;
	recover_right(sym_right_parent);											// )
	next_sym(0);
	recover_left(sym_left_brace);												// {
	next_sym(0);
	analyze_complex_sentence();													// <复合语句>
	recover_right(sym_right_brace);												// }
	if (return_counter == 0) commit_error_sema(NO_RETURN, lex_log->line_counter, func_name);
	next_sym(0);
	return 0;
}

/*
	＜无返回值函数定义＞  ::= void＜标识符＞'('＜参数表＞')''{'＜复合语句＞'}'
	错误恢复策略：
		采取括号恢复策略；
		复合语句错误时，根据错误结果：
			若跳转到分号，继续分析；
			若跳转到右大括号，退出函数定义分析；
*/
int analyze_func_def() {
	char *func_name;
	symbol jump_flag;
	return_counter = 0;															// 重置函数返回语句
	flag_func = REVOID;															// 重置函数返回类型标记
	next_sym(0);																	// void
	func_name = (char*)malloc(sizeof(char) * strlen(lex_log->str));
	strcpy(func_name, lex_log->str);
	enter_func(lex_log->str, REVOID, lex_log->line_counter);
	gen(FUNC, "VOID", lex_log->str, NULL);										// FUNC VOID function
	next_sym(0);
	recover_left(sym_left_parent);												// (
	next_sym(0);
	jump_flag = analyze_para_list();											// <参数表>
	if (jump_flag == sym_semicolon) {
		jump(JUMP_RBR, 1);
		return 0;
	}
	else if (jump_flag == sym_right_brace)	return 0;
	recover_right(sym_right_parent);											// )
	next_sym(0);
	recover_left(sym_left_brace);												// {
	next_sym(0);
	analyze_complex_sentence();													// <复合语句>
	recover_right(sym_right_brace);												// }
	if (return_counter == 0) {													// 检查有效返回语句
		commit_error_sema(NO_RETURN, lex_log->line_counter, func_name);
		gen(RET, NULL, NULL, NULL);
	}
	next_sym(0);
	return 0;
}

/*
	<函数说明> ::= {＜有返回值函数定义＞|＜无返回值函数定义＞}
	根据函数声明的种类调用不同的分析函数；
*/
int analyze_func_dec() {
	int v;
	do {
		if (lex_log->sym == key_int || lex_log->sym == key_char) {
//			printf_s("This is a function declaration with a return value at line %d. \n", lex_log->line_counter);
			analyze_refunc_def();							// ＜有返回值函数定义＞
		}
		else{
//			printf_s("This is a function declaration with no return value at line %d. \n", lex_log->line_counter);
			analyze_func_def();								// ＜无返回值函数定义＞
		}
		v = analyze_branch_program();
	} while (v == BRANCH_FUNC_DEF || v == BRANCH_REFUNC_DEF);
	return 0;
}

/*
	＜主函数＞::= void main'('')''{'＜复合语句＞'}'
	错误恢复策略：
		采取括号恢复策略；
		复合语句错误时，根据错误结果：
			若跳转到分号，继续分析；
			若跳转到右大括号，退出函数定义分析；
*/
// FINISHED
int analyze_main() {
	next_sym(0);												// void
	return_counter = 0;
	flag_func = REVOID;
	enter_func("main", REVOID, lex_log->line_counter);
	gen(FUNC, "VOID", "main", NULL);						// FUNC VOID main
	next_sym(0);												// main
	recover_left(sym_left_parent);							// (
	next_sym(0);
	recover_right(sym_right_parent);						// )
	next_sym(0);
	recover_left(sym_left_brace);							// {
	next_sym(0);
	analyze_complex_sentence();								// <复合语句>
	recover_right(sym_right_brace);							// }
	if (return_counter == 0) {													// 检查有效返回语句
		commit_error_sema(NO_RETURN, lex_log->line_counter, "main");
		gen(RET, NULL, NULL, NULL);
	}
	next_sym(1);
	return 0;
}

/*
	＜因子＞::= ＜标识符＞
			｜＜标识符＞'['＜表达式＞']'
			｜'('＜表达式＞')'
			｜＜整数＞
			｜＜字符＞
			｜＜有返回值函数调用语句＞    
*/
// FINISHED
int analyze_factor(symbol head, char** ans, Type* type) {
	char *index, *temp;
	Type type_index;
	Func_type ret_type;
	tbl_item item;
	int array_flag = 1, num;
	if (lex_log->sym == identifier) {																	// <标识符>
		if ((item = find_id(lex_log->str)) == NULL) {													// 查表
			commit_error_sema(UNDEFINED, lex_log->line_counter, lex_log->str);
			return -1;
		}
		next_sym(0);
		if (lex_log->sym == sym_left_bracket) {															// [
			if (item->array_length <= 0) {																// 查表检查是否为数组
				commit_error_sema(WRONG_TYPE, lex_log->line_counter, item->id);
				array_flag = 0;				
			}
			next_sym(0);
			if (analyze_exp(&index, &type_index) != 0) {												// <表达式>
				return -1;
			}
			if (type_index != INT) commit_error_sema(WRONG_INDEX_TYPE, lex_log->line_counter, "");		// 检查索引是否为整型
			if (array_flag == 1 && (isdigit(index[0]) || index[0] == '-') && (atoi(index) >= item->array_length || atoi(index) < 0)) {			// 检查索引是否越界
				commit_error_sema(INDEX_OVERFLOW, lex_log->line_counter, item->id);
				index = "0";
			}
			if (lex_log->sym != sym_right_bracket) {													// ]
				commit_error_gramma(MISSMATCH_RIGHT_BRACKET, lex_log->line_counter);
				return -1;
			}
			next_sym(0);
			if (array_flag) {
				alloc_temp(ans);
				gen(GEI, *ans, item->id, index);														// GEI temp a index		
				if (head == sym_minus) gen(SUB, *ans, "0", *ans);										// SUB ans 0 ans		// 前缀负号
				*type = item->type;
				return 0;
			}
		}
		else if (lex_log->sym == sym_left_parent) {														// (
		bk_sym(1);
		if (item->kind != Func) {																	// 检查调用的是否为函数
			commit_error_sema(WRONG_KIND, lex_log->line_counter, item->id);
			return -1;
		}
		else if (item->return_type == REVOID) {														// 检查调用的是否为有返回值函数
			commit_error_sema(ILLEGAL_FUNC_CALL_RET, lex_log->line_counter, item->id);
			return -1;
		}
		if (analyze_call(&ret_type) != 0) {															// <有返回值函数调用>
			return -1;
		}

		if (head == sym_minus) {
			alloc_temp(ans);
			gen(SUB, *ans, "0", "$RET");															// SUB ans 0 RET
			*type = INT;
		}
		else {
			alloc_temp(ans);
			gen(ASN, *ans, "$RET", NULL);															// ASN ans RET 0
			*type = (item->return_type == REINT ? INT : CHAR);
		}
		check_overflow(ans);
		return 0;
		}
		if (item->kind == Const) {																		// 常量
			*type = item->type;
			*ans = (char*)malloc(sizeof(char) * 20);
			sprintf(*ans, "%d", item->const_value);
		}
		// Tip: LEFT HANDLE
		else if (item->field_num == 0) {
			alloc_temp(ans);
			gen(ASN, *ans, item->id, NULL);
			*type = item->type;
		}
		else {
			*ans = (char*)malloc(sizeof(char) * strlen(item->id));
			strcpy(*ans, item->id);
			*type = item->type;
		}
	}
	else if (lex_log->sym == sym_left_parent) {															// (
		next_sym(0);
		if (analyze_exp(ans, type) != 0) {																// <表达式>
			return -1;
		}
		if (lex_log->sym != sym_right_parent) {															// )
			commit_error_gramma(MISSMATCH_RIGHT_PARENT, lex_log->line_counter);
			return -1;
		}
		*type = INT;
		next_sym(0);
	}
	else if (lex_log->sym == const_char) {																// <字符>
		*ans = (char*)malloc(sizeof(char) * 4);
		sprintf(*ans, "%d", (int)(lex_log->ch));
		*type = CHAR;
		next_sym(0);
	}
	else {
		if (analyze_int(&num, ans) != 0) {																// <整数>
			commit_error_gramma(MISSBRANCH_FACTOR, lex_log->line_counter);
			return -1;
		}
		*type = INT;
	}
	if (head == sym_minus) {
		alloc_temp(&temp);
		if (isdigit(*ans[0]) || *ans[0] == '-')
			sprintf(*ans, "%lld", -atoll(*ans));
		else {
			gen(SUB, temp, "0", *ans);																	// SUB temp 0 ans
			*ans = temp;
		}
		*type = INT;
	}
	else if (head == sym_plus) {
		*type = INT;
	}
	check_overflow(ans);
	return 0;
}

/*
	＜项＞::= ＜因子＞{＜乘法运算符＞＜因子＞}
*/
// FINISHED
int analyze_item(symbol head, char** ans, Type* type) {
	char *factor, *temp;
	Type factor_type;
	symbol op;
	if (analyze_factor(head, ans, type) != 0) {
		return -1;
	}
	while (lex_log->sym == sym_multiply || lex_log->sym == sym_divide){
		op = lex_log->sym;
		next_sym(0);
		if (analyze_factor(0, &factor, &factor_type) != 0) {								// <因子>
			return -1;
		}
		if ((isdigit(*ans[0]) || *ans[0] == '-') && (isdigit(factor[0]) || factor[0] == '-')) {
			char num[20];
			sprintf(num, "%lld", op == sym_multiply ? atoll(*ans) * atoll(factor) : atoll(*ans) / atoll(factor));
			free(*ans);
			*ans = (char*)malloc(sizeof(char) * 20);
			strcpy(*ans, num);
		}
		else {
			alloc_temp(&temp);
			gen(op == sym_multiply ? MUL : DIV, temp, *ans, factor);						// MUL/DIV temp ans factor
			*ans = temp;
		}
		*type = INT;
		check_overflow(ans);
	}
	return 0;
}

/*
	＜表达式＞::= ［＋｜－］＜项＞{＜加法运算符＞＜项＞}   
	[+|-]只作用于第一个<项>
	-(5*6)
*/
// FINISHED
int analyze_exp(char** ans, Type* type) {
	symbol op = 0;
	char *temp, *item;
	Type item_type;
	//printf_s("This is a expression at line %d. \n", lex_log->line_counter);
	if (lex_log->sym == sym_plus || lex_log->sym == sym_minus) {					// + / -
		op = lex_log->sym;
		next_sym(0);
	}
	if (analyze_item(op, ans, type) != 0) {											// <项>
		return -1;
	}
	while (lex_log->sym == sym_plus || lex_log->sym == sym_minus) {
		op = lex_log->sym;
		next_sym(0);
		if (analyze_item(0, &item, &item_type) != 0) {								// <项>
			return -1;
		}
		if ((isdigit(*ans[0]) || *ans[0] == '-') && (isdigit(item[0]) || item[0] == '-')) {
			char num[20];
			sprintf(num, "%lld", op == sym_plus ? atoll(*ans) + atoll(item) : atoll(*ans) - atoll(item));
			free(*ans);
			*ans = (char*)malloc(sizeof(char) * 20);
			strcpy(*ans, num);
		}
		else {
			alloc_temp(&temp);
			gen(op == sym_plus ? ADD : SUB, temp, *ans, item);						// MUL/DIV temp ans factor
			*ans = temp;
		}
		*type = INT;
		check_overflow(ans);
	}
	return 0;
}

/*
	＜程序＞::=［＜常量说明＞］［＜变量说明＞］{<函数说明>}＜主函数＞
*/
void analyze_program() {
	int v;
	next_sym(0);
	init_tbl();
	init_mid_code();
//	printf_s("This is a promgram at line %d. \n", lex_log->line_counter);
	analyze_const_dec();															// [<常量说明>]

	v = analyze_branch_program();
	if (v == BRANCH_VAR_DEC) {					
		analyze_var_dec();															// [<变量说明>]
		v = analyze_branch_program();
	}
	if (v == BRANCH_FUNC_DEF || v == BRANCH_REFUNC_DEF) {							// {<函数说明>}
		analyze_func_dec();
		v = analyze_branch_program();
	}
	while (v == BRANCH_VAR_DEC) {
		commit_error_gramma(MISSBRANCH_PROGRAM, lex_log->line_counter);
		jump(JUMP_SEMI_RBR, 1);
	}
	if (v == BRANCH_MAIN) {															// ＜主函数＞
//		printf_s("This is a main declaration at line %d. \n", lex_log->line_counter);
		analyze_main();
	}
	else {
		commit_error_gramma(MISS_MAIN, lex_log->line_counter);
		return;
	}
	print_table();
	print_str_table();
	print_all_mid_code();
	reset_table();
	reset_mid_code();
	while (!to_file_end) {
		next_sym(0);
		commit_error_gramma(EXCESSIVE_PROGRAM, lex_log->line_counter);
	}
}

