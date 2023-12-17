// #include <fstream>
// #include <iostream>
// #include <thread>
// #include <vector>
// #include <sys/time.h>
// #include <sys/wait.h>
// #include <algorithm>

// #include "BoundedBuffer.h"
// #include "common.h"
// #include "Histogram.h"
// #include "HistogramCollection.h"
// #include "FIFORequestChannel.h"

// // ecgno to use for datamsgs
// #define EGCNO 1

// using namespace std;

// struct PatientData{
//     int p_no;
//     double response;

// };

// void patient_thread_function (int p_no, int n, BoundedBuffer* request_buffer) {
//     // functionality of the patient threads

//     // take a patient num p_no
//     // for n requests produce a datamsg(num, time, ECGNO) and push to request_buffer
//     //    -time dependent on current requests
//     //   - at 0 -> time = 0.000; at 1 -> time = 0.004; at 2 -> time= 0.008
//     //double time= 0.0;
//     for (int i=0; i<n; i++){
//         datamsg x(p_no, i*0.004, EGCNO);
//         request_buffer->push((char*)&x, sizeof(datamsg));
//         //time= i*0.004;
//     }
// }

// void file_thread_function (BoundedBuffer *request_buffer, int file_size, string filename, int bufsize) {
    
//     // functionality of the file thread
//     // look at PA1
//     // create file and recieved dir
//     //filemsg f(0, 0);
//     // use dyanamic allocation
//     // get file size - send file msg with 0,0 offest len, server replies with filesize long long int
//     // transfer file in chunks
//     // chunks pushed to request buffer -- push offset and lenght
//     // push - file msg
//     // divide into chunks
    
   
//     // file size
//     // open output file; allocate the memory fseek; close the file
//     // while offset < filesize, produce a filemsg (offset,m )+ filename and push to req buffer
//     //        -incrementing offset and be careful with the final message
    

//     int len = static_cast<int>(sizeof(filemsg) + filename.size() + 1);
//     __int64_t offset = 0;

//     while (offset < file_size) {
//         int length = min(static_cast<__int64_t>(bufsize), file_size - offset);
//         char* buffer = new char[len];

//         filemsg fmsg(offset, length);
//         memcpy(buffer, &fmsg, sizeof(filemsg));
//         strcpy(buffer + sizeof(filemsg), filename.c_str());

//         request_buffer->push(buffer, len);
//         offset += length;
//         delete[] buffer;
//     }
// }

// void worker_thread_function (BoundedBuffer* request_buffer, BoundedBuffer* response_buffer, FIFORequestChannel* chan , int bufsize/*, buffer size*/) {
//     // functionality of the worker threads
//     //

//     // forever loop
//     // pop message from the request buffer
//     // view line 120 in server ; process request function for how to decide current message
//     //if  data msg, quit,
//     // if  file msg
//     // if quit ms --- deley chan
//     // if data:
//     //      send the message acrooss A FIFO channel to server
//     //      collect response
//     //      create a pair or struct of p_no from message and response from server
//     //      push pair to the response buffer 

//     // quit condition

//     // if file:
//     //      send the message across A FIFO channel to server
//     //      collect response
//     //      collect the file name from the message you popped in the buffer
//     //      open the file in update mode
//     //      fseek (SEEK_Set) to offset of the filemsg, write the buffer from ther server
//     //      

//     // quit condition
//     // cerr<< bufsize<< endl;
//     char* buf = new char[bufsize];
//     while (true) {
        
//         request_buffer->pop(buf, bufsize);
        
//         MESSAGE_TYPE m = *((MESSAGE_TYPE*) buf);
        
//         if(m == QUIT_MSG){
//             chan->cwrite((char*)&m, sizeof(MESSAGE_TYPE));
//             break; //quit
//             //delete channel
//         }

//         if(m == DATA_MSG){
//             // do i need to create a datamsg instance??
//             datamsg d  = *((datamsg*) buf);
//             chan->cwrite(buf, sizeof(datamsg));
//             double reply;
            
//             //int len_reply = chan->cread(&reply, sizeof(reply));
//             chan->cread(&reply, sizeof(reply));
//             // PatientData pdata;
//             // pdata.p_no = d.person;
//             // pdata.response = reply;
//             PatientData pdata{d.person, reply};
//             //std::cout<< pdata.response <<endl;
//             // = {d.person, reply};

