#include"dag.h"
#include"error.h"
#include"blocks.h"
#include"symbol_table.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

FILE *out;

block current_blk;							// 当前基本块
dag_node_list node_list = NULL;				// DAG图结点表
dag_node dag[MAX_SIZE_DAG];					// DAG图
int size_dag = 0;							// DAG图大小
four_code export_code[SIZE_MID_CODE * 2];	// 导出代码
int length_export;							// 导出位置
char *func_name;

/*
	初始化DAG图
*/
int init_dag(block blk) {
	size_dag = 0;
	current_blk = blk;
	if (node_list == NULL) {
		node_list = (dag_node_list)malloc(sizeof(dag_node_list_obj));
		node_list->var_name = (char**)malloc(sizeof(char*) * MAX_SIZE_DAG);
		node_list->index = (int*)malloc(sizeof(int) * MAX_SIZE_DAG);
		node_list->need_export = (int*)malloc(sizeof(int) * MAX_SIZE_DAG);
		node_list->sum = 0;
	}
	else {
		node_list->sum = 0;
	}
	return 0;
}

/*
	打印DAG图
*/
void print_dag() {
	static int i = 0;
	for (; i < length_export; i++) {
		print_four_code_insert((void*)out, export_code[i]);
	}
}

/*
	初始化结点
*/
int create_node(dag_node *ret) {
	dag_node new_node = (dag_node)malloc(sizeof(dag_node_obj));
	new_node->parent = (int*)malloc(sizeof(int) * MAX_SIZE_DAG);
	new_node->sum_parent = 0;
	new_node->op = 0;
	new_node->left = 0;
	new_node->right = 0;
	new_node->isExport = 0;
	*ret = new_node;
	return 0;
}

/*
	生成中间代码至缓存区
*/
int gen_opt_code(OP_FOUR_CODE op, char* a, char* b, char* c) {
	four_code new_code = (four_code)malloc(sizeof(four_code_obj));
	new_code->op = op;
	if (a != NULL) {
		new_code->a = (char*)malloc(sizeof(char) * strlen(a));
		strcpy(new_code->a, a);
	}
	else new_code->a = NULL;
	if (b != NULL) {
		new_code->b = (char*)malloc(sizeof(char) * strlen(b));
		strcpy(new_code->b, b);
	}
	else new_code->b = NULL;
	if (c != NULL) {
		new_code->c = (char*)malloc(sizeof(char) * strlen(c));
		strcpy(new_code->c, c);
	}
	else new_code->c = NULL;
	new_code->is_break_point = NORMAL;
	export_code[length_export++] = new_code;
	return 0;
}

/*
	将结点加入至结点表，不存在则新建表项
*/
int enter_node(char* name, int index, int is_leaf) {
	int i, j; 
	for (i = 0; i < node_list->sum; i++) {
		if (strcmp(node_list->var_name[i], name) == 0) {				// 已存在
			if (dag[node_list->index[i]]->isExport == 1 
				&& dag[node_list->index[i]]->left != -1 
				&& dag[node_list->index[i]]->right != -1) {
				char *var;
				get_var(node_list->index[i], &var);
				if (strcmp(var, name) == 0) {
					dag[node_list->index[i]]->isExport = 0;
				}
			}
			node_list->index[i] = index;
			return i;
		}
	}
	if (is_leaf && !(isdigit(name[0]) || name[0] == '-' || strcmp("$RET", name) == 0)) {						// 变量
		char* temp;
		alloc_temp_in(&temp, func_name);
		gen_opt_code(ASN, temp, name, NULL);
		node_list->var_name[node_list->sum] = (char*)malloc(sizeof(char) * strlen(temp));
		strcpy(node_list->var_name[node_list->sum], temp);
		node_list->index[node_list->sum] = index;
		node_list->need_export[node_list->sum] = 0;
		node_list->sum++;
	}
	node_list->var_name[node_list->sum] = (char*)malloc(sizeof(char) * strlen(name));
	strcpy(node_list->var_name[node_list->sum], name);
	node_list->index[node_list->sum] = index;
	return node_list->sum++;
}

/*
	判断是否需要导出
*/
int need_export(char* name) {
	if (name == NULL) return 0;
	if (is_global(name)) {								// 全局变量
		return 1;
	}
	int i;
	for (i = 0; i < current_blk->live_var->sum_out; i++) {				// 搜索是否活跃
		if (strcmp(name, current_blk->live_var->out[i]) == 0) {
			return 1;
		}
	}
	return 0;
}

