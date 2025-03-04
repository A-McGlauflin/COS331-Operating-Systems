#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

/* 
    DONE ON A MAC DEVICE, INPUT IS SET TO 7
    Buffer length of 7 did not work, but setting it to 8 did

    Code by: Aiden McGlauflin

*/

//Creating necessary variables for mimicry
char IR[6], PSW[2], memory[100][6];
short int PC = 0, P0, P1, P2, P3;
int R0, R1, R2, R3, ACC, OPcode;

int main() {
    //input buffer array for reading from file
    char input[7];

    //File descriptor, ret, and program line are created
    int fd = open("testProcess", O_RDONLY),
    ret = read(fd, input, 8),
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
        ret = read(fd, input, 8);

        program_line++;
    }

    //Keep incrementing the PC until the opcode is the halt command
    for(PC = 0; OPcode != 99; PC++){
        //Load instruction into the IR
        for(int i = 0; i < 6; i++)
            IR[i] = memory[PC][i];
        
        //Add the first two bytes of the IR to the OPcode
        OPcode = ((int) (IR[0] - 48) * 10) + (int) (IR[1] - 48);

        //Debug to see if it's the right opcode
        printf("Current OPcode: %d\n", OPcode);
    }

    return 0;
}