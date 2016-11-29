
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Pending global variables

#define _GNU_SOURCE
#define BINSEARCHFLAG 1
//Memory is in Integer format and Opcode is in String format
#define DEBUG 1
#define BINSEARCHFLAG 1

#define RENUM 8
#define PC_INC 4

/*//Instruction counter
int noOfInstructions = 0;

char **InstructionBucket;

char **localInstructions;

const char **referInstructions;*/
//Total memory size(TM)
#define MEM_DEPTH 65536
//512 Instruction Addresses (0-to-511),  Data Addresses(512-to-65535)
#define INS_START_ADDR 0x0A
#define DAT_START_ADDR 0x200
// Stack 
#define STACK_SIZE 8

/*opcode definition*/
#define LD  0x40
#define ST  0x41

// Arithemeatic instructions
#define ADD 0x50
#define SUB 0x51
#define MUL 0x52
#define DIV 0x53
#define MOD 0x54

// Stack Operations
#define PUSH 0x60
#define POP 0x61

#define JMP 0x80
#define JNE 0x81
#define JE  0x82
#define JB  0x83
#define JA  0x84
#define CMP 0x90

#define LEA 0x70

typedef unsigned int myInt;

// Structure for operation
typedef struct opes{
	short op;       	//op code
	short src1;
	short src2;
	short des;
	unsigned int addr;  //memory address
}ope;

// Structure for absolute address for labels
typedef struct absAddr{
        char *key;
        int val;
}absAddr;

absAddr labelAddr[10];

//register
myInt reg[RENUM];

//memory
myInt mem[MEM_DEPTH];

//PC, memory address, memory data, instruction, stack , stack pointer
myInt	ins;

unsigned int insCnt;

unsigned int PC;

int absCnt=-1;

char flag;

short overflowFlag;
short zeroFlag;
short signFlag;

unsigned int d_recentAddr[16];

int stack[STACK_SIZE];

//Define Instructions Load/Store as strings(Instruction Strings)
char *sIns[512];


int SP;

//read Instruction input file
int parseInsFile();

//initialize the instruction memory, data memory, etc.
void init();

//translate assembly "LD R2, $200" to hex
void getInput();

//generate next PC address
void calNextPC(ope curop);
myInt mapIns(char * inst);

//stage 1 of processor: fetch instruction
void fetchIns();

//stage 2 of processor: decode the instruction
ope decodeIns();

//stage 3 of processor: execute
void executeop(ope curop);

void dump(int currIns);

//we will dump the 16 most recently used memory
void pushAddr(unsigned int addr);

// Arithematic operations 
myInt Ins_Add(myInt a, myInt b);
myInt Ins_Sub(myInt a, myInt b);
myInt Ins_Mul(myInt a, myInt b);
myInt Ins_Div(myInt a, myInt b);
myInt Ins_Mod(myInt a, myInt b);

// Stack Operations
void push(myInt value);
myInt pop();

// Comparing function
void compare(myInt val1,myInt val2);

//Inserting/retreiving Label Address 
void putLabelAddr(char* key,int val);
int getLabelAddr();
void binarySearchInput();
int binSearch();

void init()
{
	unsigned int i;
	
	PC = INS_START_ADDR;
	flag = 'N';
	overflowFlag = 0;
	insCnt = 0;
	SP = -1;
	for (i = 0; i<RENUM; i++)
	{
		reg[i] = 0;
	}

	for (i = 0; i<10; i++)
	{
		mem[DAT_START_ADDR+i] = i;

	}

	for (i = 0; i<STACK_SIZE; i++)
	{
		stack[i] = 0;

	}

	dump(-1);
	getInput();

}

int parseInsFile()
{
	char file_name[25];
	FILE *fp;
	unsigned int i = 0;
    // Read file name with instructions
	printf("Enter the name of file you wish to read: ");
	gets(file_name);

	fp = fopen(file_name, "r"); // read mode

	if (fp == NULL)
	{
		printf("File not Found.\n");
		getchar();
		return 0;
	}

	// Reading instruction from file, printing on screen and storing it in array for CPU processing
	//printf("Your input instructions in file %s are :\n", file_name);
	char line[50];
	while (fgets(line, 50, fp) != NULL)
	{
		//printf("Inst[%d] : %s", (i + 1), line);
		sIns[i] = (char*)malloc(sizeof(line));
		strcpy(sIns[i], line);  // Reading instructions from file and storing it in array for CPU processing
		char *token1=strtok(line, " ");
		if (strstr(token1,"Label"))
        {
           char *temp_token = strtok(NULL, "\n");
           putLabelAddr(temp_token, INS_START_ADDR+i);
           i--;
        }
        i++;
	}

	fclose(fp);
    return 1;
}

