#ifndef __MIPS_GEN_
#define __MIPS_GEN_
#define _CRT_SECURE_NO_WARNINGS
// MIPS instructuins
#define MIPS_ADD	"add"
#define MIPS_ADDI	"addi"
#define MIPS_ADDU	"addu"
#define MIPS_SUB	"sub"
#define MIPS_SUBU	"subu"
#define MIPS_MUL	"mul"
#define MIPS_DIV	"div"
#define MIPS_MFLO	"mflo"
#define MIPS_SLL	"sll"
#define MIPS_SRL	"srl"

#define MIPS_BEQ	"beq"
#define MIPS_BGT	"bgt"
#define MIPS_BGE	"bge"
#define MIPS_BLE	"ble"
#define MIPS_BLT	"blt"
#define MIPS_BNE	"bne"
#define MIPS_BGEZ	"bgez"
#define MIPS_BGTZ	"bgtz"
#define MIPS_BLEZ	"blez"
#define MIPS_BLTZ	"bltz"
#define MIPS_SLT	"slt"
#define MIPS_SLTI	"slti"

#define MIPS_SW		"sw"
#define MIPS_LW		"lw"
#define MIPS_LA		"la"

#define MIPS_J		"j"
#define MIPS_JR		"jr"
#define MIPS_JAL	"jal"
#define MIPS_JALR	"jalr"

#define MIPS_CALL	"syscall"
#define MIPS_DATA	".data"
#define MIPS_ASCII	".asciiz"
#define MIPS_TEXT	".text"
#define MIPS_LABEL	""

// MIPS registers
#define REG_0		"$0"
#define REG_v0		"$v0"
#define REG_v1		"$v1"
#define REG_a0		"$a0"			// argument
#define REG_a1		"$a1"
#define REG_a2		"$a2"
#define REG_a3		"$a3"
#define REG_t0		"$t0"			// temporary
#define REG_t1		"$t1"
#define REG_t2		"$t2"
#define REG_t3		"$t3"
#define REG_t4		"$t4"
#define REG_t5		"$t5"
#define REG_t6		"$t6"
#define REG_t7		"$t7"
#define REG_t8		"$t8"
#define REG_t9		"$t9"
#define REG_s0		"$s0"			// saved temporary
#define REG_s1		"$s1"
#define REG_s2		"$s2"
#define REG_s3		"$s3"
#define REG_s4		"$s4"
#define REG_s5		"$s5"
#define REG_s6		"$s6"
#define REG_s7		"$s7"
#define REG_k0		"$k0"			
#define REG_k1		"$k1"
#define REG_gp		"$gp"			// global area
#define REG_sp		"$sp"			// stack pointer
#define REG_fp		"$fp"			// frame pointer
#define REG_ra		"$ra"			// return address

/*
	LRU调度项
*/
typedef struct LRU_ITEM {
	int reg;						// 分配的临时寄存器（索引值）
	char *name;	 					// 对应的变量名
	int dirty;						// 脏位
} *lru_item, lru_item_obj;

void start_generator();
#endif __MIPS_GEN_
