#include <stdio.h>
#include <string.h>

#define MAX_FILE_NAME 256

void assembly(char *str);
void build_branch_table(char * file_name);

struct __branch_table__{
	char label[15];
	int line;
}branch_table[10];

int line_code = 0;
int target_num = 0;

int main(int argc, char **argv){
	char str[MAX_FILE_NAME];
	char file_name[MAX_FILE_NAME];
	char out_file[MAX_FILE_NAME];
	char *p;
	int i;
	FILE * pf_i, *pf_o;
	if ( 2 != argc	){
		printf("Usage:\t command [filename].\n");
		return 0;
	}
	strcpy(file_name,argv[1]);
	i = strlen(file_name);
	file_name[i-3] = 'b';
	file_name[i-2] = 'i';
	file_name[i-1] = 'n';
	freopen(file_name,"w",stdout);
	build_branch_table(argv[1]);
	
	pf_i = fopen(argv[1],"r");
	pf_o = fopen(file_name,"r");

	while ( fgets(str, 256, pf_i) != NULL ){
		assembly(str);
		line_code++;
	}
	fclose(pf_i);
	fclose(pf_o);
}

// This is a simple branch table just contain the target
void build_branch_table(char * file_name){
	int count = 0, line = 0;
	char *p, str[256];
	FILE *pf_i = fopen(file_name, "r");
	while ( fgets(str, 256, pf_i) != NULL){
		p = str;
		while ( *p!='\n' && (*p)!=':' )	++p;	
		//find a target
		if ( '\n' != *p){
			p = strtok(str," \t:");
			if ( p[strlen(p)]!='\0' ) 
				printf("!!!!!!!!!!!!!!!!!\n");
			strcpy(branch_table[count].label, p);
			branch_table[count].line = line;
			count++;
		}
		line++;
	}
	target_num = count;
	fclose(pf_i);
}

void assembly(char *str){
	char *p;
	char opc;
	int rs, rt, rd, immediate;
	int base, offset;
	//test whether is target
	p = str;
	while ( *p!='\n' && (*p)!=':' )	++p;	
	if ( '\n' == *p)
		p = strtok(str,"\t :");
	else{
		p = strtok(str,"\t :");
		p = strtok(NULL,"\t :");
	}

	if ( 0 == strcmp(p,"DADDI") ){
		opc = 0b011000;
		p = strtok(NULL," ");
		rt = atoi(p+1);
		p = strtok(NULL," ");
		rs = atoi(p+1);
		p = strtok(NULL," ");
		immediate = atoi(p+1);
		printf("%d %d %d %d\n", opc, rt, rs, immediate);
	}
	else if ( 0 == strcmp(p,"DADD") ){
		opc = 0b101100;
		p = strtok(NULL," ");
		rd = atoi(p+1);
		p = strtok(NULL," ");
		rs = atoi(p+1);
		p = strtok(NULL," ");
		rt = atoi(p+1);
		printf("%d %d %d %d\n", opc, rd, rt, rs);
	}
	else if ( 0 == strcmp(p,"DSUB") ){
		opc = 0b101110;
		p = strtok(NULL," ");
		rd = atoi(p+1);
		p = strtok(NULL," ");
		rs = atoi(p+1);
		p = strtok(NULL," ");
		rt = atoi(p+1);
		printf("%d %d %d %d\n", opc, rd, rs, rt);
	}	
	else if ( 0 == strcmp(p,"LD") ){
		opc = 0b110111;
		p = strtok(NULL," ");
		rt = atoi(p+1);
		p = strtok(NULL," ()");
		offset = atoi(p+1);
		p = strtok(NULL," ()");
		base = atoi(p+1);
		printf("%d %d %d %d\n", opc, rt, base, offset);
	}	
	else if ( 0 == strcmp(p,"SD") ){
		opc = 0b111111;
		p = strtok(NULL," ");
		rt = atoi(p+1);
		p = strtok(NULL," ()");
		offset = atoi(p+1);
		p = strtok(NULL," ()");
		base = atoi(p+1);
		printf("%d %d %d %d\n", opc, rt, base, offset);
	}
	else if ( 0 == strcmp(p,"BLTZ")){
		opc = 0b000001;
		p = strtok(NULL," ");
		rs = atoi(p+1);
		p = strtok(NULL," \n");

		offset = target_to_dist(p) - (line_code+1);
		printf("%d %d %d\n", opc, rs, offset);
	}
	else if ( 0 == strcmp(p,"BNEZ")){
		opc = 0b000011;
		p = strtok(NULL," ");
		rs = atoi(p+1);
		p = strtok(NULL," \n");
		offset = target_to_dist(p) - (line_code+1);
		printf("%d %d %d\n", opc, rs, offset);
	}

}

target_to_dist(char * str){
	int i;
	for(i=0; i<target_num; ++i){
		if ( strcmp(str,branch_table[i].label)==0 )
			return branch_table[i].line;
	}
}
