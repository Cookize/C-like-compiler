#include"mips_generator_opt.h"
#include"mips_generator.h"
#include"blocks.h"
#include"dag.h"
#include"mid_code_opt.h"

void optimize() {
	int i;
	for (i = 0; i < 10; i++) {
		rewrite();							// 初步复写
		delete_dead();
	}
	print_mid_code_to("mide_code_1.txt");
	build_blocks();						// 分块
	live_variable_analysis();			// 活跃变量分析
	print_block();						// 打印块信息
	remove_common_exp();				// DAG图消除局部公共子表达式
	for (i = 0; i < 10; i++) {
		rewrite();						// 初步复写
		delete_dead();
	}
	print_mid_code_to("mide_code_2.txt");
	build_blocks();						// 分块
	live_variable_analysis();			// 活跃变量分析
	print_mid_code_to("mide_code_3.txt");
	print_block();						// 打印块信息
	start_generator();
	start_generator_opt();					// 目标代码生成
	return;
}