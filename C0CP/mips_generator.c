#include"mips_generator.h"
#include"mid_code.h"
#include"symbol_table.h"
#include"error.h"
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define SUM_TEMP_REG		8		
#define TEMPREG(x)			temp_regs[x]

extern tbl table;
extern stbl str_table;
extern four_code mid_code[SIZE_MID_CODE];
extern int length_mid_code;

int mid_code_pointer = 0;										// Ŀ�����ָ��
FILE *mips_out;													// ����ļ�
tbl_field current_field;										// ��ǰ����

int lru_sum = 0;
lru_item lru_list[SUM_TEMP_REG] = { NULL };						// LRU��
char temp_regs[SUM_TEMP_REG + 1][5] = {
	REG_t0, REG_t1, REG_t2, REG_t3, REG_t4, REG_t5, REG_t6, REG_t7, REG_v0
};
int temp_regs_usage[SUM_TEMP_REG] = {							// ��ʱ�Ĵ���ʹ�������0---δʹ�ã�1---פ����2---��ʱʹ�ã�
	0, 0, 0, 0, 0, 0, 0, 0
};

/*
	��ʼ��
*/
void init_generator() {
	mid_code_pointer = 0;
	if (fopen_s(&mips_out, "mips.txt", "w") != 0) {
		// �޷����ļ����������С�
	}
}

/*
	�ͷ���Դ
*/
void reset_generator() {
	fclose(mips_out);
}

void print_LRU() {
	int i;
	printf("%d:\t", mid_code_pointer);
	for (i = 0; i < SUM_TEMP_REG; i++) {
		printf("%d", temp_regs_usage[i]);
	}
	printf("\n");
	return;
}

/*
	����MPIS������
	TODO:�޸������ʽ
*/
void gen_mips(char* op, char* reg_1, char* reg_2, char* reg_3) {
	if (strcmp(op, MIPS_LABEL) == 0) {
		fprintf(mips_out, "%s:\n", reg_1);
		return;
	}
	else if (strcmp(op, MIPS_SW) == 0
		|| strcmp(op, MIPS_LW) == 0) {
		fprintf(mips_out, "%s %s, %s(%s)\n", op, reg_1, reg_2, reg_3);
		return;
	}
	else if (strcmp(op, MIPS_ADD) == 0
		|| strcmp(op, MIPS_ADDI) == 0
		|| strcmp(op, MIPS_ADDU) == 0
		|| strcmp(op, MIPS_SUB) == 0
		|| strcmp(op, MIPS_SUBU) == 0
		|| strcmp(op, MIPS_SLL) == 0
		|| strcmp(op, MIPS_BEQ) == 0
		|| strcmp(op, MIPS_BGT) == 0
		|| strcmp(op, MIPS_BGE) == 0
		|| strcmp(op, MIPS_BLE) == 0
		|| strcmp(op, MIPS_BLT) == 0
		|| strcmp(op, MIPS_BNE) == 0) {
		fprintf(mips_out, "%s %s, %s, %s\n", op, reg_1, reg_2, reg_3);
		return;
	}
	else if (strcmp(op, MIPS_MUL) == 0
		|| strcmp(op, MIPS_DIV) == 0
		|| strcmp(op, MIPS_LA) == 0) {
		fprintf(mips_out, "%s %s, %s\n", op, reg_1, reg_2);
		return;
	}
	else if (strcmp(op, MIPS_J) == 0
		|| strcmp(op, MIPS_JR) == 0
		|| strcmp(op, MIPS_JAL) == 0
		|| strcmp(op, MIPS_JALR) == 0
		|| strcmp(op, MIPS_MFLO) == 0) {
		fprintf(mips_out, "%s %s\n", op, reg_1);
		return;
	}
	else if (strcmp(op, MIPS_CALL) == 0
		|| strcmp(op, MIPS_DATA) == 0
		|| strcmp(op, MIPS_TEXT) == 0) {
		fprintf(mips_out, "%s\n", op);
		return;
	}
}

/*
	��ȡδ��ʹ�õļĴ���
*/
int get_unused_temp_reg() {
	int i;
	for (i = 0; i < SUM_TEMP_REG; i++) {
		if (temp_regs_usage[i] == 0)	return i;
	}
	return -1;
}

