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

int CF,OF = 0;  //carry flag and overflag init
int SF,ZF =0;
//To clear the structure
static const struct instructionStructure Empty;

//Read Write structure
struct instructionStructure {
	int source1;
	int source2;
	int destination;
	int immediate;
	int value;
	const char * opcode; // LW = Read & SW = Write
};

//Function to check existence of a char
int checkExistence(char * str, char* a);

//Function to get value between 2 chars
char * getValueBetween(const char *const string, const char *const start, const char *const end);

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

//Function to store instructions to Instruction memory
int storeInstruction(char* strInst, int location);

//FUnction to get no of instructions
int getNoOfInstruction(char *inputfilePath);

/* ALU functions */
//modulus operation
int mod(int p, int q);
//division
int division(int p, int q);
//multiplication
int mul(int p,int q);
//subtraction
int sub(int p, int q);
//addition
int add(int p, int q);

//ALU function
int aluOperations(int input1,int input2,const char* operation);

int main(){

		//4294967295 4294967266
		Register_array[0] = -55;
		Register_array[1] = -20;
		Register_array[2] = 75;
		Register_array[3] = 2147483647;


	//initialisation
	pc = INS_START_ADDR;
	sp = Stack_top;
	struct instructionStructure inst;

	printf("Please provide the filepath for instructions \n");
	char filePath[100];
	scanf("%s",filePath);


	char **InstructionBucket = malloc(100 * sizeof(char *));
        char **localInstructions = malloc(100 * sizeof(char *));
	//temp path ----------------"/home/paragas12/test/ins.txt"

	InstructionBucket = takeInput(filePath);
	localInstructions = takeInput(filePath);
	noOfInstructions = getNoOfInstruction(filePath);


	//Store instructions into memory before execution
	for(int j = 0 ; j< noOfInstructions ; j++){

		if(storeInstruction(InstructionBucket[j],j+INS_START_ADDR) != 0){
			printf("Problem with instruction: %s",InstructionBucket[j]);
			exit(-1);
		}
	}

	print_values();

	//Find no. of instructions int *ptr = begin; *ptr; ptr++
	for(int i=0;i< noOfInstructions;i++){
		printf("%s\n",localInstructions[i]);
		inst = parseInstruction(localInstructions[i]);

		//Operate on given input
		operate(inst);
		pc = pc+1;
		print_values();
	}
	return 0;
}

//Function to check existence of a char
int checkExistence(char * str, char* a){
	char *checkingString = str;
   	while (*checkingString){
	      	 if (strchr(a, *checkingString))
	       	{
		  return 1;
	       	}
       		checkingString++;
   	}
   	return 0;
}


//Function to get value between 2 chars
char *getValueBetween(const char *const string, const char *const start, const char *const end)
{
    char  *a;
    char  *b;
    size_t length;
    char  *result;

    if ((string == NULL) || (start == NULL) || (end == NULL))
        return NULL;
    length = strlen(start);
    a   = strstr(string, start);
    if (a == NULL)
        return NULL;
    a += length;
    b  = strstr(a, end);
    if (b == NULL)
        return b;
    length = b - a;
    result = malloc(1 + length);
    if (result == NULL)
        return NULL;
    result[length] = '\0';

    memcpy(result, a, length);
    return result;
}


//Function to convert string into an Instruction Struct
struct instructionStructure parseInstruction(char* strInst){
	struct instructionStructure inst;
	inst.immediate =0;
  	char* position;
	strInst = replaceChar(strInst,',',' ');

