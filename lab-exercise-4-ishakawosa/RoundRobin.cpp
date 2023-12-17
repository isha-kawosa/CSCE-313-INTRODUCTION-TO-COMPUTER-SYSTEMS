#include "RoundRobin.h"
#include <queue>
#include <algorithm>

/*
This is a constructor for RoundRobin Scheduler, you should use the extractProcessInfo function first
to load process information to process_info and then sort process by arrival time;

Also initialize time_quantum
*/
RoundRobin::RoundRobin(string file, int time_quantum) : time_quantum(time_quantum) {
    extractProcessInfo(file);
    // sort(processVec.begin(), processVec.end(), [](Process a, Process b){ 
    //     return a.get_arrival_time() < b.get_arrival_time(); 
    // });
    sort(processVec.begin(), processVec.end(), [](const Process &a, const Process &b) {
        return a.get_arrival_time() < b.get_arrival_time();
    });
}

// Schedule tasks based on RoundRobin Rule
// the jobs are put in the order the arrived
// Make sure you print out the information like we put in the document

void RoundRobin::schedule_tasks() {
    std::queue<Process*> processQueue; 
    int system_time = 0;
    std::vector<Process>::size_type process_index = 0;

    // Enqueue processes that have arrived by now
    while (process_index < processVec.size() && processVec[process_index].get_arrival_time() <= system_time) {
        processQueue.push(&processVec[process_index]);
        process_index++;
    }
    while (processQueue.empty()) {
        system_time++;
        while (process_index < processVec.size() && processVec[process_index].get_arrival_time() <= system_time) {
            processQueue.push(&processVec[process_index]);
            process_index++;
        }
    }

    while (!processQueue.empty()) {

        Process *current_process = processQueue.front();
        processQueue.pop();
        current_process->Run(0);
        if (current_process->is_Completed()) {
            print(system_time, current_process->getPid(), current_process->is_Completed());
            continue;
        }

        for (int i = 0; i < time_quantum; i++) {
            //if(current_process->get_remaining_time()==1){
            print(system_time, current_process->getPid(), current_process->is_Completed());
            current_process->Run(1);
            if (current_process->is_Completed()) {
                system_time++;
                print(system_time, current_process->getPid(), current_process->is_Completed());
                break;
            }
            //print(system_time, current_process->getPid(), current_process->is_Completed());
            system_time++;
            
            

            // check for new arrivals during this quantum and enqueue them
            while (process_index < processVec.size() && processVec[process_index].get_arrival_time() <= system_time) {
                processQueue.push(&processVec[process_index]);
                process_index++;
            }
        }
        while (process_index < processVec.size() && processVec[process_index].get_arrival_time() <= system_time) {
            processQueue.push(&processVec[process_index]);
            process_index++;
        }
        if (!current_process->is_Completed()) {
            processQueue.push(current_process); // if the current process isn't completed, requeue it
        }
        while (processQueue.empty() && process_index < processVec.size()) {
            print(system_time, -1, false); // NOP 
            system_time++;
            while (process_index < processVec.size() && processVec[process_index].get_arrival_time() <= system_time) {
                processQueue.push(&processVec[process_index]);
                process_index++;
            }
        }
    }
}


/*************************** 
ALL FUNCTIONS UNDER THIS LINE ARE COMPLETED FOR YOU
You can modify them if you'd like, though :)
***************************/


// Default constructor
RoundRobin::RoundRobin() {
	time_quantum = 0;
}

// Time quantum setter
void RoundRobin::set_time_quantum(int quantum) {
	this->time_quantum = quantum;
}

// Time quantum getter
int RoundRobin::get_time_quantum() {
	return time_quantum;
}

// Print function for outputting system time as part of the schedule tasks function
void RoundRobin::print(int system_time, int pid, bool isComplete){
	string s_pid = pid == -1 ? "NOP" : to_string(pid);
	cout << "System Time [" << system_time << "].........Process[PID=" << s_pid << "] ";
	if (isComplete)
		cout << "finished its job!" << endl;
	else
		cout << "is Running" << endl;
}

// Read a process file to extract process information
// All content goes to proces_info vector
void RoundRobin::extractProcessInfo(string file){
	// open file
	ifstream processFile (file);
	if (!processFile.is_open()) {
		perror("could not open file");
		exit(1);
	}

	// read contents and populate process_info vector
	string curr_line, temp_num;
	int curr_pid, curr_arrival_time, curr_burst_time;
	while (getline(processFile, curr_line)) {
		// use string stream to seperate by comma
		stringstream ss(curr_line);
		getline(ss, temp_num, ',');
		curr_pid = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_arrival_time = stoi(temp_num);
		getline(ss, temp_num, ',');
		curr_burst_time = stoi(temp_num);
		Process p(curr_pid, curr_arrival_time, curr_burst_time);

		processVec.push_back(p);
	}

	// close file
	processFile.close();
}