/*
	��ȡ�����Ͳ������ƫ��
	��ȡ������Ԫ�ص����ƫ��
	name -------- ����
	index ------- �����±�
	offset ------ �������ƫ��
	reg --------- ��ԵļĴ���
*/
void get_offset(char *name, char** offset, char** reg) {
	int index, counter = 0;			
	char num[20];
	*offset = (char*)malloc(sizeof(char) * 20);
	*reg = (char*)malloc(sizeof(char) * 5);
	for (index = current_field->first_item_loca; index <= current_field->last_item_loca; index++) {		// �����ֲ����ű�
		if (strcmp(table->item_list[index]->id, name) == 0) {											// �ھֲ����ҵ�
			if (table->item_list[index]->kind == Para) {												// ����
				sprintf(*offset, "%d", (current_field->para_sum - table->item_list[index]->addr) * 4);						// �������$fp��ƫ��
				strcpy(*reg, "$fp");
				return;
			}
			else if (table->item_list[index]->kind == Var) {											// ����
				sprintf(*offset, "%d", -(counter + 2) * 4);												// �������$fp��ƫ��
				strcpy(*reg, "$fp");
				return;
			}
		}
		if (table->item_list[index]->kind == Var) {													// ����
			table->item_list[index]->array_length > 0 ? counter += table->item_list[index]->array_length :
				counter++;
		}
	}
	if (name[0] == '$') {																				// ��ʱ����
		strcpy(num, name);
		num[0] = '0';
		num[1] = '0';
		sprintf(*offset, "%d", -(atoi(num) + counter + 2) * 4);							// �������$fpƫ��
		strcpy(*reg, "$fp");
		return;
	}
	for (index = counter = 0; index <= table->field_list[0]->last_item_loca; index++) {
		if (strcmp(table->item_list[index]->id, name) == 0) {										// ��ȫ�����ҵ�
			if (table->item_list[index]->kind == Var) {												// ����
				sprintf(*offset, "%d", (counter) * 4);												// �������$gp��ƫ��
				strcpy(*reg, "$gp");
				return;
			}
		}
		if (table->item_list[index]->kind == Var) {													// ����
			table->item_list[index]->array_length > 0 ? counter += table->item_list[index]->array_length :
				counter++;
		}
	}
	commit_error_mips_gen(MISSMATCH_VAR, current_field->func_name, name);										// ���ʧ�ܴ���
}

/*
TODO��
	������ʱ�Ĵ���
	פ�����ԣ�
		���ڷ���������Ͳ�����������ʱ�Ĵ���ʱ���Ὣ�����LRUפ�����У���ʹ����ɺ������ͷţ�
		��������Ԫ�أ��Է�פ����ʽ������ʱ�Ĵ�����ʹ����ɺ���Ҫ����д�ز��ͷżĴ�����
		���ڼ����м����ӣ��Է�פ����ʽ������ʱ�Ĵ�����ʹ����ɺ���Ҫ����д�ز��ͷżĴ�����
*/
/*
	����פ���Ĵ�����reg_ret���ؼĴ���������
		1.����LRU��������Ƿ��Ѿ�פ��
		2.��פ����������ǰ������
		3.��δפ�����һ���ʣ��Ĵ�������¼פ�����������������
		3.��δפ�����������ڼĴ�����ȡ��Ϊ��¼��д�����ݲ���¼��פ�����������������
*/
void alloc_reg_resident(int dirty, char* name, int* reg_ret) {
	lru_item temp;
	char *offset, *reg;
	int i, j;
	if (strcmp(name, "$RET") == 0) {
		*reg_ret = SUM_TEMP_REG;
		return;
	}
	for (i = 0; i < lru_sum; i++) {											// ��LRU���в���
		if (strcmp(lru_list[i]->name, name) == 0) {							// ��LRU�����ҵ�
			temp = lru_list[i];
			for (j = i; j > 0; j--) {										// ��ǰ������
				lru_list[j] = lru_list[j - 1];
			}
			lru_list[0] = temp;
			*reg_ret = temp->reg;
			temp->dirty |= dirty;											// ������λ
			return;
		}
	}
	if ((i = get_unused_temp_reg()) != -1) {								// ���мĴ���δ����
		temp = (lru_item)malloc(sizeof(lru_item_obj));
		temp->name = (char*)malloc(sizeof(char) * strlen(name));
		strcpy(temp->name, name);											// ����name
		temp->reg = i;														// ����reg
		temp->dirty = 0;
		temp_regs_usage[temp->reg] = 1;										// ������ʱ�Ĵ���ʹ��״̬
		for (j = lru_sum; j > 0; j--) {										// ��������
			lru_list[j] = lru_list[j - 1];
		}
		lru_list[0] = temp;
		lru_sum++;
	}
	else {																	// ��ʣ��Ĵ���
		temp = lru_list[lru_sum - 1];
		for (j = lru_sum - 1; j > 0; j--) {									// ��������
			lru_list[j] = lru_list[j - 1];
		}
		lru_list[0] = temp;
		if (temp->dirty == 1) {
			get_offset(temp->name, &offset, &reg);
			gen_mips(MIPS_SW, TEMPREG(temp->reg), offset, reg);					// ��λд��
		}
		temp->name = (char*)malloc(sizeof(char) * strlen(name));			// ����ʹ�õı�����
		temp->dirty = 0;
		strcpy(temp->name, name);
	}
	get_offset(temp->name, &offset, &reg);
	temp->dirty |= dirty;													// ������λ
	gen_mips(MIPS_LW, TEMPREG(temp->reg), offset, reg);					// ����
	*reg_ret = temp->reg;
	return;
}

