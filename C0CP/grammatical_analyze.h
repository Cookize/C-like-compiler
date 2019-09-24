#ifndef __GRAMMA_H_
#define __GRAMMA_H_
#define _CRT_SECURE_NO_WARNINGS

// 非LL(1)分支选择结果
#define BRANCH_MAIN				(1)
#define BRANCH_REFUNC_DEF		(2)
#define BRANCH_FUNC_DEF			(3)
#define BRANCH_VAR_DEC			(4)

#define BRANCH_IF					(5)
#define BRANCH_WHILE				(6)
#define BRANCH_SENTENCE_LIST	(7)
#define BRANCH_CALL				(8)
#define BRANCH_ASSIGN			(9)
#define BRANCH_SCANF			(10)
#define BRANCH_PRINTF			(11)
#define BRANCH_NULL_SENTENCE	(12)
#define BRANCH_SWITCH			(13)
#define BRANCH_RETURN			(14)

// 容错跳转模式
#define JUMP_SEMI_RBR			(0)
#define JUMP_COMMA_SEMI		(1)
#define JUMP_COMMA_SEMI_RBR	(2)
#define JUMP_COMMA_RPR_SEMI_RBR		(3)
#define JUMP_RPR_SEMI_RBR		(4)
#define JUMP_CASE_DEFAULT_RBR	(5)
#define JUMP_RBR					(6)
#define JUMP_SEMI				(7)

extern void analyze_program();

#endif // !__GRAMMA_H_

