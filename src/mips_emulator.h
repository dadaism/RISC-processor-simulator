#ifndef _mips_emulator_h_
#define _mips_emulator_h_

#include <stdio.h>

#define DADD	0b101100
#define DADDI	0b011000
#define	DSUB	0b101110
#define LD		0b110111
#define	SD		0b111111
#define BLTZ	0b000001
#define BNEZ	0b000011


#define MAX_NUM_INST 50
#define INST_LEN 15

int cycle = 0;

int reg_data_hazard_a = -1;
int reg_data_hazard_b = -1;

char inst_mem[MAX_NUM_INST][INST_LEN];
static long data_mem[100]={110,1,2,12,241,5,6,7,81,9};
int reg_to_write[10];
int rd, rs, rt;
int stall = 0;

long pc = 0;
int inst_num = 0;
char ir[10];
int data_hazard = 0;
int ctr_hazard = 0;

struct __regist_file__{
	long r[10];
}register_file;

struct __IF_ID__{
	long npc;
	char *p_inst;
}IF_ID;

struct __ID_EX__{
	char opc;
	long npc;
	long operand_a;
	long operand_b;
	long immediate;
	char *p_inst;
}ID_EX;

struct __EX_MEM__{
	char opc;
	long npc;
	long result;
	long operand_b;
	int branch_taken;
	char *p_inst;
}EX_MEM;

struct __MEM_WB__{
	char opc;
	long mem_data;
	long alu_data;
	char *p_inst;
}MEM_WB;


void fetch();
void decode();
void execute();
void mem();
void writeback();

#endif
