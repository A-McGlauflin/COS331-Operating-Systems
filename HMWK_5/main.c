#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

//Debug will print each instruction and print the queue
#define DEBUG 0

/* 
    DONE ON A MAC DEVICE, INPUT IS SET TO 7

    Added arguments to the main function, type the name of the file to after ./main
    to run a specified file. At the bottom of main there is also a for loop used to
    print out ever memory address that just needs to be uncommented.

    Code by: Aiden McGlauflin
*/

//Creating necessary variables for mimicry
char IR[6], PSW[2], memory[1000][6];
short int PC = 0, P0, P1, P2, P3;
int R0, R1, R2, R3, ACC, OPcode;

struct PCB {
    int R0, R1, R2, R3, ACC, PID, BaseRegister, LimitRegister, IC, OPcode;
    short int PC, P0, P1, P2, P3;
    char PSW[2], IR[6];
    struct PCB *next_PCB;
};

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

//Debug to see IR at each call
void printIR() {
    printf("Current IR: %s\n", IR);
}

//From PX, find the P register and load it with the two byte immediate at the end
void OP00(char* pReg) {
    if (DEBUG)
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
    if (DEBUG)
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
    if (DEBUG)
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

//Each print statement is what each function generally does
void OP03(char* data) {
    if (DEBUG)
        printf("OP03: Load ACC with an Immediate\n");

    ACC = fourByteHelper(data);
}

void OP04(char* pReg, struct PCB *current) {
    if (DEBUG)
        printf("OP04: Load ACC with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));
    memcpy(PX, &pReg[0], 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            ACC = pullImmediate(P0 + current->BaseRegister);
            break;
        case 1:
            ACC = pullImmediate(P1 + current->BaseRegister);
            break;
        case 2:
            ACC = pullImmediate(P2 + current->BaseRegister);
            break;
        case 3:
            ACC = pullImmediate(P3 + current->BaseRegister);
            break;
    }

    free(PX);
}

void OP05(char* data, struct PCB *current) {
    if (DEBUG)
        printf("OP05: Load ACC with Direct Addressing\n");

    ACC = pullImmediate(twoByteHelper(0, data) + current->BaseRegister);
}

void OP06(char* pReg, struct PCB *current) {
    if (DEBUG)
        printf("OP06: Store ACC with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));
    memcpy(PX, pReg, 2 * sizeof(*pReg));

    switch(regHelper(PX)) {
        case 0:
            pushImmediate(P0 + current->BaseRegister, ACC);
            break;
        case 1:
            pushImmediate(P1 + current->BaseRegister, ACC);
            break;
        case 2:
            pushImmediate(P2 + current->BaseRegister, ACC);
            break;
        case 3:
            pushImmediate(P3 + current->BaseRegister, ACC);
            break;
    }

    free(PX);
}

void OP07(char* data, struct PCB *current) {
    if (DEBUG)
        printf("OP07: Store ACC with Direct Addressing\n");

    pushImmediate(twoByteHelper(0, data) + current->BaseRegister, ACC);
}

void OP08(char* regs, struct PCB *current) {
    if (DEBUG)
        printf("OP08: Store Register with Register Addressing\n");
    char *RX = (char *) malloc(2 * sizeof(char)), *PX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2 * sizeof(char));
    memcpy(PX, &regs[2], 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            pushImmediate(P0 + current->BaseRegister, *getRX(RX));
            break;
        case 1:
            pushImmediate(P1 + current->BaseRegister, *getRX(RX));
            break;
        case 2:
            pushImmediate(P2 + current->BaseRegister, *getRX(RX));
            break;
        case 3:
            pushImmediate(P3 + current->BaseRegister, *getRX(RX));
            break;
    }

    free(PX);
    free(RX);
}

void OP09(char* data, struct PCB *current) {
    if (DEBUG)
        printf("OP09: Store Register with Direct Addressing\n");
    char *RX = (char *) malloc(2 * sizeof(char));
    memcpy(RX, &data[0], 2 * sizeof(char));

    pushImmediate(twoByteHelper(2, data) + current->BaseRegister, *getRX(RX));

    free(RX);
}

void OP10(char* regs, struct PCB *current) {
    if (DEBUG)
        printf("OP10: Load Register with Register Addressing\n");
    char *RX = (char *) malloc(2 * sizeof(char)), *PX = (char *) malloc(2 * sizeof(char));
    
    memcpy(RX, regs, 2 * sizeof(*regs)); 
    memcpy(PX, &regs[2], 2 * sizeof(*regs));

    switch(regHelper(PX)) {
        case 0:
            *getRX(RX) = pullImmediate(P0 + current->BaseRegister);
            break;
        case 1:
            *getRX(RX) = pullImmediate(P1 + current->BaseRegister);
            break;
        case 2:
            *getRX(RX) = pullImmediate(P2 + current->BaseRegister);
            break;
        case 3:
            *getRX(RX) = pullImmediate(P3 + current->BaseRegister);
            break;
    }

    free(PX);
    free(RX);
}

void OP11(char* data, struct PCB *current) {
    if (DEBUG)
        printf("OP11: Load Register with Direct Addressing\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, &data[0], 2 * sizeof(char));

    *getRX(RX) = pullImmediate(twoByteHelper(2, data) + current->BaseRegister);

    free(RX);
}

void OP12(char* data) {
    if (DEBUG)
        printf("OP12: Load R0 with Immediate\n");

    R0 = fourByteHelper(data);
}

void OP13(char* regs) {
    if (DEBUG)
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
    if (DEBUG)
        printf("OP14: Load ACC from Register\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2 * sizeof(char));

    ACC = *getRX(RX);

    free(RX);
}

void OP15(char* regs) {
    if (DEBUG)
        printf("OP15: Load Register from ACC\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, regs, 2* sizeof(char));

    *getRX(RX) = ACC;

    free(RX);
}

void OP16(char* data) {
    if (DEBUG)
        printf("OP16: Add to ACC an Immediate\n");
    
    ACC += fourByteHelper(data);
}

void OP17(char *data) {
    if (DEBUG)
        printf("OP17: Subtract from ACC an Immediate\n");

    ACC -= fourByteHelper(data);
}

void OP18(char *regs) {
    if (DEBUG)
        printf("OP18: Add Regester to ACC\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, regs, 2 * sizeof(char));

    ACC += *getRX(RX);

    free(RX);
}

void OP19(char *regs) {
    if (DEBUG)
        printf("OP19: Subtract Register from ACC\n");
    char *RX = (char *) malloc(2 * sizeof(char));

    memcpy(RX, regs, 2 * sizeof(char));

    ACC -= *getRX(RX);

    free(RX);
}

void OP20(char *regs, struct PCB *current) {
    if (DEBUG)
        printf("OP20: Add to ACC with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            ACC += pullImmediate(P0 + current->BaseRegister);
            break;
        case 1:
            ACC += pullImmediate(P1 + current->BaseRegister);
            break;
        case 2:
            ACC += pullImmediate(P2 + current->BaseRegister);
            break;
        case 3:
            ACC += pullImmediate(P3 + current->BaseRegister);
            break;
    }

    free(PX);
}

void OP21(char *data, struct PCB *current) {
    if (DEBUG)
        printf("OP21: Add to ACC with Direct Addressing\n");
    char *DX = (char *) malloc(2 * sizeof(char));

    memcpy(DX, data, 2 * sizeof(char));

    ACC += pullImmediate(twoByteHelper(0, DX) + current->BaseRegister);

    free(DX);
}

void OP22(char *regs, struct PCB *current) {
    if (DEBUG)
        printf("OP22: Sub from ACC with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2*sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            ACC -= pullImmediate(P0 + current->BaseRegister);
            break;
        case 1:
            ACC -= pullImmediate(P1 + current->BaseRegister);
            break;
        case 2:
            ACC -= pullImmediate(P2 + current->BaseRegister);
            break;
        case 3:
            ACC -= pullImmediate(P3 + current->BaseRegister);
            break;
    }

    free(PX);
}

void OP23(char *data, struct PCB *current) {
    if (DEBUG)
        printf("OP23: Sub from ACC with Direct Addressing\n");
    char *DX = (char *) malloc(2 * sizeof(char));

    memcpy(DX, data, 2 * sizeof(char));

    ACC -= pullImmediate(twoByteHelper(0, DX) + current->BaseRegister);

    free(DX);
}

void OP24(char *regs, struct PCB *current) {
    if (DEBUG)
        printf("OP24: Compare Equal with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            if(ACC == pullImmediate(P0 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 1:
            if(ACC == pullImmediate(P1 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 2:
            if(ACC == pullImmediate(P2 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 3:
            if(ACC == pullImmediate(P3 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
    }

    free(PX);
}

void OP25(char *regs, struct PCB *current) {
    if (DEBUG)
        printf("OP25: Compare Less with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            if(ACC < pullImmediate(P0 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 1:
            if(ACC < pullImmediate(P1 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 2:
            if(ACC < pullImmediate(P2 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 3:
            if(ACC < pullImmediate(P3 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
    }

    free(PX);
}

void OP26(char *regs, struct PCB *current) {
    if (DEBUG)
        printf("OP26: Compare Greater with Register Addressing\n");
    char *PX = (char *) malloc(2 * sizeof(char));

    memcpy(PX, regs, 2 * sizeof(char));

    switch(regHelper(PX)) {
        case 0:
            if(ACC > pullImmediate(P0 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 1:
            if(ACC > pullImmediate(P1 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 2:
            if(ACC > pullImmediate(P2 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
        case 3:
            if(ACC > pullImmediate(P3 + current->BaseRegister))
                PSW[0] = 'T';
            else
                PSW[0] = 'F';
            break;
    }

    free(PX);
}

void OP27(char *data) {
    if (DEBUG)
        printf("OP27: Compare Greater with Immediate Addressing\n");

    if(ACC > fourByteHelper(data))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';
}

void OP28(char *data) {
    if (DEBUG)
        printf("OP28: Compare Equal with Immediate Addressing\n");

    if(ACC == fourByteHelper(data))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';
}

void OP29(char *data) {
    if (DEBUG)
        printf("OP29: Compare Less with Immediate Addressing\n");

    if(ACC < fourByteHelper(data))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';
}

void OP30(char *regs) {
    if (DEBUG)
        printf("OP30: Compare Equal with Register\n");
    char *RX = (char *) malloc(2 *sizeof(char));

    if(ACC == *getRX(RX))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';

    free(RX);
}

void OP31(char *regs) {
    if (DEBUG)
        printf("OP31: Compare Less with Register\n");
    char *RX = (char *) malloc(2 *sizeof(char));

    if(ACC < *getRX(RX))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';

    free(RX);
}

void OP32(char *regs) {
    if (DEBUG)
        printf("OP32: Compare Greater with Register\n");
    char *RX = (char *) malloc(2 *sizeof(char));

    if(ACC > *getRX(RX))
        PSW[0] = 'T';
    else
        PSW[0] = 'F';
    
    free(RX);
}

void OP33(char *data, struct PCB *current) {
    if (DEBUG)
        printf("OP33: Branch True Conditional\n");

    if(PSW[0] == 'T')
        PC = twoByteHelper(0, data) + current->BaseRegister;
}

void OP34(char *data, struct PCB *current) {
    if (DEBUG)
        printf("OP34: Branch False Conditional\n");

    if(PSW[0] == 'F')
        PC = twoByteHelper(0, data) + current->BaseRegister;
}

void OP35(char *data, struct PCB *current) {
    if (DEBUG)
        printf("OP35: Branch Unconditional\n");

    PC = twoByteHelper(0, data) + current->BaseRegister;
}

void segmentationFault(int address, struct PCB *current) {
    address += current->BaseRegister;
    if(address > current->LimitRegister || address < current->BaseRegister) {
        printf("Address %d is outside of memory space [%d -> %d]!\n", address, current->BaseRegister, current->LimitRegister);
        PC = -1;
    }
}

//A massive switch-case separated to make it easier to look at the functions of each opcodes
void opcodeHandler(int OPcode, char* data, struct PCB *current) {
    switch(OPcode) {
        case 0:
            PC++;
            if (DEBUG)
                printIR();
            OP00(data);
            break;
        case 1:
            PC++;
            if (DEBUG)
                printIR();
            OP01(data);
            break;
        case 2:
            PC++;
            if (DEBUG)
                printIR();
            OP02(data);
            break;
        case 3:
            PC++;
            if (DEBUG)
                printIR();
            OP03(data);
            break;
        case 4:
            PC++;
            if (DEBUG)
                printIR();
            OP04(data, current);
            break;
        case 5:
            PC++;
            if (DEBUG)
                printIR();
            OP05(data, current);
            break;
        case 6:
            PC++;
            if (DEBUG)
                printIR();
            OP06(data, current);
            break;
        case 7:
            PC++;
            if (DEBUG)
                printIR();
            OP07(data, current);
            break;
        case 8:
            PC++;
            if (DEBUG)
                printIR();
            OP08(data, current);
            break;
        case 9:
            PC++;
            if (DEBUG)
                printIR();
            OP09(data, current);
            break;
        case 10:
            PC++;
            if (DEBUG)
                printIR();
            OP10(data, current);
            break;
        case 11:
            PC++;
            if (DEBUG)
                printIR();
            OP11(data, current);
            break;
        case 12:
            PC++;
            if (DEBUG)
                printIR();
            OP12(data);
            break;
        case 13:
            PC++;
            if (DEBUG)
                printIR();
            OP13(data);
            break;
        case 14:
            PC++;
            if (DEBUG)
                printIR();
            OP14(data);
            break;
        case 15:
            PC++;
            if (DEBUG)
                printIR();
            OP15(data);
            break;
        case 16:
            PC++;
            if (DEBUG)
                printIR();
            OP16(data);
            break;
        case 17:
            PC++;
            if (DEBUG)
                printIR();
            OP17(data);
            break;
        case 18:
            PC++;
            if (DEBUG)
                printIR();
            OP18(data);
            break;
        case 19:
            PC++;
            if (DEBUG)
                printIR();
            OP19(data);
            break;
        case 20:
            PC++;
            if (DEBUG)
                printIR();
            OP20(data, current);
            break;
        case 21:
            PC++;
            if (DEBUG)
                printIR();
            OP21(data, current);
            break;
        case 22:
            PC++;
            if (DEBUG)
                printIR();
            OP22(data, current);
            break;
        case 23:
            PC++;
            if (DEBUG)
                printIR();
            OP23(data, current);
            break;
        case 24:
            PC++;
            if (DEBUG)
                printIR();
            OP24(data, current);
            break;
        case 25:
            PC++;
            if (DEBUG)
                printIR();
            OP25(data, current);
            break;
        case 26:
            PC++;
            if (DEBUG)
                printIR();
            OP26(data, current);
            break;
        case 27:
            PC++;
            if (DEBUG)
                printIR();
            OP27(data);
            break;
        case 28:
            PC++;
            if (DEBUG)
                printIR();
            OP28(data);
            break;
        case 29:
            PC++;
            if (DEBUG)
                printIR();
            OP29(data);
            break;
        case 30:
            PC++;
            if (DEBUG)
                printIR();
            OP30(data);
            break;
        case 31:
            PC++;
            if (DEBUG)
                printIR();
            OP31(data);
            break;
        case 32:
            PC++;
            if (DEBUG)
                printIR();
            OP32(data);
            break;
        case 33:
            PC++;
            if (DEBUG)
                printIR();
            OP33(data, current);
            break;
        case 34:
            PC++;
            if (DEBUG)
                printIR();
            OP34(data, current);
            break;
        case 35:
            PC++;
            if (DEBUG)
                printIR();
            OP35(data, current);
            break;
        case 99:
            if (DEBUG) {
                printf("Program Exit: PID = %d\n", current->PID);
                printf("ACC: %d\nP0: %d, P1: %d, P2: %d, P3: %d\nR0: %d, R1: %d, R2: %d, R3: %d\n", ACC, P0, P1, P2, P3, R0, R1, R2, R3);
            }
            PC = -1;
            break;
        default:
            PC = -1;
            printf("UNEXPECTED OPCODE HIT WITH IR OF:\n");
            printIR();
            break;
    }
}

//Loads the PCB with all current registers, accumulator, etc
void loadPCB(struct PCB *temp) {
    temp->ACC = ACC;
    temp->R0 = R0;
    temp->R1 = R1;
    temp->R2 = R2;
    temp->R3 = R3;
    temp->P0 = P0;
    temp->P1 = P1;
    temp->P2 = P2;
    temp->P3 = P3;
    temp->PC = PC;
    temp->OPcode = OPcode;
    memcpy(temp->IR, IR, sizeof(char) * 6);
    memcpy(temp->PSW, PSW, sizeof(char) * 2);
}

//Unloads all values of the PCB into the registers
void unloadPCB(struct PCB *temp) {
    ACC = temp->ACC;
    R0 = temp->R0;
    R1 = temp->R1;
    R2 = temp->R2;
    R3 = temp->R3;
    P0 = temp->P0;
    P1 = temp->P1;
    P2 = temp->P2;
    P3 = temp->P3;
    PC = temp->PC;
    OPcode = temp->OPcode;
    memcpy(IR, temp->IR, sizeof(char) * 6);
    memcpy(PSW, temp->PSW, sizeof(char) * 2);
}

//Prints every member of the queue while the next PCB isn't NULL
void printQueue(struct PCB *temp) {
    if(temp->next_PCB == 0) {
        printf("PID: %d\n", temp->PID);
        return;
    } else {
        printf("PID: %d, ", temp->PID);
        printQueue(temp->next_PCB);
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

    int place = 100;

    //Reads the first 6-bytes of the input line and load it into memory
    while(1) {
        //If we're out of arguments, try the next set of instructions
        if(ret <= 0) {
            //If place exceeds the maximum number of memory address we have, then we have a problem
            if (place > 1000) {
                printf("Error! Number of programs exceed memory space!");
                return -1;
            }

            //Program line is reset and the baseRegister for this set of instructions is loaded in
            program_line = 0;
            program_line += place;
            place += 100;

            //Open and read the next set of instructions
            fd = open(*(++argv), O_RDONLY);
            ret = read(fd, input, 7);

            //If the file descriptor is less than zero, then we're done
            if(fd < 0) {
                break;
            }
        }

        //Copy this line of text into our memory
        for(int i = 0; i < 6; i++) {
            memory[program_line][i] = input[i];
        }

        //Get the next line and inc program line
        ret = read(fd, input, 7);

        program_line++;
    }

    //Initializing the head and tail pointers for the circular queue
    struct PCB *head = (struct PCB*) malloc(sizeof(struct PCB)), *tail = (struct PCB*) malloc(sizeof(struct PCB));;

    //For every 100 lines of memory times the number of programs inputted, set up a process control block
    for(int i = 0; i < 100 * (argc - 1); i += 100) {
        //We need a temp pointer to hold the created PCB
        struct PCB *temp = (struct PCB*) malloc(sizeof(struct PCB));

        //Initializing all params
        temp->PID = i / 100;
        temp->PC = i;
        temp->next_PCB = 0;
        temp->IC = 10;
        temp->BaseRegister = i;
        temp->LimitRegister = i + 99;
        temp->R0 = 0;
        temp->R1 = 0;
        temp->R2 = 0;
        temp->R3 = 0;
        temp->P0 = 0;
        temp->P1 = 0;
        temp->P2 = 0;
        temp->P3 = 0;
        temp->ACC = 0;

        //If we're at zero, this is the first process, head and tail both point to temp
        //otherwise, we want to set tail's next to temp and tail to temp
        if (i == 0) {
            head = temp;
            tail = temp;
        } else {
            tail->next_PCB = temp;
            tail = temp;
        }
    }

    //Debug to look at the queue that was created
    if (DEBUG)    
        printQueue(head);

    int increment = 0;

    //Keep incrementing the PC until the opcode is the halt command
    for(PC = 0; head != 0; increment++){
        //PC will only be -1 if the OP code is 99 or invalid, in either case, we should exit
        if(PC == -1) {
            //Creates a temporary variable to potentially hold head
            struct PCB *temp = (struct PCB*) malloc(sizeof(struct PCB));

            //If head and tail aren't equal, then we want to set head to the next PCB and to free temp
            if (head != tail) {
                temp = head;
                head = head->next_PCB;
            } else {
                //Head will be freed if it's equal to tail
                free(head);
                break;
            }

            //Frees the pcb of the temp process
            free(temp);
            
            //At this point, head has been incremented to next, so we need to unload it
            increment = 0;
            unloadPCB(head);
        }

        //if IC is hit, we need to move onto the next PCB
        //tail->next_PCB = head, tail = tail->next_PCB, head = head->next_PCB, tail->next_PCB = 0
        //Just a circular queue (unintentional) but it doesn't point back to the front
        if(increment == head->IC) {
            increment = 0;
            loadPCB(head);

            tail->next_PCB = head;
            tail = tail->next_PCB;
            head = head->next_PCB;
            tail->next_PCB = 0;

            unloadPCB(head);

            //printQueue(head);
        }
        //Load instruction into the IR
        memcpy(IR, memory[PC], 6 * sizeof(char));
        
        //printf("PC: %d\n", PC);

        //Add the first two bytes of the IR to the OPcode
        OPcode = ((int) (IR[0] - 48) * 10) + (int) (IR[1] - 48);
        char hold[4];

        memcpy(hold, &IR[2], 4*sizeof(char));

        //printf("%d\n", OPcode);
        opcodeHandler(OPcode, hold, head);

        if (DEBUG)
            printf("ACC: %d PC: %d\n", ACC, PC);
    }

    //Uncomment to see memory output
    
    char *helper = (char *) malloc(6 * sizeof(char));
    
    for(int i = 0; i < 100 * (argc - 1); i++) {
        memcpy(helper, memory[i], 6 * sizeof(char));
        printf("Memory %2d: %6s\n", i, helper);
    }

    return 0;
}