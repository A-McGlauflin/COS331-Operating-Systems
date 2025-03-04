# COS331: Operating Systems

This repository is a collection of all assignments from my operating systems class taken during the fall semester of 2024. All assignments use C and build on the last to make an OS that runs PBRAIN12 assignments.

## Hardware Mimicry:

**Memory:**

Memory was mimiced using a char\* array with 6-byte words. The initial assignments in the first phase of the project was 100 words long, this later changed to 1000 words.

**Instruction Register and Process Status Word:**

The instruction register is a char array of one six-byte word. The process status word is a two-byte char array that holds the status of comparisons with either a 'T' or a 'F'.

**Program Counter and Pointer Registers:**

The program counter and pointer registers are represented by short ints. Because all other registers are represented by ints, memory needs to be converted from char arrays to integers.

**Accumulator and Data Registers:**

The accumulator and data registers are represented by ints to hold the full 4-bytes that a number can represent in memory.

**Process Control Blocks:**

Process control blocks (PCB) are represented through a C struct. The PCB holds all the register information of a process while it isn't running. PCBs also hold five additional elements:
-   PID: the process ID of the PCB
-   Base and Limit Register: The base and limit memory address of a specific process in memory.
-   Instruction Count: The number of instructions that can execute before the process switches.
-   Next PCB Pointer: A pointer to the next PCB. Necessary to create a ready queue.

**Semaphores:**

Semaphores, like PCBs, are represented through a C struct. The semaphore contains an integer to hold the number of processes that can access the shared resource. It also contains PCB pointers for the head and tail of the block-queue. 

## Installation:

1. Download the [repository](https://github.com/A-McGlauflin/COS331-Operating-Systems/archive/refs/heads/main.zip) or run `git clone https://github.com/A-McGlauflin/COS331-Operating-Systems.git` if git is installed.
2. As this assignment was done on a MacOS device, make sure any code run is from a UNIX environment as new lines are `\n` as opposed to `\r\n` on Windows devices.

## PBRAIN12:

PBRAIN12 is an assembly language. A helpful table can be found [here](https://github.com/Taidgh-Robinson/PBRAIN12/blob/master/PBRAIN_instructions_reference.Mod-1.pdf) from Taidgh Robinson's PBRAIN12 repository. Our class had three additional opcodes:
-   Trap Instruction (Opcode 36): Takes two data registers as operands, the first dictates the instruction performed, and the second dictates what's affected. The first register can be '0' for wait, '1' for signal, and '2' for getPID. If the first is '0' or '1', then the second register will dictate what type of semiphore is affected, otherwise it will be the register that holds the PID.
-   Modulo (Opcode 37): Takes two data registers as operands and performs modulo on the first as the dividend and the second as the divisor. The result of this operation is stored in the accumulator.
-   No-op (Opcode 38): Performs no operation.

## Assignment 1:

**About:**

The goal of the first assignment was to create an interpreter for a PBRAIN12 file. This assignment had us implement mimics of the hardware that would run PBRAIN12 code.

**Implemented:**

-   The registers, memory, and other hardware.
-   The main function. This includes:
    -   A while-loop to read the document
    -   A for-loop to go over memory until the halt instruction is read
    
## Assignment 2:

**About:**

The second assignment tasked us with implementing opcodes 0-15 and 99.

**Implemented:**

-   Helper Functions:
    -   `immToString(int imm)` takes an immediate and converts it a string to be stored in memory.
    -   `foutByteHelper(char* data)` takes a four-byte char array and converts it to an integer.
    -   `twoByteHelper(int position, char* data)` takes the position (either 0 for the first two bytes, or 1 for the second two) and converts it to an integer.
    -   `regHelper(char* reg)` takes a two-byte char array and converts it into 0-3 for the respective register.
    -   `pullImmediate(int address)` pulls the immediate stored at `address`.
    -   `pushImmediate(int address, int data)` pushes `data` into memory at the address of `address`.
    -   `getRX(char* RX)` takes the data register and returns a pointer to the register.
    -   `printIR()` prints the current instruction register.
-   Opcodes 0-15, and 99. The implementation uses pointers and memcpy to handle breaking the data down. The specifics of PBRAIN12 can be found [here](#pbrain12)
-   The opcode handler which takes in an integer and data and runs a switch-case to each opcode.

**Changes:**

-   The for-loop now doesn't increment the PC and calls `opcodeHandler`

## Assignment 3:

**About:**

The third assignment implemented the remaining opcodes