/*
	搜索DAG结点，不存在则新建结点
*/
int search_node(OP_FOUR_CODE op, char* name, char* left, char* right) {
	int i, j, temp;
	int left_index = -1, right_index = strcmp(right, "") == 0 ? -2 : -1;
	for (i = 0; i < node_list->sum; i++) {
		if (left_index == -1 && strcmp(node_list->var_name[i], left) == 0) {	
			left_index = node_list->index[i];								// 找到左节点
			if (right_index == -2 && dag[left_index]->left != -1 && dag[left_index]->right != -1) {
				dag[left_index]->isExport = 0;
				temp = enter_node(name, left_index, 0);								// 赋值语句更新结点表
				node_list->need_export[temp] = need_export(name);
				return 0;
			}
		}
		else if (right_index == -1 && strcmp(node_list->var_name[i], right) == 0) {	
			right_index = node_list->index[i];								// 找到右结点
		}
	}
	if (left_index != -1 && right_index != -1) {							
		for (i = 0; i < dag[left_index]->sum_parent; i++) {
			if (right_index == -2 && dag[dag[left_index]->parent[i]]->op == op) {
				dag[dag[left_index]->parent[i]]->isExport = 0;
				temp = enter_node(name, dag[left_index]->parent[i], 0);				// 赋值语句更新结点表
				node_list->need_export[temp] = need_export(name);
				return 0;
			}
			else if(right_index != -2){
				for (j = 0; j < dag[right_index]->sum_parent; j++) {
					if (dag[left_index]->parent[i] == dag[right_index]->parent[j]
						&& dag[dag[left_index]->parent[i]]->op == (int)op) {	// 找到该结点
						dag[dag[left_index]->parent[i]]->isExport = 0;
						temp = enter_node(name, dag[left_index]->parent[i], 0);			// 更新结点表
						node_list->need_export[temp] = need_export(name);
						return 0;
					}
				}
			}
		}
	}
	// 创建新节点
	dag_node new_node;
	if (left_index == -1) {													// 创建左节点
		create_node(&new_node);
		left_index = size_dag++;
		dag[left_index] = new_node;											// 加入DAG图
		new_node->op = -1;													// 叶节点
		new_node->left = -1;
		new_node->right = -1;
		new_node->isExport = 1;
		i = enter_node(left, left_index, op == GEI ? 0 : 1);
		node_list->need_export[i] = need_export(left);
	}
	if (right_index == -1) {												// 创建右结点
		create_node(&new_node);
		right_index = size_dag++;
		dag[right_index] = new_node;										// 加入DAG图
		new_node->op = -1;													// 叶节点
		new_node->left = -1;
		new_node->right = -1;
		new_node->isExport = 1;
		i = enter_node(right, right_index, 1);
		node_list->need_export[i] = need_export(right);
	}
	create_node(&new_node);
	dag[size_dag++] = new_node;
	new_node->op = (int)op;
	new_node->left = left_index;
	new_node->right = right_index;
	i = enter_node(name, size_dag - 1, 0);
	node_list->need_export[i] = need_export(name);
	dag[left_index]->parent[dag[left_index]->sum_parent++] = size_dag - 1;	// 连接父节点
	if (right_index >= 0) {
		dag[right_index]->parent[dag[right_index]->sum_parent++] = size_dag - 1;
	}
	return 0;
}

/*
	检查所有父节点是否已导出
*/
int is_exportable(dag_node node) {
	int j;
	for (j = 0; j < node->sum_parent; j++) {
		if (dag[node->parent[j]]->isExport == 0) {
			return 0;
		}
	}
	return 1;
}

/*
	获取变量代表
*/
int get_var(int index, char** ret) {
	int i, is_leaf = (dag[index]->left == -1 && dag[index]->right == -1);
	for (i = 0; i < node_list->sum; i++) {										// 搜索导出变量和常量结点
		if (index == node_list->index[i] 
			&& (node_list->need_export[i] == 1 || node_list->var_name[i][0] == '-' || isdigit(node_list->var_name[i][0]))) {
			*ret = (char*)malloc(sizeof(char) * strlen(node_list->var_name[i]));
			strcpy(*ret, node_list->var_name[i]);
			return 2;
		}
	}
	for (i = 0; i < node_list->sum; i++) {										// 搜索临时变量
		if (index == node_list->index[i]) {
			*ret = (char*)malloc(sizeof(char) * strlen(node_list->var_name[i]));
			strcpy(*ret, node_list->var_name[i]);
			return 1;
		}
	}
	alloc_temp_in(ret, func_name);												// 结点无驻留
	enter_node(*ret, index, 0);
	return 0;
}

