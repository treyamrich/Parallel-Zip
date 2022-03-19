#include <iostream>
#include <stdio.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <queue>
#include <vector>
#include <string>
using namespace std;

struct zdata {
	char c;
	int count;
};
struct task {
	string fname;
	int sem_num;
};

void* complete_task(void*);
void wzip(string& fname, vector<zdata>& zipped_data);

//Get thread resources, create task buffer, create semaphores
int cpu_resources = get_nprocs(); 
queue<task> buf;
sem_t mutex, empty;
sem_t* order_sems = nullptr;

int main(int argc, char* argv[]) {

	if(argc > 1) {

		//Get # of threads
		if(cpu_resources < 5) 
			cpu_resources = 5; 
		pthread_t tpool[cpu_resources];

		//Init semaphores/tasks and create threads
		sem_init(&mutex, 0, 1);
		sem_init(&empty, 0, 0);
		order_sems = new sem_t[cpu_resources];
		for(int i = 0; i < cpu_resources; i++) {
			sem_init(&order_sems[i], 0, 0);
			pthread_create(&tpool[i], NULL, complete_task, NULL);
		}

		//Producer thread - Add tasks to buffer
		for(int j = 1; j < argc + cpu_resources; j++) {
			task t; 
			if(j < argc) {
				t.fname = argv[j];
				t.sem_num = j-1;
			} else {
				t.sem_num = -1; //Denotes a bogus task
			}
			buf.push(t);
			sem_post(&empty); //Wake up consumers
		}

		//Join with all threads
		for(int k = 0; k < cpu_resources; k++)
			pthread_join(tpool[k], NULL);

		//Destroy all semaphores
		sem_destroy(&mutex);
		sem_destroy(&empty);
		for(int x = 0; x < cpu_resources; x++)
			sem_destroy(&order_sems[x]);
		delete [] order_sems;
	} else {
		cout << "pzip: file1 [file2...]\n";
		exit(1);
	}
	return 0;
}

/*Pre-conditions: The task's file should exist, otherwise an error is 
	printed and output is corrupted. If there are n threads, n tasks with a 
	task.sem_num should be added to the buffer, to allow sleeping threads to exit.
Post-conditions: Thread function which acts as a consumer to the unbounded 
	buffer of the global queue. Extracts task from queue and calls wzip(); 
	then waits for the thread of task number n-1. Once it's the threads turn, 
	it traverses the vector to print to stdout. Then wakes up the next thread.*/
void* complete_task(void* arg) {

	while(true) {
		sem_wait(&empty); //Check if buffer is empty
		//Critical section is just the queue
		sem_wait(&mutex);
		task t = buf.front();
		buf.pop();
		sem_post(&mutex);

		//If bogus task -> exit
		if(t.sem_num == -1)
			break;

		//Create zip data vector, and pass file name to zip it
		vector<zdata> output;
		wzip(t.fname, output);

		//If not first task, wait for task n-1 to print to stdout
		if(t.sem_num > 0)
			sem_wait(&order_sems[t.sem_num]);

		//If the file: opened, mapped and zipped data -> print binary data to stdout
		if(output.size() > 0) {
			for(vector<zdata>::iterator it = output.begin(); it != output.end(); ++it) {
				cout.write((char*) &((*it).count), sizeof(int));
				cout.write(&((*it).c), sizeof(char));
			}
		}

		//Wake up next thread if not last
		if(t.sem_num < cpu_resources - 1)
			sem_post(&order_sems[t.sem_num + 1]);
	}
	pthread_exit(0);
}

/*Pre-conditions: The task's file should exist, otherwise an error is 
	printed and output is corrupted.
Post-conditions: Opens the file with the given file name, which is then 
	mapped to memory. Performs run-length encoding on the file, which
	stores the data in a vector that holds the struct 'zdata'. */
void wzip(string& fname, vector<zdata>& zipped_data) {

	FILE* f = fopen(fname.c_str(), "r");
	if(!f) {
		cout << "pzip: cannot open file\n";
      	return;
	}
	//mmap file then close file
	fseek(f, 0, SEEK_END);
	int fsize = ftell(f);
	int fd = fileno(f);
	char* mfile = nullptr;
	mfile = (char*) mmap(NULL, fsize, PROT_READ, MAP_PRIVATE, fd, 0);
	fclose(f);
	
	//Zip file
	int count = 0;
	char prevChar;
	for(int i = 0; i < fsize; i++) {
		//If the current character is the same as the previous or if this is the first iteration, increment count by 1
		if(count == 0 || mfile[i] == prevChar) {
			count++;
		} else {
			//Store zipped data in vector
			zdata z;
			z.count = count;
			z.c = prevChar;
			zipped_data.push_back(z);
			count = 1;
		}
		prevChar = mfile[i];
	}
	//Edge cases: last distinct char and single char file
	if(count != 0) {
		zdata z;
		z.count = count;
		z.c = prevChar;
		zipped_data.push_back(z); 
	}
}