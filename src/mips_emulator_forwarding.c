/**************************************
*	File name:	mips_emulator_forwarding.c
*	Date:		2011.09.19
*	Author:		Daniel
*	Email:		daniel.lee.bit@gmail.com
***************************************/


#include "mips_emulator_forwarding.h"

void load();

int main(){
	int i;
	char str_inst[15];
	register_file.r[5] = 0;	
	load();	
	while(1){
		writeback();
		mem();
		execute();
		decode();
		fetch();
		
		printf("The %dth clock cycle ended!\n",cycle);
		printf("PC=%d\n",pc);		
		printf("%s,%s,%s,%s\n",IF_ID.p_inst, ID_EX.p_inst, EX_MEM.p_inst, MEM_WB.p_inst);	
		for(i=0; i<10; ++i)	printf("R[%d]=%d ",i,register_file.r[i]);
		//if (cycle==10) printf("Data_hazard:%d, Control_hazard:%d\n",data_hazard, ctr_hazard);
		
		printf("\n\n");
		cycle++;
		//if (cycle==20){break;
		//}
		if ( IF_ID.p_inst==NULL && \
			 ID_EX.p_inst==NULL && \
			 EX_MEM.p_inst==NULL && \
			 MEM_WB.p_inst==NULL ) break;
	}
	return 0;
}

// Load the instruction
void load(){
	char *p = (char *)malloc(15 * sizeof(char));
	while ( fgets(p,256,stdin)!=NULL ){
		strcpy( inst_mem[inst_num], p);
		inst_num++;
	}
	free(p);
	printf("Load %d instructions\n",inst_num);
}

void fetch(){	
	char *p;
	//if (cycle==6) printf("Data_hazard:%d, Control_hazard:%d\n",data_hazard, ctr_hazard);
	//if (cycle==10) printf("Branch:%d, Opcpde:%d,Inst:%d\n",EX_MEM.branch_taken_out,EX_MEM.opc_out,inst_num);	
	if (1==data_hazard)	return;
	if ( 0==data_hazard && 0==ctr_hazard){	// no hazard
		// Branch taken		
		if ( EX_MEM.branch_taken==1 && \ 	
			 (EX_MEM.opc==BNEZ || EX_MEM.opc==BLTZ)){
			pc = IF_ID.npc = EX_MEM.result;
			EX_MEM.branch_taken = 0;
			//if (cycle==30){
				//	printf("branch_taken!!\n");
				//	printf("Now the PC is %d:\n",pc);				
					//exit(0);
			//}
		}		
		if ( pc<inst_num ){
			IF_ID.p_inst = inst_mem[ pc ];
			//if(cycle==13) printf("Fetch :%s",IF_ID.p_inst);
			pc++;
			//if(cycle==30) printf("After fetch, the PC :%d\n",pc);
			IF_ID.npc = pc;
		}
		else
			IF_ID.p_inst = NULL;
	}
	else
		IF_ID.p_inst = NULL;		
}


