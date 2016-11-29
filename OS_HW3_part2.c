#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

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

char **InstructionBucket;

char **localInstructions;

const char **referInstructions;

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
	char * opcode; // LW = Read & SW = Write
	char * label;
};

//Function to check existence of a char
int checkExistence(char * str, char* a);

//Function to get value between 2 chars
char * getValueBetween(const char * const string, const char * const start,
		const char * const end);

//Function to convert string into an Instruction Struct
struct instructionStructure parseInstruction(char* strInst);

//Function to replace special chars in a string
char * replaceChar(char * str, char specialChar, char newChar);

//Function to split Instruction string and return string array
char ** splitInstruction(char * instruction, char* search);

//READ opration from Source1 address
int readValue(int source);

//WRITE opration at Destination address
void writeValue(int destination, int source);

//OPERATION function
int operate(struct instructionStructure inst, int i);

//Print all values after every instruction execution
int print_values();

//Function to take input from a file with instruction to execute
char **takeInput(char *inputfilePath);

//Function to check is String contains all numbers
int checkAllDigits(const char *str);

//Function to remove starting chars from a string
char * removeStartingChars(char * str, int i);

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
int mul(int p, int q);
//subtraction
int sub(int p, int q);
//addition
int add(int p, int q);

//Comparision function
bool branch(int a, int b, char str[4]);

//ALU function
int aluOperations(int input1, int input2, const char* operation);

//Function to find the location of the label
int findLabel(char *label);

int main() {

	Register_array[0] = 1;
	Register_array[1] = 2;
	Register_array[2] = 3;
	Register_array[3] = 4;
	/*
		//4294967295 4294967266 Register values for Part 1: Falgs  behaviour-> Uncomment this register values and comment the above values
		//Use input1.txt for testing the part 1 of the asssignment for flags behaviour
		Register_array[0] = -55;
		Register_array[1] = -20;
		Register_array[2] = 75;
		Register_array[3] = 2147483647;
	
	*/


	//initialisation
	pc = INS_START_ADDR;
	sp = Stack_top;
	struct instructionStructure inst;

	printf("Please provide the filepath for instructions \n");
	char filePath[100];
	scanf("%s",filePath);

	//temp path ----------------"/home/paragas12/Input.txt"

	InstructionBucket = malloc(100 * sizeof(char *));
	localInstructions = malloc(100 * sizeof(char *));
	referInstructions = malloc(100 * sizeof(char *));

	referInstructions = takeInput(filePath);
	InstructionBucket = takeInput(filePath);
	localInstructions = takeInput(filePath);
	noOfInstructions = getNoOfInstruction(filePath);

	//Store instructions into memory before execution
	for (int j = 0; j < noOfInstructions; j++) {

		if (storeInstruction(InstructionBucket[j], j + INS_START_ADDR) != 0) {
			printf("Problem with instruction: %s", InstructionBucket[j]);
			exit(-1);
		}
	}

	print_values();

	//Find no. of instructions int *ptr = begin; *ptr; ptr++
	for (int i = 0; i < noOfInstructions; i++) {
		printf("\n%d)%s\n", i + 1, localInstructions[i]);
		inst = parseInstruction(localInstructions[i]);

		//Operate on given input
		i = operate(inst, i);
		pc = INS_START_ADDR + i;
		//print_values();
	}
	return 0;
}

//Function to test comparision results
/*
 BGTZ = Greater Than 0
 BLTZ = Less Than 0
 BGEZ = Greater Thane Equal To 0
 BLEZ = Less Thane Equal To 0
 BEQ = Equal To
 BNE = Not Equal To
 BGT = Greater than
 BLT = Less than
 */