//             response_buffer->push((char*)&pdata, sizeof(PatientData));
//         }

//         if (m == FILE_MSG) {
//             filemsg* fm = (filemsg*) buf;
//             string received_filename = buf + sizeof(filemsg);
//             received_filename = "received/" + received_filename;
//             FILE* outputfile= fopen(received_filename.c_str(), "r+");

//             if (outputfile) {
//                 char* buf3 = new char[fm->length];
//                 chan->cwrite(buf, sizeof(filemsg) + received_filename.size() + 1); 
//                 chan->cread(buf3, fm->length); 

//                 fseek(outputfile, fm->offset, SEEK_SET);
//                 fwrite(buf3, 1, fm->length, outputfile);
//                 fclose(outputfile);

//                 delete[] buf3; 
//             } 
//             else {
//                 cerr << "Cannot open file " << received_filename << endl;
//             }
//         }

//     }
//     delete[] buf;
    
// }


// void histogram_thread_function (BoundedBuffer *response_buffer, HistogramCollection *hc, int bufsize) {
//     // functionality of the histogram threads

//     // forever loop
//     // pop response from response buffer
//     // call HC::update (resp-> p_no, resp->double)
//     char* buf= new char[bufsize];
//     while(true){
        
//         response_buffer->pop(buf, bufsize);
//         PatientData pdata = *(PatientData*) buf;

//         if(pdata.p_no==-1){
//             break;  
//             // push empty buffer
//         }
//         //datamsg *d = (datamsg *) buf;
//         //double data = *(double *)(buf + sizeof(datamsg));
//         else{
//             hc->update(pdata.p_no, pdata.response);
//         }
//     }
//     delete[] buf;

//     // quit condition
// }


// int main (int argc, char* argv[]) {
//     int n = 1000;	// default number of requests per "patient"
//     int p = 10;		// number of patients [1,15]
//     int w = 100;	// default number of worker threads
// 	int h = 20;		// default number of histogram threads
//     int b = 20;		// default capacity of the request buffer (should be changed)
// 	int m = MAX_MESSAGE;	// default capacity of the message buffer
// 	string f = "";	// name of file to be transferred
    
//     // read arguments
//     int opt;
// 	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:")) != -1) {
// 		switch (opt) {
// 			case 'n':
// 				n = atoi(optarg);
//                 break;
// 			case 'p':
// 				p = atoi(optarg);
//                 break;
// 			case 'w':
// 				w = atoi(optarg);
//                 break;
// 			case 'h':
// 				h = atoi(optarg);
// 				break;
// 			case 'b':
// 				b = atoi(optarg);
//                 break;
// 			case 'm':
// 				m = atoi(optarg);
//                 break;
// 			case 'f':
// 				f = optarg;
//                 break;
// 		}
// 	}
    
// 	// fork and exec the server
//     int pid = fork();
//     if (pid == 0) {
//         execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
//     }
    
// 	// initialize overhead (including the control channel)
// 	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
//     BoundedBuffer request_buffer(b);
//     BoundedBuffer response_buffer(b);
// 	HistogramCollection hc;

//     // array of producer threads (if data , pe lements; else if file, 1 elemt)
//     // array of FIFO - w elements
//     // array of worker threads -- w elements
//     // array of histogram threads (if data- h elements, if file- 0 elements)
//     vector<thread> producer_threads;
//     vector<thread> worker_threads;
//     vector<thread> histogram_threads;
//     vector<FIFORequestChannel*> worker_channels;

//     // making histograms and adding to collection
//     for (int i = 0; i < p; i++) {
//         Histogram* h = new Histogram(10, -2.0, 2.0);
//         hc.add(h);
//     }
	
// 	// record start time
//     struct timeval start, end;
//     gettimeofday(&start, 0);

//     /* create all threads here */
    
//     // if data: 
//     //      create p aptient threads (store in producer array )
//     //      create w worker threads (store in worker array)
//     //          create channel (store in FIFO array)
//     //      create h histogram threads (store in hist array)
//     if(f== ""){
//         for(int i=0; i<p; i++){
//             producer_threads.push_back(thread(patient_thread_function, i+1, n, &request_buffer));
//         }

