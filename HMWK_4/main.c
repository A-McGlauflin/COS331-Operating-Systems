#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* 
    DONE ON A MAC DEVICE, INPUT IS SET TO 7

    Added arguments to the main function, type the name of the file to after ./main
    to run a specified file. At the bottom of main there is also a for loop used to
    print out ever memory address that just needs to be uncommented.

    Code by: Aiden McGlauflin
*/

//Creating necessary variables for mimicry
char IR[6], PSW[2], memory[100][6];
short int PC = 0, P0, P1, P2, P3;
int R0, R1, R2, R3, ACC, OPcode;

//The only time a 4-byte immediate will need to be converted to a string is if it's going into memory
//Therefore, we should include ZZ at the beginning
char* immToString(int imm) {
    char* data = (char *) malloc(6 * sizeof(char));

    *data  = 'Z';
    *(data + sizeof(char)) = 'Z';

    for(int i = 0; i < 4; i++) {
        *(data + (5 - i) * sizeof(char)) = (char) ((imm % 10) + 48);
        imm /= 10;
    }

    return data;
}

//Returns data converted into an integer
//Used for conversion of 4-byte immediates
int fourByteHelper(char* data) {
    return (int) ((data[0] - 48) * 1000) + (int) ((data[1] - 48) * 100) + (int) ((data[2] - 48) * 10) + (int) (data[3] - 48);
}

//Position is 0 for the first two bytes, and 2 for the second two bytes
int twoByteHelper(int position, char* data) {
    return ((int) (data[position] - 48) * 10) + (int) (data[position + 1] - 48);
}

//Position isn't used because it's assumed reg is only two bytes long
//memcpy will be used to take care of """"substrings""""
int regHelper(char* reg) {
    return (int) reg[1] - 48;
}

//From the memory address specified (P0, P1, P2, P3) convert that data to an integer
int pullImmediate(int address) {
    char* data = (char *) malloc(4 * sizeof(char));
    memcpy(data, &memory[address][2], 4 * sizeof(char));

    return fourByteHelper(data);
}

//From an address (R0-3 and P0-3) and a four byte immediate, convert it into a string and copy it into memory
void pushImmediate(int address, int data) {
    memcpy(memory[address], immToString(data), 6 * sizeof(char));
}

//RX is a register, not a perscription
//Returns the address of R0, R1, R2, or R3
int* getRX(char* RX) {
    switch(regHelper(RX)) {
        case 0:
            return &R0;
            break;
        case 1:
            return &R1;
            break;
        case 2:
            return &R2;
            break;
        case 3:
            return &R3;
            break;
        default:
            //This is dumb, but I don't know what else to return for the default case
            return &R0;
    }
}

void printIR() {
    printf("Current IR: %s\n", IR);
}

//From PX, find the P register and load it with the two byte immediate at the end
void OP00(char* pReg) {
    printf("OP00: Load Pointer with an Immediate\n");
    int data = twoByteHelper(2, pReg);
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, pReg, 2 * sizeof(*pReg));

    switch(regHelper(PX)) {
        case 0:
            P0 = data;
            break;
        case 1:
            P1 = data;
            break;
        case 2:
            P2 = data;
            break;
        case 3:
            P3 = data;
            break;
    }

    free(PX);
}

//Add the two-byte immediate to the P register
void OP01(char* pReg) {
    printf("OP01: Add to Pointer an Immediate\n");
    int data = twoByteHelper(2, pReg);
    char *PX = (char *) malloc(2 * sizeof(char));
    
    memcpy(PX, &pReg[0], 2 * sizeof(*pReg));

    switch(regHelper(PX)) {
        case 0:
            P0 += data;
            break;
        case 1:
            P1 += data;
            break;
        case 2:
            P2 += data;
            break;
        case 3:
            P3 += data;
            break;
    }

    free(PX);
}

//From PX, subtract the two-byte immediate from the P register
void OP02(char* pReg) {
    printf("OP02: Sub from Pointer an Immediate\n");
    int data = twoByteHelper(2, pReg);
    char *PX = (char *) malloc(2 * sizeof(char));
    memcpy(PX, &pReg[0], 2 * sizeof(*pReg));
    
    switch(regHelper(PX)) {
        case 0:
            P0 -= data;
            break;
        case 1:
            P1 -= data;
            break;
        case 2:
            P2 -= data;
            break;
        case 3:
            P3 -= data;
            break;
    }

    free(PX);
}