/*
	����.data��
*/
void gen_data() {
	int i;
	gen_mips(MIPS_DATA, NULL, NULL, NULL);
	for (i = 0; i < str_table->sum; i++) {
		fprintf(mips_out, "s$%d : .asciiz \"%s\"\n", i, str_table->strings[i]);
	}
}

/*
	��ʼ��ջ�ռ�:
		1.��-4($sp)Ϊ$ra
		2.��-8($sp)Ϊ$fp
		3.��$fpΪ��ջ֡��ʼ��ַ
		4.����ֲ���������ʱ�����ռ��С����$spΪջ��
*/
void gen_init_stack() {
	char frame_size[20];
	int counter = 0, i;
	for (i = current_field->first_item_loca; i <= current_field->last_item_loca; i++) {			// ����ֲ�������С
		if (table->item_list[i]->kind == Var) {
			table->item_list[i]->array_length > 0 ? counter += table->item_list[i]->array_length : counter++;
		}
	}
	sprintf(frame_size, "%d", -(counter + current_field->temp_sum + 2) * 4);
	gen_mips(MIPS_SW, REG_ra, "-0", REG_sp);													// sw $ra -4($sp)
	gen_mips(MIPS_SW, REG_fp, "-4", REG_sp);													// sw $fp -8($sp)
	gen_mips(MIPS_ADDU, REG_fp, REG_0, REG_sp);													// ����$fpΪ��ջ֡��ʼλ��
	gen_mips(MIPS_ADDI, REG_sp, REG_sp, frame_size);											// ����$spΪ��ջ֡ջ��
	return;
}

/*
	���滷����
		1.ɨ������פ���ļĴ���
		2.���շ������д������Ӧ�ĵ�ַ
*/
void gen_save_env() {
	int i;
	char *offset, *reg;
	//printf_s("WB:\t");
	for (i = 0; i < lru_sum; i++) {
		if (lru_list[i]->dirty == 1) {
			get_offset(lru_list[i]->name, &offset, &reg);
			gen_mips(MIPS_SW, TEMPREG(lru_list[i]->reg), offset, reg);		// ��λд��
			//printf_s("%d", lru_list[i]->reg);
		}
		//free(lru_list[i]);
	}
	//printf_s("\n");
	lru_sum = 0;
	for (i = 0; i < SUM_TEMP_REG; i++) {
		temp_regs_usage[i] = 0;
	}
	return;
}

/*
	����������
		1.���ݺ��������õ�ǰ��
		2.������ת��ǩ
		3.main������$fp����$sp
		4.main������ʼ��ջ֡����
*/
void gen_func_def(char *name) {
	int i;
	current_field = find_field(name);															// ���õ�ǰ��
	lru_sum = 0;
	for (i = 0; i < SUM_TEMP_REG; i++) {
		temp_regs_usage[i] = 0;
	}
	gen_mips(MIPS_LABEL, name, NULL, NULL);														// ���ɺ�����ǩ
	if (strcmp(name, "main") == 0) {															// main����
		gen_mips(MIPS_ADDU, REG_fp, REG_sp, REG_0);												// ��ʼ��$fp
	}
	gen_init_stack();																			// ��ʼ��ջ֡
	return;
}