//         for(int i=0; i<w; i++){
//             MESSAGE_TYPE nc = NEWCHANNEL_MSG;
// 			chan->cwrite(&nc, sizeof(MESSAGE_TYPE));
// 			//create a variable to hold the name
// 			char* channelname= new char[MAX_MESSAGE];
// 			//cread the response from the server
// 			chan->cread(channelname, MAX_MESSAGE);
//             FIFORequestChannel* newChan = new FIFORequestChannel(channelname, FIFORequestChannel::CLIENT_SIDE);
//             delete[] channelname;
//             worker_channels.push_back(newChan);
//             // worker_channels.push_back(new FIFORequestChannel(channelname, FIFORequestChannel::CLIENT_SIDE));
            
//             worker_threads.push_back(thread(worker_thread_function, &request_buffer, &response_buffer, newChan, m));
//         }

//         for(int i=0; i<h; i++){
//             histogram_threads.push_back(thread(histogram_thread_function, &response_buffer, &hc, m));
//         }
//     }

//     // if file:
//     //      create 1 file thread (store in producer array)
//     //      create w worker threads (store in worker array)
//     //          create channel (store in FIFO array)
//     else{
//         FILE* outputfile = fopen(("received/" + f).c_str(), "w+");

//         filemsg fm(0, 0);
//         string filename = f;
//         int len = sizeof(filemsg) + (filename.size() + 1);
//         char* buf = new char[len];
//         memcpy(buf, &fm, sizeof(filemsg));
//         strcpy(buf + sizeof(filemsg), filename.c_str());

//         chan->cwrite(buf, len);
//         //cout << "write "<<endl;

//         __int64_t file_size;
//         chan->cread(&file_size, sizeof(__int64_t));
//         producer_threads.push_back(thread(file_thread_function, &request_buffer, file_size, f, m));
//         delete[] buf;
//         fclose(outputfile);

//         for(int i=0; i<w; i++){
//             MESSAGE_TYPE nc = NEWCHANNEL_MSG;
// 			chan->cwrite(&nc, sizeof(MESSAGE_TYPE));
//             //cout << "for loop " << endl;
// 			//create a variable to hold the name
// 			char* channelname= new char[MAX_MESSAGE];
// 			//cread the response from the server
// 			chan->cread(channelname, MAX_MESSAGE);
//             FIFORequestChannel* newChan = new FIFORequestChannel(channelname, FIFORequestChannel::CLIENT_SIDE);
//             delete[] channelname;
//             worker_channels.push_back(newChan);
//             //worker_channels.push_back(new FIFORequestChannel(channelname, FIFORequestChannel::CLIENT_SIDE));
            
//             worker_threads.push_back(thread(worker_thread_function, &request_buffer, &response_buffer, newChan, m));
//         }
        
//     }

// 	/* join all threads here */
//     // iterate over all thread arrays, calling join
//     //      order is imp; producers before consumers
//     for (auto& t : producer_threads) {
//         t.join();
//     }
//     //push quit msg to worker thresds
//     MESSAGE_TYPE mes = QUIT_MSG;

//     for(int i=0; i<w; i++){
//         request_buffer.push((char*)&mes, sizeof(MESSAGE_TYPE));
//     }
//     for (auto& t : worker_threads) {
//         t.join();
//     }
//     //quit to hc
//     PatientData pneg;
//     pneg.p_no= -1;
//     pneg.response =-1;
//     for(int i=0; i<h; i++){
//         response_buffer.push((char*)&pneg, sizeof(PatientData));
//     }

//     if (f == "") {
//         for (auto& t : histogram_threads) {
//             t.join();
//         }
//     }

// 	// record end time
//     gettimeofday(&end, 0);

//     // print the results
// 	if (f == "") {
// 		hc.print();
// 	}
//     int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
//     int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
//     std::cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
    

//     for(int i=0; i<w; i++){
//         MESSAGE_TYPE q = QUIT_MSG;
//         worker_channels[i]->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
        
//         delete worker_channels[i];
//     }
// 	// quit and close control channel
//     MESSAGE_TYPE q = QUIT_MSG;
//     chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
//     std::cout << "All Done!" << endl;
//     delete chan;

