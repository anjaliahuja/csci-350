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
#include <queue>
#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"

// Test out message delivery, by doing the following:
//	1. send a message to the machine with ID "farAddr", at mail box #0
//	2. wait for the other machine's message to arrive (in our mailbox #0)
//	3. send an acknowledgment for the other machine's message
//	4. wait for an acknowledgement from the other machine to our 
//	    original message

enum ServerState{Busy, Available};

Lock* lockLock = new Lock("Lock lock");
Lock* CVLock = new Lock("CVLock");
Lock* MVLock = new Lock("MVLock"); 

struct ServerLock{
    string name;
    ServerState state;
    int owner;
    queue<PacketHeader*>* packetWaiting;
    queue<MailHeader*>* mailWaiting;
    bool toBeDeleted;
    int counter; 
};

struct ServerCV{
    string name;
    queue<PacketHeader*>* packetWaiting;
    queue<MailHeader*>* mailWaiting;
    int lockIndex;
    bool toBeDeleted;
    int counter;
    int useCounter;
};

struct ServerMV{
    string name;
    int* values;
    int len;
    bool toBeDeleted;
};


void sendMessage(PacketHeader* outPktHdr, MailHeader* outMailHdr, stringstream& msg){
    int msgLen = msg.str().length() +1;
    outMailHdr->length = msgLen;
    char *message = new char[msgLen];
    std::strcpy(message, msg.str().c_str());

    if(!postOffice->Send(*outPktHdr, *outMailHdr, message)){
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
        MailHeader* inMailHdr = new MailHeader(); 

        char buffer[MaxMailSize]; 

        postOffice->Receive(SERVER_ID, inPktHdr, inMailHdr, buffer); 
        fflush(stdout);


        outPktHdr->to = inPktHdr->from;
        outMailHdr->to = inMailHdr->from;
        outPktHdr->from - inPktHdr->to;

        int type;
        stringstream ss;
        ss<<buffer;
        ss>>type; 
        

        string name;
        int lockID, cvID, mvID, mvVal, mvIndex, mvSize;
        stringstream reply; 

        switch(type){
            case RPC_CreateLock: {
                lockLock->Acquire();
                ss>>name;
                cout<<"RPC Create Lock name: " << name << endl; 

               int index = -1;
               for(int i = 0; i<SLocks->size();i++){
                if(SLocks->at(i)->name.compare(name) == 0){
                    index = i;
                    SLocks->at(i)->counter++;
                    break;
                }
               }
               if(index == -1){
                index = SLocks->size(); 
                ServerLock *lock = new ServerLock;
                lock->name = name;
                lock->packetWaiting = new queue<PacketHeader *>();
                lock->mailWaiting = new queue<MailHeader *>(); 
                lock->state = Available; 
                lock->toBeDeleted = false;
                lock->owner = -1; 
                lock->counter = 1;

                SLocks->push_back(lock);

               }
             cout<<"Index of lock: " << index <<endl;
            reply << index; 
               
              sendMessage(outPktHdr, outMailHdr, reply);
              lockLock->Release(); 
              break;
            }
            case RPC_DestroyLock: {
                lockLock->Acquire();
                ss >> lockID; 
                cout<<"RPC Destroy Lock ID: " << lockID << endl; 

                if(lockID < 0 || lockID >= SLocks->size()){
                    reply << -1;
                } else {
                    if(SLocks->at(lockID) == NULL){
                        reply << -1;
                    } else{
                        SLocks->at(lockID)->counter--;
                        reply<<lockID;
                        if(SLocks->at(lockID)->state == Available){
                            ServerLock *lock = SLocks->at(lockID);
                            SLocks->at(lockID) = NULL;
                            delete lock;
                        } else{
                        SLocks->at(lockID)->toBeDeleted = true;
                        }
                    }
                }
                sendMessage(outPktHdr, outMailHdr, reply);
                lockLock->Release(); 
                break;

            }

            case RPC_Acquire: {
                lockLock->Acquire(); 
                ss >> lockID;
                cout<<"RPC Acquire Lock ID: " << lockID << endl; 

                bool pass = true; 

                if(lockID < 0 || lockID >= SLocks->size()){
                    reply << -1;
                } else {
                    if(SLocks -> at(lockID) == NULL){
                        reply << -1;
                    } else if(SLocks->at(lockID)->owner == outPktHdr->to && SLocks->at(lockID)->state == Busy){
                        reply << -1; 
                    } else if(SLocks->at(lockID)->state== Busy){
                        pass= false;
                        SLocks->at(lockID)->packetWaiting->push(outPktHdr);
                        SLocks->at(lockID)->mailWaiting->push(outMailHdr);
                    } else{
                        SLocks->at(lockID)->owner = outPktHdr->to;
                        SLocks->at(lockID)->state = Busy;
                        reply << lockID; 
                    }
                }
                if(pass){
                    cout << "lock to acquire goes from: " << endl;
                    cout << outPktHdr->from << " to " << outPktHdr->to << endl;
                    cout << "with the reply: " << endl;
                    cout << reply.str() << endl;
                    sendMessage(outPktHdr, outMailHdr, reply);
                }
                lockLock->Release(); 
                break;
            }
            case RPC_Release: {
                lockLock->Acquire(); 
                ss >> lockID;
                cout<<"RPC Release Lock ID: " << lockID << endl; 
                if(lockID < 0 || lockID >= SLocks->size()){
                    reply << -1;
                } else{
                    if(SLocks->at(lockID)== NULL){
                        reply << -1;
                    } else if(SLocks->at(lockID)->state == Available || SLocks->at(lockID)->owner != outPktHdr->to){
                        reply << -1;
                    } else{
                        reply << -2; 
                        if(SLocks->at(lockID)->packetWaiting->empty()){ 
                            SLocks->at(lockID)->state = Available; 
                            SLocks->at(lockID)->owner = -1;
                        } else{
                            PacketHeader* tempOutPkt = SLocks->at(lockID)->packetWaiting->front();
                            SLocks->at(lockID)->packetWaiting->pop();
                            MailHeader* tempOutMail = SLocks->at(lockID)->mailWaiting->front();
                            SLocks->at(lockID)->mailWaiting->pop();

                            SLocks->at(lockID)->owner = tempOutPkt->to;
                            sendMessage(tempOutPkt, tempOutMail, reply);
                            /*if(SLocks->at(lockID)->packetWaiting->empty() && SLocks->at(lockID)->toBeDeleted == true){
                                ServerLock* lock = SLocks->at(lockID);
                                SLocks->at(lockID) = NULL;
                                delete lock;
                            }*/
                        }
                    }
                }
                sendMessage(outPktHdr, outMailHdr, reply);
                lockLock->Release();
                break;
            }
            case RPC_CreateCV: {
                CVLock->Acquire(); 
                ss>>name;
                cout<<"RPC CreateCV: " << name << endl; 
                int index = -1; 
                for(unsigned int i = 0; i<SCVs->size(); i++){
                    if(SCVs->at(i) != NULL){
                        cout << "name: " << SCVs->at(i)->name << std::endl;
                        if(SCVs->at(i)->name == name){
                            SCVs->at(i)->counter++;
                            index = i;
                            break;

                        }
                    }
                }
                if(index == -1){
                    index = SCVs->size();
                    ServerCV* scv = new ServerCV; 
                    scv->name = name;
                    scv->packetWaiting = new queue<PacketHeader *>();
                    scv->mailWaiting = new queue<MailHeader *>();
                    scv->toBeDeleted = false;
                    scv->lockIndex = -1; 
                    scv->counter = 1;
                    scv->useCounter = 0;

                    SCVs->push_back(scv); 

                    reply << SCVs->size() -1;

                }
                else {
                    reply << index; 
                }
                sendMessage(outPktHdr, outMailHdr, reply);
                CVLock->Release(); 
                break;

            }
            case RPC_DestroyCV: {
                CVLock->Acquire();
                ss >> cvID; 
                cout<<"RPC Destroy CV : " << cvID << endl; 

                if(cvID < 0 || cvID >= SCVs->size()){
                    reply << -1;
                } else{
                    if(SCVs->at(cvID) == NULL){
                        reply << -1;
                    } else{
                        reply << cvID;
                        SCVs->at(cvID)->counter--;
                        if(SCVs->at(cvID)->useCounter == 0){
                            ServerCV* scv = SCVs->at(cvID);
                            SCVs->at(cvID) = NULL;
                            delete scv; 
                        }

                    }
                }
                sendMessage(outPktHdr, outMailHdr, reply);
                CVLock->Release();
                break;
            }
            case RPC_Wait: {
                CVLock->Acquire();
                ss >> lockID >> cvID; 
                cout<<"RPC Wait : " << cvID << endl; 

                bool pass = true;

                if(lockID < 0 || lockID >= SLocks->size() || cvID < 0 || cvID >= SCVs->size()){
                    reply << -1;
                }
                else{
                    if(SLocks->at(lockID)==NULL || SCVs->at(cvID)==NULL){
                        reply<<-1;
                    } else if (SLocks->at(lockID)->owner != outPktHdr->to || (SCVs->at(cvID)->lockIndex != lockID && SCVs->at(cvID)->lockIndex != -1)){
                        reply << -1;
                    }else {
                        SCVs->at(cvID)->useCounter++;
                        pass = false;
                        if(SCVs->at(cvID)->lockIndex == -1){
                            SCVs->at(cvID)->lockIndex = lockID;
                        }
                        SCVs->at(cvID)->packetWaiting->push(outPktHdr);
                        SCVs->at(cvID)->mailWaiting->push(outMailHdr);

                        PacketHeader* tempOutPkt = SLocks->at(lockID)->packetWaiting->front();
                        MailHeader* tempOutMail = SLocks->at(lockID)->mailWaiting->front();

                        if(!(tempOutPkt)==NULL){
                            SLocks->at(lockID)->packetWaiting->pop();
                            SLocks->at(lockID)->mailWaiting->pop();
                            SLocks->at(lockID)->owner=tempOutPkt->to;
                            reply<<-2;
                            sendMessage(tempOutPkt, tempOutMail, reply);
                        }
                        else{
                            SLocks->at(lockID)->state = Available;
                        }
                    }
                }
                if(pass){
                    sendMessage(outPktHdr, outMailHdr, reply);
                }
                CVLock->Release();
                break;
            }
        case RPC_Signal: {
                CVLock->Acquire();
                ss>> lockID >> cvID;
                cout<<"RPC Signal: " << cvID << endl; 

                if(lockID < 0 || lockID >= SLocks->size() || cvID < 0 || cvID >= SCVs->size()){
                    reply << -1;
                } 
                else{
                    if(SLocks->at(lockID)==NULL||SCVs->at(cvID)== NULL){
                        reply << -1;
                    } else if(SLocks->at(lockID)->owner != outPktHdr->to || SCVs->at(cvID)->lockIndex != lockID){
                        reply << -1;
                    } else{
                        reply<<-2;
                        SCVs->at(cvID)->useCounter--;
                        PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                        SCVs->at(cvID)->packetWaiting->pop();
                        MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                        SCVs->at(cvID)->mailWaiting->pop();
                        sendMessage(tempOutPkt, tempOutMail, reply);

                        if(SCVs->at(cvID)->packetWaiting->empty()){
                            SCVs->at(cvID)->lockIndex = -1; 
                        }
                    }
                }
                cout << "lock to unwait goes from: " << endl;
                cout << outPktHdr->from << " to " << outPktHdr->to << endl;
                cout << "with the reply: " << endl;
                cout << reply << endl;
                sendMessage(outPktHdr, outMailHdr, reply);
                CVLock->Release();
                break;       

        }
        case RPC_Broadcast: {
            CVLock->Acquire();
            ss>>lockID >> cvID;
            cout<<"RPC Broadcast cv ID lock ID: " << cvID << " " << lockID<< endl; 

            if(lockID < 0 || lockID >= SLocks->size() || cvID < 0 || cvID >= SCVs->size()){
                    reply << -1;
                } 
                else{
                    if(SLocks->at(lockID)==NULL||SCVs->at(cvID)== NULL){
                        reply << -1;
                    } else if(SLocks->at(lockID)->owner != outPktHdr->to || SCVs->at(cvID)->lockIndex != lockID){
                        reply << -1;
                    } else{
                        if(SCVs->at(cvID)->packetWaiting->empty()){
                            reply<<-1;
                        } else{
                            while(!SCVs->at(cvID)->packetWaiting->empty()){
                                reply << -2;
                                SCVs->at(cvID)->useCounter--;
                                 PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                SCVs->at(cvID)->packetWaiting->pop();
                                MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                SCVs->at(cvID)->mailWaiting->pop();
                                sendMessage(tempOutPkt, tempOutMail, reply);
                            }
                            SCVs->at(cvID)->lockIndex = -1;
                        }

                    }
                }

                sendMessage(outPktHdr, outMailHdr, reply);
                CVLock->Release();
                break;       
        }

        case RPC_CreateMV: {
            lockLock->Acquire(); 
            ss>>name>>mvSize; 

            int index = -1;

            for(int i = 0; i < SMVs->size(); i++){
                if(SMVs->at(i) != NULL){
                    if(SMVs->at(i)->name == name){
                        index = i;
                        reply << i; 
                        break;
                    }
                }
            }
            if(index == -1){ // MV doesnt already exist
                ServerMV *mv = new ServerMV;
                mv->name = name;
                mv->values = new int[mvSize];
                mv->len = mvSize;
                for(unsigned int i = 0; i < mvSize; i++){
                    mv->values[i] = 0;
                }
                mv->toBeDeleted = false;
                SMVs->push_back(mv);

                reply <<SMVs->size()-1;
            }
            sendMessage(outPktHdr, outMailHdr, reply);
            break;
        }
        case RPC_DestroyMV: {
            cout<<"In destroy MV RPC " <<endl;
            ss >> mvID;
            if(mvID < 0 || mvID >= SMVs->size()){
                reply << -1;
            } else{
                if(SMVs->at(mvID)==NULL){
                    reply << -1;
                } else{
                   ServerMV *mv = SMVs->at(mvID);
                   SMVs->at(mvID)=NULL;
                   delete mv; 
                   reply << mvID; 
                }
            }

            sendMessage(outPktHdr, outMailHdr, reply);
        }

        case RPC_GetMV: {

            ss >> mvID >> mvIndex; 
            if(mvID < 0 || mvID >= SMVs->size() || mvIndex < 0){
                reply << -1;
            } else {
                if(SMVs->at(mvID)==NULL){
                    reply << -1;
                } else if(mvIndex >= SMVs->at(mvID)->len){
                    reply << -1;
                } else{
                    reply << SMVs->at(mvID)->values[mvIndex];
                }
            }
            sendMessage(outPktHdr, outMailHdr, reply);
        }

        case RPC_SetMV: {

            ss >> mvID >> mvIndex >> mvVal;

            if(mvID < 0 || mvID >= SMVs->size() || mvIndex < 0){
                reply << -1;
            }
            else{
                if(SMVs->at(mvID)==NULL){
                    reply << -1;
                } else if (mvIndex >= SMVs->at(mvID)->len){
                    reply << -1;
                } else{
                    SMVs->at(mvID)->values[mvIndex] = mvVal; 
                    reply << mvVal; 
                }
            }
            sendMessage(outPktHdr, outMailHdr, reply);
            break;
        }
    default:
        std::cout<<"Unknown RPC \n" << type << std::endl;
        continue;
        break;
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