/*
	�������ã�
		1.�����ֳ�
		2.��ʼ�������ú�����ջ�ռ�
*/
void gen_func_call(char* name) {																			// ���滷��
	gen_mips(MIPS_JAL, name, NULL, NULL);														// jal name
	return;
}

/*
	�������أ�
		1.�з���ֵ����������ֵ��ֵ$v0
		2.��$raΪ���ص�ַ
		3.��$spΪ��һջ֡����ַ
		4.��$fpΪ��һջ֡��ʼ��ַ
*/
void gen_return(char* ret) {
	char offset[20];
	int reg;
	if (ret != NULL) {																			// ����ֵ��ֵ
		if (isdigit(ret[0]) || ret[0] == '-') {
			gen_mips(MIPS_ADDI, REG_v0, REG_0, ret);											// ���س���ֵ
		}
		else {
			alloc_reg_resident(0, ret, &reg);
			gen_mips(MIPS_ADDU, REG_v0, TEMPREG(reg), REG_0);									// ������
		}
	}
	sprintf(offset, "%d", (current_field->para_sum) * 4);			
	gen_save_env();
	gen_mips(MIPS_LW, REG_ra, "-0", REG_fp);													// ��$raΪ���ص�ַ
	gen_mips(MIPS_ADDI, REG_sp, REG_fp, offset);												// ��$spΪ��һջ֡����ַ
	gen_mips(MIPS_LW, REG_fp, "-4", REG_fp);													// ��$fpΪ��һջ֡��ʼ��ַ
	gen_mips(MIPS_JR, REG_ra, NULL, NULL);														// jr $ra
	return;
}

/*
	ѡ����ת��
		1.�ֱ���رȽ�����
		1.���ݱȽ��������ɲ�������ת���
*/
void gen_B_jump(OP_FOUR_CODE compare, char* name_1, char* name_2, char* label) {
	int temp_reg_1 = 0, temp_reg_2 = 0;
	char reg_1[5], reg_2[5];
	char num[20];
	if ((isdigit(name_1[0]) || name_1[0] == '-') && (isdigit(name_2[0]) || name_2[0] == '-')) {
		sprintf(num, "%d", atoi(name_1) - atoi(name_2));
		gen_mips(MIPS_ADDI, REG_t8, REG_0, num);
		strcpy(reg_1, REG_t8);
		strcpy(reg_2, REG_0);
	}
	else if (isdigit(name_1[0]) || name_1[0] == '-') {
		alloc_reg_resident(0, name_2, &temp_reg_2);
		gen_mips(MIPS_ADDI, REG_t9, REG_0, name_1);
		strcpy(reg_1, REG_t9);
		strcpy(reg_2, TEMPREG(temp_reg_2));
	}
	else if (isdigit(name_2[0]) || name_2[0] == '-') {
		alloc_reg_resident(0, name_1, &temp_reg_1);
		gen_mips(MIPS_ADDI, REG_t8, REG_0, name_2);
		strcpy(reg_1, TEMPREG(temp_reg_1));
		strcpy(reg_2, REG_t8);
	}
	else {
		alloc_reg_resident(0, name_1, &temp_reg_1);
		alloc_reg_resident(0, name_2, &temp_reg_2);
		strcpy(reg_1, TEMPREG(temp_reg_1));
		strcpy(reg_2, TEMPREG(temp_reg_2));
	}
	gen_save_env();
	switch (compare) {
	case LES:
		gen_mips(MIPS_BGE, reg_1, reg_2, label);		// bge name_1 name_2 label
		break;
	case LEQ:
		gen_mips(MIPS_BGT, reg_1, reg_2, label);		// bgt name_1 name_2 label
		break;
	case GTR:
		gen_mips(MIPS_BLE, reg_1, reg_2, label);		// ble name_1 name_2 label
		break;
	case GEQ:
		gen_mips(MIPS_BLT, reg_1, reg_2, label);		// blt name_1 name_2 label
		break;
	case EQL:
		gen_mips(MIPS_BNE, reg_1, reg_2, label);		// bne name_1 name_2 label
		break;
	case NEQ:
		gen_mips(MIPS_BEQ, reg_1, reg_2, label);		// beq name_1 name_2 label
		break;
	}
	return;
}