	char **strArray = splitInstruction(strInst);
	if(sizeof(strArray) < 3)
		return inst;
	inst.opcode = strArray[0];


if(strstr(strArray[0], "SW") == NULL && strstr(strArray[0], "LW") == NULL){

		char* parameter1 = removeStartingChars(strArray[1],1); //R1
		char* parameter2 = removeStartingChars(strArray[2],1);//R2
		char* parameter3 = removeStartingChars(strArray[3],1);//R3
		if(strstr(inst.opcode, "ADD") != NULL || strstr(inst.opcode, "SUB") != NULL || strstr(inst.opcode, "MOD") != NULL || strstr(inst.opcode, "MUL") != NULL || strstr(inst.opcode, "DIV") != NULL){

			// Add R1,R2,R3   SUB R1,R2,R3    DIV R1,R2,R3     MUL R1,R2,R3
			// R1 do operation R2--> store result in R3(destination)
			inst.source1 = (int)strtol(parameter2,&position,sizeof(parameter2));
			inst.source2 = (int)strtol(parameter3,&position,sizeof(parameter3));
			inst.destination = (int)strtol(parameter1,&position,sizeof(parameter1));
		}
	}else{

	if(strchr(strArray[2],'(')==NULL){
		char* parameter1 = removeStartingChars(strArray[1],1);
		char* parameter2 = removeStartingChars(strArray[2],1);

		//SW R1,R2
		//LW R2,R3
		if(strstr(inst.opcode, "SW") != NULL){
			inst.source1 = (int)strtol(parameter1,&position,sizeof(parameter1));
			inst.destination = (int)strtol(parameter2,&position,sizeof(parameter2));

		} else if (strstr(inst.opcode, "LW") != NULL){
			inst.source1 = (int)strtol(parameter2,&position,sizeof(parameter2));
			inst.destination = (int)strtol(parameter1,&position,sizeof(parameter1));
		}
	}else{

		char* parameter1 = removeStartingChars(strArray[1],1);
		char* parameter2 = removeStartingChars(getValueBetween(strArray[2],"(",")"),1);
		char* immidiateValue = getValueBetween(strArray[2],"","(");


		//SW R1,5(R2)
		//LW R2,5(R1)
		if(strstr(inst.opcode, "SW") != NULL){
			inst.source1 = (int)strtol(parameter1,&position,sizeof(parameter1));
			inst.destination = (int)strtol(parameter2,&position,sizeof(parameter2));
			inst.immediate = (int)strtol(immidiateValue,&position,sizeof(immidiateValue));

		} else if (strstr(inst.opcode, "LW") != NULL){
			inst.source1 = (int)strtol(parameter2,&position,sizeof(parameter2));
			inst.destination = (int)strtol(parameter1,&position,sizeof(parameter1));
			inst.immediate = (int)strtol(immidiateValue,&position,sizeof(immidiateValue));
		}
	}
}
	return inst;
}