int main()
{
	unsigned int i = 0;
	ope curop;
    if(parseInsFile()==0)return 0;

	// initializing CPU
	init();
	if (BINSEARCHFLAG)
    	binarySearchInput();                                                                                                                                                                                                                                                                                                                       

    while(i<insCnt-1)
	{
   		i=PC-INS_START_ADDR;
	//(1)fetch instruction first
		fetchIns();
		//(2)decode the instruction and generate
		curop = decodeIns();
		//(3)do real things here
		executeop(curop);
		if (DEBUG)
			dump(i);
        //getch();
	}
	if (BINSEARCHFLAG)
	{
       int loc = binSearch();
       if (loc >= 0)
          printf("\nElement Found at location : %d", loc);
       else 
            printf("\nElement Not Found");
    
    }
	return 0;
}


void fetchIns()                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                  
{
	ins = mem[PC];								//fetch hex value of instruction from Instruction memory and store it on ins
}

ope decodeIns()
{
	ope curop;
	curop.op = (ins >> 24);											//Mask 31-24bits of ins to get an opcode
	curop.des = ((ins << 8) >> 24);									//Mask 8-15 bits of inst to get an reg address
	curop.src1 = ((ins << 16) >> 24);
	curop.src2 = (ins >> 0) & 0x000000FF;
	curop.addr = (ins >> 0) & 0x0000FFFF;							//Mask last 8 bits to get the address

	return curop;
}
/*Here is data exchange is taking place from Reg-Mem/Mem-Reg*/
void executeop(ope curop)
{
    int disp,scale,temp;

	switch (curop.op)
	{
	case LD:                                                                                                                                                                                                                                                                                                                                                                                                                   
		reg[curop.des] = mem[DAT_START_ADDR + curop.addr];		//Loading data from memory data to register
		pushAddr(curop.addr);											//pushing it to recent change stack
		flag = 'L';
		break;

	case ST:
		mem[DAT_START_ADDR + curop.addr] = reg[curop.des];		//storing data from Reg from memory data addresses
		pushAddr(curop.addr);											//pushing it to recent change stack
		flag = 'S';
		break;

     // Arithematic operations
     
	case ADD:                 // Addition
		reg[curop.des] = Ins_Add(reg[curop.src1], reg[curop.src2]);
		flag = 'A';
		break;

	case SUB:             // Subtraction
		reg[curop.des] = Ins_Sub(reg[curop.src1], reg[curop.src2]);
		flag = 'I';
		break;

	case MUL:             // Multiplication
		reg[curop.des] = Ins_Mul(reg[curop.src1], reg[curop.src2]);
		flag = 'M';
		break;

	case DIV:             // Division
		reg[curop.des] = Ins_Div(reg[curop.src1], reg[curop.src2]);
		flag = 'D';
		break;

	case MOD:             // Modulous
		reg[curop.des] = Ins_Mod(reg[curop.src1], reg[curop.src2]);
		flag = 'O';
		break;

	case PUSH:             // Push number from Register
		push(reg[curop.des]);
		flag = 'U';
		break;

	case POP:             // Pop into register
		reg[curop.des] = pop();
		flag = 'P';
		break;

	case JA:
	case JB:
	case JNE:
	case JE:
    case JMP:             // JMP,JNE,JE,JA,JB instrction
		flag = 'J';
		break;

	case CMP:             // Compare instrction
		compare(reg[curop.des],reg[curop.src1]);
		flag = 'C';
		break;

	case LEA:             // LEA instrction
        disp=curop.src2 >>4;
        scale=curop.src2 & 0x0000000F;
        curop.src2=curop.src1 & 0x0000000F;
        curop.src1=curop.src1 >> 4;
        reg[curop.des] = mem[DAT_START_ADDR + reg[curop.src1] + disp];
        if (scale>0)
           reg[curop.des] = reg[curop.des] + (scale*mem[DAT_START_ADDR+reg[curop.src2]]);
		flag = 'E';
		break;
	}

    calNextPC(curop);
	 
}


//Bitwise Addition Function
myInt Ins_Add(myInt a, myInt b)
{
	myInt c;
	myInt a2;
	myInt b2;
	a2 = a;
	b2 = b;
    while (b2 != 0)
    {
		c = a2&b2;
		a2 = a2^b2;
		b2 = c << 1;
	}
	if ((a2<a) || (a2<b))
       overflowFlag=1;
    else 
       overflowFlag=0;
    zeroFlag = (a2==0);
    signFlag =0;
	return a2;
}

//Bitwise Substraction Function
myInt Ins_Sub(myInt a, myInt b)
{
	myInt c = 0, neg = 0;

	if(a < b)
		neg = 1;

	b = ~b;
    b = b + 1;

    a= Ins_Add(a,b);
    
    if(neg == 1)
    {
    	a = ~a;
        a = a + 1;
    }
    overflowFlag=0;
    zeroFlag = (a==0);
    signFlag = (neg == 1);
    return a;
}