/*
	������䣺
		1.���ݼ����������ɲ�ͬ����
	TODO������Խ����
*/
void gen_cul(OP_FOUR_CODE op, char* dst, char* rec_1, char* rec_2) {
	int temp_reg_1 = 0, temp_reg_2 = 0, temp_reg_3 = 0;
	char *offset_1;
	char *reg_1;
	char num_1[20], num_2[20];
	switch (op) {
	case GEI:
		get_offset(rec_1, &offset_1, &reg_1);															// ��ȡ������Ԫ�ص�ƫ��
		if (isdigit(rec_2[0]) || rec_2[0] == '-') {
			sprintf(num_1, "%d", strcmp(reg_1, REG_fp) == 0 ? -atoi(rec_2) * 4 : atoi(rec_2) * 4);
			gen_mips(MIPS_ADDI, REG_t8, reg_1, num_1);
		}
		else {
			alloc_reg_resident(0, rec_2, &temp_reg_2);													// ���������±�
			gen_mips(MIPS_SLL, REG_t9, TEMPREG(temp_reg_2), "2");										// ����������ƫ��
			gen_mips(strcmp(reg_1, REG_fp) == 0 ? MIPS_SUBU : MIPS_ADDU, REG_t8, reg_1, REG_t9);
		}
		alloc_reg_resident(1, dst, &temp_reg_3);
		gen_mips(MIPS_LW, TEMPREG(temp_reg_3), offset_1, REG_t8);										// ��������Ԫ��ֵ
		break;
	case AEI:
		get_offset(dst, &offset_1, &reg_1);																// ��ȡ������Ԫ�ص�ƫ��
		if (isdigit(rec_1[0]) || rec_1[0] == '-') {
			sprintf(num_1, "%d", strcmp(reg_1, REG_fp) == 0 ? -atoi(rec_1) * 4 : atoi(rec_1) * 4);
			gen_mips(MIPS_ADDI, REG_t8, reg_1, num_1);									// ����ƫ��
		}
		else {
			alloc_reg_resident(0, rec_1, &temp_reg_3);
			gen_mips(MIPS_SLL, REG_t9, TEMPREG(temp_reg_3), "2");
			gen_mips(strcmp(reg_1, REG_fp) == 0 ? MIPS_SUBU : MIPS_ADDU, REG_t8, reg_1, REG_t9);													// ����ƫ��
		}
		if (isdigit(rec_2[0]) || rec_2[0] == '-') {
			gen_mips(MIPS_ADDI, REG_t9, REG_0, rec_2);													// ����ֵ
			gen_mips(MIPS_SW, REG_t9, offset_1, REG_t8);
		}
		else {
			alloc_reg_resident(0, rec_2, &temp_reg_2);														// ����ֵ
			gen_mips(MIPS_SW, TEMPREG(temp_reg_2), offset_1, REG_t8);
		}
		break;
	case ASN:
		if (isdigit(rec_1[0]) || rec_1[0] == '-') {
			alloc_reg_resident(1, dst, &temp_reg_1);
			gen_mips(MIPS_ADDI, TEMPREG(temp_reg_1), REG_0, rec_1);
		}
		else {
			alloc_reg_resident(0, rec_1, &temp_reg_2);
			alloc_reg_resident(1, dst, &temp_reg_1);
			gen_mips(MIPS_ADDU, TEMPREG(temp_reg_1), TEMPREG(temp_reg_2), REG_0);
		}
		break;
	default:
		if ((isdigit(rec_1[0]) || rec_1[0] == '-') && (isdigit(rec_2[0]) || rec_2[0] == '-')) {
			alloc_reg_resident(1, dst, &temp_reg_1);
			if (op == ADD) {
				sprintf(num_1, "%d", atoi(rec_1) + atoi(rec_2));
			}
			else if (op == SUB) {
				sprintf(num_1, "%d", atoi(rec_1) - atoi(rec_2));
			}
			else if (op == MUL) {
				sprintf(num_1, "%d", atoi(rec_1) * atoi(rec_2));
			}
			else{
				sprintf(num_1, "%d", atoi(rec_1) / atoi(rec_2));
			}
			gen_mips(MIPS_ADDI, TEMPREG(temp_reg_1), REG_0, num_1);
		}
		else if (isdigit(rec_2[0]) || rec_2[0] == '-') {
			alloc_reg_resident(0, rec_1, &temp_reg_2);
			alloc_reg_resident(1, dst, &temp_reg_1);
			sprintf(num_1, "%d", atoi(rec_2));
			sprintf(num_2, "%d", -atoi(rec_2));
			if (op == ADD) {
				gen_mips(MIPS_ADDI, TEMPREG(temp_reg_1), TEMPREG(temp_reg_2), num_1);
			}
			else if (op == SUB) {
				gen_mips(MIPS_ADDI, TEMPREG(temp_reg_1), TEMPREG(temp_reg_2), num_2);
			}
			else if (op == MUL) {
				gen_mips(MIPS_ADDI, REG_t8, REG_0, num_1);
				gen_mips(MIPS_MUL, TEMPREG(temp_reg_2), REG_t8, NULL);
				gen_mips(MIPS_MFLO, TEMPREG(temp_reg_1), NULL, NULL);
			}
			else {
				gen_mips(MIPS_ADDI, REG_t9, REG_0, num_1);
				gen_mips(MIPS_DIV, TEMPREG(temp_reg_2), REG_t9, NULL);
				gen_mips(MIPS_MFLO, TEMPREG(temp_reg_1), NULL, NULL);
			}
		}
		else if (isdigit(rec_1[0]) || rec_1[0] == '-') {
			alloc_reg_resident(0, rec_2, &temp_reg_2);
			alloc_reg_resident(1, dst, &temp_reg_1);
			sprintf(num_1, "%d", atoi(rec_1));
			sprintf(num_2, "%d", -atoi(rec_1));
			if (op == ADD) {
				gen_mips(MIPS_ADDI, TEMPREG(temp_reg_1), TEMPREG(temp_reg_2), num_1);
			}
			else if (op == SUB) {
				gen_mips(MIPS_SUB, REG_t8, REG_0, TEMPREG(temp_reg_2));
				gen_mips(MIPS_ADDI, TEMPREG(temp_reg_1), REG_t8, num_1);
			}
			else if (op == MUL) {
				gen_mips(MIPS_ADDI, REG_t9, REG_0, num_1);
				gen_mips(MIPS_MUL, REG_t9, TEMPREG(temp_reg_2), NULL);
				gen_mips(MIPS_MFLO, TEMPREG(temp_reg_1), NULL, NULL);
			}
			else {
				gen_mips(MIPS_ADDI, REG_t8, REG_0, num_1);
				gen_mips(MIPS_DIV, REG_t8, TEMPREG(temp_reg_2), NULL);
				gen_mips(MIPS_MFLO, TEMPREG(temp_reg_1), NULL, NULL);
			}
		}
		else {
			alloc_reg_resident(0, rec_1, &temp_reg_2);
			alloc_reg_resident(0, rec_2, &temp_reg_3);
			alloc_reg_resident(1, dst, &temp_reg_1);
			switch (op) {
			case ADD:
				gen_mips(MIPS_ADDU, TEMPREG(temp_reg_1), TEMPREG(temp_reg_2), TEMPREG(temp_reg_3));
				break;
			case SUB:
				gen_mips(MIPS_SUB, TEMPREG(temp_reg_1), TEMPREG(temp_reg_2), TEMPREG(temp_reg_3));
				break;
			case MUL:
				gen_mips(MIPS_MUL, TEMPREG(temp_reg_2), TEMPREG(temp_reg_3), NULL);
				gen_mips(MIPS_MFLO, TEMPREG(temp_reg_1), NULL, NULL);
				break;
			case DIV:
				gen_mips(MIPS_DIV, TEMPREG(temp_reg_2), TEMPREG(temp_reg_3), NULL);
				gen_mips(MIPS_MFLO, TEMPREG(temp_reg_1), NULL, NULL);
				break;
			}
		}	
	}
	return;
}