void OP03(char* data) {
    printf("OP03: Load ACC with an Immediate\n");

    ACC = fourByteHelper(data);
}

void OP04(char* pReg) {
    printf("OP04: Load ACC with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));
    memcpy(PX, &pReg[0], 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            ACC = pullImmediate(P0);
            break;
        case 1:
            ACC = pullImmediate(P1);
            break;
        case 2:
            ACC = pullImmediate(P2);
            break;
        case 3:
            ACC = pullImmediate(P3);
            break;
    }

    free(PX);
}

void OP05(char* data) {
    printf("OP05: Load ACC with Direct Addressing\n");

    ACC = pullImmediate(twoByteHelper(0, data));
}

void OP06(char* pReg) {
    printf("OP06: Store ACC with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));
    memcpy(PX, pReg, 2 * sizeof(*pReg));

    printf("ACC: %d\n", ACC);

    switch(regHelper(PX)) {
        case 0:
            pushImmediate(P0, ACC);
            break;
        case 1:
            pushImmediate(P1, ACC);
            break;
        case 2:
            pushImmediate(P2, ACC);
            break;
        case 3:
            pushImmediate(P3, ACC);
            break;
    }

    free(PX);
}

void OP07(char* data) {
    printf("OP07: Store ACC with Direct Addressing\n");

    pushImmediate(twoByteHelper(0, data), ACC);
}

void OP08(char* regs) {
    printf("OP08: Store Register with Register Addressing\n");
    char *RX = (char *) malloc(2 * sizeof(char)), *PX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2 * sizeof(char));
    memcpy(PX, &regs[2], 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            pushImmediate(P0, *getRX(RX));
            break;
        case 1:
            pushImmediate(P1, *getRX(RX));
            break;
        case 2:
            pushImmediate(P2, *getRX(RX));
            break;
        case 3:
            pushImmediate(P3, *getRX(RX));
            break;
    }

    free(PX);
    free(RX);
}

void OP09(char* data) {
    printf("OP09: Store Register with Direct Addressing\n");
    char *RX = (char *) malloc(2 * sizeof(char));
    memcpy(RX, &data[0], 2 * sizeof(char));

    pushImmediate(twoByteHelper(2, data), *getRX(RX));

    free(RX);
}

void OP10(char* regs) {
    printf("OP10: Load Register with Register Addressing, regs: %s\n", regs);
    char *RX = (char *) malloc(2 * sizeof(char)), *PX = (char *) malloc(2 * sizeof(char));
    
    memcpy(RX, regs, 2 * sizeof(*regs)); 
    memcpy(PX, &regs[2], 2 * sizeof(*regs));

    switch(regHelper(PX)) {
        case 0:
            *getRX(RX) = pullImmediate(P0);
            break;
        case 1:
            *getRX(RX) = pullImmediate(P1);
            break;
        case 2:
            *getRX(RX) = pullImmediate(P2);
            break;
        case 3:
            *getRX(RX) = pullImmediate(P3);
            break;
    }

    free(PX);
    free(RX);
}

void OP11(char* data) {
    printf("OP11: Load Register with Direct Addressing\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, &data[0], 2 * sizeof(char));

    *getRX(RX) = twoByteHelper(2, data);

    free(RX);
}

void OP12(char* data) {
    printf("OP12: Load R0 with Immediate\n");

    R0 = fourByteHelper(data);
}

void OP13(char* regs) {
    printf("OP13: Load Register with Other Register\n");
    //RX <- RY
    char *RX = (char *) malloc(2 * sizeof(char)), *RY = (char *) malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2 * sizeof(char));
    memcpy(RY, &regs[2], 2 * sizeof(char));

    *getRX(RX) = *getRX(RY);

    free(RX);
    free(RY);
}

void OP14(char* regs) {
    printf("OP14: Load ACC from Register\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2 * sizeof(char));

    ACC = *getRX(RX);

    free(RX);
}

void OP15(char* regs) {
    printf("OP15: Load Register from ACC\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, regs, 2* sizeof(char));

    *getRX(RX) = ACC;

    free(RX);
}

void OP16(char* data) {
    printf("OP16: Add to ACC an Immediate\n");
    
    ACC += fourByteHelper(data);
}

void OP17(char *data) {
    printf("OP17: Subtract from ACC an Immediate\n");

    ACC -= fourByteHelper(data);
}

void OP18(char *regs) {
    printf("OP18: Add Regester to ACC\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, regs, 2 * sizeof(char));

    ACC += *getRX(RX);

    free(RX);
}

void OP19(char *regs) {
    printf("OP19: Subtract Register from ACC\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, regs, 2 * sizeof(char));

    ACC -= *getRX(RX);

    free(RX);
}

void OP20(char *regs) {
    printf("OP20: Add to ACC with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            ACC += pullImmediate(P0);
            break;
        case 1:
            ACC += pullImmediate(P1);
            break;
        case 2:
            ACC += pullImmediate(P2);
            break;
        case 3:
            ACC += pullImmediate(P3);
            break;
    }

    free(PX);
}

void OP21(char *data) {
    printf("OP21: Add to ACC with Direct Addressing\n");
    char *DX = (char *) malloc(2 * sizeof(char));

    memcpy(DX, data, 2 * sizeof(char));

    ACC += pullImmediate(regHelper(DX));

    free(DX);
}

void OP22(char *regs) {
    printf("OP22: Sub from ACC with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2*sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            ACC -= pullImmediate(P0);
            break;
        case 1:
            ACC -= pullImmediate(P1);
            break;
        case 2:
            ACC -= pullImmediate(P2);
            break;
        case 3:
            ACC -= pullImmediate(P3);
            break;
    }

    free(PX);
}

void OP23(char *data) {
    printf("OP23: Add to ACC with Direct Addressing\n");
    char *DX = (char *) malloc(2 * sizeof(char));

    memcpy(DX, data, 2 * sizeof(char));

    ACC -= pullImmediate(regHelper(DX));

    free(DX);
}

void OP24(char *regs) {
    printf("OP24: Compare Equal with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            if(ACC == pullImmediate(P0))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 1:
            if(ACC == pullImmediate(P1))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 2:
            if(ACC == pullImmediate(P2))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 3:
            if(ACC == pullImmediate(P3))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
    }

    free(PX);
}

void OP25(char *regs) {
    printf("OP25: Compare Less with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            if(ACC < pullImmediate(P0))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 1:
            if(ACC < pullImmediate(P1))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 2:
            if(ACC < pullImmediate(P2))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 3:
            if(ACC < pullImmediate(P3))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
    }

    free(PX);
}

void OP26(char *regs) {
    printf("OP26: Compare Greater with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            if(ACC > pullImmediate(P0))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 1:
            if(ACC > pullImmediate(P1))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 2:
            if(ACC > pullImmediate(P2))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 3:
            if(ACC > pullImmediate(P3))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
    }

    free(PX);
}

void OP27(char *data) {
    printf("OP27: Compare Greater with Immediate Addressing\n");

    if(ACC > fourByteHelper(data))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';
}

void OP28(char *data) {
    printf("OP28: Compare Equal with Immediate Addressing\n");

    if(ACC == fourByteHelper(data))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';
}

void OP29(char *data) {
    printf("OP29: Compare Less with Immediate Addressing\n");

    if(ACC < fourByteHelper(data))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';
}

void OP30(char *regs) {
    printf("OP30: Compare Equal with Register\n");
    char *RX = (char *) malloc(2 *sizeof(char));

    if(ACC == *getRX(RX))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';

    free(RX);
}

void OP31(char *regs) {
    printf("OP31: Compare Less with Register\n");
    char *RX = (char *) malloc(2 *sizeof(char));

    if(ACC < *getRX(RX))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';

    free(RX);
}

void OP32(char *regs) {
    printf("OP32: Compare Greater with Register\n");
    char *RX = (char *) malloc(2 *sizeof(char));

    if(ACC > *getRX(RX))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';
    
    free(RX);
}

void OP33(char *data) {
    printf("OP33: Branch True Conditional\n");

    if(PSW[0] == 'T')
        PC = twoByteHelper(0, data);
}

void OP34(char *data) {
    printf("OP34: Branch False Conditional\n");

    if(PSW[0] == 'F')
        PC = twoByteHelper(0, data);
}

void OP35(char *data) {
    printf("OP35: Branch Unconditional\n");

    PC = twoByteHelper(0, data);
}

void opcodeHandler(int OPcode, char* data) {
    switch(OPcode) {
        case 0:
            PC++;
            printIR();
            OP00(data);
            break;
        case 1:
            PC++;
            printIR();
            OP01(data);
            break;
        case 2:
            PC++;
            printIR();
            OP02(data);
            break;
        case 3:
            PC++;
            printIR();
            OP03(data);
            break;
        case 4:
            PC++;
            printIR();
            OP04(data);
            break;
        case 5:
            PC++;
            printIR();
            OP05(data);
            break;
        case 6:
            PC++;
            printIR();
            OP06(data);
            break;
        case 7:
            PC++;
            printIR();
            OP07(data);
            break;
        case 8:
            PC++;
            printIR();
            OP08(data);
            break;
        case 9:
            PC++;
            printIR();
            OP09(data);
            break;
        case 10:
            PC++;
            printIR();
            OP10(data);
            break;
        case 11:
            PC++;
            printIR();
            OP11(data);
            break;
        case 12:
            PC++;
            printIR();
            OP12(data);
            break;
        case 13:
            PC++;
            printIR();
            OP13(data);
            break;
        case 14:
            PC++;
            printIR();
            OP14(data);
            break;
        case 15:
            PC++;
            printIR();
            OP15(data);
            break;
        case 16:
            PC++;
            printIR();
            OP16(data);
            break;
        case 17:
            PC++;
            printIR();
            OP17(data);
            break;
        case 18:
            PC++;
            printIR();
            OP18(data);
            break;
        case 19:
            PC++;
            printIR();
            OP19(data);
            break;
        case 20:
            PC++;
            printIR();
            OP20(data);
            break;
        case 21:
            PC++;
            printIR();
            OP21(data);
            break;
        case 22:
            PC++;
            printIR();
            OP22(data);
            break;
        case 23:
            PC++;
            printIR();
            OP23(data);
            break;
        case 24:
            PC++;
            printIR();
            OP24(data);
            break;
        case 25:
            PC++;
            printIR();
            OP25(data);
            break;
        case 26:
            PC++;
            printIR();
            OP26(data);
            break;
        case 27:
            PC++;
            printIR();
            OP27(data);
            break;
        case 28:
            PC++;
            printIR();
            OP28(data);
            break;
        case 29:
            PC++;
            printIR();
            OP29(data);
            break;
        case 30:
            PC++;
            printIR();
            OP30(data);
            break;
        case 31:
            PC++;
            printIR();
            OP31(data);
            break;
        case 32:
            PC++;
            printIR();
            OP32(data);
            break;
        case 33:
            PC++;
            printIR();
            OP33(data);
            break;
        case 34:
            PC++;
            printIR();
            OP34(data);
            break;
        case 35:
            PC++;
            printIR();
            OP35(data);
            break;
        case 99:
            printf("Program Exit:\n");
            printf("ACC: %d\nP0: %d, P1: %d, P2: %d, P3: %d\nR0: %d, R1: %d, R2: %d, R3: %d\n", ACC, P0, P1, P2, P3, R0, R1, R2, R3);
            PC = -1;
            break;
        default:
            PC = -1;
            printf("UNEXPECTED OPCODE HIT WITH IR OF:\n");
            printIR();
            break;
    }
}

int main(int argc, char* argv[]) {
    //input buffer array for reading from file
    char input[7];

    //File descriptor, ret, and program line are created
    int fd = open(*(++argv), O_RDONLY),
    ret = read(fd, input, 7),
    program_line = 0;
 
    //If the file can't be read, exit as an error
    if(fd < 0)
        return -1;

    //Reads the first 6-bytes of the input line and load it into memory
    while(1) {
        if(ret <= 0)
            break;
        for(int i = 0; i < 6; i++) {
            memory[program_line][i] = input[i];
        }

        //Get the next line and inc program line
        ret = read(fd, input, 7);

        program_line++;
    }

    //Keep incrementing the PC until the opcode is the halt command
    for(PC = 0; OPcode != 99;){
        if(PC == -1)
            break;
        //Load instruction into the IR
        memcpy(IR, memory[PC], 6 * sizeof(char));
        
        //printf("PC: %d\n", PC);

        //Add the first two bytes of the IR to the OPcode
        OPcode = ((int) (IR[0] - 48) * 10) + (int) (IR[1] - 48);
        char hold[4];

        memcpy(hold, &IR[2], 4*sizeof(char));

        //printf("%d\n", OPcode);
        opcodeHandler(OPcode, hold);

        printf("ACC: %d\n", ACC);
    }

    
    //Uncomment to see memory output
    
    char *helper = (char *) malloc(6 * sizeof(char));
    
    for(int i = 0; i < 100; i++) {
        memcpy(helper, memory[i], 6 * sizeof(char));
        printf("Memory %2d: %6s\n", i, helper);
    }
    

    return 0;
}