#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

//Pending global variables

#define _GNU_SOURCE

//Memory is in Integer format and Opcode is in String format

//Total CPU registers=8
#define REG_NUM 8
int Register_array[REG_NUM];
//DEfining memory size
#define MEM_SIZE 1024
int memory_array[MEM_SIZE];
//program counter
int pc;
//stack pointer
int sp;
//Instruction counter
int noOfInstructions = 0;

//defining physical memory
#define OS_BOOT_Base 0
#define INS_START_ADDR 128
#define DAT_START_ADDR 384
#define Stack_Base 896
#define Stack_top 1024

//defining opcode
#define LD  0x40
#define ST  0x41

//To clear the structure
static const struct instructionStructure Empty;

//Read Write structure
struct instructionStructure {
	int source1;
	int source2;
	int destination;
	int value;
	const char * opcode; // RD = Read & WR = Write
}; 

//Function to convert string into an Instruction Struct
struct instructionStructure parseInstruction(char* strInst);

//Function to replace special chars in a string
char * replaceChar(char * str, char specialChar, char newChar);

//Function to split Instruction string and return string array
char ** splitInstruction(char * instruction);

//READ opration from Source1 address
int readValue(int source);

//WRITE opration at Destination address
void writeValue(int destination, int source);

//OPERATION function
void operate(struct instructionStructure inst);

//Print all values after every instruction execution
int print_values();

//Function to take input from a file with instruction to execute
char **takeInput(char *inputfilePath);

//Function to check is String contains all numbers
int checkAllDigits(const char *str);

//Function to remove starting chars from a string
char * removeStartingChars(char * str,int i);

int main(){


	Register_array[1] = 10;
	//initialisation
	pc = INS_START_ADDR;
	sp = Stack_top;
	struct instructionStructure inst;

	printf("Please provide the filepath for instructions \n");
	char filePath[100];
	scanf("%s",filePath);

	char **InstructionBucket = malloc(100 * sizeof(char *));
	//temp path ----------------"/home/paragas12/test/ins.txt"

	InstructionBucket = takeInput(filePath);

	//Find no. of instructions
	for(int j = 0 ; j< noOfInstructions-1 && InstructionBucket[j] != NULL && InstructionBucket[j] != ""; j++){
		//printf("%s",InstructionBucket[j]);
		inst = parseInstruction(InstructionBucket[j]);

		print_values();
		//Operate on given input 
		operate(inst);
		pc = pc+4;
		print_values();
	}
	


	return 0;
}

//Function to convert string into an Instruction Struct
struct instructionStructure parseInstruction(char* strInst){
	struct instructionStructure inst;

  	char* position;
	strInst = replaceChar(strInst,',',' ');
	
	char **strArray = splitInstruction(strInst);
	if(sizeof(strArray) < 3)
		return inst;
	inst.opcode = strArray[0];

	char* parameter1 = removeStartingChars(strArray[1],1);
	char* parameter2 = removeStartingChars(strArray[2],1);
	

//SW R1,$5
//LD R2,$5
	if(strstr(inst.opcode, "SW") != NULL){

		inst.source1 = (int)strtol(parameter1,&position,sizeof(parameter1));
		inst.destination = (int)strtol(parameter2,&position,sizeof(parameter2));

	} else if (strstr(inst.opcode, "LD") != NULL){

		inst.source1 = (int)strtol(parameter2,&position,sizeof(parameter2));
		inst.destination = (int)strtol(parameter1,&position,sizeof(parameter1));
	}

	return inst;
}



//Function to remove starting chars from a string
char * removeStartingChars(char* str,int i){
	char *newStr = malloc(sizeof(str) * sizeof(char));
	for(int j = 0;j<sizeof(str)-1;j++)
		newStr[j] = str[j+1];
	newStr[sizeof(str)-1] = '\0';

	return newStr;
}


//Function to check is String contains all numbers
int checkAllDigits(const char *str){
	while (*str) {
		if (isdigit(*str++) == 0)
			return 0;
	}
	return 1;
}

//Function to split Instruction string and return string array
char ** splitInstruction(char * instruction){
	char *position = strtok (instruction, " ");
	char **array = malloc(10 * sizeof(char *));
	int i = 0;
	while (position != NULL)
 	{
		array[i++] = position;
		position = strtok (NULL, " ");
	}
	return array;
}

//Function to replace special chars in a string
char * replaceChar(char * str, char specialChar, char newChar){
	for (char* p = str; p = strchr(p,specialChar); ++p) {
		*p = newChar;
	}
	return str;
}

