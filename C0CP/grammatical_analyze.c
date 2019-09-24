#include"symbol_table.h"
#include"mid_code.h"
#include"grammatical_analyze.h"
#include"lex_analyze.h"
#include"error.h"
#include<string.h>
#include<stdlib.h>
#include<ctype.h>
#include<limits.h>

// ���
#include<stdio.h>
/*
	����ָ������е��н��������������ڵ��⣩
		�� --- 1
		�� --- 2
		�� --- 3
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

int return_counter = 0;											// ���㺯��������䡣
Func_type flag_func = REVOID;									// �����������ͱ�ǡ�

/*
	�������ż��
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
	�������ݴ���
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
	�������ݴ�
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
	�ָ����ݴ�
*/
int recover(symbol sym) {
	if (lex_log->sym != sym) {
		bk_sym(1);
		return -1;
	}
	return 0;
}
/*
	�ݴ���ת������
	��ת��jump_list�����еķ��ŵ�λ�á�
	����ֵΪֹͣʱ���Ŷ�Ӧ��ö���ࡣ
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
	if (isdigit(*ans[0]) || *ans[0] == '-') {														// ����ʱ���Խ�磬Խ������
		if (atoll(*ans) < INT_MIN || atoll(*ans) > INT_MAX) {
			commit_error_gramma(INT_OVERFLOW, lex_log->line_counter);
			*ans = "0";
		}
	}
}

/*
	��������::=�ۣ������ݣ��޷���������
	����ָ����ԣ�
		��������������ʶ���ϱ�����
*/
int analyze_int(int *num, char **nums) {
	long long temp = 1;
	*nums = (char*)malloc(sizeof(char) * 20);
	*num = 1;
	if (lex_log->sym == sym_plus) {									// ������
		temp = 1;
		next_sym(0);
	}
	else if (lex_log->sym == sym_minus) {							// ������
		temp = -1;
		next_sym(0);
	}
	if (lex_log->sym != const_int) {								// ƥ���޷�����
		commit_error_gramma(MISSMATCH_INT, lex_log->line_counter);
		return -1;
	}
	temp *= lex_log->num;
	if (temp > INT_MAX || temp < INT_MIN) {							// ����ʶ��ʱԽ����
		commit_error_gramma(INT_OVERFLOW, lex_log->line_counter);
		temp = 0;
	}
	*num = (int)temp;
	next_sym(0);
	sprintf(*nums, "%d", *num);
	return 0;
	
}

