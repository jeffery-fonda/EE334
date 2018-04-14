#include "header.h"

//determine the index (entry#) of the PC
int indexing(char pc[], int mode)
{
	char index_2048[4];
	index_2048[0] = pc[2];
	index_2048[1] = pc[3];
	index_2048[2] = pc[4];
	index_2048[3] = pc[5];
	char index[3];
	index[0] = pc[3];
	index[1] = pc[4];
	index[2] = pc[5];
	int returnIndex = 0;
	if (mode == 1)//256 entries
	{
		returnIndex = (int)((strtol(index, NULL, 16) % 1024) >> 2);//take the last 10-bits then shift right by 2 bits to calculate index
	}
	else if (mode == 2)//512 entries
	{
		returnIndex = (int)((strtol(index, NULL, 16) % 2048) >> 2);//take the last 11-bits then shift right by 2 bits to calculate index
	}
	else if (mode == 3)//1024 entries
	{
		returnIndex = (int)(strtol(index, NULL, 16) >> 2);//take the last 12-bits then shift right by 2 bits to calculate index
	}
	else //2048 entries
	{
		returnIndex = (int)((strtol(index_2048, NULL, 16) % 8192) >> 2); //take the last 13 bits then shift right by 2 bits to calculate index
	}
	return returnIndex;

}

//see if there is a branch
int check_for_branching(int pc, int next_pc)
{
	if (pc + 4 != next_pc)
		return 1;//branch
	else
		return 0;//no branch
}

//intialize BTB
void initial(BTB branch_target_buffer[2048])
{
	for (int i = 0; i < 2048; i++)
	{
		branch_target_buffer[i].pc = 0;
		branch_target_buffer[i].target = 0;
		branch_target_buffer[i].prediction = 0;
	}
}

//get the next state based on whether the branch taken or not and the current state
int prediction_state_machine(int is_correct, int current_state)
{
	int new_state = 0;
	if (stateMachine == 0) //state machine given in class
	{
		if (current_state == s1)
		{
			new_state = (is_correct == 1) ? s1 : s2;
		}
		else if (current_state == s2)
		{
			new_state = (is_correct == 1) ? s1 : s3;
		}
		else if (current_state == s3)
		{
			new_state = (is_correct == 1) ? s2 : s4;
		}
		else
		{
			new_state = (is_correct == 1) ? s3 : s4;
		}
	}
	else //assign state machine
	{
		if (current_state == s1)
		{
			new_state = (is_correct == 1) ? s1 : s2;
		}
		else if (current_state == s2)
		{
			new_state = (is_correct == 1) ? s1 : s3;
		}
		else if (current_state == s3)
		{
			new_state = (is_correct == 1) ? s1 : s4;
		}
		else
		{
			new_state = (is_correct == 1) ? s3 : s4;
		}
	}

	return new_state;
}