//Bitwise Multiplication Function
myInt Ins_Mul(myInt a, myInt b)
{
    int mask, c = 0;
    int flag;
    flag = 0;
	while(b != 0)
    {
		mask = 1;
		mask = mask & b;

		if(mask)
		{
			c = Ins_Add(a,c);
			if(overflowFlag) flag=1;
        }   
		a = a << 1;
        b = b >> 1;
    }
    overflowFlag = flag;
    zeroFlag = (c==0);
    signFlag =0;
	return c;

}
//Bitwise Division Function
myInt Ins_Div(myInt a, myInt b)
{
	int mask = 0x1;
	int quot = 0;

	while (b <= a)
	{
		b <<= 1;
		mask <<= 1;
	}

	while (mask > 1)
	{
		b >>= 1;
		mask >>= 1;

		if (a >= b)
		{
			a = a-b;
			quot |= mask;
		}
	}
    overflowFlag=0;
    zeroFlag = (quot==0);
    signFlag =0;
	return quot;
}

//Bitwise Modulo Function
myInt Ins_Mod(myInt a, myInt b)
{
	myInt sum = 0, lsum = 0, rem = 0;
	sum = b;

	while(sum < a)
	{
		lsum = sum;
		sum = Ins_Add(sum,b);
	}

	rem = Ins_Sub(a,lsum);
	overflowFlag=0;
    zeroFlag = (rem==0);
	return rem;
}

//Push
void push(myInt value)
{

	if(SP < STACK_SIZE)
	{
		SP++;
		stack[SP] = value;
    }
	else
	    printf ("Stack OverFlow !!");
}

//Pop
myInt pop()
{
	myInt popVal = -1;

	if(SP == -1)
		printf("Stack Empty");
	else
	{
		popVal = stack[SP];
		stack[SP] = 0;
		SP--;
	}

	return popVal;
}


/*Printing Function*/
void dump(int currIns)
{
	int i;
	if (currIns >= 0)
   		printf("\n Instruction : %s \n", sIns[currIns]);
    else
		printf("\n\n\tInitial CPU Status\n");

	printf(" ---------------------------------------------------------------\n");

	printf("|    REG        ADDR  DATA(BEN)   ADDR  DATA(LEN)    Stack\t|\n");

	for (i = 0; i < RENUM; i++){

		printf("|R%01d: %08x   %04x: %08x    %04x: %08x\t%x", i, reg[i], i, mem[DAT_START_ADDR + i], RENUM-1-i,mem[DAT_START_ADDR+RENUM-1-i], stack[RENUM-1-i] );
		if ((RENUM-1-i)== SP)
		   printf("<--SP");
        printf("\t|\n");

    }


	return;
}

/*mapIns() : returns hex value of the predefined instruction in 32bits hex*/
myInt mapIns(char * inst)
{
	int hex, opcode = 0, reg[3]={0}, memAdd=-1, reg1,leaFlag=0;
	char temp[5] = { '\0' };
    int shift = 16,shiftBits=8;
	char tempIns[20];
	strcpy(tempIns, inst);
	int len = 0, i, temp_i;
	char *token1,*token2,*temp_token;

	//library function to break up the predefined instruction using tokens to operate instruction
	//uses LD instruction from Instruction addresses in case of getting L 
	//uses ST instruction from instruction addresses in case of getting S
	token1 = strtok(tempIns, " ");
	switch (token1[0]){
    case 'C':
		opcode=CMP;
		break;

    case 'J':
		if (strstr(token1,"JMP"))
           opcode = JMP;
        else if (strstr(token1,"JNE"))
           opcode = JNE;
        else if (strstr(token1,"JB"))
            opcode = JB;
        else if (strstr(token1,"JA"))
            opcode = JA;
        else 
            opcode = JE;
        temp_token = strtok(NULL, "\n");
        memAdd=getLabelAddr(temp_token);
		break;
	case 'L':
        if (strstr(token1,"LEA")){
           opcode = LEA;
           shiftBits=4;
           memAdd=0;
           }
        else   
           opcode = LD;
		break;
	case 'S':
		if (strstr(token1,"SUB"))
           opcode = SUB;
        else
            opcode = ST;
		break;
	case 'A':
		opcode = ADD;
		break;
	case 'M':
        if (strstr(token1,"MUL"))
           opcode = MUL;
        else
            opcode = MOD;
		break;
	case 'D':
        opcode = DIV;
		break;
	case 'P':
        if (strstr(token1,"PUSH"))
           opcode = PUSH;
        else
            opcode = POP;
		break;
    default:
		printf("Incorrect Instruction\n");
		break;
	}
	/*again uses library function to breakup the instruction string further
	*Case R, converts characters following R untill “,” token, converts to int and set as the value of Reg variable
	*Case $, converts characters following $ untill NULL token, converts to int and set as the value of the memAdd variable
	*/
	temp_i = 0;
	token2 = strtok(NULL, " ,()");
	while (token2 != NULL)
	{
		switch (token2[0])
		{
		case 'R':
			temp[0] = token2[1];
			reg[temp_i] = atoi(temp);
			temp_i= temp_i+1;
			break;
		case '$':
			len = strlen(token2);
			i = 0;
			while (i < len)
			{
				temp[i] = token2[i + 1];
				i++;
			}
			memAdd = atoi(temp);
			break;
        case 'L':
             break;
        case '1':
        case '2':
        case '4':
        case '8':
             temp[0]=token2[0];
             temp[1]='\0';
             memAdd += atoi(temp);
             if(temp_i==1)
                memAdd = (memAdd<<4);
             break;
		default:
			printf("Incorrect Instruction usage !! ");
			break;
		}
		token2 = strtok(NULL, " ,()\n");
	}
	
	hex = opcode << 24;
    i=0;
	while (i<temp_i)
    {
       	hex += reg[i] << shift;
       	shift = shift-shiftBits;
       	i++;
    }

    if (memAdd > -1)
		hex += memAdd;
		    	
	return hex;
}

