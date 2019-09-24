#ifndef __DAG_H_
#define __DAG_H_

#define MAX_SIZE_DAG	100

/*
	DAG图结点
*/
typedef struct DAG_NODE {
	int op;				// 操作符
	int left;			// 左子结点
	int right;			// 右子结点
	int* parent;		// 父节点
	int sum_parent;		// 父节点数量
//	int* var;			// 驻留导出变量在结点表的索引
//	int sum_var;		// 驻留导出变量数量
	int isExport;		// 是否导出
} *dag_node, dag_node_obj;

/*
	DAG图结点表
*/
typedef struct DAG_NODE_LIST {
	char** var_name;	// 变量名
	int* need_export;	// 是否需要导出
	int* index;			// 驻留结点的索引
	int sum;			// 变量数量
} *dag_node_list, dag_node_list_obj;

extern int remove_common_exp();

#endif __DAG_H_