void decode(){
	char *p;

	if (IF_ID.p_inst==NULL){
		ID_EX.p_inst = NULL;
		return;
	}
	if (1==load_issued){	// check data hazard for load instruction
		strcpy(ir,IF_ID.p_inst);
		p = strtok(ir," ");
		switch(atoi(p)){
			case DADDI:	p = strtok(NULL," ");
						if ( load_reg==atoi(strtok(NULL," ")) ){
							data_hazard = 1;
							ID_EX.p_inst = NULL;	
							return;						
						}
						break;
		
			case DADD:	
			case DSUB:	p = strtok(NULL," ");
						rs = atoi(strtok(NULL," "));
						rt = atoi(strtok(NULL," "));
						if ( load_reg==rs || load_reg==rt ){
							data_hazard = 1;
							ID_EX.p_inst = NULL;
							return;						
						}
						break;
			case SD:
			case BLTZ:	
			case BNEZ:	rs = atoi(strtok(NULL," "));
						if ( load_reg==rs ){
							data_hazard = 1;
							ID_EX.p_inst = NULL;	
							return;						
						}
						break;
			case LD:	break;
			default:	break;
		}
	}
	//Instruction can be issued
	strcpy(ir,IF_ID.p_inst);
	p = strtok(ir," ");
	ID_EX.opc = atoi(p);
	switch(ID_EX.opc){
		case DADDI:	rt = atoi(strtok(NULL," "));
					rs = atoi(strtok(NULL," "));
					if ( 1==reg_to_write[rs] ){
						if ( rs==load_reg ){
							ID_EX.operand_a = MEM_WB.mem_data;
							data_hazard = 0;	// end the hazard
						}
						else 
							ID_EX.operand_a = EX_MEM.result;
						reg_to_write[rs] = 0;					
					}
					else 
						ID_EX.operand_a = register_file.r[ rs ];
					ID_EX.immediate = atoi(strtok(NULL," "));
					reg_to_write[rt] = 1;
					//printf("DADDI Operand A:%ld Immediate %ld\n",ID_EX.operand_a, ID_EX.immediate_in);
					break;
		
		case DADD:	
		case DSUB:	printf("DSUB or DADD in decode stage\n");
					rd = atoi(strtok(NULL," "));
					rs = atoi(strtok(NULL," "));
					rt = atoi(strtok(NULL," "));
					//printf("Operand A :reg%d  Operand B :reg%d\n",rs, rt);
					//printf("Used reg %d= %d %d= %d\n",rs, reg_to_write[rs], rt, reg_to_write[rt]);			
					if ( 1==reg_to_write[rs] ){
						if ( rs==load_reg ){	// caused by load
							ID_EX.operand_a = MEM_WB.mem_data;
							data_hazard = 0;	// end the hazard
						}						
						else 
							ID_EX.operand_a = EX_MEM.result;
						//printf("The output of ALU %d\n",EX_MEM.result);
						reg_to_write[rs] = 0;
						ID_EX.operand_b = register_file.r[rt];					
					}
					else if ( 1==reg_to_write[rt] ){
						ID_EX.operand_a = register_file.r[rs];												
						if ( rt==load_reg ){	// caused by load
							ID_EX.operand_b = MEM_WB.mem_data;
							data_hazard = 0;	// end the hazard
						}						
						else 
							ID_EX.operand_b = EX_MEM.result;						
						reg_to_write[rt] = 0;					
					}
					reg_to_write[rd] = 1;
					printf("Operand A:%ld Oprand B:%ld\n",ID_EX.operand_a, ID_EX.operand_b);
					break;

		case BLTZ:	
		case BNEZ:	rs = atoi(strtok(NULL," "));
					if ( 1==reg_to_write[rs] ){
						if ( rt==load_reg ){	// caused by load
							ID_EX.operand_a = MEM_WB.mem_data;
							data_hazard = 0;	// end the hazard
						}
						else
							ID_EX.operand_a = EX_MEM.result;
						reg_to_write[rs] = 0;							
					}		
					else 
						ID_EX.operand_a = register_file.r[rs];
					ID_EX.immediate = atoi(strtok(NULL," "));
					ctr_hazard = 1;
					break;
		case LD:	rt = atoi(strtok(NULL," "));
					ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ]; //base
					ID_EX.immediate = atoi(strtok(NULL," "));					//offset
					reg_to_write[rt] = 1;
					load_issued = 1;
					load_reg = rt;		

					break;
		case SD:	ID_EX.operand_b = register_file.r[ atoi(strtok(NULL," ")) ];	//date to be store
					ID_EX.operand_a = register_file.r[ atoi(strtok(NULL," ")) ];	//base
					ID_EX.immediate = atoi(strtok(NULL," "));					//offset
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
		case DADDI:	EX_MEM.result = ID_EX.operand_a +ID_EX.immediate;
					break;
		
		// ALU instruction with 2 registers
		case DADD:	EX_MEM.result = ID_EX.operand_a +ID_EX.operand_b;
					break;
		
		case DSUB:	EX_MEM.result = ID_EX.operand_a - ID_EX.operand_b;
					break;

		// branch instruction
		// Reduce control stall 
		case BLTZ:	ctr_hazard = 0;			
					if (ID_EX.operand_a<0){
						EX_MEM.branch_taken=1;
						printf("Branch BLTZ taken!\n");
						EX_MEM.result = ID_EX.npc + ID_EX.immediate;
					}
					else
						EX_MEM.branch_taken=0;
					break;

		case BNEZ:	ctr_hazard = 0;
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
					//printf("Memory Address is %d\n", EX_MEM.result);
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
		case DADDI:	rt = atoi(strtok(NULL," "));			
					break;
		case DADD:	
		case DSUB:	rd = atoi(strtok(NULL," "));
					
					break;
		case BLTZ:
		case BNEZ:	break;
		case LD:	MEM_WB.mem_data = data_mem[EX_MEM.result];
					load_issued = 0;	// finish loading
													
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
					printf("Write back to register #%d\n",rt);
					register_file.r[rt] = MEM_WB.alu_data;
					//printf("RAW dependence on Register #%d\n now elimate\n",rt);
					reg_to_write[rt] = 0;
					break;
		case DADD:	
		case DSUB:	printf("DADD or DSUB write back!\n");
					rd = atoi(strtok(NULL," "));
					printf("Result is %d, Register is %d\n",MEM_WB.alu_data, rd);
					register_file.r[rd] = MEM_WB.alu_data;
					//printf("RAW dependence on Register #%d\n now elimate\n",rd);				
					reg_to_write[rd] = 0;
					break;
		case LD:	rt = atoi(strtok(NULL," "));
					register_file.r[rt] = MEM_WB.mem_data;
					//printf("RAW dependence on Register #%d\n now elimate\n",rt);
					reg_to_write[rt] = 0;
					load_reg = -1;			
					break;
		default: 	break;
	}
	
}