void pushAddr(unsigned int addr)
{
	unsigned int i=0, isNewAdd;
	// Check if new address to be pushed is present in the recent addresses array or not
	// If present then donot push the new address to the recent address array
	for (i = 0; i<16; i++)
	{
		if (addr == d_recentAddr[i])
		{
			isNewAdd = 1;
			break;
		}
	}

	if (isNewAdd != 1){
		//shift the array: d[15] = d[14]; d[14] = d[13]......
		for (i = 16 - 1; i>0; i--)
		{
			d_recentAddr[i] = d_recentAddr[i - 1];
		}
		//then d[0] = new_addr
		d_recentAddr[0] = addr;
	}
}

void getInput()
{
	//Adding instruction to Instruction Addresses in memory consecutively
	int i=0;
	for (i = 0; sIns[i] != NULL; i++)
        mem[INS_START_ADDR + insCnt++] = mapIns(sIns[i]);		//storing hex value of pre defined instructuions to memory Instruction addresses, for all instructions available in input file
}

void calNextPC(ope curop)
{
     if (curop.op == JMP)
        PC = curop.addr;			
     else if(curop.op == JNE && !zeroFlag)
          PC = curop.addr;
     else if(curop.op == JE && zeroFlag)
          PC = curop.addr;
     else if(curop.op == JB && signFlag)
          PC = curop.addr;
     else if(curop.op == JA && !signFlag && !zeroFlag)
          PC = curop.addr;
     else 
          PC = PC + 1;
}

void compare(myInt val1,myInt val2)
{
     if (val1==val2)
        zeroFlag = 1;
     else
     {
         zeroFlag = 0;
         signFlag = (signFlag || (val1 < val2));
     }
}

// Insert the Label equivalent absolute address for JUMP intructons     
void putLabelAddr(char* key,int val)
{
     absCnt++;
     labelAddr[absCnt].key= (char*)malloc(sizeof(key));
     strcpy(labelAddr[absCnt].key,key);
     labelAddr[absCnt].val=val;
}

// Get the Label equivalent absolute address to move the PC to that location for JUMP intructons     
int getLabelAddr(char *key)
{
     int i =0;
     for (i=0;i<=absCnt;i++)
     {
         if(strcmp(labelAddr[i].key,key)==0)
              return labelAddr[i].val;
     }        
     return -1;
}
// Taking input from user
// R0 - No. TO BE SEARCHED
// R1 - MIN , R3 - SIZE OF ARRAY, R2- MID LOC
// R7 - 2 to DIV (R1+R3) BY 2
// R6 - SUB OR ADD BY 1
// R2 - Location of Element 
//-----------------Binarysearch input function---------------------------------------------------------------------------------------------
void binarySearchInput()
{
     int i,key,n;
     printf("\n Enter the no of elements : ");
     scanf("%d",&n);
     printf("\n Enter sorted elements : ");
     for (i=0;i<n;i++)
     {
         scanf("%d",&mem[DAT_START_ADDR+i]);
     }
     printf("\n Enter the element to be searched: ");
     scanf("%d",&key);
     reg[0]=key;
     reg[1]=0;
     reg[3]=n-1;
     reg[7]=2;
     reg[6]=1;
}   
//------------------------------------function to get the index as return value-------------------------------------------------------------
// Binary Search output           
int binSearch()
{
    int loc;
    if (reg[0]==reg[4])
       loc = reg[2];
    else
        loc = -1;
    return loc;
}
