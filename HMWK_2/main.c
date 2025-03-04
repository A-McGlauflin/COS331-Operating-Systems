#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* 
    DONE ON A MAC DEVICE, INPUT IS SET TO 7

    Code by: Aiden McGlauflin

*/

//Creating necessary variables for mimicry
char IR[6], PSW[2], memory[100][6];
short int PC = 0, P0, P1, P2, P3;
int R0, R1, R2, R3, ACC, OPcode;

//The only time a 4-byte immediate will need to be converted to a string is if it's going into memory
//Therefore, we should include ZZ at the beginning
char* immToString(int imm) {
    char* data = malloc(6 * sizeof(char));

    *data  = 'Z';
    *(data + sizeof(char)) = 'Z';

    for(int i = 0; i < 4; i++) {
        *(data + (5 - i) * sizeof(char)) = (char) ((imm % 10) + 48);
        imm /= 10;
    }

    //It breaks when I delete this print statement...
    return data;
}

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

int pullImmediate(int address) {
    char* data = malloc(4 * sizeof(char));
    memcpy(data, &memory[address][2], 4*sizeof(char));

    return fourByteHelper(data);
}

void pushImmediate(int address, int data) {
    memcpy(memory[address], immToString(data), 6*sizeof(char));
}

//RX is a register, not a perscription
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
            //This is dumb, but I don't know what else to return
            return &R0;
    }
}

void printIR() {
    printf("Current IR: %s\n", IR);
}

void OP00(char* pReg) {
    printf("OP00: Load Pointer with an Immediate\n");
    int data = twoByteHelper(2, pReg);
    char *PX = malloc(2 * sizeof(char));

    memcpy(PX, pReg, 2*sizeof(*pReg));

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

void OP01(char* pReg) {
    printf("OP01: Add to Pointer an Immediate\n");
    int data = twoByteHelper(2, pReg);
    char *PX = malloc(2 * sizeof(char));
    
    memcpy(PX, &pReg[0], 2*sizeof(*pReg));

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

void OP02(char* pReg) {
    printf("OP02: Sub from Pointer an Immediate\n");
    int data = twoByteHelper(2, pReg);
    char *PX = malloc(2 * sizeof(char));
    memcpy(PX, &pReg[0], 2*sizeof(*pReg));
    
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
    char *PX = malloc(2 * sizeof(char));
    memcpy(PX, &pReg[0], 2*sizeof(*pReg));

    switch(regHelper(PX)) {
        case 0:
            ACC = pullImmediate(P0);
            break;
        case 1:
            ACC = pullImmediate(P1);
            break;
        case 2:
            ACC = pullImmediate(P3);
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
    char *PX = malloc(2 * sizeof(char));
    memcpy(PX, &pReg[0], 2*sizeof(*pReg));

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
    char *RX = malloc(2 * sizeof(char)), *PX = malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2*sizeof(char));
    memcpy(PX, &regs[2], 2*sizeof(char));

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
    char *RX = malloc(2 * sizeof(char));
    memcpy(RX, &data[0], 2*sizeof(char));

    pushImmediate(twoByteHelper(2, data), *getRX(RX));

    free(RX);
}

void OP10(char* regs) {
    printf("OP10: Load Register with Register Addressing, regs: %s\n", regs);
    char *RX = malloc(2 * sizeof(char)), *PX = malloc(2 * sizeof(char));
    
    memcpy(RX, regs, 2*sizeof(*regs)); 
    memcpy(PX, &regs[2], 2*sizeof(*regs));

    switch(regHelper(PX)) {
        case 0:
            printf("AFTER REG HELPER\n");
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
    char *RX = malloc(2 * sizeof(char)), *PX = malloc(2 * sizeof(char));

    memcpy(RX, &data[0], 2*sizeof(char));

    *getRX(RX) = twoByteHelper(2, data);

    free(PX);
    free(RX);
}

void OP12(char* data) {
    printf("OP12: Load R0 with Immediate\n");

    R0 = fourByteHelper(data);
}

void OP13(char* regs) {
    printf("OP13: Load Register with Other Register\n");
    //RX <- RY
    char *RX = malloc(2 * sizeof(char)), *RY = malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2*sizeof(char));
    memcpy(RY, &regs[2], 2*sizeof(char));

    *getRX(RX) = *getRX(RY);

    free(RX);
    free(RY);
}

void OP14(char* regs) {
    printf("OP14: Load ACC from Register\n");
    char *RX = malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2*sizeof(char));

    ACC = *getRX(RX);

    free(RX);
}

void OP15(char* regs) {
    printf("OP15: Load Register from ACC\n");
    char *RX = malloc(2 * sizeof(char));

    memcpy(RX, &regs[0], 2*sizeof(char));

    *getRX(RX) = ACC;

    free(RX);
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
        case 99:
            printf("ACC: %d, P0: %d, P1: %d, R3: %d, R2: %d\n", ACC, P0, P1, R3, R2);
            PC = -1;
            break;
        default:
            PC = -1;
            //printIR();
            //printf("UNEXPECTED OPCODE HIT!\n");
            break;
    }
}

int main() {
    //input buffer array for reading from file
    char input[7];

    //File descriptor, ret, and program line are created
    int fd = open("project1Program", O_RDONLY),
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
        for(int i = 0; i < 6; i++)
            IR[i] = memory[PC][i];
        
        //printf("PC: %d\n", PC);

        //Add the first two bytes of the IR to the OPcode
        OPcode = ((int) (IR[0] - 48) * 10) + (int) (IR[1] - 48);
        char hold[4];

        memcpy(hold, &IR[2], 4*sizeof(char));

        //printf("%d\n", OPcode);
        opcodeHandler(OPcode, hold);
    }

    return 0;
}