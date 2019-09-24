#ifndef __DAG_H_
#define __DAG_H_

#define MAX_SIZE_DAG	100

/*
	DAGͼ���
*/
typedef struct DAG_NODE {
	int op;				// ������
	int left;			// ���ӽ��
	int right;			// ���ӽ��
	int* parent;		// ���ڵ�
	int sum_parent;		// ���ڵ�����
//	int* var;			// פ�����������ڽ��������
//	int sum_var;		// פ��������������
	int isExport;		// �Ƿ񵼳�
} *dag_node, dag_node_obj;

/*
	DAGͼ����
*/
typedef struct DAG_NODE_LIST {
	char** var_name;	// ������
	int* need_export;	// �Ƿ���Ҫ����
	int* index;			// פ����������
	int sum;			// ��������
} *dag_node_list, dag_node_list_obj;

extern int remove_common_exp();

#endif __DAG_H_