//Function to store instructions to Instruction memory
int storeInstruction(char* strInst, int location){
  	char* position;
	char instruction[100];
	strInst = replaceChar(strInst,',',' ');

	char **strArray = splitInstruction(strInst);
	if(sizeof(strArray) < 3)
		return -1;

/******************* Opcodes ****************/
/*
LW 40
SW 41
ADD 42
SUB 43
MUL 44
DIV 45
MOD 46 */

	char* op;
	if(strstr(strArray[0], "LW") != NULL){
		op = "40";
	}else if(strcmp(strArray[0], "SW") != 0){
		op = "41";
	}else if(strcmp(strArray[0], "ADD") != 0){
		op = "42";
	}else if(strcmp(strArray[0], "SUB") != 0){
		op = "43";
	}else if(strcmp(strArray[0], "MUL") != 0){
		op = "44";
	}else if(strcmp(strArray[0], "DIV") != 0){
		op = "45";
	}else if(strcmp(strArray[0], "MOD") != 0){
		op = "46";
	}else{
		op = "-1";
		return -1;
	}

//Instructin format
//Add Op 2, Rt 1, Rs 1 and Rt/Immidiate 2
//Lw Op 2, Rt 1, Rs 1 and Immidiate 2

if(strstr(strArray[0], "SW") == NULL && strstr(strArray[0], "LW") == NULL){

		char* parameter1 = removeStartingChars(strArray[1],1); //R1
		char* parameter2 = removeStartingChars(strArray[2],1);//R2
		char* parameter3 = removeStartingChars(strArray[3],1);//R3
		if(strstr(strArray[0], "ADD") != NULL || strstr(strArray[0], "SUB") != NULL || strstr(strArray[0], "MOD") != NULL || strstr(strArray[0], "MUL") != NULL || strstr(strArray[0], "DIV") != NULL){
			// Add R1,R2,R3   SUB R1,R2,R3    DIV R1,R2,R3     MUL R1,R2,R3
			// R1 do operation R2--> store result in R3(destination)
			int p1 = (int)strtol(parameter1,&position,sizeof(parameter1));
			int p2 = (int)strtol(parameter2,&position,sizeof(parameter2));
			int p3 = (int)strtol(parameter3,&position,sizeof(parameter3));

			sprintf(instruction,"%s%01d%01d%02d",op,p3,p1,p2);
			memory_array[location] = atoi(instruction);
		}
	}else{
	if(strchr(strArray[2],'(')==NULL){
		char* parameter1 = removeStartingChars(strArray[1],1);
		char* parameter2 = removeStartingChars(strArray[2],1);

		//SW R1,R2
		//LW R2,R3
		if(strstr(strArray[0], "SW") != NULL){
			int s = (int)strtol(parameter1,&position,sizeof(parameter1));
			int d = (int)strtol(parameter2,&position,sizeof(parameter2));

			sprintf(instruction,"%s%01d%01d00",op,d,s);
			memory_array[location] = atoi(instruction);

		} else if (strstr(strArray[0], "LW") != NULL){
			int s = (int)strtol(parameter2,&position,sizeof(parameter2));
			int d = (int)strtol(parameter1,&position,sizeof(parameter1));

			sprintf(instruction,"%s%01d%01d00",op,d,s);
			memory_array[location] = atoi(instruction);
		}
	}else{

		char* parameter1 = removeStartingChars(strArray[1],1);
		char* parameter2 = removeStartingChars(getValueBetween(strArray[2],"(",")"),1);
		char* immidiateValue = getValueBetween(strArray[2],"","(");

		//SW R1,5(R2)
		//LW R2,5(R1)
		if(strstr(strArray[0], "SW") != NULL){
			int s = (int)strtol(parameter1,&position,sizeof(parameter1));
			int d = (int)strtol(parameter2,&position,sizeof(parameter2));
			int i = (int)strtol(immidiateValue,&position,sizeof(immidiateValue));

			sprintf(instruction,"%s%01d%01d%02d",op,d,s,i);
			memory_array[location] = atoi(instruction);

		} else if (strstr(strArray[0], "LW") != NULL){
			int s = (int)strtol(parameter2,&position,sizeof(parameter2));
			int d = (int)strtol(parameter1,&position,sizeof(parameter1));
			int i = (int)strtol(immidiateValue,&position,sizeof(immidiateValue));

			sprintf(instruction,"%s%01d%01d%02d",op,d,s,i);
			memory_array[location] = atoi(instruction);
		}
	}
}


	return 0;
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
	FILE * fp;
	char line[50];
	char **instructionContainer = malloc(100 * sizeof(char *));
	int i=0;

	fp = fopen(inputfilePath, "r");

	if (fp) {
		while(fgets(line,sizeof(line),fp)!= NULL && line !=""){
			instructionContainer[i] = malloc (sizeof(line));
			strcpy(instructionContainer[i], line);
			i++;
		}

	} else {

		printf("File not found !! \n");
		exit(-1);
	}

	fclose(fp);
	return instructionContainer;
}


//FUnction to get no of instructions
int getNoOfInstruction(char *inputfilePath){
	FILE * fp = fopen(inputfilePath, "r");
	int total =0;
	//Get no of nstructions
	int ch =0;

	if(fp == NULL)	return 0;

	while(!feof(fp)){
		ch = fgetc(fp);
		if(ch == '\n'){
			total++;
		}
	}
	fclose(fp);
return total;
}


