/**************************************
*	File name:	mips_emulator.c
*	Date:		2011.09.19
*	Author:		Daniel
*	Email:		daniel.lee.bit@gmail.com
***************************************/

#include "mips_emulator.h"

void load();
void update_pipeline_reg();

int main(){
	int i;
	char str_inst[15];
	register_file.r[5] = 1;	
	load();	
	while(1){
		writeback();
		mem();
		execute();
		decode();
		fetch();
		
		printf("The %dth clock cycle ended!\n",cycle);
		printf("PC=%ld\n",pc);		
		printf("%s,%s,%s,%s\n",IF_ID.p_inst, ID_EX.p_inst, EX_MEM.p_inst, MEM_WB.p_inst);	
		for(i=0; i<10; ++i)	printf("R[%d]=%ld ",i,register_file.r[i]);
		
		printf("\n\n");
		cycle++;
		//if (cycle==29){break;
		//}
		if ( IF_ID.p_inst==NULL && \
			 ID_EX.p_inst==NULL && \
			 EX_MEM.p_inst==NULL && \
			 MEM_WB.p_inst==NULL ) break;
	}
	return 0;
}

// load the instruction
void load(){
	char *p = (char *)malloc(15 * sizeof(char));
	while ( fgets(p,256,stdin)!=NULL ){
		strcpy( inst_mem[inst_num], p);
		inst_num++;
	}
	free(p);
	printf("Load %d instrucions\n",inst_num);
}

void fetch(){	
	char *p;
	//printf("PC=%d, Inst_num=%d\n",pc, inst_num);
	//printf("PC=%d\n",pc);
	//if (cycle==10) printf("Data_hazard:%d, Control_hazard:%d\n",data_hazard, ctr_hazard);
	//if (cycle==10) printf("Branch:%d, Opcpde:%d,Inst:%d\n",EX_MEM.branch_taken_out,EX_MEM.opc_out,inst_num);	
	if ( 0==data_hazard && 0==ctr_hazard){	// no hazard
		// Branch taken		
		if ( EX_MEM.branch_taken==1 && \ 	
			 (EX_MEM.opc==BNEZ || EX_MEM.opc==BLTZ)){
			pc = IF_ID.npc = EX_MEM.result;
			EX_MEM.branch_taken = 0;
		}		
		if ( pc<inst_num ){;
			IF_ID.p_inst = inst_mem[ pc ];
			//if(cycle==13) printf("Fetch :%s",IF_ID.p_inst);
			pc++;
			//if(cycle==30) printf("After fetch, the PC :%d\n",pc);
			IF_ID.npc = pc;
		}
		else
			IF_ID.p_inst = NULL;
	}
	else if (1==ctr_hazard)	IF_ID.p_inst = NULL;
}


void decode(){
	char *p;
	if (IF_ID.p_inst==NULL){
		ID_EX.p_inst = NULL;
		return;
	}
	//check hazard
	strcpy(ir,IF_ID.p_inst);
	p = strtok(ir," ");
	ID_EX.opc = atoi(p);
	switch(ID_EX.opc){
		case DADDI:	rt = atoi(strtok(NULL," "));
					rs = atoi(strtok(NULL," "));
					if ( 1==reg_to_write[rs] ){
						data_hazard = 1;	//data hazard!
						ID_EX.npc = -1;
						ID_EX.p_inst = NULL;
						return;												
					}
					break;
		case DADD:	
		case DSUB:	rd = atoi(strtok(NULL," "));
					rs = atoi(strtok(NULL," "));
					rt = atoi(strtok(NULL," "));
					if ( 1==reg_to_write[rs] || 1==reg_to_write[rt] ){						
						data_hazard = 1;	//data hazard!
						ID_EX.npc = -1;
						ID_EX.p_inst = NULL;												
						return;					
					}
					break;
					
		case BLTZ:	
		case BNEZ:	rs = atoi(strtok(NULL," "));
					 	printf("rs = %d\n",rs);
					if ( 1==reg_to_write[rs] ){
						data_hazard = 1;	//data hazard!
						ID_EX.npc = -1;
						ID_EX.p_inst = NULL;
						return;												
					}					
					break;
					
		case LD:	rt = atoi(strtok(NULL," "));
					rs = atoi(strtok(NULL," "));
					if ( 1==reg_to_write[rs] ){						
						data_hazard = 1;	//data hazard!
						ID_EX.npc = -1;
						ID_EX.p_inst = NULL;
						return;												
					}
					break;
		case SD:	rt = atoi(strtok(NULL," "));	//date to be store
					rs = atoi(strtok(NULL," "));	//base
					if ( 1==reg_to_write[rs] || 1==reg_to_write[rt] ){
						data_hazard = 1;	//data hazard!
						ID_EX.npc = -1;
						ID_EX.p_inst = NULL;
						return;													
					}
					break;
	}
	//Instruction can be issued
	strcpy(ir,IF_ID.p_inst);
	p = strtok(ir," ");
	ID_EX.opc = atoi(p);
	switch(ID_EX.opc){
		case DADDI:	p = strtok(NULL," ");
					ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ];
					ID_EX.immediate = atoi(strtok(NULL," "));
					reg_to_write[rt] = 1;
					//printf("DADDI Operand A:%ld Immediate %ld\n",ID_EX.operand_a, ID_EX.immediate_in);
					break;
		
		case DADD:	p = strtok(NULL," ");
					ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ];
					ID_EX.operand_b = register_file.r[ atoi(strtok(NULL," ")) ];
					reg_to_write[rd] = 1;
					//printf("Operand A:%ld Oprand B:%ld\n",ID_EX.operand_a, ID_EX.operand_b_in);
					break;
		
		case DSUB:	p = strtok(NULL," ");
					ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ];
					ID_EX.operand_b = register_file.r[ atoi(strtok(NULL," ")) ];
					reg_to_write[rd] = 1;
					//printf("Operand A:%ld Oprand B:%ld\n",ID_EX.operand_a, ID_EX.operand_b_in);
					break;
		case BLTZ:	ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ];
					printf("BLTZ issued. Operand a in %d.\n",ID_EX.operand_a);
					ID_EX.immediate = atoi(strtok(NULL," "));
					ctr_hazard = 1;
					//printf("Operand A:%ld Immediate :%ld\n",ID_EX.operand_a, ID_EX.immediate_in);
					break;
		case BNEZ:	
					ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ];
					//printf("BNEZ issued. Operand a in %d.\n",ID_EX.operand_a);
					ID_EX.immediate = atoi(strtok(NULL," "));
					ctr_hazard = 1;
					break;
		case LD:	p = strtok(NULL," ");
					ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ]; //base
					ID_EX.immediate = atoi(strtok(NULL," "));					//offset
					reg_to_write[rt] = 1;
					//printf("Baseaddress:%ld Offset :%ld\n",ID_EX.operand_a, ID_EX.immediate_in);
					break;
		case SD:	ID_EX.operand_b = register_file.r[ atoi(strtok(NULL," ")) ];	//date to be store
					ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ];	//base
					ID_EX.immediate = atoi(strtok(NULL," "));					//offset
					//printf("Baseaddress :%ld Offset :%ld\n",ID_EX.operand_a, ID_EX.immediate_in);
					break;
	}
	ID_EX.npc = IF_ID.npc;
	ID_EX.p_inst = IF_ID.p_inst;
}