// 	// wait for server to exit
// 	wait(nullptr);
// }
#include <fstream>
#include <iostream>
#include <thread>
#include <sys/time.h>
#include <sys/wait.h>

#include "BoundedBuffer.h"
#include "common.h"
#include "Histogram.h"
#include "HistogramCollection.h"
#include "FIFORequestChannel.h"

// ecgno to use for datamsgs
#define ECGNO 1

using namespace std;


struct PVPair {
	int p;
	double v;
	
	PVPair (int _p, double _v) : p(_p), v(_v) {}
};

FIFORequestChannel* create_new_channel (FIFORequestChannel* chan, int b) {
    FIFORequestChannel* nchan = nullptr;

    // request new channel
    MESSAGE_TYPE m = NEWCHANNEL_MSG;
    chan->cwrite(&m, sizeof(MESSAGE_TYPE));
    
    unique_ptr<char[]> chname = make_unique<char[]>(b);
    chan->cread(chname.get(), b);

    // open channel on client side
    nchan = new FIFORequestChannel(chname.get(), FIFORequestChannel::CLIENT_SIDE);

    return nchan;
}

void patient_thread_function (int n, int pno, BoundedBuffer* rb) {
    datamsg d(pno, 0.0, ECGNO);

    // populate data messages
    for (int i = 0; i < n; i++) {
        rb->push((char*) &d, sizeof(datamsg));
        d.seconds += 0.004;
    }
}

void file_thread_function (string fname, FIFORequestChannel* chan, int b, BoundedBuffer* rb) {
    // open receiving file and truncate appropriate number of bytes
    filemsg f(0, 0);
    size_t bufsize = sizeof(filemsg) + fname.size() + 1;
    unique_ptr<char[]> buf = make_unique<char[]>(bufsize);
    memcpy(buf.get(), &f, sizeof(filemsg));
    strcpy(buf.get() + sizeof(filemsg), fname.c_str());
    
    chan->cwrite(buf.get(), bufsize);
	
    __int64_t flen;
    chan->cread(&flen, sizeof(__int64_t));
    
    FILE* myfile = fopen(("received/" + fname).c_str(), "wb");
    fseek(myfile, flen, SEEK_SET);
    fclose(myfile);

    // populate file messages
	__int64_t remainingbytes = flen;
    int buffer = b;
    int offset = 0;
    filemsg* fm = (filemsg*) buf.get();
    while (remainingbytes > 0) {
        if (buffer > remainingbytes) {
            buffer = remainingbytes;
        }
        offset = flen - remainingbytes;

        fm->offset = offset;
        fm->length = buffer;
        rb->push(buf.get(), bufsize);

        remainingbytes -= buffer;
    }
}

void worker_thread_function (FIFORequestChannel* chan, BoundedBuffer* rb, BoundedBuffer* sb, int b) {
    unique_ptr<char[]> buf = make_unique<char[]>(b);
    double result = 0.0;
    unique_ptr<char[]> recvbuf = make_unique<char[]>(b);
    
    while (true) {
        rb->pop(buf.get(), b);
        MESSAGE_TYPE* m = (MESSAGE_TYPE*) buf.get();

        if (*m == DATA_MSG) {
            chan->cwrite(buf.get(), sizeof(datamsg));
            chan->cread(&result, sizeof(double));
			PVPair pv(((datamsg*) buf.get())->person, result);
            sb->push((char*) &pv, sizeof(PVPair));
        }
        else if (*m == FILE_MSG) {
            filemsg* fm = (filemsg*) buf.get();
            string fname = (char*) (fm + 1);
            chan->cwrite(buf.get(), (sizeof(filemsg) + fname.size() + 1));
			chan->cread(recvbuf.get(), fm->length);

            FILE* myfile = fopen(("received/" + fname).c_str(), "rb+");
			fseek(myfile, fm->offset, SEEK_SET);
            fwrite(recvbuf.get(), 1, fm->length, myfile);
            fclose(myfile);
        }
        else if (*m == QUIT_MSG) {
            chan->cwrite(m, sizeof(MESSAGE_TYPE));
            delete chan;
            break;
        }
    }
}