/*
	�������:
	TODO��������
*/
void gen_scanf(char* dst, char* type) {
	int reg;
	alloc_reg_resident(1, dst, &reg);
	if (strcmp(type, "INT") == 0) {
		gen_mips(MIPS_ADDI, REG_v0, REG_0, "5");
	}
	else {
		gen_mips(MIPS_ADDI, REG_v0, REG_0, "12");
	}
	gen_mips(MIPS_CALL, NULL, NULL, NULL);
	gen_mips(MIPS_ADDU, TEMPREG(reg), REG_v0, REG_0);
	return;
}

/*
	������
*/
void gen_printf(char* rec, char* type) {
	int reg;
	if (strcmp(type, "STR") == 0) {								// ����ַ���
		rec[0] = 's';
		rec[1] = '$';
		gen_mips(MIPS_LA, REG_a0, rec, NULL);
		gen_mips(MIPS_ADDI, REG_v0, REG_0, "4");
		gen_mips(MIPS_CALL, NULL, NULL, NULL);
	}
	else if (strcmp(type, "INT") == 0) {						// �������
		gen_mips(MIPS_ADDI, REG_v0, REG_0, "1");
		if (isdigit(rec[0]) || rec[0] == '-') {
			gen_mips(MIPS_ADDI, REG_a0, REG_0, rec);
		}
		else {
			alloc_reg_resident(0, rec, &reg);
			gen_mips(MIPS_ADDU, REG_a0, TEMPREG(reg), REG_0);
		}
		gen_mips(MIPS_CALL, NULL, NULL, NULL);
	}
	else if (strcmp(type, "CHAR") == 0) {						// ����ַ�
		gen_mips(MIPS_ADDI, REG_v0, REG_0, "11");
		if (isdigit(rec[0]) || rec[0] == '-') {
			gen_mips(MIPS_ADDI, REG_a0, REG_0, rec);
		}
		else {
			alloc_reg_resident(0, rec, &reg);
			gen_mips(MIPS_ADDU, REG_a0, TEMPREG(reg), REG_0);
		}
		gen_mips(MIPS_CALL, NULL, NULL, NULL);
	}
	return;
}