/*
	���������壾::= int����ʶ��������������{,����ʶ��������������}
                  | char����ʶ���������ַ���{,����ʶ���������ַ���}
	����ָ����ԣ�
		���ͱ�ʶ��ƥ�����ʱ���������γ��������ʶ���ϱ�����
		��ʶ��ƥ�����ʱ���������γ�������������ת�ҽ����
			����ת�����ţ�����������
			����ת���ֺŻ��Ҵ����ţ��ϱ�����
		�������ַ�ƥ�����ʱ��ͬ��ʶ��ƥ�����Ĵ���
		��=��ȱʧʱ����ȫ��
	�ϱ�����
		��ת���ֺţ�

*/
int analyze_const_def() {
	symbol jump_flag;
	symbol type_flag = lex_log->sym;
	char *id, *constant;
	int const_int;
	if (type_flag != key_int && type_flag != key_char) {												// δƥ�䵽��int����char��
		commit_error_gramma(MISSMATCH_TYPE_DEF, lex_log->line_counter);
		return jump(JUMP_SEMI, 0);																		// ��ת���ֺ�			
	}
	do {
		jump_flag = 0;
		next_sym(0);
		if (lex_log->sym != identifier) {																// ƥ���ʶ��
			commit_error_gramma(MISSMATCH_IDENTIFIER, lex_log->line_counter);
			jump_flag = jump(JUMP_COMMA_SEMI, 0);
			continue;
		}
		id = (char*)malloc(sizeof(char) * strlen(lex_log->str));
		strcpy(id, lex_log->str);
		next_sym(0);																						// ƥ�䡰=��
		if(recover(sym_become) != 0)	commit_error_gramma(MISSMATCH_BECOME, lex_log->line_counter);
		next_sym(0);
		if (type_flag == key_int) {																		// check <����>
			if (analyze_int(&const_int, &constant) != 0) {
				jump_flag = jump(JUMP_COMMA_SEMI, 0);
				continue;
			}
			enter_const(id, INT, const_int, lex_log->line_counter);										// enter tbl
			gen(CONST, "INT", id, constant);															// CONST INT id constant
		}
		else {																							// check �ַ�
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
	} while (lex_log->sym == sym_comma);																// ƥ�䶺�ţ�ѭ��ƥ��
	return jump_flag;
}

/*
	������˵����::= const���������壾;{ const���������壾;}
	����ָ����ԣ�
		ȱ�ٷֺ�ʱ����ȫ�ֺţ�
		�����������ʱ��������ת�����
			����ת���ֺţ�����������
			����ת���Ҵ����ţ��ϱ�����
		�׸���const���޷�ƥ��ʱ���˳�����˵����ʶ�𣬵�������
	���ϱ�����
*/
int analyze_const_dec() {
	if (lex_log->sym != key_const)	return 0;															// ƥ�䡰const��
	do {
//		printf_s("This is a const declaration at line %d. \n", lex_log->line_counter);
		next_sym(0);
		analyze_const_def();																			// <��������>����ת���ֺţ���������
		if(recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);// ƥ��ֺ�
		next_sym(0);
	} while (lex_log->sym == key_const);
	return 0;
}

/*
	����<����>�в�����LL(1)�ķ��ķ�֧���������ֵΪ��֧ѡ������
*/
int analyze_branch_program() {
	if (lex_log->sym == key_void) {														// <�޷���ֵ��������>��<������>
		next_sym(0);
		if (lex_log->sym == func_main) {								
			bk_sym(1);
			return BRANCH_MAIN;															// <������>
		}
		bk_sym(1);
		return BRANCH_FUNC_DEF;															// <�޷���ֵ��������>
	}
	else if (lex_log->sym == key_char || lex_log->sym == key_int) {						// <�з���ֵ��������>��<��������>
		next_sym(0);
		if (lex_log->sym == identifier) {
			next_sym(0);
			if (lex_log->sym == sym_left_parent) {
				bk_sym(2);
				return BRANCH_REFUNC_DEF;												// <�з���ֵ��������>
			}
			bk_sym(1);
		}
		bk_sym(1);
		return BRANCH_VAR_DEC;															// <��������>
	}
	return -1;
}

/*
	���������壾::=�����ͱ�ʶ����(����ʶ����|����ʶ����'['���޷���������']'){,(����ʶ����|����ʶ����'['���޷���������']' )} 
	ע�⣺���޷�����������ʾ����Ԫ�صĸ�������ֵ�����0
	����ָ����ԣ�
		ʹ�����Żָ����ԣ�
		���ͱ�ʶ��ƥ�����ʱ�����������������ʶ����ת���ҽ����
			��ת���ֺŻ��Ҵ����ţ��ϱ�����
		��ʶ�����޷�������ƥ�����ʱ���������α���������ʶ����ת�����������ҽ����
			����ת�����ţ�����������
			����ת���ֺŻ��Ҵ����ţ��ϱ�����
	�ϱ�����
		��ת���ֺţ�
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
		if (lex_log->sym != identifier) {										// <��ʶ��>
			commit_error_gramma(MISSMATCH_IDENTIFIER, lex_log->line_counter);
			jump_flag = jump(JUMP_COMMA_SEMI, 0);								// ��ת
			continue;
		}
		id = (char*)malloc(sizeof(char) * strlen(lex_log->str));
		strcpy(id, lex_log->str);
		next_sym(0);
		if (lex_log->sym == sym_left_bracket) {									// [ , ����
			next_sym(0);
			if (lex_log->sym != const_int) {									// <�޷�������>
				commit_error_gramma(MISSMATCH_CONST_INT, lex_log->line_counter);
				jump_flag = jump(JUMP_COMMA_SEMI, 0);							// ��ת
				continue;
			}
			if (lex_log->num <= 0) {											// ������鳤��
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
	������˵����::=���������壾;{���������壾;}
	����ָ����ԣ�
		ȱ�ٷֺ�ʱ����ȫ�ֺţ�
		�����������ʱ��������ת�����
			��ת���ֺţ�����������
			��ת���Ҵ����ţ��ϱ�����
	���ϱ�����
*/
int analyze_var_dec() {
	do {
		analyze_var_def();																						// ���������壾	
		if (recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);	// ;
		next_sym(0);
	} while (analyze_branch_program() == BRANCH_VAR_DEC);
	return 0;
}

/*
	��������	::= ��������{,��������}
					|����>
	��������	::= �����ͱ�ʶ��������ʶ����
	����ָ����ԣ�
		����ʶ����ִ���ʱ����ת���������ҽ�������š������ţ���
			����ת�����ţ�����������
			����ת�������š��ֺš��Ҵ����ţ��ϱ�����
	�ϱ�����
		��ת�������š��ֺš��Ҵ����ţ�
*/
// FINISHED
int analyze_para_list() {
	Type type;
	if (lex_log->sym == sym_right_parent)	return 0;							// <��>
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
		if (lex_log->sym != identifier) {										// <��ʶ��>
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
	����<���>�в�����LL(1)�ķ��ķ�֧���������ֵΪ��֧ѡ������
*/
int analyze_branch_sentence() {
	if (lex_log->sym == key_if) {													// < if ��� >
		return BRANCH_IF;
	}
	else if (lex_log->sym == key_while) {											// < while ��� >
		return BRANCH_WHILE;
	}
	else if (lex_log->sym == sym_left_brace) {										// {<�����>}
		return BRANCH_SENTENCE_LIST;
	}
	else if (lex_log->sym == identifier) {											// <�����������>��<��ֵ���>
		next_sym(0);
		if (lex_log->sym == sym_left_parent) {										// <�����������>
			bk_sym(1);
			return BRANCH_CALL;
		}
		else if(lex_log->sym == sym_become || lex_log->sym == sym_left_bracket) {
			bk_sym(1);
			return BRANCH_ASSIGN;													// <��ֵ���>
		}
	}
	else if (lex_log->sym == func_scanf) {											// <�����>
		return BRANCH_SCANF;
	}
	else if (lex_log->sym == func_printf) {											// <д���>
		return BRANCH_PRINTF;				
	}
	else if (lex_log->sym == sym_semicolon) {										// <�����>
		return BRANCH_NULL_SENTENCE;
	}
	else if (lex_log->sym == key_switch) {											// <������>
		return BRANCH_SWITCH;
	}
	else if (lex_log->sym == key_return) {											// <�������>
		return BRANCH_RETURN;
	}
	return -1;
}

// ����
int analyze_sentence();
int analyze_exp(char**, Type*);

/*	
	��������::=  �����ʽ������ϵ������������ʽ��
				�������ʽ��  // ���ʽΪ0����Ϊ�٣�����Ϊ��
	����ָ����ԣ�
		�׸����ʽƥ�����ʱ���˳�������ʶ�𣬲�Ĭ������ֵΪ0��
		��ϵ�����ƥ�����ʱ���˳�������ʶ�𣬲�Ĭ������ֵΪ�׸����ʽ��ֵ��
		�μ����ʽƥ�����ʱ���˳�������ʶ�𣬲�Ĭ������ֵΪ�׸����ʽ��ֵ��
		�˳�����ת���ҽ����
			��ת�������š��ֺŻ��Ҵ����ţ��ϱ�����
	�ϱ�����
		��ת�������š��ֺš��Ҵ����ţ�
*/
// FINISHED
int analyze_condition() {
	char *exp_1, *exp_2;
	symbol op = sym_equal;
	Type type_1, type_2;
	if (analyze_exp(&exp_1, &type_1) != 0) {								// <���ʽ>
		return jump(JUMP_RPR_SEMI_RBR, 0);									// ��ת�������š��ֺŻ��Ҵ����ţ�				
	}
	if (type_1 != INT) {													// ���Ƚ�ʽ����
		commit_error_sema(ILLEGAL_COMPARE_TYPE, lex_log->line_counter, "");
	}
	if (lex_log->sym == sym_less 
		|| lex_log->sym == sym_less_equal
		|| lex_log->sym == sym_greater
		|| lex_log->sym == sym_greater_equal
		|| lex_log->sym == sym_equal
		|| lex_log->sym == sym_not_equal) {										// ��ϵ�����
		op = lex_log->sym;
	}
	else if (lex_log->sym == sym_right_parent) {							// �޹�ϵ�����
		gen(NEQ, exp_1, "0", NULL);										// EQL exp_1 0
		return 0;
	}
	else {
		commit_error_gramma(MISSMATCH_COMPARE, lex_log->line_counter);
		bk_sym(1);
	}
	next_sym(0);
	if (analyze_exp(&exp_2, &type_2) != 0) {								// <���ʽ>
		return jump(JUMP_RPR_SEMI_RBR, 0);									// ��ת�������š��ֺŻ��Ҵ����ţ�	
	}
	if (type_2 != INT) {
		commit_error_sema(ILLEGAL_COMPARE_TYPE, lex_log->line_counter, "");
	}
	gen(SYM2COMPARE((int)op), exp_1, exp_2, NULL);							// COMPARE exp_1 exp_2
	return 0;
}

/*
	��������䣾::=  if '('��������')'����䣾
	����ָ����ԣ�
		�������Żָ����ԣ�
		�ؼ��֡�if��ƥ�����ʱ��������������ʶ����ת���ֺ����ϣ��ϱ�����
		����ƥ�����ʱ��
			����ת�������ţ�����������
			����ת���ֺ����ϣ��ϱ�����
		���ƥ�����ʱ���˳���������ʶ�𣬸��ݵ�׼��Ϣ�ϱ�����
	�ϱ�����
		��ת���ֺš��Ҵ����ţ�
*/
// FINISHED
int analyze_if() {
	symbol jump_flag = 0;
	char* label;
	next_sym(0);
	recover_left(sym_left_parent);																	// (
	next_sym(0);
	if ((jump_flag = analyze_condition()) != sym_right_parent && jump_flag != 0) return jump_flag;	// <����>
	if (jump_flag == 0) {
		recover_right(sym_right_parent);															// )
	}
	alloc_label(&label);
	gen(BZ, label, NULL, NULL);																		// BZ label
	next_sym(0);
	jump_flag = analyze_sentence();
	gen(LABEL, label, NULL, NULL);																	// LABEL label
	return jump_flag;																				// <���>
}

/*
	��ѭ����䣾::=  while '('��������')'����䣾
	����ָ����ԣ�
		�������Żָ����ԣ�
		�ؼ��֡�while��ƥ�����ʱ��������������ʶ����ת���ֺ����ϣ��ϱ�����
		����ƥ�����ʱ��
			����ת�������ţ�����������
			����ת���ֺ����ϣ��ϱ�����
		���ƥ�����ʱ���˳���������ʶ�𣬸��ݵ�׼��Ϣ�ϱ�����
	�ϱ�����
		��ת���ֺš��Ҵ����ţ�
*/
// FINISHED
int analyze_while() {
	int start, end;
	symbol jump_flag = 0;
	char *label_start, *label_end;
	alloc_label(&label_start);
	alloc_label(&label_end);
	next_sym(0);

	start = mark_mid_code();																	// ��ʼ��¼�����жϵĴ����
	recover_left(sym_left_parent);																	// (
	next_sym(0);
	if ((jump_flag = analyze_condition()) != sym_right_parent && jump_flag != 0) return jump_flag;	// <����>
	recover_right(sym_right_parent);																// )
	end = mark_mid_code();																		// ������¼�����жϵĴ����

	gen(BZ, label_end, NULL, NULL);																	// BZ label
	gen(LABEL, label_start, NULL, NULL);															// LABEL label_start
	next_sym(0);
	jump_flag = analyze_sentence();																	// ���
	copy_marked_mid_code(start, end);																// ���������жϴ����
	reverse_top_compare();
	gen(BZ, label_start, NULL, NULL);																// BZ label

	gen(LABEL, label_end, NULL, NULL);																// LABEL label_end
	return jump_flag;
}

/*
	��ֵ������::=�����ʽ��{,�����ʽ��}�����գ�
	����ָ����ԣ�
		���ʽƥ�����ʱ�����ݱ��ʽ��׼��Ϣ��
			����ת�����ţ�����������
			����ת�����������ϣ��ϱ�����
		����ȷ���ʽ�ǣ�ֵ������ȼ���<��>��
	�ϱ�����
		��ת�������š��ֺš��Ҵ����ţ�
*/
// FINISHD
int analyze_value_list(tbl_field func) {
	char *exp, **para_list;
	Type type;
	int counter = 1;																			// ������
	tbl_item para;
	if (lex_log->sym == sym_right_parent)	return 0;											// <��>
	symbol jump_flag = 0;
	bk_sym(1);
	para_list = (char**)malloc(sizeof(char*) * func->para_sum);									// ���洫�����
	do {
		jump_flag = 0;
		next_sym(0);
		if (analyze_exp(&exp, &type) != 0) {													// <���ʽ>
			jump_flag = jump(JUMP_COMMA_RPR_SEMI_RBR, 0);										// ��ת�����š������š��ֺš��Ҵ�����
			continue;					
		}
		if ((para = get_id(func->first_item_loca + counter)) != NULL && type != para->type) {
			commit_error_sema(WRONG_TYPE, lex_log->line_counter, "");							// ֵ�������ͼ��
		}
		para_list[counter - 1] = exp;
		//gen(PUSH, exp, NULL, NULL);
		counter++;
	} while (lex_log->sym == sym_comma);														// ,
	if (counter != func->para_sum + 1)															// ֵ�����������
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
	���з���ֵ����������䣾 ::= ����ʶ����'('��ֵ������')'
	���޷���ֵ����������䣾 ::= ����ʶ����'('��ֵ������')'
	����ָ����ԣ�
		����ȡ���Żָ����ԣ�
		��ʶ��ƥ�����ʱ����ת���ֺ����ϣ��ϱ�����
		ֵ������ƥ�����ʱ��������ת��Ϣ��
			����ת�������ţ�����������
			����ת���ֺ����ϣ��ϱ�����
	�ϱ�����
		��ת���ֺš��Ҵ����ţ�
*/
// FINISHED
int analyze_call(Func_type* func_type) {
	symbol jump_flag;
	tbl_item item = find_id(lex_log->str);											// <��ʶ��>
	if (item == NULL || item->kind != Func) {											// ����Ƿ�Ϊ����
		commit_error_sema(ILLEGAL_FUNC_CALL, lex_log->line_counter, lex_log->str);
		return jump(JUMP_SEMI_RBR, 0);
	}
	*func_type = item->return_type;
	tbl_field func = find_field(lex_log->str);
	next_sym(0);																			// (
	next_sym(0);
	if ((jump_flag = analyze_value_list(func)) != sym_right_parent && jump_flag != 0) {	// <ֵ������>
		return jump_flag;
	}
	recover_right(sym_right_parent);													// )
	gen(CALL, item->id, NULL, NULL);													// CALL func
	next_sym(0);
	return 0;
}

/*
	����ֵ��䣾::=����ʶ�����������ʽ��
				|����ʶ����'['�����ʽ��']'=�����ʽ��
	����ָ����ԣ�
		�κδ�����˳���ֵ����ʶ����ת���ֺ����ϣ��ϱ�����
	�ϱ�����
		��ת���ֺš��Ҵ�����
*/
// FINISHED
int analyze_assign() {
	int array_flag = 0;
	char *exp_1, *exp_2 = "";
	Type type_1, type_2;
	tbl_item id = find_id(lex_log->str);															// �������ű�
	if (id == NULL) {																				// �������Ƿ����
		commit_error_sema(UNDEFINED, lex_log->line_counter, lex_log->str);
		return jump(JUMP_SEMI_RBR, 0);
	}
	else if(!(id->kind == Var || id->kind == Para)){												// �������Ƿ�Ϊ���������
		commit_error_sema(WRONG_KIND, lex_log->line_counter, lex_log->str);
		return jump(JUMP_SEMI_RBR, 0);
	}
	next_sym(0);
	if (lex_log->sym == sym_left_bracket) {															// [
		if(id->array_length > 0) array_flag = 1;													// �������Ƿ�Ϊ����
		next_sym(0);
		if (analyze_exp(&exp_2, &type_2) != 0) return jump(JUMP_SEMI_RBR, 0);						// <���ʽ>
		if (array_flag == 1 && isdigit(exp_2[0]) && atoi(exp_2) >= id->array_length) {				// ��������Ƿ�Խ��
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
	if (analyze_exp(&exp_1, &type_1) != 0) return jump(JUMP_SEMI_RBR, 0);							// <���ʽ>
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
	������䣾::= scanf '('����ʶ����{,����ʶ����}')'
	����ָ����ԣ�
		��ȡ���Żָ����ԣ�
		��ʶ��ʶ�����ʱ����ת���н����
			����ת���ֺ����ϣ��ϱ�����
	�ϱ�����
		��ת���ֺš��Ҵ����ţ�
*/
int analyze_scanf() {
	tbl_item item;
	next_sym(0);																	// scanf
	recover_left(sym_left_parent);												// (
	do {
		next_sym(0);
		if (lex_log->sym != identifier) {										// <��ʶ��>
			commit_error_gramma(MISSMATCH_IDENTIFIER, lex_log->line_counter);
			return jump(JUMP_SEMI_RBR, 0);										// ��ת���ֺš��Ҵ�����					
		}	
		if ((item = find_id(lex_log->str)) == NULL) {							// ���
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
	��д��䣾::= printf '(' ���ַ�����,�����ʽ�� ')'
				| printf '('���ַ����� ')'
				| printf '('�����ʽ��')'
	����ָ����ԣ�
		��ȡ���Żָ����ԣ�
		�ڲ�����ʱ����ת���ҽ����
			����ת���ֺ����ϣ��ϱ�����
	�ϱ�����
		��ת���ֺš��Ҵ����ţ�
*/
// TODO: 
int analyze_printf() {
	char *exp, *temp;
	Type type;
	next_sym(0);										// printf
	recover_left(sym_left_parent);					// (
	next_sym(0);
	if (lex_log->sym == const_string) {				// <�ַ���>
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
	if (analyze_exp(&exp, &type) != 0) return jump(JUMP_SEMI_RBR, 0);		// ��ת���ֺš��Ҵ�����	
	gen(PRINT, exp, type == INT ? "INT" : "CHAR", NULL);
	gen(PRINT, "10", "CHAR", NULL);
	recover_right(sym_right_parent);				// )
	next_sym(0);
	return 0;
}

/*
	�������		::=  ���������䣾{���������䣾}
	���������䣾  ::=  case��������������䣾
	����ָ����ԣ�
		����ƥ�����ʱ����ת���ҽ����
			����ת����case��������������
			����ת����default�����ϱ�����
			����ת���Ҵ����ţ��ϱ�����
		���ʶ����󣬼����ϱ�����
		ð��ƥ�����ʱ����ȫð�ţ�
	�ϱ�����
		��ת��default���Ҵ����ţ�
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
		if (lex_log->sym == const_char) {														// <�ַ�>
			char ch[5];
			sprintf(ch, "%d", (int)lex_log->ch);
			if (type != CHAR) commit_error_sema(WRONG_TYPE, lex_log->line_counter, ch);			// ���ͼ��
			gen(EQL, exp, ch, NULL);															// EQL exp ch
			gen(BZ, label_case, NULL, NULL);													// BZ label_case
			next_sym(0);
		}
		else {
			char* constant;
			if (analyze_int(&num, &constant) != 0) {											// <����>
				jump_flag = jump(JUMP_CASE_DEFAULT_RBR, 0);										// δ��ת��case���ϱ�����
				continue;
			}
			if (type != INT) commit_error_sema(WRONG_TYPE, lex_log->line_counter, constant);	// ���ͼ��
			gen(EQL, exp, constant, NULL);														// EQL exp constant
			gen(BZ, label_case, NULL, NULL);													// BZ label_case
		}
		if(recover(sym_colon) != 0)	commit_error_gramma(MISSMATCH_COLON, lex_log->line_counter);// :
		next_sym(0);
		jump_flag = analyze_sentence();															// <���>
		gen(GOTO, label_end, NULL, NULL);														// GOTO label_end
		gen(LABEL, label_case, NULL, NULL);														// LABEL lebel_case
	} while (lex_log->sym == key_case);															// case
	return 0;
}

/*
	��ȱʡ��::=  default : ����䣾|���գ�
	����ָ����ԣ�
		�ؼ��֡�default��ƥ�����ʱ������ȱʡ��ʶ����ת���ֺ����ϣ��ϱ�����
		ð��ƥ�����ʱ����ȫ��
		���ƥ�����ʱ�������ϱ�����
	�ϱ�����
		��ת���Ҵ����ţ�
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
	if (lex_log->sym == sym_right_brace) {														// <��>
		return 0;
	}
	return analyze_sentence();					
}

/*
	�������䣾::= switch '('�����ʽ��')' '{'���������ȱʡ��'}'
	����ָ����ԣ�
		�ؼ��֡�switch��ƥ�����ʱ�������������ʶ��
		��ȡ���Żָ����ԣ�
		���ʽƥ�����ʱ���˳��������ʶ����ת���Ҵ����ţ�����������
		�����ƥ�����ʱ����ת���Ҵ����ţ�����������
		ȱʡƥ�����ʱ����ת���Ҵ����ţ�����������
	���ϱ�����
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
	if (analyze_exp(&exp, &type) != 0) {			// <���ʽ>
		if ((jump_flag = jump(JUMP_RPR_SEMI_RBR, 0)) != sym_right_parent && jump_flag != 0)	return jump_flag;	// δ��ת��������
	}
	recover_right(sym_right_parent);				// )
	next_sym(0);
	recover_left(sym_left_brace);					// {
	next_sym(0);
	jump_flag = analyze_case(exp, type, label);		// <�����>
	if (jump_flag == sym_semicolon)
		next_sym(0);
	if (lex_log->sym == key_default) {				// <ȱʡ>
		analyze_default();
	}
	recover_right(sym_right_brace);					// }
	gen(LABEL, label, NULL, NULL);					// LABEL label
	next_sym(0);
	return 0;
}

/*
	��������䣾::= return['('�����ʽ��')'] 
	����ָ����ԣ�
		���д�����˳���������ʶ����ת���ֺż����ϣ��ϱ�����
	�ϱ�����
		��ת���ֺš��Ҵ����ţ�
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
		else	commit_error_sema(WRONG_RETURN_TYPE, lex_log->line_counter, "");	// �з���ֵ����δ����ֵ
		return 0;
	}
	else if(lex_log->sym != sym_left_parent) {
		commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);
		gen(RET, NULL, NULL, NULL);													// RET
		return 0;
	}
	next_sym(0);
	if (analyze_exp(&exp, &type) != 0)	return jump(JUMP_SEMI_RBR, 0);				// <���ʽ>
	recover_right(sym_right_parent);												// )
	if (flag_func == REVOID) {														// �޷���ֵ��������ֵ
		commit_error_sema(WRONG_RETURN_TYPE, lex_log->line_counter, "");			// �з���ֵ����δ����ֵ
		gen(RET, NULL, NULL, NULL);													// RET
	}
	else if ((flag_func == REINT && type == CHAR) || (flag_func == RECHAR && type == INT)) {
		commit_error_sema(WRONG_RETURN_TYPE, lex_log->line_counter, "");			// �з���ֵ����δ����ֵ
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
	������У�::=������䣾��
	����ָ����ԣ�
		���ʶ�����ʱ����ת���ֺ����ϣ��ϱ�����
	�ϱ�����
		��ת���Ҵ�����
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
	����䣾::= ��������䣾����ѭ����䣾| '{'������У�'}'| ���з���ֵ����������䣾;
			|���޷���ֵ����������䣾;������ֵ��䣾;��������䣾;����д��䣾;�����գ�;|�������䣾����������䣾;
	����������ԣ�
		�޺�׺���ƥ�����ʱ����ת���ֺż����ϣ��ϱ�����
		ȱ�ٷֺ�ʱ������ֺţ�
	�ϱ�����
		��ת���ֺš��Ҵ����ţ�
*/
int analyze_sentence() {
	int v;
	symbol jump_flag;
	Func_type ft;
	v = analyze_branch_sentence();
	switch (v) {
	case BRANCH_IF:																									// ��������䣾
//		printf_s("This is a if sentence at line %d. \n", lex_log->line_counter);
		return analyze_if();
	case BRANCH_WHILE:																								// ��ѭ����䣾
//		printf_s("This is a while sentence at line %d. \n", lex_log->line_counter);
		return analyze_while();
	case BRANCH_SENTENCE_LIST:																						// {<�����>}
//		printf_s("This is a block at line %d. \n", lex_log->line_counter);
		next_sym(0);
		jump_flag = analyze_sentence_list();																		// <�����>
		recover_right(sym_right_brace);																				// }
		next_sym(0);
		break;
	case BRANCH_CALL:																								// ���з���ֵ����������䣾; | ���޷���ֵ����������䣾;
//		printf_s("This is a function call at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_call(&ft))  == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if(recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);			// ;
		next_sym(0);
		break;
	case BRANCH_ASSIGN:																								// ����ֵ��䣾;
//		printf_s("This is a assign sentence at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_assign()) == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if (recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);		// ;
		next_sym(0);
		break;
	case BRANCH_SCANF:																								// ������䣾;
		//printf_s("This is a scanf sentence at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_scanf()) == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if (recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);		// ;
		next_sym(0);
		break;
	case BRANCH_PRINTF:																								// ��д��䣾;
		//printf_s("This is a printf sentence at line %d. \n", lex_log->line_counter);
		if ((jump_flag = analyze_printf()) == sym_right_brace && jump_flag != 0) {
			return jump_flag;
		}
		if(recover(sym_semicolon) != 0)	commit_error_gramma(MISSMATCH_SEMICOLON, lex_log->line_counter);			// ;
		next_sym(0);
		break;
	case BRANCH_NULL_SENTENCE:																						// ���գ�;
		//printf_s("This is a null sentence at line %d. \n", lex_log->line_counter);
		next_sym(0);
		break;	
	case BRANCH_SWITCH:																								// <������>
//		printf_s("This is a switch sentence at line %d. \n", lex_log->line_counter);
		analyze_switch();
		break;
	case BRANCH_RETURN:																								// ��������䣾;
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
	��������䣾::=�ۣ�����˵�����ݣۣ�����˵�����ݣ�����У�
	����ָ����ԣ�
		����˵��ƥ�����ʱ������������
		����˵��ƥ�����ʱ������������
		�����ƥ�����ʱ������������
*/
int analyze_complex_sentence() {
	if (lex_log->sym == key_const) {								// �ۣ�����˵������
		analyze_const_dec();
	}
	if (lex_log->sym == key_int || lex_log->sym == key_char) {		// �ۣ�����˵������
		analyze_var_dec();
	}
	if (analyze_sentence_list() == sym_right_brace) {				// ������У�
		bk_sym(1);
	}										
	return 0;
}

/*
	���з���ֵ�������壾  ::= ������ͷ����'('��������')' '{'��������䣾'}'
	����ָ����ԣ�
		��ȡ���Żָ����ԣ�
		����������ʱ�����ݴ�������
			����ת���ֺţ�����������
			����ת���Ҵ����ţ��˳��������������

*/
int analyze_refunc_def() {
	char *func_name;
	symbol jump_flag;
	Func_type func_type = (lex_log->sym == key_int ? REINT : RECHAR);
	return_counter = 0;															// ���ú����������
	flag_func = func_type;														// ���ú����������ͱ��
	next_sym(0);
	func_name = (char*)malloc(sizeof(char) * strlen(lex_log->str));
	strcpy(func_name, lex_log->str);
	enter_func(lex_log->str, func_type, lex_log->line_counter);
	gen(FUNC, func_type == REINT ? "INT" : "CHAR", lex_log->str, NULL);			// FUNC INT/CHAR function
	next_sym(0);
	recover_left(sym_left_parent);												// (
	next_sym(0);
	jump_flag = analyze_para_list();											// <������>
	if (jump_flag == sym_semicolon) {
		jump(JUMP_RBR, 1);
		return 0;
	}
	else if (jump_flag == sym_right_brace)	return 0;
	recover_right(sym_right_parent);											// )
	next_sym(0);
	recover_left(sym_left_brace);												// {
	next_sym(0);
	analyze_complex_sentence();													// <�������>
	recover_right(sym_right_brace);												// }
	if (return_counter == 0) commit_error_sema(NO_RETURN, lex_log->line_counter, func_name);
	next_sym(0);
	return 0;
}

/*
	���޷���ֵ�������壾  ::= void����ʶ����'('��������')''{'��������䣾'}'
	����ָ����ԣ�
		��ȡ���Żָ����ԣ�
		����������ʱ�����ݴ�������
			����ת���ֺţ�����������
			����ת���Ҵ����ţ��˳��������������
*/
int analyze_func_def() {
	char *func_name;
	symbol jump_flag;
	return_counter = 0;															// ���ú����������
	flag_func = REVOID;															// ���ú����������ͱ��
	next_sym(0);																	// void
	func_name = (char*)malloc(sizeof(char) * strlen(lex_log->str));
	strcpy(func_name, lex_log->str);
	enter_func(lex_log->str, REVOID, lex_log->line_counter);
	gen(FUNC, "VOID", lex_log->str, NULL);										// FUNC VOID function
	next_sym(0);
	recover_left(sym_left_parent);												// (
	next_sym(0);
	jump_flag = analyze_para_list();											// <������>
	if (jump_flag == sym_semicolon) {
		jump(JUMP_RBR, 1);
		return 0;
	}
	else if (jump_flag == sym_right_brace)	return 0;
	recover_right(sym_right_parent);											// )
	next_sym(0);
	recover_left(sym_left_brace);												// {
	next_sym(0);
	analyze_complex_sentence();													// <�������>
	recover_right(sym_right_brace);												// }
	if (return_counter == 0) {													// �����Ч�������
		commit_error_sema(NO_RETURN, lex_log->line_counter, func_name);
		gen(RET, NULL, NULL, NULL);
	}
	next_sym(0);
	return 0;
}

/*
	<����˵��> ::= {���з���ֵ�������壾|���޷���ֵ�������壾}
	���ݺ���������������ò�ͬ�ķ���������
*/
int analyze_func_dec() {
	int v;
	do {
		if (lex_log->sym == key_int || lex_log->sym == key_char) {
//			printf_s("This is a function declaration with a return value at line %d. \n", lex_log->line_counter);
			analyze_refunc_def();							// ���з���ֵ�������壾
		}
		else{
//			printf_s("This is a function declaration with no return value at line %d. \n", lex_log->line_counter);
			analyze_func_def();								// ���޷���ֵ�������壾
		}
		v = analyze_branch_program();
	} while (v == BRANCH_FUNC_DEF || v == BRANCH_REFUNC_DEF);
	return 0;
}

/*
	����������::= void main'('')''{'��������䣾'}'
	����ָ����ԣ�
		��ȡ���Żָ����ԣ�
		����������ʱ�����ݴ�������
			����ת���ֺţ�����������
			����ת���Ҵ����ţ��˳��������������
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
	analyze_complex_sentence();								// <�������>
	recover_right(sym_right_brace);							// }
	if (return_counter == 0) {													// �����Ч�������
		commit_error_sema(NO_RETURN, lex_log->line_counter, "main");
		gen(RET, NULL, NULL, NULL);
	}
	next_sym(1);
	return 0;
}

/*
	�����ӣ�::= ����ʶ����
			������ʶ����'['�����ʽ��']'
			��'('�����ʽ��')'
			����������
			�����ַ���
			�����з���ֵ����������䣾    
*/
// FINISHED
int analyze_factor(symbol head, char** ans, Type* type) {
	char *index, *temp;
	Type type_index;
	Func_type ret_type;
	tbl_item item;
	int array_flag = 1, num;
	if (lex_log->sym == identifier) {																	// <��ʶ��>
		if ((item = find_id(lex_log->str)) == NULL) {													// ���
			commit_error_sema(UNDEFINED, lex_log->line_counter, lex_log->str);
			return -1;
		}
		next_sym(0);
		if (lex_log->sym == sym_left_bracket) {															// [
			if (item->array_length <= 0) {																// ������Ƿ�Ϊ����
				commit_error_sema(WRONG_TYPE, lex_log->line_counter, item->id);
				array_flag = 0;				
			}
			next_sym(0);
			if (analyze_exp(&index, &type_index) != 0) {												// <���ʽ>
				return -1;
			}
			if (type_index != INT) commit_error_sema(WRONG_INDEX_TYPE, lex_log->line_counter, "");		// ��������Ƿ�Ϊ����
			if (array_flag == 1 && (isdigit(index[0]) || index[0] == '-') && (atoi(index) >= item->array_length || atoi(index) < 0)) {			// ��������Ƿ�Խ��
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
				if (head == sym_minus) gen(SUB, *ans, "0", *ans);										// SUB ans 0 ans		// ǰ׺����
				*type = item->type;
				return 0;
			}
		}
		else if (lex_log->sym == sym_left_parent) {														// (
		bk_sym(1);
		if (item->kind != Func) {																	// �����õ��Ƿ�Ϊ����
			commit_error_sema(WRONG_KIND, lex_log->line_counter, item->id);
			return -1;
		}
		else if (item->return_type == REVOID) {														// �����õ��Ƿ�Ϊ�з���ֵ����
			commit_error_sema(ILLEGAL_FUNC_CALL_RET, lex_log->line_counter, item->id);
			return -1;
		}
		if (analyze_call(&ret_type) != 0) {															// <�з���ֵ��������>
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
		if (item->kind == Const) {																		// ����
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
		if (analyze_exp(ans, type) != 0) {																// <���ʽ>
			return -1;
		}
		if (lex_log->sym != sym_right_parent) {															// )
			commit_error_gramma(MISSMATCH_RIGHT_PARENT, lex_log->line_counter);
			return -1;
		}
		*type = INT;
		next_sym(0);
	}
	else if (lex_log->sym == const_char) {																// <�ַ�>
		*ans = (char*)malloc(sizeof(char) * 4);
		sprintf(*ans, "%d", (int)(lex_log->ch));
		*type = CHAR;
		next_sym(0);
	}
	else {
		if (analyze_int(&num, ans) != 0) {																// <����>
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
	���::= �����ӣ�{���˷�������������ӣ�}
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
		if (analyze_factor(0, &factor, &factor_type) != 0) {								// <����>
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
	�����ʽ��::= �ۣ������ݣ��{���ӷ�����������}   
	[+|-]ֻ�����ڵ�һ��<��>
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
	if (analyze_item(op, ans, type) != 0) {											// <��>
		return -1;
	}
	while (lex_log->sym == sym_plus || lex_log->sym == sym_minus) {
		op = lex_log->sym;
		next_sym(0);
		if (analyze_item(0, &item, &item_type) != 0) {								// <��>
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
	������::=�ۣ�����˵�����ݣۣ�����˵������{<����˵��>}����������
*/
void analyze_program() {
	int v;
	next_sym(0);
	init_tbl();
	init_mid_code();
//	printf_s("This is a promgram at line %d. \n", lex_log->line_counter);
	analyze_const_dec();															// [<����˵��>]

	v = analyze_branch_program();
	if (v == BRANCH_VAR_DEC) {					
		analyze_var_dec();															// [<����˵��>]
		v = analyze_branch_program();
	}
	if (v == BRANCH_FUNC_DEF || v == BRANCH_REFUNC_DEF) {							// {<����˵��>}
		analyze_func_dec();
		v = analyze_branch_program();
	}
	while (v == BRANCH_VAR_DEC) {
		commit_error_gramma(MISSBRANCH_PROGRAM, lex_log->line_counter);
		jump(JUMP_SEMI_RBR, 1);
	}
	if (v == BRANCH_MAIN) {															// ����������
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