/*
	导出结点
*/
int export_node(int index, char** name) {
	char *left, *right;
	dag_node node = dag[index];
	if (node->left == -1 && node->right == -1) {				// 叶子节点
		get_var(index, name);
		return 0;
	}
	if (dag[node->left]->isExport == 0) {						// 导出左节点
		export_node(node->left, &left);
	}
	else {
		get_var(node->left, &left);
	}
	if (node->right != -2) {
		if (dag[node->right]->isExport == 0) {					// 导出右结点
			export_node(node->right, &right);
		}
		else {
			get_var(node->right, &right);
		}
	}
	else {
		right = NULL;
	}
	if (get_var(index, name) != 2) {							// 导出本身
		gen_opt_code(node->op, *name, left, right);
	}
	else {
		int i;
		for (i = 0; i < node_list->sum; i++) {
			if (index == node_list->index[i]
				&& node_list->need_export[i] == 1) {
				gen_opt_code(node->op, node_list->var_name[i], left, right);
			}
		}
	}
	node->isExport = 1;
	return 0;
}

/*
	导出已生成的DAG图
*/
int export_dag() {
	int i, j, is_root = 0, isEnd = 0;
	char* ret;
	while (!isEnd){
		isEnd = 1;
		for (i = 0; i < size_dag; i++) {
			if (dag[i]->isExport == 0) {
				is_root = 1;
				for (j = 0; j < dag[i]->sum_parent; j++) {
					if (dag[dag[i]->parent[j]]->isExport == 0) {
						is_root = 0;
					}
				}
				isEnd = 0;
				if (is_root) {
					export_node(i, &ret);
				}
			}
		}
	}
	return 0;
}

/*
	重置返回变量
*/
int clear_ret() {
	int i;
	for (i = 0; i < node_list->sum; i++) {
		if (strcmp(node_list->var_name[i], "$RET") == 0) {
			node_list->var_name[i][1] = '$';
		}
		if (is_global(node_list->var_name[i])) {
			node_list->var_name[i][0] = '$';
			node_list->var_name[i][0] = '$';
		}
	}
	return 0;
}

