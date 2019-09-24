#include"mips_generator_opt.h"
#include"mips_generator.h"
#include"blocks.h"
#include"dag.h"
#include"mid_code_opt.h"

void optimize() {
	int i;
	for (i = 0; i < 10; i++) {
		rewrite();							// ������д
		delete_dead();
	}
	print_mid_code_to("mide_code_1.txt");
	build_blocks();						// �ֿ�
	live_variable_analysis();			// ��Ծ��������
	print_block();						// ��ӡ����Ϣ
	remove_common_exp();				// DAGͼ�����ֲ������ӱ��ʽ
	for (i = 0; i < 10; i++) {
		rewrite();						// ������д
		delete_dead();
	}
	print_mid_code_to("mide_code_2.txt");
	build_blocks();						// �ֿ�
	live_variable_analysis();			// ��Ծ��������
	print_mid_code_to("mide_code_3.txt");
	print_block();						// ��ӡ����Ϣ
	start_generator();
	start_generator_opt();					// Ŀ���������
	return;
}