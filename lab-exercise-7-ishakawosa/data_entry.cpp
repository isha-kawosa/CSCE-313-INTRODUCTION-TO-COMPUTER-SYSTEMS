#include <vector> // vector, push_back, at
#include <string> // string
#include <iostream> // cin, getline
#include <fstream> // ofstream
#include <unistd.h> // getopt, exit, EXIT_FAILURE
#include <assert.h> // assert
#include <thread> // thread, join
#include <sstream> // stringstream
#include <thread>
#include "BoundedBuffer.h" // BoundedBuffer class
#include <string.h>

#define MAX_MSG_LEN 256

using namespace std;

/************** Helper Function Declarations **************/

void parse_column_names(vector<string>& _colnames, const string& _opt_input);
void write_to_file(const string& _filename, const string& _text, bool _first_input=false);

/************** Thread Function Definitions **************/

// "primary thread will be a UI data entry point"
void ui_thread_function(BoundedBuffer* bb) {
    // TODO: implement UI Thread Function
    // infinite loop, continuously prompt user for input 
    // if input is exit, break
    // else, push input onto bb
    while(true){
        string str;
        cout << "enter data>" ;
        getline(cin, str);
        if(str=="Exit"){
            break;
        }
        vector<char> input_data(str.begin(), str.end());
        bb->push(input_data.data(), input_data.size());
    }

}

// "second thread will be the data processing thread"
// "will open, write to, and close a csv file"
void data_thread_function(BoundedBuffer* bb, string filename, const vector<string>& colnames) {
    // TODO: implement Data Thread Function
    // (NOTE: use "write_to_file" function to write to file)
    // infinite loop, pop from bb
    // if its the exit packet , break(compare to exit)
    // else call write_to_file
    char buf[MAX_MSG_LEN];
    int size =0;
    bool firstinput = true;
    int count =0;

    stringstream col;
    for(size_t i=0; i <colnames.size(); i++){
        col << colnames[i];
        if(i<colnames.size()-1){
            col <<", ";
        }
    }
    write_to_file(filename, col.str()+ "\n", firstinput);
    firstinput = false;

    while (true)
    {
        size= bb->pop(buf, MAX_MSG_LEN);
        string data(buf, size);

        if(data=="Exit"){
            break;
        }

        count++;
        string formattedData = data;
        if(count % 3 == 0){
            formattedData += "\n";
        } 

        else{
            formattedData += ", ";
        }
        write_to_file(filename, formattedData, false);
        
    }
    
}

/************** Main Function **************/

int main(int argc, char* argv[]) {

    // variables to be used throughout the program
    vector<string> colnames; // column names
    string fname; // filename
    BoundedBuffer* bb = new BoundedBuffer(3); // BoundedBuffer with cap of 3

    // read flags from input
    int opt;
    while ((opt = getopt(argc, argv, "c:f:")) != -1) {
        switch (opt) {
            case 'c': // parse col names into vector "colnames"
                parse_column_names(colnames, optarg);
                break;
            case 'f':
                fname = optarg;
                break;
            default: // invalid input, based on https://linux.die.net/man/3/getopt
                fprintf(stderr, "Usage: %s [-c colnames] [-f filename]\n", argv[0]);
                exit(EXIT_FAILURE);
        }
    }
    
    // set first input to 1
    

    // TODO: instantiate ui and data threads
    thread uiThread(ui_thread_function, bb);
    thread dataThread(data_thread_function, bb, fname, colnames);
    // TODO: join ui_thread
    uiThread.join();

    // TODO: "Once the user has entered 'Exit', the main thread will
    // "send a signal through the message queue to stop the data thread"
    string x = "Exit";

    vector<char> exit_data(x.begin(), x.end());
    bb->push(exit_data.data(), exit_data.size());
    
    // TODO: join data thread
    dataThread.join();

    // CLEANUP: delete members on heap
    delete bb;
}

/************** Helper Function Definitions **************/

// function to parse column names into vector
// input: _colnames (vector of column name strings), _opt_input(input from optarg for -c)
void parse_column_names(vector<string>& _colnames, const string& _opt_input) {
    stringstream sstream(_opt_input);
    string tmp;
    while (sstream >> tmp) {
        _colnames.push_back(tmp);
    }
}

// function to append "text" to end of file
// input: filename (name of file), text (text to add to file), first_input (whether or not this is the first input of the file)
void write_to_file(const string& _filename, const string& _text, bool _first_input) {
    // based on https://stackoverflow.com/questions/26084885/appending-to-a-file-with-ofstream
    // open file to either append or clear file
    ofstream ofile;
    if (_first_input)
        ofile.open(_filename);
    else
        ofile.open(_filename, ofstream::app);
    if (!ofile.is_open()) {
        perror("ofstream open");
        exit(-1);
    }

    // sleep for a random period up to 5 seconds
    usleep(rand() % 5000);

    // add data to csv
    ofile << _text;

    // close file
    ofile.close();
}