/*
	生成DAG图
*/
int gen_dag() {
	int mid_code_pointer;
	for (mid_code_pointer = current_blk->first_index; mid_code_pointer <= current_blk->last_index; mid_code_pointer++) {
		four_code code = mid_code[mid_code_pointer];
		switch (code->op) {
		case AEI:
			export_dag();
			int i, index_flag = 0, num_flag = 0;
			char *index = NULL, *num = NULL;
			for (i = 0; i < node_list->sum; i++) {
				if (index_flag == 0 && strcmp(code->b, node_list->var_name[i]) == 0) {
					get_var(node_list->index[i], &index);
					index_flag = 1;
				}
				if (num_flag == 0 && strcmp(code->c, node_list->var_name[i]) == 0) {
					get_var(node_list->index[i], &num);
					num_flag = 1;
				}
			}
			gen_opt_code(code->op, code->a, index_flag ? index : code->b, num_flag ? num : code->c);
			break;
		case GEI:
		case ADD:
		case SUB:
		case MUL:
		case DIV:
			search_node(code->op, code->a, code->b, code->c);
			break;
		case ASN:
			search_node(code->op, code->a, code->b, "");
			break;
		case LES:
		case LEQ:
		case GTR:
		case GEQ:
		case EQL:
		case NEQ:
			export_dag();
			index_flag = 0;
			num_flag = 0;
			index = NULL;
			num = NULL;
			for (i = 0; i < node_list->sum; i++) {
				if (index_flag == 0 && strcmp(code->a, node_list->var_name[i]) == 0) {
					get_var(node_list->index[i], &index);
					index_flag = 1;
				}
				if (num_flag == 0 && strcmp(code->b, node_list->var_name[i]) == 0) {
					get_var(node_list->index[i], &num);
					num_flag = 1;
				}
			}
			gen_opt_code(code->op, index_flag ? index : code->a, num_flag ? num : code->b, NULL);
			break;
		case CALL:
			export_dag();
			clear_ret();
			gen_opt_code(code->op, code->a, NULL, NULL);
			break;
		case GOTO:
			export_dag();
			gen_opt_code(code->op, code->a, NULL, NULL);
			break;
		case RET:
			export_dag();
			if(code->a != NULL){
				int i;
				char* aim = NULL;
				for (i = 0; i < node_list->sum; i++) {
					if (strcmp(code->a, node_list->var_name[i]) == 0) {
						get_var(node_list->index[i], &aim);
						break;
					}
				}
				gen_opt_code(code->op, i < node_list->sum ? aim : code->a, NULL, NULL);
			}
			else {
				gen_opt_code(code->op, NULL, NULL, NULL);
			}
			break;
		case PUSH:
			export_dag();
			do {
				int i;
				char* aim = NULL;
				for (i = 0; i < node_list->sum; i++) {
					if (strcmp(code->a, node_list->var_name[i]) == 0) {
						get_var(node_list->index[i], &aim);
						break;
					}
				}
				gen_opt_code(code->op, i < node_list->sum ? aim : code->a, code->b, NULL);
				mid_code_pointer++;
				code = mid_code[mid_code_pointer];
			} while (code->op == PUSH);
			gen_opt_code(code->op, code->a, NULL, NULL);
			clear_ret();
			break;
		case READ:
			int j, is_exist = 0;
			char* aim = NULL;
			for (j = 0; j < node_list->sum; j++) {
				if (strcmp(code->a, node_list->var_name[j]) == 0) {
					node_list->var_name[j][0] = '$';
					break;
				}
			}
			gen_opt_code(code->op, code->a, code->b, NULL);
			break;
		case PRINT:
			export_dag();
			aim = NULL;
			for (j = 0; j < node_list->sum; j++) {
				if (strcmp(code->a, node_list->var_name[j]) == 0) {
					get_var(node_list->index[j], &aim);
					break;
				}
			}
			gen_opt_code(code->op, j < node_list->sum ? aim : code->a, code->b, NULL);
			break;
		case LABEL:
		case BZ:
			gen_opt_code(code->op, code->a, NULL, NULL);
			break;
		default:
			break;
		}
	}
	return 0;
}

/*
	写回优化后的中间代码
*/
void write_back() {
	int i;
	if (fopen_s(&out, "dags_export_mid.txt", "w") != 0) {
		error_stand(FILE_NOT_FOUND);
		// 无法打开文件，结束运行。
	}
	for (i = 0, length_mid_code = 0; i < length_export && i < SIZE_MID_CODE; i++) {
		mid_code[length_mid_code++] = export_code[i];
		insert_break_point(i);
		print_four_code_insert(out, mid_code[i]);
	}
	fclose(out);
	return;
}

/*
	块内公共子表达式消除
*/
int remove_common_exp() {
	int func_pointer, pointer, i;
	if (fopen_s(&out, "dags.txt", "w") != 0) {
		error_stand(FILE_NOT_FOUND);
		// 无法打开文件，结束运行。
	}
	for (pointer = 0; pointer < length_mid_code && mid_code[pointer]->is_break_point != FUNCSTART; pointer++) {
		export_code[length_export++] = mid_code[pointer];
	}
	for (func_pointer = 0; func_pointer < length_func_blk; func_pointer++) {
		fprintf_s(out, "FUNC:\t%s\n", func_blk_list[func_pointer]->func_name);
		func_name = func_blk_list[func_pointer]->func_name;
		while (pointer < length_mid_code
			&& (mid_code[pointer]->is_break_point == IGNORE || mid_code[pointer]->is_break_point == FUNCSTART)) {
			export_code[length_export++] = mid_code[pointer++];
		}
		int blk_pointer;
		for (blk_pointer = 0; blk_pointer < func_blk_list[func_pointer]->sum_block; blk_pointer++) {
			init_dag(func_blk_list[func_pointer]->block_list[blk_pointer]);
			gen_dag();
			export_dag();
			fprintf_s(out, "BLK:\t%d\n", blk_pointer);
			print_dag();
			fprintf_s(out, "*----------------------------------------------------------*\n");
		}
		fprintf_s(out, "*************************************************************\n");
		pointer = func_blk_list[func_pointer]->block_list[func_blk_list[func_pointer]->sum_block - 1]->last_index + 1;
	}
	fclose(out);
	write_back();					// 写回
	return 0;
}