void histogram_thread_function (BoundedBuffer* rb, HistogramCollection* hc) {
	char buf[sizeof(PVPair)];
	while (true) {
		rb->pop(buf, sizeof(PVPair));
		PVPair pv = *(PVPair*) buf;
		if (pv.p <= 0) {
			break;
		}
		hc->update(pv.p, pv.v);
	}
}


int main (int argc, char *argv[]) {
    int n = 1000;	// default number of requests per "patient"
    int p = 10;		// number of patients [1,15]
    int w = 100;	// default number of worker threads
	int h = 20;		// default number of histogram threads
    int b = 30;		// default capacity of the request buffer
	int m = MAX_MESSAGE;	// default capacity of the message buffer
	string f = "";	// name of file to be transferred
    
    // read arguments
    int opt;
	while ((opt = getopt(argc, argv, "n:p:w:h:b:m:f:")) != -1) {
		switch (opt) {
			case 'n':
				n = atoi(optarg);
                break;
			case 'p':
				p = atoi(optarg);
                break;
			case 'w':
				w = atoi(optarg);
                break;
			case 'h':
				h = atoi(optarg);
				break;
			case 'b':
				b = atoi(optarg);
                break;
			case 'm':
				m = atoi(optarg);
                break;
			case 'f':
				f = optarg;
                break;
		}
	}
    bool filereq = (f != "");
    
	// fork and exec the server
    int pid = fork();
    if (pid == 0) {
        execl("./server", "./server", "-m", (char*) to_string(m).c_str(), nullptr);
    }
    
	// control overhead (including the control channel)
	FIFORequestChannel* chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    BoundedBuffer request_buffer(b);
    BoundedBuffer response_buffer(b);
	HistogramCollection hc;

    // making histograms and adding to collection
    for (int i = 0; i < p; i++) {
        Histogram* h = new Histogram(10, -2.0, 2.0);
        hc.add(h);
    }

    // making worker channels
    unique_ptr<FIFORequestChannel*[]> wchans = make_unique<FIFORequestChannel*[]>(w);
    for (int i = 0; i < w; i++) {
        wchans[i] = create_new_channel(chan, m);
    }
	
	// record start time
    struct timeval start, end;
    gettimeofday(&start, 0);

    /* Start all threads here */
    unique_ptr<thread[]> patients = make_unique<thread[]>(p);
    thread file;
    if (!filereq) {
        for (int i = 0; i < p; i++) {
            patients[i] = thread(patient_thread_function, n, (i+1), &request_buffer);
        }
    }
    else {
        file = thread(file_thread_function, f, chan, m, &request_buffer);
    }

    unique_ptr<thread[]> workers = make_unique<thread[]>(w);
    for (int i = 0; i < w; i++) {
        workers[i] = thread(worker_thread_function, wchans[i], &request_buffer, &response_buffer, m);
    }

    unique_ptr<thread[]> hists = make_unique<thread[]>(h);
    for (int i = 0; i < h; i++) {
        hists[i] = thread(histogram_thread_function, &response_buffer, &hc);
    }
	
	/* Join all threads here */
    if (!filereq) {
        for (int i = 0; i < p; i++) {
            patients[i].join();
        }
    }
    else {
        file.join();
    }

    for (int i = 0; i < w; i++) {
        MESSAGE_TYPE q = QUIT_MSG;
        request_buffer.push((char*) &q, sizeof(MESSAGE_TYPE));
    }

    for (int i = 0; i < w; i++) {
        workers[i].join();
    }
	
	for (int i = 0; i < h; i++) {
		PVPair pv(0, 0);
		response_buffer.push((char*) &pv, sizeof(PVPair));
	}
	
	for (int i = 0; i < h; i++) {
		hists[i].join();
	}

	// record end time
    gettimeofday(&end, 0);

    // print the results
	if (f == "") {
		hc.print();
	}
    int secs = ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) / ((int) 1e6);
    int usecs = (int) ((1e6*end.tv_sec - 1e6*start.tv_sec) + (end.tv_usec - start.tv_usec)) % ((int) 1e6);
    cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;

	// quit and close control channel
    MESSAGE_TYPE q = QUIT_MSG;
    chan->cwrite ((char *) &q, sizeof (MESSAGE_TYPE));
    cout << "All Done!" << endl;
    delete chan;

	// wait for server to exit
	wait(nullptr);
}