void execute(){
	int opc,i;
	char *p;	
	EX_MEM.opc = ID_EX.opc;
	EX_MEM.npc = ID_EX.npc;
	EX_MEM.p_inst = ID_EX.p_inst;	
	if ( NULL==ID_EX.p_inst )	return;
	
	switch(ID_EX.opc){
		// ALU instruction with immediate operation		
		case DADDI:	printf("DADDI executed!\n");
					EX_MEM.result = ID_EX.operand_a +ID_EX.immediate;
					break;
		
		// ALU instruction with 2 registers
		case DADD:	EX_MEM.result = ID_EX.operand_a +ID_EX.operand_b;
					//printf("Operand A:%ld Oprand B:%ld\n",ID_EX.operand_a_out, ID_EX.operand_b_out);
					break;
		
		case DSUB:	EX_MEM.result = ID_EX.operand_a - ID_EX.operand_b;
					//printf("Operand A:%ld Oprand B:%ld\n",ID_EX.operand_a_out, ID_EX.operand_b_out);
					break;

		// branch instruction
		// Reduce control stall 
		case BLTZ:	//printf("In execute stage, Branch BLTZ : %d\n",ID_EX.operand_a_out);
					ctr_hazard = 0;			
					if (ID_EX.operand_a<0){
						EX_MEM.branch_taken=1;
						printf("Branch BLTZ taken!\n");
						EX_MEM.result = ID_EX.npc + ID_EX.immediate;
					}
					else
						EX_MEM.branch_taken=0;
					break;

		case BNEZ:	ctr_hazard = 0;
					//printf("In execute stage, Branch BNEZ : %d\n",ID_EX.operand_a_out);
					if (ID_EX.operand_a!=0){
						EX_MEM.branch_taken=1;
						EX_MEM.result = ID_EX.npc + ID_EX.immediate;
						//printf("NPC is %d, offset is %d\n",ID_EX.npc,ID_EX.immediate);
						//printf("Branch BNEZ taken! jump to %d\n",EX_MEM.result);
					}
					else
						EX_MEM.branch_taken=0;
					break;
		// Load or store instruction
		case LD:	
		case SD:	EX_MEM.result = ID_EX.operand_a +ID_EX.immediate;
					break;
	}
}


void mem(){
	char *p;
	
	MEM_WB.opc = EX_MEM.opc;
	MEM_WB.alu_data = EX_MEM.result;
	MEM_WB.p_inst = EX_MEM.p_inst;
	if ( NULL==EX_MEM.p_inst )	return;
	
	strcpy(ir,EX_MEM.p_inst);
	p = strtok(ir," ");	// skip the opcode

	switch(EX_MEM.opc){
		case DADDI:						
					break;
		case DADD:	
		case DSUB:	
					break;
		case BLTZ:
		case BNEZ:					
					break;
		case LD:	MEM_WB.mem_data = data_mem[EX_MEM.result];					
					break;
		case SD:	data_mem[EX_MEM.result] = EX_MEM.operand_b;
					break;
		default: 	break;	
	}
}

void writeback(){
	char *p;
	
	if ( NULL==MEM_WB.p_inst )	return;	
	
	strcpy(ir,MEM_WB.p_inst);
	p = strtok(ir," ");	// skip the opcode
		
	switch(MEM_WB.opc){
		case DADDI:	rt = atoi(strtok(NULL," "));
					register_file.r[rt] = MEM_WB.alu_data;
					reg_to_write[rt] = 0;
					data_hazard = 0;
					break;
		case DADD:	
		case DSUB:	rd = atoi(strtok(NULL," "));
					register_file.r[rd] = MEM_WB.alu_data;
					data_hazard = 0;					
					reg_to_write[rd] = 0;
					break;
		case LD:	rt = atoi(strtok(NULL," "));
					register_file.r[rt] = MEM_WB.mem_data;
					reg_to_write[rt] = 0;					
					data_hazard = 0;
					break;
		default: 	break;
	}
	
}
