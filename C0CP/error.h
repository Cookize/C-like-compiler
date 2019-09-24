#ifndef __ERROR_H_
#define __ERROR_H_

/*
	error code
*/
/*
	standard error
*/
#define	ILLEGAL_NUM				1					// ���������������
#define FILE_NOT_FOUND			2					// �ļ���ʧ��
#define END_FILE				3					// ��ȡ���ļ�β������������
#define OVERFLOW_MID_CODE		4					// �м���볬��

/*
	lex error
*/
#define LEX_ERROR				1					// �ʷ�����
#define TOO_LARGE_INT			2					// �����������
#define ILLEGAL_INT				3					// �Ƿ����ָ�ʽ
#define TOO_LONG_STRING			4					// �����ַ������
#define MISSMATCH_SYM_QUOTE			5		// ������ƥ�����
#define MISSMATCH_SYM_DOUBLE_QUOTE	6		// ˫����ƥ�����
#define MISSMATCH_SYM_NOT_EQUAL		7		// ���Ⱥ�ƥ�����
#define MISSMATCH_CHAR			8					// �ַ�ƥ�����
#define ILLEGAL_CHAR			9					// �Ƿ���ʶ��
#define EXCEPTIONAL_END			10					// �쳣����

/*
	gramma error
*/
#define MISSMATCH_CONST_INT				1		// δƥ�䵽����
#define MISSMATCH_IDENTIFIER			2		// δƥ�䵽��ʶ��
#define MISS_MAIN						3		// δƥ�䵽������
#define MISSMATCH_INT					4		// δƥ�䵽�޷�������
#define MISSMATCH_CONST_CHAR			5		// δƥ�䵽�ַ�����
#define MISSMATCH_TYPE_DEF				6		// δƥ�䵽�������������֡�int����char��
#define MISSMATCH_VOID					7		// δƥ�䵽��void��
#define	MISSMATCH_KEY_CONST				8		// δƥ�䵽��const��
#define MISSMATCH_MAIN					9		// δƥ�䵽��main��
#define MISSMATCH_IF					10		// δƥ�䵽��if��
#define MISSMATCH_SCANF					11		// δƥ�䵽��scanf��
#define MISSMATCH_PRINTF				12		// δƥ�䵽��printf��
#define MISSMATCH_SWITCH				13		// δƥ�䵽��switch��
#define MISSMATCH_CASE					14		// δƥ�䵽��case��
#define MISSMATCH_DEFAULT				15		// δƥ�䵽��default��
#define MISSMATCH_RETURN				16		// δƥ�䵽��return��
#define MISSMATCH_SEMICOLON				17		// δƥ�䵽�ֺ�
#define MISSMATCH_RIGHT_BRACKET			18		// δƥ�䵽��������
#define MISSMATCH_LEFT_PARENT			19		// δƥ�䵽������
#define MISSMATCH_RIGHT_PARENT			20		// δƥ�䵽������
#define MISSMATCH_LEFT_BRACE			21		// δƥ�䵽�������
#define MISSMATCH_RIGHT_BRACE			22		// δƥ�䵽�Ҵ�����
#define MISSMATCH_BECOME				23		// δƥ�䵽��ֵ��
#define MISSMATCH_COMPARE				24		// δƥ�䵽��ϵ�����
#define MISSMATCH_COLON					25		// δƥ�䵽ð��
#define MISSMATCH_PARALIST_END			26		// �����������ƥ�����
#define	MISSMATCH_COMMA					27		// δƥ�䵽����
#define INT_OVERFLOW					28		// ����Խ��

#define MISSBRANCH_PROGRAM				101		// <����>�з�֧����
#define MISSBRANCH_SENTENCE				102		// <���>�з�֧����
#define MISSBRANCH_FACTOR				103		// <����>�з�֧����

#define EXCESSIVE_PROGRAM				110		// Դ���򳬳�<����>
#define RECOVER_RIGHT					140		// �����Żָ���
#define RECOVER_LEFT					141		// �����Żָ���


/*
	sema error
*/
#define	DUPLICATE						1		// �ظ�����
#define WRONG_TYPE						2		// ����ʱ���ʹ���
#define ILLEGAL_COMPARE_TYPE			3		// �Ƚ�ʽ���ʹ���
#define UNDEFINED						4		// δ����
#define MISTAKE_SUM_VALUE_PARA			5		// ��������ֵ��������������
#define ILLEGAL_FUNC_CALL				6		// �޺�������
#define ILLEGAL_ARRAY_SIZE				7		// ���鳤�ȶ������
#define INDEX_OVERFLOW					8		// �����±�Խ��
#define ILLEGAL_FUNC_CALL_RET			9		// �Ƿ������޷���ֵ����
#define WRONG_ASSIGN_TYPE				10		// ��ֵ���Ͳ�ƥ��
#define WRONG_KIND						11		// �Ƿ�����
#define WRONG_INDEX_TYPE				12		// �Ƿ������±�����
#define NO_RETURN						13		// ��������Ч�������
#define WRONG_RETURN_TYPE				14		// ����ֵ���ʹ���

/*
	block error
*/
#define ILLEGAL_BLOCK					1		// �ֿ����

/*
	mips generate error
*/
#define MISSMATCH_VAR					1		// ������
extern void commit_error_lex(int, int, int);
extern void commit_error_gramma(int, int);
extern void commit_error_sema(int, int, char*);
extern void error_stand(int);
extern void commit_jump_info(int line, char *id);
extern void commit_error_mips_gen(int error_code, char* info_1, char* info_2);
extern void commit_error_block(int, int);

#endif // !__ERROR_H_
