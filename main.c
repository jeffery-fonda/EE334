#include "header.h"
#include "functions.c"

int main(void)
{
	//declaration
	//the index of this array will be use as the index in BTB
	BTB branch_target_buffer[2048];
	int mode = 0, index = 0, table_entries = 0;
	char pc_c[8];
	char next_pc_c[8];
	//char next_next_pc_c[8];
	int pc = 0, next_pc = 0, predict_pc = 0;
	int hits = 0, misses = 0, instruction_count = 0, correct_prediction = 0, incorrect_prediction = 0, collision = 0, overallStalls = 0, noBTBStalls = 0;
	int takenBranchWithoutPrediction = 0;
	int toWrongAddress = 0;
	int is_branching = 0;
	FILE *input = NULL, *output = NULL, *log = NULL;

	//initialize the BTB
	initial(branch_target_buffer);

	//ask for BTB size
	while (mode != 1 && mode != 2 && mode != 3 && mode != 4)
	{
		printf("Please choose an option:\n1. BTB with 256 entries\n2. BTB with 512 entries\n3. BTB with 1024 entries\n4. BTB with 2048 entries\n>> ");
		scanf("%d", &mode);
		if (mode != 1 && mode != 2 && mode != 3 && mode != 4)
			printf("Please choose one of those option (1 or 2)\n");
	}
	//determine the size of the BTB
	if (mode == 1)
		table_entries = 256;
	else if (mode == 2)
		table_entries = 512;
	else if (mode == 3)
		table_entries = 1024;
	else if (mode == 4)
		table_entries = 2048;
	else
		table_entries = 1024; //fail safe

	//open files
	input = fopen("trace_sample.txt", "r");
	output = fopen("BTB.txt", "w");
	log = fopen("log.txt", "w");

	//check if file open successfully
	if (input == NULL || output == NULL || log == NULL)
	{
		printf("cannot open file!\n");
		return 0;
	}

	//first time, get two address from trace_sample.txt
	fgets(pc_c, 10, input);
	fgets(next_pc_c, 10, input);
	instruction_count = 2;//update instruction count
	//populate the BTB
	for (int i = 0; !feof(input); i++)
	{
		//convert the string to int 
		pc = (int)strtol(pc_c, NULL, 16);
		next_pc = (int)strtol(next_pc_c, NULL, 16);

		//determine index
		index = indexing(pc_c, mode);
		//This portion of the code determine the total # of taken branches without using the BTB
		//In other word without using the BTB, any taken branch will result in a stall
		if (check_for_branching(pc, next_pc) == 1)
		{
			takenBranchWithoutPrediction += 1;
			noBTBStalls += 1;//assume any taken branch will result in one stall, there is no way to check if the instruction is load word.
		}

		//determine if hit
		if (branch_target_buffer[index].pc == pc)
		{
			hits += 1;
			//determine predict_pc
			if (branch_target_buffer[index].prediction == s1 || branch_target_buffer[index].prediction == s2)
			{
				predict_pc = branch_target_buffer[index].target;
			}
			else
				predict_pc = branch_target_buffer[index].pc + 4;

			//check to see right or wrong prediction
			//correct prediction, no stall
			if (predict_pc == next_pc)
			{
				correct_prediction += 1;
				//update prediction value in BTB
				branch_target_buffer[index].prediction = prediction_state_machine(check_for_branching(pc, next_pc), branch_target_buffer[index].prediction);
			}
			//wrong prediction, stall
			else if (branch_target_buffer[index].pc + 4 == next_pc)
			{
				incorrect_prediction += 1;
				branch_target_buffer[index].prediction = prediction_state_machine(check_for_branching(pc, next_pc), branch_target_buffer[index].prediction);
				overallStalls += 1;//entry is found but wrong prediction, stall 1 cycle
			}
			//branch to a new place
			else if (check_for_branching(branch_target_buffer[index].pc, next_pc) == 1)
			{
				incorrect_prediction += 1;
				//update new value
				branch_target_buffer[index].target = next_pc;
				branch_target_buffer[index].prediction = InitialStage;//initial state
				fprintf(log, "Entry %d in the BTB branches to a new location. Replace target %x with %x\n", index, predict_pc, next_pc);
				overallStalls += 1;//entry is found but branch to a wrong PC, stall 2 cycles
				toWrongAddress += 1;
			}
		}
		//determine if miss
		else if (branch_target_buffer[index].pc != pc)
		{
			if (check_for_branching(pc, next_pc) == 1)
			{
				misses += 1;
				//determine insert new entry or collision
				if (branch_target_buffer[index].pc != 0)
				{
					collision += 1;
					fprintf(log, "Collision detected at entry %d\nReplace %x with %x at entry %d\n", index, branch_target_buffer[index].pc, pc, index);
				}
				//insert new entry
				else
				{
					fprintf(log, "Insert PC = %x at entry %d\n", pc, index);
				}
				//update entry
				branch_target_buffer[index].pc = pc;
				branch_target_buffer[index].target = next_pc;
				branch_target_buffer[index].prediction = InitialStage;//initial state
				overallStalls += 1; //when the branch is taken and the entry is not there is the BTB, stall 1 cycle
			}
			else
			{
				//normal instruction execute
			}

		}
		strcpy(pc_c, next_pc_c);
		fgets(next_pc_c, 10, input);
		instruction_count += 1;

	}

	fprintf(output, "Total instruction executed: %d\nNumber of hits: %d\nNumber of misses: %d\nNumber of correct predictions: %d\nNumber of incorrect predictions: %d\n", instruction_count, hits, misses, correct_prediction, incorrect_prediction);
	fprintf(output, "Number of branch that predicted taken but have the wrong target address: %d\n",toWrongAddress);
	fprintf(output,"<------------------------------------------------------------------->\n");
	fprintf(output, "Hit percentage: %.2lf%%\nAccuracy percentage: %.2lf%%\n", ((double)hits / (double)(hits + misses))*100, ((double)correct_prediction / (double)(hits)) * 100);
	fprintf(output, "<------------------------------------------------------------------->\n");
	fprintf(output, "Total taken branches without prediction is %d, and causing %d stalls (no way to know if a load, 1 stall for each branch)\n", takenBranchWithoutPrediction, noBTBStalls);
	fprintf(output, "Stalls using BTB: %d\nNumber of Collision: %d\n", overallStalls, collision);
	fprintf(output, "<------------------------------------------------------------------->\n");

	fprintf(output,"Entry,PC,Target PC,Prediction\n");
	for (int i = 0; i < table_entries; i++)
	{
		if (branch_target_buffer[i].pc != 0)
		{
			fprintf(output, "%d,%x,%x,", i, branch_target_buffer[i].pc, branch_target_buffer[i].target);
			if (branch_target_buffer[i].prediction == s1)
				fprintf(output, "00\n");
			else if (branch_target_buffer[i].prediction == s2)
				fprintf(output, "01\n");
			else if (branch_target_buffer[i].prediction == s3)
				fprintf(output, "10\n");
			else if (branch_target_buffer[i].prediction == s4)
				fprintf(output, "11\n");
			else
				fprintf(output, "N/A\n");


		}

		//else
			//fprintf(output, "%d,000000,000000,00\n", i);
	}
	//close files
	fclose(output);
	fclose(input);
	fclose(log);
	return 0;
}