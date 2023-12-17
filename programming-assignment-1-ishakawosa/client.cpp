/*
	Original author of the starter code
    Tanzir Ahmed
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 2/8/20
	
	Please include your Name, UIN, and the date below
	Name: Isha Kawosa
	UIN: 331006079
	Date: 9/9/2023
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <iostream>
#include <unistd.h> // For fork, execvp
#include <algorithm>
#include <sys/wait.h>
#include <chrono> //for time
using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1;
	int e = -1;
	int64_t m= MAX_MESSAGE;
	bool new_chan = false;
	vector<FIFORequestChannel*> channels;
	FIFORequestChannel* newChan = nullptr;

	string filename = "";
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) {
		switch (opt) {
			case 'p':
				p = atoi (optarg);
				break;
			case 't':
				t = atof (optarg);
				break;
			case 'e':
				e = atoi (optarg);
				break;
			case 'f':
				filename = optarg;
				break;
			case 'm':
				m = atoi (optarg);
				break;
			case 'c':
				new_chan =true;
				break;

		}
	}


	//give arguments for the server
	//server needs "./server", '-m', <val for -m arg>, 'NULL'
	//fork
	//in the child, run execvp using the server arguments

	pid_t child_process = fork();

	if(child_process==-1){
		cerr << "Fork failed" <<endl;
		exit(1);
	}
	
	if(child_process==0){
		char* server_args[]= {(char*)"./server", (char*)"-m", (char *) to_string(m).c_str(), nullptr};
		execvp(server_args[0], server_args);
		cerr<< "Server execvp failed" <<endl;
		exit(1);
	}

	else{

		FIFORequestChannel cont_chan("control", FIFORequestChannel::CLIENT_SIDE);
		channels.push_back(&cont_chan);

		if(new_chan){
			//send new channel request to server
			MESSAGE_TYPE nc = NEWCHANNEL_MSG;
			cont_chan.cwrite(&nc, sizeof(MESSAGE_TYPE));
			//create a variable to hold the name
			char channelname[MAX_MESSAGE];
			//cread the response from the server
			cont_chan.cread(&channelname, sizeof(channelname));
			cout << "Received channel name from server: " << channelname << endl;
			//call the fifo request channel constructor with the name from the server
			//how do i dynamically allocate this newChan using new keyword
			newChan = new FIFORequestChannel(channelname, FIFORequestChannel::CLIENT_SIDE);
			channels.push_back(newChan);
		}

		//using the last channel that was pushed back in the vector
		FIFORequestChannel chan = *(channels.back());

		//Single data point only when p, t, e !=-1
		// example data point request
		if(p!=-1 && t!=1 && e!=-1){
			char buf[MAX_MESSAGE]; // 256
			datamsg x(p, t, e);
			
			memcpy(buf, &x, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // question
			double reply;
			chan.cread(&reply, sizeof(double)); //answer
			cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		}

		//cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		
		//else if p!=-1, request 1000 datapoints
		//loop over 1st 1000 lines
		// send request for ecg 1 and ecg 2
		//write line to received/x1.csv

		else if(p!=-1){
			ofstream outputFile("received/x1.csv");
			for(int i=0; i<1000; i++){
				char buf[MAX_MESSAGE]; // 256
				datamsg x(p, i*0.004, 1);//requesting data point in 0.004 time increments for ecg1
				
				memcpy(buf, &x, sizeof(datamsg));
				chan.cwrite(buf, sizeof(datamsg)); // question
				double reply1;
				chan.cread(&reply1, sizeof(double)); //answer

				datamsg x2(p, i*0.004, 2);//requesting data point in 0.004 time increments for ecg2
				memcpy(buf, &x2, sizeof(datamsg));
				chan.cwrite(buf, sizeof(datamsg)); // question
				double reply2;
				chan.cread(&reply2, sizeof(double)); //answer

				double time= i*0.004;
				outputFile << time << "," << reply1 << "," << reply2 << endl;

			}
			outputFile.close();
			
		}
		// sending a non-sense message, you need to change this
		if(filename!=""){
            filemsg fm(0, 0); //sends message to server to get filesize
            string fname = filename;
            
            int len = sizeof(filemsg) + (fname.size() + 1);
            char* buf2 = new char[len];
            memcpy(buf2, &fm, sizeof(filemsg));
            strcpy(buf2 + sizeof(filemsg), fname.c_str());
            chan.cwrite(buf2, len);  // I want the file length;

            int64_t filesize =0;
            chan.cread(&filesize, sizeof(int64_t)); //reading response from server of the filesize
			cout<< "The file size is: " << filesize<<endl;
            char* buf3 = new char[m];//create buffer of size m, buff capacity

            //loop over segemnts in the file= filesize/buff capacity (m)
			//int64_t offset=0;
			
            //create filemsg instance
            filemsg* file_req = (filemsg*)buf2;
            file_req->offset= 0; //set offset in the file
            file_req->length = m; //set the length, be careful of the last length
            int64_t remaining =filesize;
            string received_filename= "received/"+fname;
			ofstream received_file(received_filename, ios::binary | ios::app);
			std::cout << "Starting File Transfer: " << std::endl; //starting clock
            auto start = chrono::steady_clock::now();
            while(remaining>0){
				//sendthe request (buf2)
                chan.cwrite(buf2, len);
                //receive the response
                //cread into buf3 length file_req->len
                chan.cread(buf3, file_req->length);
                //write buf3 into file : received/filename               
                received_file.write(buf3, file_req->length);

				//filemsg* file_req = (filemsg*)buf2;
                //file_req->offset += m; //buffer cap
				file_req->offset += file_req->length; 
				remaining -= (file_req->length);
				file_req->length= std::min(m, remaining);
                
            }
			received_file.close();
			//closing clock         
			auto end = chrono::steady_clock::now();
			std::cout << "Elapsed time (ms): " << chrono::duration_cast<chrono::milliseconds>(end-start).count() << std::endl;

            delete[] buf2;
            delete[] buf3;
			// if(new_chan){
			// 	MESSAGE_TYPE m = QUIT_MSG;
			// 	newChan->cwrite(&m, sizeof(MESSAGE_TYPE));
			// }
			//delete newChan;

        }
		if(new_chan){
			MESSAGE_TYPE m = QUIT_MSG;
			newChan->cwrite(&m, sizeof(MESSAGE_TYPE));
			delete newChan;
		}
		// closing the channel    
		MESSAGE_TYPE m = QUIT_MSG;
		cont_chan.cwrite(&m, sizeof(MESSAGE_TYPE));
		wait(nullptr);

	}

}
