/*
	Author of the starter code
    Yifan Ren
    Department of Computer Science & Engineering
    Texas A&M University
    Date: 9/15/2024
	
	Please include your Name, UIN, and the date below
	Name: Yuexin Zhang
	UIN: 532000781
	Date: 9/17/2024
*/
#include "common.h"
#include "FIFORequestChannel.h"
#include <fstream>
#include <unistd.h>
#include <iostream>

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1.0;
	int e = -1;
	string filename = "";
    int msg = MAX_MESSAGE;
    // bool new_chan = false;
	// vector<FIFORequestChannel*> channels;

	//Add other arguments here
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
				msg = atoi(optarg); // convert optarg to an integer
				break;
			// case 'c':
			// 	new_chan = true;
			// 	break;
		}
	}

	//Task 1:
	//Run the server process as a child of the client process
    pid_t pid = fork();
    if (pid == 0){
        char* const server_args[] = {
            (char*)"./server",   
            (char*)"-m",         
            (char*)to_string(msg).c_str(),                
            nullptr               
        };
		execvp(server_args[0], server_args);
    }
    FIFORequestChannel chan("control", FIFORequestChannel::CLIENT_SIDE);
    
    //Task 4:
    //Request a new channel
    
    //Task 2:
    //Request data points
    char buf[MAX_MESSAGE];
    // datamsg x(1, 0.0, 1);
    if((p != -1) & (t != -1) & (e != -1)){
			datamsg x(p, t, e);
			memcpy(buf, &x, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // write to server through control channel
			double reply;
			chan.cread(&reply, sizeof(double)); // read server response
			cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
    else if (p != -1){
        ofstream file("./received/x1.csv");
        if (!file.is_open()) {
            std::cerr << "Error opening file!" << std::endl;
            return 1;
        }

        // loop 1000 datapoints
        int counter = 0;
        t = 0;
        while(counter < 1000){
            file << t << ',';
            datamsg x(p, t, 1);
            memcpy(buf, &x, sizeof(datamsg));
            chan.cwrite(buf, sizeof(datamsg)); // write to server through control channel
            double reply1;
            chan.cread(&reply1, sizeof(double)); // read server response
            file << reply1 << ',';

            datamsg y(p, t, 2);
            memcpy(buf, &y, sizeof(datamsg));
            chan.cwrite(buf, sizeof(datamsg)); // write to server through control channel
            double reply2;
            chan.cread(&reply2, sizeof(double)); // read server response
            file << reply2 << '\n';

            t += 0.004;
            counter++;
        }
        file.close();
    }
    
    //Task 3:
    //Request files
    filemsg fm(0, 0);
    string fname = filename;
    
    int len = sizeof(filemsg) + (fname.size() + 1);
    char* buf2 = new char[len];
    memcpy(buf2, &fm, sizeof(filemsg));
    strcpy(buf2 + sizeof(filemsg), fname.c_str());
    chan.cwrite(buf2, len);

    __int64_t file_length;
    chan.cread(&file_length, sizeof(__int64_t));
    cout << "The length of " << fname << " is " << file_length << endl;

    // response buffer
	char* buf3 = new char[msg];// of buff capacity (m)
    strint full_path = "received/" + filename;
    ofstream file2(full_path.string());
    if (!file2.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }
    // loop over the filesize / buff capacity
    char* req_file = new char[msg];
    while(filesize != 0 ){
        // create file message instance
        filemsg* file_req = (filemsg*)buf2; // casting buf2 to file massage pointer
        file_req->offset = m if m < filename else filename; // set offset in the file 
        file_req->length = msg if msg < filesize else filesize; // set the ength, watch the last segment

        // send the request buf2
        chan.cwrite(buf2, len); 
        chan.cread(&filesize, sizeof(int64_t)); 
        file2.write(buf3, file_req->length);

        if (filesize >= file_req->length){
            counter = filesize - length;
        }
        else {
            counter = 0;
        }

        counter /= m;  
    }
    
    delete[] buf2;
    //Task 5:
    // Closing all the channels
    MESSAGE_TYPE m = QUIT_MSG;
    chan.cwrite(&m, sizeof(MESSAGE_TYPE));
}