//OPERATION function
void operate(struct instructionStructure inst) {
	if (strstr(inst.opcode, "LW") != NULL) {
		printf("Reading from %d\n",DAT_START_ADDR +inst.immediate+ Register_array[inst.source1]);
		Register_array[inst.destination] = readValue(DAT_START_ADDR +inst.immediate+ Register_array[inst.source1]);
	}
	else if (strstr(inst.opcode, "SW") != NULL) {
		printf("Writing to %d\n",Register_array[inst.destination] + inst.immediate + DAT_START_ADDR );
		writeValue(Register_array[inst.destination] + inst.immediate + DAT_START_ADDR,Register_array[inst.source1]);
	}else if (strstr(inst.opcode, "ADD") != NULL || strstr(inst.opcode, "SUB") != NULL || strstr(inst.opcode, "MUL") != NULL || strstr(inst.opcode, "MOD") != NULL || strstr(inst.opcode, "DIV") != NULL) 	{
		Register_array[inst.destination] = aluOperations(Register_array[inst.source1],Register_array[inst.source2],inst.opcode);

		printf("\tResult:  %d",Register_array[inst.destination]);

		if(Register_array[inst.destination] == 0){

				ZF = 1;
		}
		if (Register_array[inst.destination] == 2147483647 || Register_array[inst.destination] == -2147483574){
			CF = 1;
		}
		if(Register_array[inst.destination] < 0){
						SF = 1;
		}
		if(Register_array[inst.source1] > 0  &&  Register_array[inst.source2] > 0){
			if(Register_array[inst.destination] < 00){
				OF = 1;
			}
		}else if(Register_array[inst.source1] < 0  &&  Register_array[inst.source2] < 0){
			if(Register_array[inst.destination] > 0){
				OF = 1;
			}

		}else{
			OF = 0;

		}
	}else{
		printf("Invalid opcode\n" );
		exit(-1);
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

//ALU function
int aluOperations(int input1,int input2,const char* operation){
	int result = 0;
 	if (strstr(operation, "ADD") != NULL) {
 		printf("reached add ");
		printf("\ninuput 1: %d \tinput 2: %d",input1,input2);
 		result = add(input1,input2);

	}else if (strstr(operation, "SUB") != NULL) {
		result = sub(input1,input2);

	}else if (strstr(operation, "DIV") != NULL) {
		result = division(input1,input2);

	}else if (strstr(operation, "MUL") != NULL) {
		result = mul(input1,input2);

	}else if (strstr(operation, "MOD") != NULL) {
		result = mod(input1,input2);

	}
 	//printf("Result: %d",result);
//Carry overflow flag
       if (result > 2147483647 || result < -2147483647)
	    {
    	   printf("Reached carry flag if");
    	   CF = 1;
    	   printf("Carry occurs during operation: %s",operation);

	    }
       return result;

}

//-----------------------------------Bitwise arthimetic Functions-----------------------------------------------------------------------

//addition
int add(int p, int q)
{
    if (q == 0)
        return p;
    else
        return add( p ^ q, (p & q) << 1);

}

//subtraction
int sub(int p, int q)
{
    if (q == 0)
        return p;
    return sub(p ^ q, (~p & q) << 1);
}

//multiplication
int mul(int p,int q)
{
    int mulvalue=0;
    while (q!= 0)
    {
        if (q & 1)
        {
            mulvalue = add(mulvalue,p);
        }

        p <<= 1;
        q>>= 1;
    }
    return mulvalue;
}

//division
int division(int p, int q)
{
    int r = 1, temp1;
    temp1 = q;
if (p== 0 || q == 0)
    {
        printf("Division by zero exception in division function\n");
    }
    if ((p != 0 && q != 0) && (q < p))
    {
        while (((temp1 << 1) - p) < 0)
        {
            temp1 = temp1 << 1;
            r = r<< 1;
        }
        while ((temp1 + q) <= p)
        {
            temp1 = add(temp1, q);
            r =add(r,1);
        }
    }
    if (q>0)q=r;
return q;
}

//modulus operation
int mod(int p, int q)
{
    int remainder,n = 0;
 if (p== 0 || q == 0)
    {
        printf("Division by zero exception in mod function\n");
    }
    if (n>0 && q == 2^n)
    {
        remainder = (p& (q - 1));
    }
    else
    {
        remainder = (p - (mul(q,(p/q))));
    }
    return remainder;
}


//Print all values after every execution
int print_values()
{

	printf("\n########################################################\n");
	printf("\t\tRegisters values\n");
	printf("########################################################\n");
	printf("R0 = %d\t\tR1 = %d\t\tR2 = %d\t\tR3 = %d\n",Register_array[0],Register_array[1],Register_array[2],Register_array[3]);
	printf("R4 = %d\t\tR5 = %d\t\tR6 = %d\t\tR7 = %d\n",Register_array[4],Register_array[5],Register_array[6],Register_array[7]);
	printf("Flags::\t\tCF=%d\t\tOF =%d\t\tZF =%d\t\tSF =%d\n",CF,OF,ZF,SF);


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

	printf("########################################################\n");
	printf("\tInstruction Memory\n");
	printf("########################################################\n");
	printf("Address    \t\tMemory\n");
	for (int i=INS_START_ADDR; i <DAT_START_ADDR; i = i+16)
	{
		printf("%06d   %02X %02X %02X %02X  ", i, memory_array[i], memory_array[i+1], memory_array[i+2], memory_array[i+3]);
		printf("%02X %02X %02X %02X  ", memory_array[i+4], memory_array[i+5], memory_array[i+6], memory_array[i+7]);
		printf("%02X %02X %02X %02X  ", memory_array[i+8], memory_array[i+9], memory_array[i+10], memory_array[i+11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i+12], memory_array[i+13], memory_array[i+14], memory_array[i+15]);
	}

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