/*
	���ɴ����:
		˳������м����
*/
void gen_text() {
	int push_counter, reg;
	char offset[20];
	gen_mips(MIPS_TEXT, NULL, NULL, NULL);
	gen_mips(MIPS_JAL, "main", NULL, NULL);
	gen_mips(MIPS_ADDI, REG_v0, REG_0, "10");
	gen_mips(MIPS_CALL, NULL, NULL, NULL);
	while (mid_code_pointer < length_mid_code) {
		switch (mid_code[mid_code_pointer]->op) {
		case GEI:
		case AEI:
		case ADD:
		case SUB:
		case MUL:
		case DIV:
		case ASN:
			gen_cul(mid_code[mid_code_pointer]->op,
				mid_code[mid_code_pointer]->a,
				mid_code[mid_code_pointer]->b,
				mid_code[mid_code_pointer]->c);
			break;
		case LES:
		case LEQ:
		case GTR:
		case GEQ:
		case EQL:
		case NEQ:
			gen_B_jump(mid_code[mid_code_pointer]->op,
				mid_code[mid_code_pointer]->a,
				mid_code[mid_code_pointer]->b,
				mid_code[mid_code_pointer + 1]->a);
			mid_code_pointer++;
			break;
		case GOTO:
			gen_save_env();
			gen_mips(MIPS_J, mid_code[mid_code_pointer]->a, NULL, NULL);
			break;
		case FUNC:
			gen_func_def(mid_code[mid_code_pointer]->b);
			break;
		case CALL:
			gen_save_env();
			gen_func_call(mid_code[mid_code_pointer]->a);
			break;
		case PUSH:
			push_counter = 0;
			do {
				sprintf(offset, "%d", -(push_counter) * 4);
				if (isdigit(mid_code[mid_code_pointer]->a[0]) || mid_code[mid_code_pointer]->a[0] == '-') {
					gen_mips(MIPS_ADDI, REG_t9, REG_0, mid_code[mid_code_pointer]->a);
					gen_mips(MIPS_SW, REG_t9, offset, REG_sp);
				}
				else {
					alloc_reg_resident(0, mid_code[mid_code_pointer]->a, &reg);
					gen_mips(MIPS_SW, TEMPREG(reg), offset, REG_sp);
				}
				push_counter++;
				mid_code_pointer++;
			} while (mid_code[mid_code_pointer]->op == PUSH);
			sprintf(offset, "%d", -push_counter * 4);
			gen_mips(MIPS_ADDI, REG_sp, REG_sp, offset);
			continue;
		case RET:
			gen_return(mid_code[mid_code_pointer]->a);
			break;
		case READ:
			gen_scanf(mid_code[mid_code_pointer]->a, mid_code[mid_code_pointer]->b);
			break;
		case PRINT:
			gen_printf(mid_code[mid_code_pointer]->a, mid_code[mid_code_pointer]->b);
			break;
		case LABEL:
			gen_save_env();
			gen_mips(MIPS_LABEL, mid_code[mid_code_pointer]->a, NULL, NULL);
			break;
		}
		mid_code_pointer++;
		//print_LRU();
	}
}

/*
	�������ɳ�����������
*/
void start_generator() {
	init_generator();
	gen_data();
	gen_text();
	reset_generator();
}