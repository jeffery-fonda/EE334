#define HEADER_H
#ifndef HEADER_H

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define s1 0
#define s2 1
#define s3 2
#define s4 3
#define InitialStage s1 //change initial stage
#define stateMachine 0 //0 for normal state machine (given in class), 1 for assign state machine

struct btb {
	//int entry; //range from 0 to 511 or 0 to 1023
	int pc;
	int target;
	int prediction;
};

typedef struct btb BTB;

int indexing(char pc[], int mode);
int check_for_branching(int pc, int next_pc);
void initial(BTB branch_target_buffer[2048]);
int prediction_state_machine(int is_correct, int current_state);
//int predict_pc(int prediction);


#endif