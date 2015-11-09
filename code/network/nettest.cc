// nettest.cc 
//	Test out message delivery between two "Nachos" machines,
//	using the Post Office to coordinate delivery.
//
//	Two caveats:
//	  1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//		./nachos -m 0 -o 1 &
//		./nachos -m 1 -o 0 &
//
//	  2. You need an implementation of condition variables,
//	     which is *not* provided as part of the baseline threads 
//	     implementation.  The Post Office won't work without
//	     a correct implementation of condition variables.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include <iostream>
#include <sstream>
#include <string>
#include <vector> 
#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"
#include "../network/post.h"

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

enum ServerState{Busy, Available};

struct ServerLock{
    string name;
    ServerState state;
    int owner;
    queue<PacketHeader*>* packetWaiting;
    queue<MailHeader*>* mailWaiting;
    bool toBeDeleted;
};

struct ServerCV{
    string name;
    queue<PacketHeader*>* packetWaiting;
    queue<MailHeader*>* mailWaiting;
    int lockIndex;
    bool toBeDeleted;
};

struct ServerMV{
    string name;
    int* val;
    int len;
    bool toBeDeleted;
};

void initNetworkMessaging(PacketHeader &inPktHdr, PacketHeader &outPktHdr, MailHeader &inMailHdr, MailHeader &outMailHdr, int len)
{
    outPktHdr.to = inPktHdr.from;
    outPktHdr.from = inPktHdr.to;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.from = inMailHdr.to;
    outMailHdr.length = len+1;
}

void sendMessage(PacketHeader* outPktHdr, MailHeader* outMailHdr, PacketHeader* inPktHdr, MailHeader* inMailHdr, stringstream& msg){
    int msgLen = msg.str().length();
    outMailHdr->length = msgLen;
    char *message = new char[msgLen];
    std::strcpy(message, msg.c_str());

    initNetworkMessaging(inPktHdr, outPktHdr, inMailHdr, outMailHdr, strlen(message));
    if(!postOffice->Send(outPktHdr, outMailHdr, data)){
        printf("Error in server, cannot send message \n");
    }
}

void Server(){
    vector<ServerLock*>* SLocks = new vector<ServerLock*>;
    vector<ServerCV*>* SCVs = new vector<ServerCV*>;
    vector<ServerMV*>* SMVs = new vector<ServerMV*>; 

    while(true){
        PacketHeader* outPktHdr = new PacketHeader();
        PacketHeader* inPktHdr = new PacketHeader();
        MailHeader* outMailHdr = new MailHeader();
        MailHeadr* inMailHdr = new MailHeader(); 

        char buffer[MaxMailSize]; 

        postOffice->Receive(0, inPktHdr, inMailHdr, buffer); 

        int type;
        stringstream ss;
        ss<<buffer;
        ss>>type; 

        string name;
        int lockID, cvID, mvID, mvVal;
        stringstream reply; 

        switch(type){
            case RPC_CreateLock: 
                ss>>name;
               int index = -1;
               for(int i = 0; i<SLocks.size();i++){
                if(SLocks->at(i)->name.compare(name) == 0){
                    index = i;
                    break;
                }
               }
               if(index == -1){
                index = SLocks.size(); 
                ServerLock *lock = new ServerLock;
                lock->name = name;
                lock->packetWaiting = new queue<PacketHeader *>();
                lock->mailWaiting = new queue<MailHeader *>(); 
                lock->state = Available; 
                lock->toBeDeleted = false;
                lock->owner = -1; 

                SLocks->push_back(lock);

                reply << SLocks -> size() -1;

               }
               else{
                reply << index; 
               }
              sendMessage(outPktHdr, OutMailHdr, reply);
              break;
            case RPC_DestroyLock: 

            }
        }
    }
}

void
MailTest(int farAddr)
{
    PacketHeader outPktHdr, inPktHdr;
    MailHeader outMailHdr, inMailHdr;
    char *data = "Hello there!";
    char *ack = "Got it!";
    char buffer[MaxMailSize];

    // construct packet, mail header for original message
    // To: destination machine, mailbox 0
    // From: our machine, reply to: mailbox 1
    outPktHdr.to = farAddr;		
    outMailHdr.to = 0;
    outMailHdr.from = 1;
    outMailHdr.length = strlen(data) + 1;

    // Send the first message
    bool success = postOffice->Send(outPktHdr, outMailHdr, data); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the first message from the other machine
    postOffice->Receive(0, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Send acknowledgement to the other machine (using "reply to" mailbox
    // in the message that just arrived
    outPktHdr.to = inPktHdr.from;
    outMailHdr.to = inMailHdr.from;
    outMailHdr.length = strlen(ack) + 1;
    success = postOffice->Send(outPktHdr, outMailHdr, ack); 

    if ( !success ) {
      printf("The postOffice Send failed. You must not have the other Nachos running. Terminating Nachos.\n");
      interrupt->Halt();
    }

    // Wait for the ack from the other machine to the first message we sent.
    postOffice->Receive(1, &inPktHdr, &inMailHdr, buffer);
    printf("Got \"%s\" from %d, box %d\n",buffer,inPktHdr.from,inMailHdr.from);
    fflush(stdout);

    // Then we're done!
    interrupt->Halt();
}



