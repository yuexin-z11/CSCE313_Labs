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
#include <chrono>

using namespace std;
using namespace std::chrono;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1.0;
	int e = -1;
	string filename = "";
    int msg = MAX_MESSAGE;
    bool new_chan = false;
	vector<FIFORequestChannel*> channels;

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
			case 'c':
				new_chan = true;
				break;
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
    FIFORequestChannel* cont_chan = new FIFORequestChannel("control", FIFORequestChannel::CLIENT_SIDE);
    channels.push_back(cont_chan);
    
    //Task 4:
    //Request a new channel
    if (new_chan){
        // send new channel request to channel
        MESSAGE_TYPE nc = NEWCHANNEL_MSG;
        cont_chan->cwrite(&nc, sizeof(MESSAGE_TYPE)); // request
        // create a variable to hold the name
        char new_name[MAX_MESSAGE];
        // get the response from the server
        cont_chan->cread(new_name, MAX_MESSAGE);
        // call the fiforequest channel constructor with the name from the server
        FIFORequestChannel* new_chan = new FIFORequestChannel(new_name, FIFORequestChannel::CLIENT_SIDE);
        channels.push_back(new_chan);
	}

    FIFORequestChannel* chan = channels.back();
    
    //Task 2:
    //Request data points
    char buf[MAX_MESSAGE];
    // datamsg x(1, 0.0, 1);
    if((p != -1) & (t != -1) & (e != -1)){
			datamsg x(p, t, e);
			memcpy(buf, &x, sizeof(datamsg));
			chan->cwrite(buf, sizeof(datamsg)); // write to server through control channel
			double reply;
			chan->cread(&reply, sizeof(double)); // read server response
			cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
	}
    else if (p != -1){
        ofstream file("./received/x1.csv");
        if (!file.is_open()) {
            std::cerr << "Error opening file!" << std::endl;
            return 1;
        }

        // loop 1000 datapoints
        t = 0;
        for (int i = 0; i < 1000; ++i){
            file << t << ',';
            datamsg x(p, t, 1);
            memcpy(buf, &x, sizeof(datamsg));
            chan->cwrite(buf, sizeof(datamsg)); // write to server through control channel
            double reply1;
            chan->cread(&reply1, sizeof(double)); // read server response
            file << reply1 << ',';

            datamsg y(p, t, 2);
            memcpy(buf, &y, sizeof(datamsg));
            chan->cwrite(buf, sizeof(datamsg)); // write to server through control channel
            double reply2;
            chan->cread(&reply2, sizeof(double)); // read server response
            file << reply2 << '\n';

            t += 0.004;
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
    chan->cwrite(buf2, len);

    __int64_t file_length;
    chan->cread(&file_length, sizeof(__int64_t));
    cout << "The length of " << fname << " is " << file_length << endl;

    // response buffer
	char* buf3 = new char[msg]; // of buff capacity (m)
    string full_path = "received/" + filename;
    ofstream file2(full_path);
    if (!file2.is_open()) {
        std::cerr << "Error opening file!" << std::endl;
        return 1;
    }

    auto start_time = high_resolution_clock::now();  // start time

    // loop over the filesize / buff capacity
    __int64_t remaining_length = file_length;
    __int64_t offset = 0;
    while(remaining_length > 0){
        // create file message instance
        __int64_t chunk_size = (remaining_length > msg) ? msg : remaining_length;
        filemsg* file_req = (filemsg*)buf2; // casting buf2 to file massage pointer
        file_req->offset = offset; // set offset in the file 
        file_req->length = chunk_size; // set the length, watch the last segment

        // send the request buf2
        chan->cwrite(buf2, len); 
        chan->cread(buf3, chunk_size); 
        file2.write(buf3, chunk_size);

        offset += chunk_size;
        remaining_length -= chunk_size;
    }
    auto end_time = high_resolution_clock::now();  // end time

    file2.close();

    auto duration = duration_cast<milliseconds>(end_time - start_time);
cout << "Time taken to transfer " << fname << ": " << duration.count() << " ms" << endl;
    
    delete[] buf2;
    delete[] buf3;

    if (new_chan){
		MESSAGE_TYPE mes1 = QUIT_MSG;
		chan->cwrite(&mes1, sizeof(MESSAGE_TYPE));
		delete channels.back();
        channels.pop_back();
	}

	//Task 5:
	// Closing all the channels
	MESSAGE_TYPE mes = QUIT_MSG;
	cont_chan->cwrite(&mes, sizeof(MESSAGE_TYPE));

    for (FIFORequestChannel* ch : channels){
        delete ch;
    }

    return 0;
}
