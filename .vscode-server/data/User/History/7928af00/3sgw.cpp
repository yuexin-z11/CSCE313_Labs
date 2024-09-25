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
#include <iostream>

using namespace std;


int main (int argc, char *argv[]) {
	int opt;
	int p = -1;
	double t = -1.0;
	int e = -1;
	string filename = "";
	int m = MAX_MESSAGE; // default value for buffer capacity 
	bool new_chan = false;
	vector<FIFORequestChannel*> channels;

	//Add other arguments here
	while ((opt = getopt(argc, argv, "p:t:e:f:m:c")) != -1) { // collon is used when flag have a value associated with it
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
				m = atoi(optarg); // convert optarg to an integer
				break;
			case 'c':
				new_chan = true;
				break;
		}
	}

	// give arguments for the server 
	// server needs: path, -m, value for -m arg, null
	// fork
	// in the child, run execvp using the server arguments.

	//Task 1:
	//Run the server process as a child of the client process
	pid_t pid = fork(); // get the child 
	if (pid < 0){
		cerr << "Error: Failed the fork the process" << endl;
	}
	else if (pid == 0){ // this is the child
		char m_arg[10];
		snprintf(m_arg, sizeof(m_arg), "%d", m); // convert m to string
		char* const server_args[] = {
            (char*)"./server",   
            (char*)"-m",         
            m_arg,                
            nullptr               
        };
		execvp(server_args[0], server_args);
	}
	else {
		FIFORequestChannel cont_chan("control", FIFORequestChannel::CLIENT_SIDE);
		channels.push_back(&cont_chan);
		//Task 4:
		//Request a new channel
		if (new_chan){
			// send new channel request to channel
			MESSAGE_TYPE nc = NEWCHANNEL_MSG;
			cont_chan.cwrite(&nc, sizeof(MESSAGE_TYPE)); // request
			// create a variable to hold the name
			char new_name[MAX_MESSAGE];
			// get the response from the server
			cont_chan.cread(new_name, sizeof(char));
			// call the fiforequest channel constructor with the name from the server
			FIFORequestChannel new_chan(new_name, FIFORequestChannel::CLIENT_SIDE);
			channels.push_back(&new_chan);
			// dynamically create the channel
			// push the new channel into the vector
		}

		FIFORequestChannel chan = *(channels.back());
		
		//Task 2:
		// run single datapoint, p, t, e != -1
		//Request data points
		char buf[MAX_MESSAGE];
		// datamsg x(1, 0.0, 1); // change the hard coding to user values 
		if(p != -1 & t != -1 & e != -1){
			datamsg x(p, t, e);
			memcpy(buf, &x, sizeof(datamsg));
			chan.cwrite(buf, sizeof(datamsg)); // write to server through control channel
			double reply;
			chan.cread(&reply, sizeof(double)); // read server response
			cout << "For person " << p << ", at time " << t << ", the value of ecg " << e << " is " << reply << endl;
		}
		else {
			// else if p != -1 requst 1000 datapoints
			// out this in a loop, 2000 times
			// send quest for ecg1, ecg2, write line to received x1.csv
			ofstream file("./received/x1.csv");
			if (!file.is_open()) {
				std::cerr << "Error opening file!" << std::endl;
				return 1;
			}

			// loop 1000 datapoints
			int counter = 0;
			while(counter != 1000){
				t = 0;
				file << t << ',';
				e1 = 1;
				datamsg x(p, t, e1);
				memcpy(buf, &x, sizeof(datamsg));
				chan.cwrite(buf, sizeof(datamsg)); // write to server through control channel
				double reply1;
				chan.cread(&reply1, sizeof(double)); // read server response
				file << reply << ',';

				e2 = 2;
				datamsg x(p, t, e2);
				memcpy(buf, &x, sizeof(datamsg));
				chan.cwrite(buf, sizeof(datamsg)); // write to server through control channel
				double reply2;
				chan.cread(&reply2, sizeof(double)); // read server response
				file << reply2 << '\n';

				t += 0.004;
			}
		}

		//Task 3:
		//Request files
		filemsg fm(0, 0);
		string fname = f; // filename
		
		int len = sizeof(filemsg) + (fname.size() + 1); 
		char* buf2 = new char[len];
		memcpy(buf2, &fm, sizeof(filemsg)); // copy the file massage into the buffer
		strcpy(buf2 + sizeof(filemsg), fname.c_str()); // copy file name into the buffer
		chan.cwrite(buf2, len); // send buffer to server

		int84_t filesize = 0; // store file size // change later
		chan.cread(&filesize, sizeof(int64_t)); // the receive the file length

		// response buffer
		char* buf3 = m;// of buff capacity (m)

		// loop over the filesize / buff capacity (m if not specified)
		size_t counter = min(filesize, buf3);
		while(counter != 1000 ){
			// create file message instance
			filemsg* file_req = (filemsg*)buf2; // casting buf2 to file massage pointer
			// convert back a message from the char*
			// you can cast in c++ any pointer of one type to a pointer of any other type
			file_req->offset = min(m, MAX_MESSAGE) // set offset in the file 
			file_req->length = min(buf3, filesize)// set the ength, watch the last segment
			// send the request buf2
			chan.cwrite(buf2, len); 
			// receive the response
			// cread into buf3 length file_req->len
			chan.cread(&filesize, sizeof(int64_t)); 

			// write buf3 into file: received/filename
			std::filesystem::path directory = "received/";
			std::filesystem::path filename = f;
			std::filesystem::path full_path = directory / filename;
			ofstream file2(full_path.string());
			if (!file2.is_open()) {
				std::cerr << "Error opening file!" << std::endl;
				return 1;
			}
			
			file2 << filesize;

			counter++;
		}
		


		delete[] buf2;
		delete[] buf3;
		// if necessary, close and delete the new channel
		if (new_chan){
			// delete 
			delete new_chan;
		}

		__int64_t file_length;
		chan.cread(&file_length, sizeof(__int64_t));
		cout << "The length of " << fname << " is " << file_length << endl;
		
		//Task 5:
		// Closing all the channels
		MESSAGE_TYPE m = QUIT_MSG;
		cont_chan.cwrite(&m, sizeof(MESSAGE_TYPE));
		delete cont_chan;
	}

}