bool branch(int a, int b, char str[4]) {
	if (strstr(str, "BGEZ") != NULL) {
		if (a >= b)
			return true;
	} else if (strstr(str, "BLEZ") != NULL) {
		if (a <= b)
			return true;
	} else if (strstr(str, "BGTZ") != NULL) {
		if (a > b)
			return true;
	} else if (strstr(str, "BLTZ") != NULL) {
		if (a < b)
			return true;
	} else if (strstr(str, "BNE") != NULL) {
		if (a != b)
			return true;
	} else if (strstr(str, "BEQ") != NULL) {
		if (a == b)
			return true;
	} else if (strstr(str, "BGT") != NULL) {
		if (a > b)
			return true;
	} else if (strstr(str, "BLT") != NULL) {
		if (a < b)
			return true;
	}
	return false;
}

//Function to check existence of a char
int checkExistence(char * str, char* a) {
	char *checkingString = str;
	while (*checkingString) {
		if (strchr(a, *checkingString)) {
			return 1;
		}
		checkingString++;
	}
	return 0;
}

//Function to get value between 2 chars
char *getValueBetween(const char * const string, const char * const start,
		const char * const end) {
	char *a;
	char *b;
	size_t length;
	char *result;

	if ((string == NULL) || (start == NULL) || (end == NULL))
		return NULL;
	length = strlen(start);
	a = strstr(string, start);
	if (a == NULL)
		return NULL;
	a += length;
	b = strstr(a, end);
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
struct instructionStructure parseInstruction(char* strInst) {
	char* str;
	char tempStr[15];
	strcpy(tempStr, strInst);
	//Check if  a label present
	if (strstr(strInst, ":") != NULL) {
		str = splitInstruction(strInst, ":")[1];
	} else {
		str = strInst;
	}
	struct instructionStructure inst;
	inst.immediate = 0;
	if (strstr(strInst, "J") == NULL) {
		str = replaceChar(str, ',', ' ');
	}
	char **strArray = splitInstruction(str, " ");
	inst.opcode = strArray[0];

	if (strstr(strArray[0], "SW") == NULL && strstr(strArray[0], "LW") == NULL
			&& strstr(strArray[0], "LEA") == NULL) {

		char* parameter1;
		if (strstr(strInst, "J") == NULL) {
			parameter1 = removeStartingChars(strArray[1], 1); //R1
		} else {
			parameter1 = strArray[1];
		}
		char* parameter2;
		if (strstr(strInst, "J") == NULL)
			parameter2 = removeStartingChars(strArray[2], 1); //R2
		char* parameter3;
		if (strstr(strInst, "J") == NULL && strstr(strInst, "B") == NULL)
			parameter3 = removeStartingChars(strArray[3], 1); //R3
		if (strstr(inst.opcode, "ADD") != NULL
				|| strstr(inst.opcode, "SUB") != NULL
				|| strstr(inst.opcode, "MOD") != NULL
				|| strstr(inst.opcode, "MUL") != NULL
				|| strstr(inst.opcode, "DIV") != NULL) {
			// Add R1,R2,R3   SUB R1,R2,R3    DIV R1,R2,R3     MUL R1,R2,R3
			// R1 do operation R2--> store result in R3(destination)
			inst.source1 = (int) strtol(parameter2, (char **) NULL, 10);
			inst.source2 = (int) strtol(parameter3, (char **) NULL, 10);
			inst.destination = (int) strtol(parameter1, (char **) NULL, 10);
		}

//Branch and Jump instruction format to be added

		else if (strstr(inst.opcode, "BGTZ") != NULL
				|| strstr(inst.opcode, "BLTZ") != NULL
				|| strstr(inst.opcode, "BGEZ") != NULL
				|| strstr(inst.opcode, "BLEZ") != NULL) {
			// BGTZ R1,Label similarly
			inst.source1 = (int) strtol(parameter1, (char **) NULL, 10);
			inst.source2 = 0;
			inst.label = strArray[3];
		} else if (strstr(inst.opcode, "BEQ") != NULL
				|| strstr(inst.opcode, "BNE") != NULL
				|| strstr(inst.opcode, "BGT") != NULL
				|| strstr(inst.opcode, "BLT") != NULL) {
			// BEQ R1,R2,Label     BNE R1,R2,Label similarly
			inst.source1 = (int) strtol(parameter1, (char **) NULL, 10);
			inst.source2 = (int) strtol(parameter2, (char **) NULL, 10);
			inst.label = strArray[3];
		} else if (strstr(inst.opcode, "JR") != NULL) {
			// JR R1
			inst.destination = (int) strtol(parameter1, (char **) NULL, 10);
		} else if (strstr(inst.opcode, "J") != NULL) {
			// J 138
			inst.immediate = (int) strtol(strArray[1], (char **) NULL, 10);
		}

	} else {

		if (strchr(strArray[2], '(') == NULL) {
			char* parameter1 = removeStartingChars(strArray[1], 1);
			char* parameter2 = removeStartingChars(strArray[2], 1);

			//SW R1,R2
			//LW R2,R3
			//LEA
			if (strstr(inst.opcode, "SW") != NULL) {
				inst.source1 = (int) strtol(parameter1, (char **) NULL, 10);
				inst.destination = (int) strtol(parameter2, (char **) NULL, 10);

			} else if (strstr(inst.opcode, "LW") != NULL
					|| strstr(inst.opcode, "LEA") != NULL) {
				inst.source1 = (int) strtol(parameter2, (char **) NULL, 10);
				inst.destination = (int) strtol(parameter1, (char **) NULL, 10);

			}
		} else if (strlen(getValueBetween(tempStr, "(", ")"))
				> 2&& strstr(inst.opcode, "LEA") != NULL) {
			char* parameter1 = removeStartingChars(strArray[1], 1);
			char* value = getValueBetween(tempStr, "(", ")");
			value = replaceChar(value, ',', ' ');
			char **array = splitInstruction(value, " ");
			inst.source1 = (int) strtol(array[0], (char **) NULL, 10);
			inst.source2 = (int) strtol(array[1], (char **) NULL, 10);
			inst.immediate = (int) strtol(array[2], (char **) NULL, 10);
			inst.destination = (int) strtol(parameter1, (char **) NULL, 10);
		} else {

			char* parameter1 = removeStartingChars(strArray[1], 1);
			char* parameter2 = removeStartingChars(
					getValueBetween(strArray[2], "(", ")"), 1);
			char* immidiateValue = getValueBetween(strArray[2], "", "(");

			//SW R1,5(R2)
			//LW R2,5(R1)
			if (strstr(inst.opcode, "SW") != NULL) {
				inst.source1 = (int) strtol(parameter1, (char **) NULL, 10);
				inst.destination = (int) strtol(parameter2, (char **) NULL, 10);
				inst.immediate = (int) strtol(immidiateValue, (char **) NULL,
						10);

			} else if (strstr(inst.opcode, "LW") != NULL
					|| strstr(inst.opcode, "LEA") != NULL) {
				inst.source1 = (int) strtol(parameter2, (char **) NULL, 10);
				inst.destination = (int) strtol(parameter1, (char **) NULL, 10);
				inst.immediate = (int) strtol(immidiateValue, (char **) NULL,
						10);
			}
		}
	}
	return inst;
}

//Function to store instructions to Instruction memory
int storeInstruction(char* strInst, int location) {
	char instruction[100];
	char tempStr[15];
	strcpy(tempStr, strInst);
	//Check if  a label present
	if (strstr(strInst, ":") != NULL) {
		strInst = splitInstruction(strInst, ":")[1];
	}

	if (strstr(strInst, "J") == NULL) {
		strInst = replaceChar(strInst, ',', ' ');
	}
	char **strArray = splitInstruction(strInst, " ");

	/******************* Instruction set - Opcodes ****************/
	/*
	 LW 40    SW 41	LEA 55
	 ADD 42    SUB 43    MUL 44    DIV 45    MOD 46
	 BEQ 47    BNE 48	BGT 56	BLT 57
	 BGTZ 49    BLTZ 50    BGEZ 51    BLEZ 52
	 JR 53 J 54
	 */

	char* op;
	if (strstr(strArray[0], "LW") != NULL) {
		op = "40";
	} else if (strstr(strArray[0], "SW") != NULL) {
		op = "41";
	} else if (strstr(strArray[0], "ADD") != NULL) {
		op = "42";
	} else if (strstr(strArray[0], "SUB") != NULL) {
		op = "43";
	} else if (strstr(strArray[0], "MUL") != NULL) {
		op = "44";
	} else if (strstr(strArray[0], "DIV") != NULL) {
		op = "45";
	} else if (strstr(strArray[0], "MOD") != NULL) {
		op = "46";
	} else if (strstr(strArray[0], "BEQ") != NULL) {
		op = "47";
	} else if (strstr(strArray[0], "BNE") != NULL) {
		op = "48";
	} else if (strstr(strArray[0], "BGTZ") != NULL) {
		op = "49";
	} else if (strstr(strArray[0], "BLTZ") != NULL) {
		op = "50";
	} else if (strstr(strArray[0], "BGEZ") != NULL) {
		op = "51";
	} else if (strstr(strArray[0], "BLEZ") != NULL) {
		op = "52";
	} else if (strstr(strArray[0], "JR") != NULL) {
		op = "53";
	} else if (strstr(strArray[0], "J") != NULL) {
		op = "54";
	} else if (strstr(strArray[0], "LEA") != NULL) {
		op = "55";
	} else if (strstr(strArray[0], "BGT") != NULL) {
		op = "56";
	} else if (strstr(strArray[0], "BLT") != NULL) {
		op = "57";
	} else {
		op = "-1";
		return -1;
	}

//Instructin format
//Add Op 2, Rt 1, Rs 1 and Rt/Immidiate 2
//Lw Op 2, Rt 1, Rs 1 and Immidiate 2

	if (strstr(strArray[0], "SW") == NULL && strstr(strArray[0], "LW") == NULL
			&& strstr(strArray[0], "LEA") == NULL) {

		char* parameter1;
		if (strstr(strInst, "J") == NULL) {
			parameter1 = removeStartingChars(strArray[1], 1); //R1
		} else {
			parameter1 = strArray[1];
		}
		char* parameter2;
		if (strstr(strInst, "J") == NULL)
			parameter2 = removeStartingChars(strArray[2], 1); //R2
		char* parameter3;
		if (strstr(strInst, "J") == NULL && strstr(strInst, "B") == NULL)
			parameter3 = removeStartingChars(strArray[3], 1); //R3
		if (strstr(strArray[0], "ADD") != NULL
				|| strstr(strArray[0], "SUB") != NULL
				|| strstr(strArray[0], "MOD") != NULL
				|| strstr(strArray[0], "MUL") != NULL
				|| strstr(strArray[0], "DIV") != NULL) {
			// Add R1,R2,R3   SUB R1,R2,R3    DIV R1,R2,R3     MUL R1,R2,R3
			// R1 do operation R2--> store result in R3(destination)
			int p1 = (int) strtol(parameter1, (char **) NULL, 10);
			int p2 = (int) strtol(parameter2, (char **) NULL, 10);
			int p3 = (int) strtol(parameter3, (char **) NULL, 10);

			sprintf(instruction, "%s%01d%01d%02d", op, p3, p1, p2);
			memory_array[location] = atoi(instruction);
		}

//Branch and jump to be added
		else if (strstr(strArray[0], "BGTZ") != NULL
				|| strstr(strArray[0], "BLTZ") != NULL
				|| strstr(strArray[0], "BGEZ") != NULL
				|| strstr(strArray[0], "BLEZ") != NULL) {
			// BGTZ R1,Label similarly
			int p1 = (int) strtol(parameter1, (char **) NULL, 10);
			int label = findLabel(strArray[3]);

			sprintf(instruction, "%s%01d00%02d", op, p1, label);
			memory_array[location] = atoi(instruction);
		} else if (strstr(strArray[0], "BEQ") != NULL
				|| strstr(strArray[0], "BNE") != NULL
				|| strstr(strArray[0], "BGT") != NULL
				|| strstr(strArray[0], "BLT") != NULL) {
			// BEQ R1,R2,Label     BNE R1,R2,Label
			int p1 = (int) strtol(parameter1, (char **) NULL, 10);
			int p2 = (int) strtol(parameter2, (char **) NULL, 10);
			int label = findLabel(strArray[3]);

			sprintf(instruction, "%s%01d%01d%02d", op, p1, p2, label);
			memory_array[location] = atoi(instruction);
		} else if (strstr(strArray[0], "JR") != NULL) {
			// JR R1
			int p1 = (int) strtol(parameter1, (char **) NULL, 10);

			sprintf(instruction, "%s00%01d00", op, p1);
			memory_array[location] = atoi(instruction);

		} else if (strstr(strArray[0], "J") != NULL) {
			// J 138
			int p1 = (int) strtol(strArray[1], (char **) NULL, 10);

			sprintf(instruction, "%s0000%02d", op, p1);
			memory_array[location] = atoi(instruction);
		}

	} else {
		if (strchr(strArray[2], '(') == NULL) {
			char* parameter1 = removeStartingChars(strArray[1], 1);
			char* parameter2 = removeStartingChars(strArray[2], 1);

			//SW R1,R2
			//LW R2,R3
			if (strstr(strArray[0], "SW") != NULL) {
				int s = (int) strtol(parameter1, (char **) NULL, 10);
				int d = (int) strtol(parameter2, (char **) NULL, 10);

				sprintf(instruction, "%s%01d%01d00", op, d, s);
				memory_array[location] = atoi(instruction);

			} else if (strstr(strArray[0], "LW") != NULL) {
				int s = (int) strtol(parameter2, (char **) NULL, 10);
				int d = (int) strtol(parameter1, (char **) NULL, 10);

				sprintf(instruction, "%s%01d%01d00", op, d, s);
				memory_array[location] = atoi(instruction);

			} else if (strstr(strArray[0], "LEA") != NULL) {
				int s = (int) strtol(parameter2, (char **) NULL, 10);
				int d = (int) strtol(parameter1, (char **) NULL, 10);

				sprintf(instruction, "%s%01d%01d00", op, d, s);
				memory_array[location] = atoi(instruction);
			}
		} else if (strlen(getValueBetween(tempStr, "(", ")"))
				> 2&& strstr(strArray[0], "LEA") != NULL) {
			char* parameter1 = removeStartingChars(strArray[1], 1);
			char* value = getValueBetween(tempStr, "(", ")");
			value = replaceChar(value, ',', ' ');
			char **array = splitInstruction(value, " ");
			int s = (int) strtol(array[0], (char **) NULL, 10);
			int i = (int) strtol(array[2], (char **) NULL, 10);
			int d = (int) strtol(parameter1, (char **) NULL, 10);

			sprintf(instruction, "%s%01d%01d%02d", op, d, s, i);
			memory_array[location] = atoi(instruction);

		} else {

			char* parameter1 = removeStartingChars(strArray[1], 1);
			char* parameter2 = removeStartingChars(
					getValueBetween(strArray[2], "(", ")"), 1);
			char* immidiateValue = getValueBetween(strArray[2], "", "(");

			//SW R1,5(R2)
			//LW R2,5(R1)
			if (strstr(strArray[0], "SW") != NULL) {
				int s = (int) strtol(parameter1, (char **) NULL, 10);
				int d = (int) strtol(parameter2, (char **) NULL, 10);
				int i = (int) strtol(immidiateValue, (char **) NULL, 10);

				sprintf(instruction, "%s%01d%01d%02d", op, d, s, i);
				memory_array[location] = atoi(instruction);

			} else if (strstr(strArray[0], "LW") != NULL) {
				int s = (int) strtol(parameter2, (char **) NULL, 10);
				int d = (int) strtol(parameter1, (char **) NULL, 10);
				int i = (int) strtol(immidiateValue, (char **) NULL, 10);

				sprintf(instruction, "%s%01d%01d%02d", op, d, s, i);

				memory_array[location] = atoi(instruction);
			} else if (strstr(strArray[0], "LEA") != NULL) {
				int s = (int) strtol(parameter2, (char **) NULL, 10);
				int d = (int) strtol(parameter1, (char **) NULL, 10);
				int i = (int) strtol(immidiateValue, (char **) NULL, 10);

				sprintf(instruction, "%s%01d%01d%02d", op, d, s, i);
				memory_array[location] = atoi(instruction);
			}
		}
	}

	return 0;
}

//Function to remove starting chars from a string
char * removeStartingChars(char* str, int i) {
	char *newStr = malloc(sizeof(str) * sizeof(char));
	for (int j = 0; j < sizeof(str) - 1; j++)
		newStr[j] = str[j + 1];
	newStr[sizeof(str) - 1] = '\0';

	return newStr;
}

//Function to check is String contains all numbers
int checkAllDigits(const char *str) {
	while (*str) {
		if (isdigit(*str++) == 0)
			return 0;
	}
	return 1;
}

//Function to split Instruction string and return string array
char ** splitInstruction(char * instruction, char* search) {
	char *position = strtok(instruction, search);
	char **array = malloc(10 * sizeof(char *));
	int i = 0;
	while (position != NULL) {
		array[i++] = position;
		position = strtok(NULL, search);
	}
	return array;
}

//Function to replace special chars in a string
char * replaceChar(char * str, char specialChar, char newChar) {
	for (char* p = str; p = strchr(p, specialChar); ++p) {
		*p = newChar;
	}
	return str;
}

//Function to take input from a file with instruction to execute
char **takeInput(char *inputfilePath) {
	FILE * fp;
	char line[50];
	char **instructionContainer = malloc(100 * sizeof(char *));
	int i = 0;

	fp = fopen(inputfilePath, "r");

	if (fp) {
		while (fgets(line, sizeof(line), fp) != NULL && line != "") {
			instructionContainer[i] = malloc(sizeof(line));
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
int getNoOfInstruction(char *inputfilePath) {
	FILE * fp = fopen(inputfilePath, "r");
	int total = 0;
	//Get no of instructions
	int ch = 0;

	if (fp == NULL)
		return 0;

	while (!feof(fp)) {
		ch = fgetc(fp);
		if (ch == '\n') {
			total++;
		}
	}
	fclose(fp);
	return total;
}

//OPERATION function
int operate(struct instructionStructure inst, int i) {
	char* str = referInstructions[i];
	if (strstr(str, "(") != NULL && strstr(inst.opcode, "LEA") != NULL
			&& sizeof(str) > 13) {
		if (strlen(getValueBetween(str, "(", ")")) > 2) {
			printf("Reading Effective address from %d\n",
					DAT_START_ADDR
							+ (Register_array[inst.source1]
									+ Register_array[inst.source2])
									* inst.immediate);

			Register_array[inst.destination] = readValue(
					DAT_START_ADDR
							+ (Register_array[inst.source1]
									+ Register_array[inst.source2])
									* inst.immediate);
		}
	} else if (strstr(inst.opcode, "LW") != NULL
			|| strstr(inst.opcode, "LEA") != NULL) {
		printf("Reading from %d\n",
		DAT_START_ADDR + inst.immediate + Register_array[inst.source1]);
		Register_array[inst.destination] = readValue(
		DAT_START_ADDR + inst.immediate + Register_array[inst.source1]);
	} else if (strstr(inst.opcode, "SW") != NULL) {
		printf("Writing to %d\n",
				Register_array[inst.destination] + inst.immediate
						+ DAT_START_ADDR);
		writeValue(
				Register_array[inst.destination] + inst.immediate
						+ DAT_START_ADDR, Register_array[inst.source1]);
	} else if (strstr(inst.opcode, "ADD") != NULL
			|| strstr(inst.opcode, "SUB") != NULL
			|| strstr(inst.opcode, "MUL") != NULL
			|| strstr(inst.opcode, "MOD") != NULL
			|| strstr(inst.opcode, "DIV") != NULL) {
		Register_array[inst.destination] = aluOperations(
				Register_array[inst.source1], Register_array[inst.source2],
				inst.opcode);
		printf("Result-->>%d", Register_array[inst.destination]);
		if(Register_array[inst.destination] == 0){
				ZF = 1;
		}
		if(Register_array[inst.destination] < 0){
				SF = 1;
		}
		if (Register_array[inst.source1] > 0
				&& Register_array[inst.source2] > 0) {
			if (Register_array[inst.destination] < 0) {
				OF = 1;
			}
		} else if (Register_array[inst.source1] < 0
				&& Register_array[inst.source2] < 0) {
			if (Register_array[inst.destination] > 0) {
				OF = 1;
			}
		} else {
			OF = 0;
		}
	} else if (strstr(inst.opcode, "BEQ") != NULL
			|| strstr(inst.opcode, "BNE") != NULL
			|| strstr(inst.opcode, "BGT") != NULL
			|| strstr(inst.opcode, "BLT") != NULL
			|| strstr(inst.opcode, "BGTZ") != NULL
			|| strstr(inst.opcode, "BLTZ") != NULL
			|| strstr(inst.opcode, "BGEZ") != NULL
			|| strstr(inst.opcode, "BLEZ") != NULL) {
		if (branch(Register_array[inst.source1], Register_array[inst.source2], inst.opcode)) {
			for (int j = 0; j < noOfInstructions; j++) {
				char* str = splitInstruction(inst.label, "\n")[0];
				str[sizeof(str) - 1] = '\0';
				if (strstr(localInstructions[j], str) != NULL) {

					return findLabel(inst.label);
				}
			}
		}
	} else if (strstr(inst.opcode, "JR") != NULL) {
		// JR R1
		return (Register_array[inst.destination] - INS_START_ADDR - 1);
	} else if (strstr(inst.opcode, "J") != NULL) {
		// J 138
		return (inst.immediate - INS_START_ADDR - 1);
	} else {
		printf("Invalid opcode\n");
		exit(-1);
	}
	return i;
}

//Function to find the location of the label
int findLabel(char *label) {
	for (int j = 0; j < noOfInstructions; j++) {
		char* str = splitInstruction(label, "\n")[0];
		str[sizeof(str) - 1] = '\0';
		if (strstr(localInstructions[j], str) != NULL) {
			return j - 1;
		}
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
int aluOperations(int input1, int input2, const char* operation) {
	int result = 0;
	if (strstr(operation, "ADD") != NULL) {
		result = add(input1, input2);
	} else if (strstr(operation, "SUB") != NULL) {
		result = sub(input1, input2);
	} else if (strstr(operation, "DIV") != NULL) {
		result = division(input1, input2);
	} else if (strstr(operation, "MUL") != NULL) {
		result = mul(input1, input2);
	} else if (strstr(operation, "MOD") != NULL) {
		result = mod(input1, input2);
	}
//Carry overflow flag
	if (result > 2147483647) {
		CF = 1;
		printf("Carry occurs during operation: %s", operation);

	}
	return result;

}

//-----------------------------------Bitwise arthimetic Functions-----------------------------------------------------------------------

//addition
int add(int p, int q) {
	if (q == 0)
		return p;
	else
		return add(p ^ q, (p & q) << 1);

}

//subtraction
int sub(int p, int q) {
	if (q == 0)
		return p;
	return sub(p ^ q, (~p & q) << 1);
}

//multiplication
int mul(int p, int q) {
	int mulvalue = 0;
	while (q != 0) {
		if (q & 1) {
			mulvalue = add(mulvalue, p);
		}

		p <<= 1;
		q >>= 1;
	}
	return mulvalue;
}

//division
int division(int p, int q) {
	int r = 1, temp1;
	temp1 = q;
	if (p == 0 || q == 0) {
		printf("Division by zero exception in division function\n");
	}
	if ((p != 0 && q != 0) && (q < p)) {
		while (((temp1 << 1) - p) < 0) {
			temp1 = temp1 << 1;
			r = r << 1;
		}
		while ((temp1 + q) <= p) {
			temp1 = add(temp1, q);
			r = add(r, 1);
		}
	}
	if (q > 0)
		q = r;
	return q;
}

//modulus operation
int mod(int p, int q) {
	int remainder, n = 0;
	if (p == 0 || q == 0) {
		printf("Division by zero exception in mod function\n");
	}
	if (n > 0 && q == 2 ^ n) {
		remainder = (p & (q - 1));
	} else {
		remainder = (p - (mul(q, (p / q))));
	}
	return remainder;
}

//Print all values after every execution
int print_values() {

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

	for (int i = 0; i < INS_START_ADDR; i = i + 16) {
		printf("%06d   %02X %02X %02X %02X  ", i, memory_array[i],
				memory_array[i + 1], memory_array[i + 2], memory_array[i + 3]);
		printf("%02X %02X %02X %02X  ", memory_array[i + 4],
				memory_array[i + 5], memory_array[i + 6], memory_array[i + 7]);
		printf("%02X %02X %02X %02X  ", memory_array[i + 8],
				memory_array[i + 9], memory_array[i + 10],
				memory_array[i + 11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i + 12],
				memory_array[i + 13], memory_array[i + 14],
				memory_array[i + 15]);
	}

	printf("########################################################\n");
	printf("\tInstruction Memory\n");
	printf("########################################################\n");
	printf("Address    \t\tMemory\n");
	for (int i = INS_START_ADDR; i < DAT_START_ADDR; i = i + 16) {
		printf("%06d   %02X %02X %02X %02X  ", i, memory_array[i],
				memory_array[i + 1], memory_array[i + 2], memory_array[i + 3]);
		printf("%02X %02X %02X %02X  ", memory_array[i + 4],
				memory_array[i + 5], memory_array[i + 6], memory_array[i + 7]);
		printf("%02X %02X %02X %02X  ", memory_array[i + 8],
				memory_array[i + 9], memory_array[i + 10],
				memory_array[i + 11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i + 12],
				memory_array[i + 13], memory_array[i + 14],
				memory_array[i + 15]);
	}

	printf("########################################################\n");
	printf("\tData Memory\n");
	printf("########################################################\n");
	printf("Address    \t\tMemory\n");
	for (int i = DAT_START_ADDR; i < Stack_Base; i = i + 16) {
		printf("%06d   %02X %02X %02X %02X  ", i, memory_array[i],
				memory_array[i + 1], memory_array[i + 2], memory_array[i + 3]);
		printf("%02X %02X %02X %02X  ", memory_array[i + 4],
				memory_array[i + 5], memory_array[i + 6], memory_array[i + 7]);
		printf("%02X %02X %02X %02X  ", memory_array[i + 8],
				memory_array[i + 9], memory_array[i + 10],
				memory_array[i + 11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i + 12],
				memory_array[i + 13], memory_array[i + 14],
				memory_array[i + 15]);
	}

	printf("########################################################\n");
	printf("\tStack Memory\n");
	printf("########################################################\n");
	printf("Address    \t\tMemory\n");
	for (int i = Stack_Base; i < 1024; i = i + 16) {
		printf("%06d   %02X %02X %02X %02X  ", i, memory_array[i],
				memory_array[i + 1], memory_array[i + 2], memory_array[i + 3]);
		printf("%02X %02X %02X %02X  ", memory_array[i + 4],
				memory_array[i + 5], memory_array[i + 6], memory_array[i + 7]);
		printf("%02X %02X %02X %02X  ", memory_array[i + 8],
				memory_array[i + 9], memory_array[i + 10],
				memory_array[i + 11]);
		printf("%02X %02X %02X %02X  \n", memory_array[i + 12],
				memory_array[i + 13], memory_array[i + 14],
				memory_array[i + 15]);
	}
	printf("\n");
}