//Function to take input from a file with instruction to execute
char **takeInput(char *inputfilePath){
	//printf("%s",inputfilePath);
	FILE * fp;
	char line[50];
	char **instructionContainer = malloc(100 * sizeof(char *));
	int i=0;

	fp = fopen(inputfilePath, "r");

	if (fp) {
		while(fgets(line,sizeof(line),fp)!= NULL && line !=""){
			instructionContainer[i] = malloc (sizeof(line));
			strcpy(instructionContainer[i], line);
			i++;noOfInstructions++;
		}
	} else
		printf("File not found !! \n");

	fclose(fp);
	return instructionContainer;
}


//OPERATION function
void operate(struct instructionStructure inst) {
	if (strstr(inst.opcode, "LD") != NULL) {
		printf("Reading\n" );
		Register_array[inst.destination] = readValue(DAT_START_ADDR + inst.source1);
	} 
	else if (strstr(inst.opcode, "SW") != NULL) {
		printf("Writing\n" );
		writeValue(inst.destination + DAT_START_ADDR,Register_array[inst.source1]);
	}
	else {
		printf("Invalid opcode\n" );
	}
}

//READ opration from Source1 address
int readValue(int source) {
	return memory_array[source];
}

//WRITE opration at Destination address
void writeValue(int destination, int source) {
	memory_array[destination] = source;
}

//Print all values after every execution
int print_values()
{
 
	printf("\n########################################################\n");
	printf("\t\tRegisters values\n");
	printf("########################################################\n");
	printf("R0 = %u\t\tR1 = %u\t\tR2 = %u\t\tR3 = %u\n",Register_array[0],Register_array[1],Register_array[2],Register_array[3]);
	printf("R4 = %u\t\tR5 = %u\t\tR6 = %u\t\tR7 = %u\n",Register_array[4],Register_array[5],Register_array[6],Register_array[7]);
 
	printf("########################################################\n");
	printf("\tProgram Counter\n");
	printf("########################################################\n");
	printf("\tPC = %u\n", pc);
	printf("########################################################\n");
	printf("\tOperating System allocated Memory\n");
	printf("########################################################\n");
	printf("Address    \t\tMemory\n");      
         
	for (int i = 0; i <INS_START_ADDR; i = i + 16)
	{
		printf("%06d   %02X %02X %02X %02X  ", i, memory_array[i], memory_array[i+1], memory_array[i+2], memory_array[i+3]);
		printf("%02X %02X %02X %02X  ", memory_array[i+4], memory_array[i+5], memory_array[i+6], memory_array[i+7]);
		printf("%02X %02X %02X %02X  ", memory_array[i+8], memory_array[i+9], memory_array[i+10], memory_array[i+11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i+12], memory_array[i+13], memory_array[i+14],memory_array[i+15]);
	}
 /*
	printf("\n########################################################\n");
	printf("\tInstruction Memory\n");
	printf("\n########################################################\n");
	printf("Address    \t\tMemory\n");
	for (int i=INS_START_ADDR; i <DAT_START_ADDR; i = i+16)
	{
		printf("%06d   %02X %02X %02X %02X  ", i, memory_array[i], memory_array[i+1], memory_array[i+2], memory_array[i+3]);
		printf("%02X %02X %02X %02X  ", memory_array[i+4], memory_array[i+5], memory_array[i+6], memory_array[i+7]);
		printf("%02X %02X %02X %02X  ", memory_array[i+8], memory_array[i+9], memory_array[i+10], memory_array[i+11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i+12], memory_array[i+13], memory_array[i+14], memory_array[i+15]);
	}
 */
	printf("########################################################\n");
	printf("\tData Memory\n");
	printf("########################################################\n");
	printf("Address    \t\tMemory\n");
	for (int i =DAT_START_ADDR; i < Stack_Base; i = i+16)
	{
		printf("%06d   %02X %02X %02X %02X  ", i, memory_array[i], memory_array[i+1], memory_array[i+2], memory_array[i+3]);
		printf("%02X %02X %02X %02X  ", memory_array[i+4], memory_array[i+5], memory_array[i+6], memory_array[i+7]);
		printf("%02X %02X %02X %02X  ", memory_array[i+8], memory_array[i+9], memory_array[i+10], memory_array[i+11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i+12], memory_array[i+13], memory_array[i+14], memory_array[i+15]);
	}
 
	printf("########################################################\n");
	printf("\tStack Memory\n");
	printf("########################################################\n");
	printf("Address    \t\tMemory\n");
	for (int i = Stack_Base; i < 1024; i = i+16)
	{
		printf("%06d   %02X %02X %02X %02X  ", i,  memory_array[i], memory_array[i+1], memory_array[i+2], memory_array[i+3]);
		printf("%02X %02X %02X %02X  ", memory_array[i+4], memory_array[i+5], memory_array[i+6], memory_array[i+7]);
		printf("%02X %02X %02X %02X  ", memory_array[i+8], memory_array[i+9], memory_array[i+10], memory_array[i+11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i+12], memory_array[i+13], memory_array[i+14], memory_array[i+15]);
	}
	printf("\n");   
}


