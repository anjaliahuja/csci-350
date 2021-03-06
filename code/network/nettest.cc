// nettest.cc 
//  Test out message delivery between two "Nachos" machines,
//  using the Post Office to coordinate delivery.
//
//  Two caveats:
//    1. Two copies of Nachos must be running, with machine ID's 0 and 1:
//      ./nachos -m 0 -o 1 &
//      ./nachos -m 1 -o 0 &
//
//    2. You need an implementation of condition variables,
//       which is *not* provided as part of the baseline threads 
//       implementation.  The Post Office won't work without
//       a correct implementation of condition variables.
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
#include <map>
#include "system.h"
#include "network.h"
#include "post.h"
#include "interrupt.h"

// Test out message delivery, by doing the following:
//  1. send a message to the machine with ID "farAddr", at mail box #0
//  2. wait for the other machine's message to arrive (in our mailbox #0)
//  3. send an acknowledgment for the other machine's message
//  4. wait for an acknowledgement from the other machine to our 
//      original message

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
    int index;
};

struct ServerCV{
    string name;
    queue<PacketHeader*>* packetWaiting;
    queue<MailHeader*>* mailWaiting;
    int lockIndex;
    bool toBeDeleted;
    int counter;
    int useCounter;
    int index;
};

struct ServerMV{
    string name;
    int* values;
    int len;
    bool toBeDeleted;
    int index;
};

struct ServerRequest{
    string name;
    int requestID;
    int reqType;
    int machineID;
    int mailbox;
    int arg1;
    int arg2;
    int arg3;

    /*server side data*/
    bool yes; // if yes reply is received
    int noCount; // number of no counts received;
    bool lockFound;
    bool cvFound;
    int lockCount;
    int cvCount;

    PacketHeader* replyServerMachineID;
    MailHeader* replyServerMailbox;
    PacketHeader* replyServerMachineID_two;
    MailHeader* replyServerMailbox_two;

};

void sendMessage(PacketHeader* outPktHdr, MailHeader* outMailHdr, stringstream& msg){
    //outPktHdr is machineID, outMailHdr is mailbox num
    int msgLen = msg.str().length() +1;
    outMailHdr->length = msgLen;
    char *message = new char[msgLen];
    std::strcpy(message, msg.str().c_str());
    cout << "sending! from: " << outMailHdr->from << " to: " << outMailHdr->to << " with message: " << endl << message << endl;
    if(!postOffice->Send(*outPktHdr, *outMailHdr, message)){
        printf("Error in server, cannot send message \n");
    }
    msg.clear();
}

void sendReplyToServer(PacketHeader* outPktHdr, MailHeader* outMailHdr, int requestType, int requestID, int machineID, int mailbox, int reply){
    stringstream msg; 
    msg << requestType << " " << requestID << " " << machineID << " " << mailbox << " " << reply;
    sendMessage(outPktHdr, outMailHdr, msg);
}

void sendReplyToClient(int machineID, int mailbox, int reply){
     PacketHeader* outPktHdr = new PacketHeader();
    MailHeader* outMailHdr = new MailHeader();

    outPktHdr->to = machineID; //clinet machineID
    outMailHdr->to = mailbox;
    outMailHdr->from = netname;
    
    stringstream message;
    message << reply; 
    sendMessage(outPktHdr, outMailHdr, message);

}

void CreateServerRequest(vector<ServerRequest*>* serverRequests, string name, int reqType, int machineID, int mailbox, int arg1, int arg2, int arg3){
    ServerRequest *sr = new ServerRequest; 
    serverRequests->push_back(sr);

    sr->requestID = serverRequests->size()-1;
    sr->machineID = machineID;
    sr->mailbox = mailbox;
    sr->reqType = reqType;
    sr->arg1 = arg1;
    sr->arg2 = arg2;
    sr->arg3 = arg3;

    sr->noCount = 0;
    sr->yes = false;
    sr->name = name; 
    sr->lockCount = 0;
    sr->cvCount = 0;
    sr->lockFound = false;
    sr->cvFound  = false;

   

    for (int i = 0; i < NUM_SERVERS; ++i) // 5 being num max servers 
    {
        if (i != netname)
        {
            cout << "Creating server request, sending to: " << i << endl;
            PacketHeader* outPktHdr = new PacketHeader();
            MailHeader* outMailHdr = new MailHeader();
            outPktHdr->to = i;
            outMailHdr->to = i;
            outMailHdr->from = netname; 

            std::stringstream ss;

            if(reqType == RPC_Server_CreateCV || reqType == RPC_Server_CreateLock || reqType == RPC_Server_CreateMV){
                ss << sr->reqType << " " << sr->requestID << " "<< sr->machineID << " " << sr->mailbox << " " << sr->name; 
            } else if (reqType == RPC_Server_Acquire || reqType == RPC_Server_Release || reqType == RPC_Server_DestroyLock || reqType == RPC_Server_DestroyCV || reqType == RPC_Server_DestroyMV){
                ss << sr->reqType << " " << sr->requestID << " "<<sr->machineID << " " << sr->mailbox << " " << sr->arg1; 
            } else if(reqType == RPC_GetMV || reqType == RPC_Server_Wait1 || reqType == RPC_Server_Wait2 || reqType == RPC_Server_Wait3 || reqType == RPC_Server_Signal1 || reqType == RPC_Server_Signal2 || reqType == RPC_Server_Signal3 || reqType == RPC_Server_Broadcast1 || reqType == RPC_Server_Broadcast2 || reqType == RPC_Server_Broadcast3){
                ss << sr->reqType << " " << sr->requestID << " "<<sr->machineID << " " << sr->mailbox << " " << sr->arg1 << " " <<sr->arg2; 
            } else {
                ss << sr->reqType << " " << sr->requestID << " "<<sr->machineID << " " << sr->mailbox << " " << sr->arg1 << " " <<sr->arg2 << " " << sr->arg3; 
            }
            sendMessage(outPktHdr, outMailHdr, ss);
        }
    }
}


bool validChecks(vector<ServerRequest*>* serverRequests, vector<ServerLock*>* SLocks, vector<ServerCV*>* SCVs, int lockID, int cvID, int machineID, int mailbox, int reqType) {
    bool ifValid = true;
    cout<<"in valid checks"<<endl;
    if(lockID / 100 != netname || cvID / 100 != netname ){
        ifValid = false;
        if(lockID / 100 != netname && cvID / 100 !=netname){ // if you don't own lock or CV
            cout<<"dont own lock or CV"<<endl;
            CreateServerRequest(serverRequests, "", reqType + 2, machineID, mailbox, lockID, cvID, 0);
        } else if (lockID / 100 != netname){
            //lock doesnt exist, cv does so make sure cv info is valid 
            cout<<"Lock not on server, cv is"<<endl;
            int cv = cvID % 100; 

            if(cv < 0 || cv >= SCVs->size()){
                sendReplyToClient(machineID, mailbox, -1);
            }else if(SCVs->at(cv) == NULL){
                sendReplyToClient(machineID, mailbox, -1);
            }else if((reqType == RPC_Server_Signal1 && SCVs->at(cv)->lockIndex != lockID ) ||  (reqType == RPC_Server_Wait1 && SCVs->at(cv)->lockIndex != lockID && SCVs->at(cv)->lockIndex != -1)){
                sendReplyToClient(machineID, mailbox, -1);
            }else{
                CreateServerRequest(serverRequests, "", reqType+1, machineID, mailbox, lockID, cvID, 0);
            }
        } else if (cvID / 100 != netname){
            cout<<"cv not on server, lock is"<<endl;
            //CV doesnt exist, lock does
            int lock = lockID % 100; 
            if(lock < 0 || lock >= SLocks->size()){
                sendReplyToClient(machineID, mailbox, -1);
            }else if (SLocks->at(lock)==NULL){
                sendReplyToClient(machineID, mailbox, -1);
            } else if(SLocks->at(lock)->owner != machineID){
                sendReplyToClient(machineID, mailbox, -1);
            } else {
                CreateServerRequest(serverRequests, "", reqType, machineID, mailbox, lockID, cvID, 0);
            }
        }


        }

    
    return ifValid;
}

void Server2(){
    cout << "server " << netname << " has started" << endl;
    vector<ServerLock*>* SLocks = new vector<ServerLock*>;
    vector<ServerCV*>* SCVs = new vector<ServerCV*>;
    vector<ServerMV*>* SMVs = new vector<ServerMV*>;
    vector<ServerRequest *> *serverRequests = new vector<ServerRequest*>;


    map<int, int> requestTable;
    int requestNum = 0;

    int lockID, cvID, mvID, mvVal, mvIndex, mvSize;

    int uniqueID = netname * 100; 

    while(true){
        PacketHeader* outPktHdr = new PacketHeader();
        PacketHeader* outPktHdr2 = new PacketHeader();
        PacketHeader* inPktHdr = new PacketHeader();
        MailHeader* outMailHdr = new MailHeader();
        MailHeader* outMailHdr2 = new MailHeader();
        MailHeader* inMailHdr = new MailHeader(); 

        char buffer[MaxMailSize]; 

        cout << "waiting for message!" << endl;
        postOffice->Receive(netname, inPktHdr, inMailHdr, buffer); 
        cout << "got message! from: " << inMailHdr->from << " to: " << inMailHdr->to << " with message: " << endl << buffer << endl;
        fflush(stdout);

        int RPCType;
        stringstream ss;
        ss<<buffer;
        ss>>RPCType;


        string name;
        int lockID, cvID, mvSize, mvID, mvPos, mvVal;
        int requestID, machineID, mailbox, arg1, arg2, arg3, response;
        stringstream reply;

        if (RPCType/100 == 0) { //server vs. client 

            outPktHdr->to = inPktHdr->from;
            outMailHdr->to = inMailHdr->from;
            outMailHdr->from = netname;

            switch(RPCType){
                case RPC_CreateLock: {
                    string name = "";
                    ss >> name;
                    cout << "CREATELOCK w/ name " << name << endl;
                    int existingLock = -1;

                    for(int i = 0; i<SLocks->size(); i++){
                        cout << "comparing-" << SLocks->at(i)->name << "- to -" << name << endl;
                        if(SLocks->at(i)->name == name){
                            existingLock = i;
                            break;
                        }
                    }
                    if(existingLock == -1){
                        CreateServerRequest(serverRequests, name, RPC_Server_CreateLock, inPktHdr->from, inMailHdr->from, 0, 0, 0);
                    } else {
                        cout<<"found lock!! " << existingLock<<endl;
                        reply << existingLock + uniqueID; 
                        sendMessage(outPktHdr, outMailHdr, reply);
                    }
                    break;
                }
                case RPC_DestroyLock: {
                    ss >> lockID; 
                    if(lockID / 100 != netname){
                        CreateServerRequest(serverRequests, "", RPC_Server_DestroyLock, inPktHdr->from, inMailHdr->from, lockID, 0, 0);
                    } else{
                        lockID = lockID % 100; 
                        if(lockID < 0 || lockID >= SLocks->size()){
                            reply << -1; 
                        }else{
                            if(SLocks->at(lockID)==NULL){
                                reply<<-1;
                            }else{
                                SLocks->at(lockID)->toBeDeleted = true;
                                reply<<-2;
                            }
                        }
                        sendMessage(outPktHdr, outMailHdr, reply);
                    }
                    break;
                }
                case RPC_CreateCV: {
                    string name = "";
                    ss >> name;

                    int existingCV = -1;

                    for(int i =0; i< SCVs->size(); i++){
                        if(SCVs->at(i)->name == name){
                            cout<<"CV FOUND "<<endl;
                            existingCV = i;
                            break;
                        }
                    }

                    if(existingCV == -1){
                        cout<<"CV Not found" <<endl;
                        CreateServerRequest(serverRequests, name, RPC_Server_CreateCV, inPktHdr->from, inMailHdr->from, 0, 0, 0);
                    }
                    else{
                        reply << existingCV + uniqueID; 
                        sendMessage(outPktHdr, outMailHdr, reply);
                    }
                    break;
                }
                case RPC_DestroyCV: {
                    ss >> cvID; 
                    if(cvID / 100 != netname){
                        CreateServerRequest(serverRequests, "", RPC_Server_DestroyCV, inPktHdr->from, inMailHdr->from, lockID, 0, 0);
                    } else{
                        cvID = cvID % 100; 
                        if(cvID < 0 || cvID >= SCVs->size()){
                            reply << -1; 
                        }else{
                            if(SCVs->at(cvID)==NULL){
                                reply<<-1;
                            }else{
                                SCVs->at(cvID)->toBeDeleted = true;
                                reply<<-2;
                            }
                        }
                        sendMessage(outPktHdr, outMailHdr, reply);
                    }
                    break;
                }
                case RPC_Acquire: {
                    ss >> lockID;
                    bool pass = true;

                    if(lockID / 100 != netname){
                        cout << "Couldn't find lock to ACQUIRE" << endl;
                        CreateServerRequest(serverRequests, "", RPC_Server_Acquire, inPktHdr->from, inMailHdr->from, lockID, 0, 0);
                    } else{
                        lockID = lockID%100;
                        if(SLocks -> at(lockID) == NULL){
                            reply << -1;
                        } else if(SLocks->at(lockID)->owner == outPktHdr->to && SLocks->at(lockID)->state == Busy){
                            reply << -1; 
                        } else if(SLocks->at(lockID)->state == Busy){
                            pass= false;
                            SLocks->at(lockID)->packetWaiting->push(outPktHdr);
                            int sizeofPacket = SLocks->at(lockID)->packetWaiting->size();
                            SLocks->at(lockID)->mailWaiting->push(outMailHdr);
                        } else{
                            SLocks->at(lockID)->owner = outPktHdr->to;
                            cout<<"Owner of lock after acquiring is: " << SLocks->at(lockID)->owner << endl;
                            SLocks->at(lockID)->state = Busy;
                            reply << -2; 
                        }
                    }
                    if(pass){
                        sendMessage(outPktHdr, outMailHdr, reply);
                    }
                    break;
                }
                case RPC_Release: {
                    ss>>lockID;
                    if(lockID / 100 != netname){
                        CreateServerRequest(serverRequests, "", RPC_Server_Release, inPktHdr->from, inMailHdr->from, lockID, 0, 0);
                    } else {
                        if(lockID < 0 || lockID >= SLocks->size()){
                            reply << -1;
                        }
                        else{
                        if(SLocks->at(lockID)== NULL){
                            reply << -1;
                        } else if(SLocks->at(lockID)->state == Available || SLocks->at(lockID)->owner != outPktHdr->to){

                            reply << -1;
                        } else{
                            reply << -2; 
                            if(SLocks->at(lockID)->packetWaiting->empty() && SLocks->at(lockID)->mailWaiting->empty()){ 
                                //check if anyone is waiting on the lock
                                SLocks->at(lockID)->state = Available; 
                                SLocks->at(lockID)->owner = -1;
                            } else{
                                //change ownership, send message to waiting client
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
                }
                    break;
                }
                case RPC_Wait: {
                    ss>>lockID >> cvID;
                    int lockNum = lockID;
                    int cvNum = cvID;

                    bool ifValid = validChecks(serverRequests, SLocks, SCVs, lockID, cvID, inPktHdr->from, inMailHdr->from, RPC_Server_Wait1);
                    if(ifValid){

                        lockID = lockID%100;
                        cvID = cvID%100;
                        bool pass = true;
                    

                        if(lockID < 0 || lockID >= SLocks->size() || cvID < 0 || cvID >= SCVs->size()){
                            reply << -1;
                        }
                        else{
                            if(SLocks->at(lockID)==NULL || SCVs->at(cvID)==NULL){
                                reply<<-1;
                            } else if (SLocks->at(lockID)->owner != outPktHdr->to || (SCVs->at(cvID)->lockIndex != lockNum && SCVs->at(cvID)->lockIndex != -1)){
                                reply << -1;
                            }else {
                                SCVs->at(cvID)->useCounter++;
                                pass = false;
                                if(SCVs->at(cvID)->lockIndex == -1){
                                    SCVs->at(cvID)->lockIndex = lockNum;
                                }
                                SCVs->at(cvID)->packetWaiting->push(outPktHdr);
                                SCVs->at(cvID)->mailWaiting->push(outMailHdr);


                                PacketHeader* waitOutPkt = SLocks->at(lockID)->packetWaiting->front();
                                MailHeader* waitOutMail = SLocks->at(lockID)->mailWaiting->front();

                                if(!(waitOutPkt == NULL)){
                                    SLocks->at(lockID)->packetWaiting->pop();
                                    SLocks->at(lockID)->mailWaiting->pop();
                                    SLocks->at(lockID)->owner = waitOutPkt->to;
                                    reply << -2;
                                    sendMessage(waitOutPkt, waitOutMail, reply);
                                } else{
                                    SLocks->at(lockID)->state = Available; 
                                }

                            }
                        }
                    if(pass){
                        sendMessage(outPktHdr, outMailHdr, reply);
                    }
                }
                    break;
                }
                case RPC_Signal: {
                    ss >> lockID >> cvID; 
                    cout<<"LockID: "<<lockID<<endl;
                    cout<<"cvID: "<<cvID<<endl;
                    int lockNum = lockID;
                    bool ifValid = validChecks(serverRequests, SLocks, SCVs, lockID, cvID, inPktHdr->from, inMailHdr->from, RPC_Server_Signal1);
                    if (ifValid){
                        
                        lockID = lockID%100;
                        cvID = cvID%100;
                         
                        if(lockID < 0 || lockID >= SLocks->size() || cvID < 0 || cvID >= SCVs->size()){
                            cout<<"Case 1 error"<<endl;
                            reply << -1;
                        }
                     
                        else{
                            if(SLocks->at(lockID)==NULL||SCVs->at(cvID)== NULL){
                                reply << -1;
                            } else if(SLocks->at(lockID)->owner != outPktHdr->to || SCVs->at(cvID)->lockIndex != lockNum){
                                reply << -1;
                            } else{

                                if(SCVs->at(cvID)->packetWaiting->empty()){
                                    reply<<-1;
                                } else{
                                    reply<<-2;
                                    SCVs->at(cvID)->useCounter--;
                                    PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                    SLocks->at(lockID)->packetWaiting->push(tempOutPkt);
                                    SCVs->at(cvID)->packetWaiting->pop();
                                    MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                    SLocks->at(lockID)->mailWaiting->push(tempOutMail);
                                    SCVs->at(cvID)->mailWaiting->pop();
                                    sendMessage(tempOutPkt, tempOutMail, reply);

                                    if(SCVs->at(cvID)->packetWaiting->empty()){
                                        SCVs->at(cvID)->lockIndex
                                         = -1; 
                                    }
                                }
                            }
                        }
                    }
                    sendMessage(outPktHdr, outMailHdr, reply);
                    break;  

                }
                case RPC_Broadcast: {
                    ss>>lockID >> cvID;
                    cout<<"RPC Broadcast cv ID lock ID: " << cvID << " " << lockID<< endl; 
                    bool ifValid = validChecks(serverRequests, SLocks, SCVs, lockID, cvID, inPktHdr->from, inMailHdr->from, RPC_Server_Broadcast1);
                    
                    if(ifValid){
                        lockID = lockID % 100;
                        cvID = cvID % 100; 

                        if(lockID < 0 || lockID >= SLocks->size() || cvID < 0 || cvID >= SCVs->size()){
                                reply << -1;
                        }else{
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
                                       PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                        SLocks->at(lockID)->packetWaiting->push(tempOutPkt);
                                        SCVs->at(cvID)->packetWaiting->pop();
                                        MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                        SLocks->at(lockID)->mailWaiting->push(tempOutMail);
                                        SCVs->at(cvID)->mailWaiting->pop();
                                        sendMessage(tempOutPkt, tempOutMail, reply);
                                    }
                                    SCVs->at(cvID)->lockIndex = -1;
                                }

                            }
                        }
                        sendMessage(outPktHdr, outMailHdr, reply);
                    }
                        
                    break;       
                }
                case RPC_CreateMV: {
                    ss>>name>>mvSize; 
                    cout<<"RPC CreateMV: " << name << endl;

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
                    if(index == -1)
                    {
                      CreateServerRequest(serverRequests, name, RPC_Server_CreateMV, inPktHdr->from, inMailHdr->from, mvSize, 0, 0);
                    } else {
                        reply << index + uniqueID;
                        sendMessage(outPktHdr, outMailHdr, reply);
                    }
                   
                    break;
                }
                case RPC_DestroyMV: {
                    cout<<"RPC Destroy MV: "<<mvID <<endl;
                    ss >> mvID >> mvIndex >> mvVal; 

                    if(mvID / 100 != netname){
                        CreateServerRequest(serverRequests, "", RPC_Server_DestroyMV, inPktHdr->from, inMailHdr->from, mvID, 0, 0);
                    }else{
                        mvID = mvID % 100;
                        if(mvID < 0 || mvID >= SMVs->size()){
                            reply << -1;
                        } else{
                            if(SMVs->at(mvID)==NULL){
                                reply << -1;
                            } else{
                              SMVs->at(mvID)->toBeDeleted = true;
                              reply<<-2;
                            }
                         }
                                sendMessage(outPktHdr, outMailHdr, reply);
                    }
                    break;
                }
                case RPC_SetMV: {
                     ss >> mvID >> mvIndex >> mvVal;

                     if(mvID / 100 != netname){
                        CreateServerRequest(serverRequests, "", RPC_Server_SetMV, inPktHdr->from, inMailHdr->from, mvID, mvIndex, mvVal);
                     } else {
                            cout<<"RPC SetMV: " << mvID << " at index "<<mvIndex<< " to value "<<mvVal<<endl;
                            mvID = mvID % 100;
                            if(mvID < 0 || mvID >= SMVs->size() || mvIndex < 0){
                                reply << -1;
                            } else{
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
                        }
                        break;
                }
                case RPC_GetMV:{
                    ss >> mvID >> mvIndex; 
                    if(mvID / 100 != netname){
                        CreateServerRequest(serverRequests, "", RPC_Server_GetMV, inPktHdr->from, inMailHdr->from, mvID, mvIndex, 0);
                    } else {
                        mvID = mvID % 100; 
                        cout<<"RPC GetMV: " << mvID << " at index "<<mvIndex<<endl;
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
                    break;
                }
                default: {
                    cout << "Unknown message type. \n";
                    continue;
                }
            }
        }
        else if (RPCType / 100 == 1){ // dealing with server request 
            ss >> requestID;
            ss >> machineID;
            ss >> mailbox;

            outPktHdr->to = inPktHdr->from;
            outMailHdr->to = inMailHdr->from;
            outMailHdr->from = netname;


            switch(RPCType){
                case RPC_Server_CreateLock: {
                    string name = "";
                        ss >> name;
                        int existingLock = -1;

                        for(int i = 0; i<SLocks->size(); i++){
                            if(SLocks->at(i)->name == name){
                                existingLock = i;
                                break;
                            }
                        }
                        if(existingLock == -1){
                            sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_CreateLock, requestID, machineID, mailbox, 0);
                        } else {
                           sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_CreateLock, requestID, machineID, mailbox, 1);
                           sendReplyToClient(machineID, mailbox, existingLock + uniqueID);
                        }
                    break;
                }
                case RPC_Server_DestroyLock: {
                    ss >> lockID; 
                    if(lockID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_DestroyLock, requestID, machineID, mailbox, 0);
                    } else{
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_DestroyLock, requestID, machineID, mailbox, 1);

                        lockID = lockID % 100; 
                        if(lockID < 0 || lockID >= SLocks->size()){
                            sendReplyToClient(machineID, mailbox, -1);
                        }else{
                            if(SLocks->at(lockID)==NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                            }else{
                                SLocks->at(lockID)->toBeDeleted = true;
                                sendReplyToClient(machineID, mailbox, -2);
                            }
                        }
                    }
                    break;
                }
                case RPC_Server_CreateCV: {
                    string name = "";
                    ss >> name;

                    int existingCV = -1;

                    for(int i =0; i< SCVs->size(); i++){
                        if(SCVs->at(i)->name == name){
                            existingCV = i;
                            break;
                        }
                    }

                    if(existingCV = -1){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_CreateCV, requestID, machineID, mailbox, 0);
                    }
                    else{
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_CreateCV, requestID, machineID, mailbox, 1);
                        sendReplyToClient(machineID, mailbox, existingCV + uniqueID);
                    }
                    break;
                }
                case RPC_Server_DestroyCV: {
                    ss >> cvID; 
                    if(cvID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_DestroyCV, requestID, machineID, mailbox, 0);
                    } else{
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_DestroyCV, requestID, machineID, mailbox, 1);

                        cvID = cvID % 100; 
                        if(cvID < 0 || cvID >= SCVs->size()){
                            sendReplyToClient(machineID, mailbox, -1);
                        }else{
                            if(SCVs->at(cvID)==NULL){
                            sendReplyToClient(machineID, mailbox, -1);
                            }else{
                                SCVs->at(cvID)->toBeDeleted = true;
                            sendReplyToClient(machineID, mailbox, -2);
                            }
                        }
                    }
                    break;
                }
                case RPC_Server_Acquire: {
                    ss >> lockID;

                    if(lockID / 100 != netname){
                        cout<<"Acquire case 1 " << endl;
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Acquire, requestID, machineID, mailbox, 0);
                    } else{
                        cout<<"Acquire case 2 " << endl;
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_DestroyCV, requestID, machineID, mailbox, 1);

                        lockID = lockID % 100;

                        if(lockID < 0 || lockID >= SLocks->size()){
                            sendReplyToClient(machineID, mailbox, -1);
                            cout<<"Acquire case 3 " << endl;
                        }
                        else {
                        if(SLocks -> at(lockID) == NULL){
                            sendReplyToClient(machineID, mailbox, -1);
                            cout<<"Acquire case 4 " << endl;
                        } else if(SLocks->at(lockID)->owner == machineID && SLocks->at(lockID)->state == Busy){
                            sendReplyToClient(machineID, mailbox, -1);
                            cout<<"Acquire case 5 " << endl;
                        } else if(SLocks->at(lockID)->state == Busy){
                            cout<<"Lock is busy, adding to wait queue"<<endl;
                            cout<<"Lock owner is: " << SLocks->at(lockID)->owner << endl;
                            PacketHeader *tempOutPkt = new PacketHeader();
                            MailHeader *tempOutMail = new MailHeader();

                            tempOutPkt->to = machineID;
                            tempOutMail->to = mailbox;
                            tempOutMail->from = netname;


                            SLocks->at(lockID)->packetWaiting->push(tempOutPkt);
                            int sizeofPacket = SLocks->at(lockID)->packetWaiting->size();
                            SLocks->at(lockID)->mailWaiting->push(tempOutMail);
                        } else{
                            SLocks->at(lockID)->owner = machineID;
                            cout<<"Acquire changed lock ownership"<<endl;
                            cout<<"Lock owner is: " << SLocks->at(lockID)->owner << endl;
                            SLocks->at(lockID)->state = Busy;
                            sendReplyToClient(machineID, mailbox, -2);
                        }
                    }
                   }
                    break;
                }
                case RPC_Server_Release: {
                    ss>>lockID;
                    if(lockID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Release, requestID, machineID, mailbox, 0);
                    } else {
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Release, requestID, machineID, mailbox, 1);

                        if(lockID < 0 || lockID >= SLocks->size()){
                        reply << -1;
                        } else{
                            if(SLocks->at(lockID)== NULL){
                            sendReplyToClient(machineID, mailbox, -1);
                            } else if(SLocks->at(lockID)->state == Available){
                                 sendReplyToClient(machineID, mailbox, -1);
                            } else{
                                if(SLocks->at(lockID)->packetWaiting->empty() && SLocks->at(lockID)->mailWaiting->empty()){ 

                                    SLocks->at(lockID)->state = Available; 

                                    SLocks->at(lockID)->owner = -1;
                                } else{
                                    PacketHeader* tempOutPkt = SLocks->at(lockID)->packetWaiting->front();
                                    SLocks->at(lockID)->packetWaiting->pop();
                                    MailHeader* tempOutMail = SLocks->at(lockID)->mailWaiting->front();
                                    SLocks->at(lockID)->mailWaiting->pop();

                                    SLocks->at(lockID)->owner = tempOutPkt->to;
                                    sendReplyToClient(tempOutPkt->to, tempOutMail->to, -2);
                                    /*if(SLocks->at(lockID)->packetWaiting->empty() && SLocks->at(lockID)->toBeDeleted == true){
                                        ServerLock* lock = SLocks->at(lockID);
                                        SLocks->at(lockID) = NULL;
                                        delete lock;
                                }*/
                            }
                            sendReplyToClient(machineID, mailbox, -2);
                            }
                        }
                    }
                    break;
                }
                case RPC_Server_Signal1: {
                    //Case where lock is on server but CV is on a different server 

                    ss >> lockID >> cvID; 

                    cout <<"Server signal 1, lock on server but CV on different server " << endl;

                    if (cvID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal1, requestID, machineID, mailbox, 0);
                    } else {
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal1, requestID, machineID, mailbox, 1);

                        cvID = cvID % 100; // index of CV 

                         if (cvID < 0 || cvID >= SCVs->size()){
                                sendReplyToClient(machineID, mailbox, -1);
                        } else {
                            if(SCVs->at(cvID) == NULL){
                            sendReplyToClient(machineID, mailbox, -1);
                            } else if (SCVs->at(cvID)->lockIndex == lockID){ // waiting client
                                if(SCVs->at(cvID)->packetWaiting->empty()){
                                    sendReplyToClient(machineID, mailbox, -1);
                                } else {
                                    reply << -2;

                                    PacketHeader *tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                    SCVs->at(cvID)->packetWaiting->pop();

                                    MailHeader *tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                    SCVs->at(cvID)->mailWaiting->pop();

                                    sendMessage(tempOutPkt, tempOutMail, reply);

                                    if(SCVs->at(cvID)->packetWaiting->empty()){
                                        SCVs->at(cvID)->lockIndex = -1;
                                    }
                                    sendReplyToClient(machineID, mailbox, -2);
                                }
                            }
                        }
                    }
                }
                case RPC_Server_Signal2: {
                    cout<<"Signal 2: cv on server but lock on different server"<<endl;
                    //case where cv is on server but lock is on a different server 
                    //Replies:
                    //-1: error in lock
                    //0: not found
                    //1: lock found and valid
                    ss >> lockID >> cvID; 

                    if(lockID/100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal2, requestID, machineID, mailbox, 0); 
                    } else {
                        lockID = lockID % 100;


                        if(lockID < 0 || lockID >= SLocks->size()){
                            sendReplyToClient(machineID, mailbox, -1);
                            sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal2, requestID, machineID, mailbox, -1); 

                        } else {
                            if(SLocks->at(lockID) == NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal2, requestID, machineID, mailbox, -1); 
                            } else if (SLocks->at(lockID)->owner != netname){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal2, requestID, machineID, mailbox, -1); 
                            } else {
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal2, requestID, machineID, mailbox, 1); // client owns lock, send Yes to server 
                            }
                        }
                    }
                }
                case RPC_Server_Signal3: {
                    //CV AND LOCK are on different servers 


                    cout<<"Server wait 3, no lock or CV on current server " << endl;
                    ss>>lockID>>cvID;

                    bool foundLock = false;
                    bool invalidLock = false;
                    int lockNum = lockID;
                    
                    if(lockID / 100 != netname){
                        foundLock = false;
                    } else {
                        lockID = lockID%100; 
                        foundLock = true; // lock is on server

                        if(lockID < 0 || lockID >= SLocks->size()){
                            sendReplyToClient(machineID, mailbox, -1);
                            invalidLock = true;

                        } else {
                            if(SLocks->at(lockID) == NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                                invalidLock = true;
                            } else if (SLocks->at(lockID)->owner != netname){
                                sendReplyToClient(machineID, mailbox, 1);
                                invalidLock = true;
                            } else {

                            }
                        }
                    }
                    //Reply cases: -1 is error, 0 is no lock nor CV, 1 is both lock and CV, 2 is only lock, 3 is only CV
                    if(foundLock && invalidLock){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, -1);
                    } else {
                        //check lock ID
                        if(cvID / 100 != netname){
                            if(foundLock && !invalidLock){
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, 2); // found lock but not CV 
                            } else if(!foundLock){ // no lock or CV, reply with 0
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, 0); 
                                }
                            } else {
                                cvID = cvID % 100;

                                if (cvID < 0 || cvID >= SCVs->size()){
                                    sendReplyToClient(machineID, mailbox, -1);
                                } else {
                                    if(SCVs->at(cvID) == NULL || (SCVs->at(cvID)->lockIndex != lockID && SCVs->at(cvID)->lockIndex != -1)){
                                        sendReplyToClient(machineID, mailbox, -1);
                                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, -1);
                                    } else { //CV exists, get lock

                                        cvID = cvID % 100;
                                        if(cvID < 0 || cvID >= SCVs->size()){
                                            sendReplyToClient(machineID, mailbox, -1); 
                                        } else {
                                            if(SCVs->at(cvID)== NULL){
                                                sendReplyToClient(machineID, mailbox, -1);
                                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, -1); 
                                            } else if(SCVs->at(cvID)->lockIndex == lockNum){
                                                if(lockNum / 100 == netname){
                                                    if(SCVs->at(cvID)->packetWaiting->empty()){
                                                        sendReplyToClient(machineID, mailbox, -1);
                                                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, -1);
                                                    } else {
                                                        reply << -2;
                                                         PacketHeader *tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                                        SCVs->at(cvID)->packetWaiting->pop();

                                                        MailHeader *tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                                        SCVs->at(cvID)->mailWaiting->pop();

                                                        sendMessage(tempOutPkt, tempOutMail, reply);

                                                        if(SCVs->at(cvID)->packetWaiting->empty()){
                                                            SCVs->at(cvID)->lockIndex = -1;
                                                        }
                                                        sendReplyToClient(machineID, mailbox, -2);
                                                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, 1);
                                                    }
                                                } else { // lock doesn't exist on server
                                                    sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, 3);
                                                }
                                            } else {
                                                sendReplyToClient(machineID, mailbox, -1);
                                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, -1); 
                                                //lockID doesn't match cvs lock index so send error message back 
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;
                    }
                
                case RPC_Server_Wait1: { // lock on server, CV not on server
                    cout<<"Wait 1: lock on server, CV not on server" << endl;
                    ss >> lockID >> cvID; 
                    if(cvID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr,RPC_ServerReply_Wait1, requestID, machineID, mailbox, 0);
                    } else {

                            cvID = cvID%100;

                            if(cvID < 0 || cvID >= SCVs->size()){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr,RPC_ServerReply_Wait1, requestID, machineID, mailbox, -1);
                            }
                            else{
                                if(SLocks->at(lockID)==NULL || SCVs->at(cvID)==NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr,RPC_ServerReply_Wait1, requestID, machineID, mailbox, -1);
                                } 

                                else if (SLocks->at(lockID)->owner != outPktHdr->to || (SCVs->at(cvID)->lockIndex != lockID && SCVs->at(cvID)->lockIndex != -1)){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr,RPC_ServerReply_Wait1, requestID, machineID, mailbox, -1);
                                }else {
                                    SCVs->at(cvID)->useCounter++;
                                    if(SCVs->at(cvID)->lockIndex == -1){
                                        SCVs->at(cvID)->lockIndex = lockID;
                                    }

                                    PacketHeader* tempOutPkt = new PacketHeader();
                                    MailHeader* tempOutMail = new MailHeader();

                                    tempOutPkt->to = machineID;
                                    tempOutMail->to = mailbox;
                                    tempOutMail->from = netname;
                                    SCVs->at(cvID)->packetWaiting->push(tempOutPkt);
                                    SCVs->at(cvID)->mailWaiting->push(tempOutMail);

                                    sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Wait1, requestID, machineID, mailbox, 1);
                                }
                            }
                        }
                    break;
                }
                case RPC_Server_Wait2: {
                    cout<<"Wait 2: CV on server, lock not on server" << endl;
                    ss >> lockID >> cvID; 
                    if(lockID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr,RPC_ServerReply_Wait2, requestID, machineID, mailbox, 0);
                    } else {

                            int lockNum = lockID;
                            lockID = lockID%100;

                            if(lockID < 0 || lockID >= SLocks->size()){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr,RPC_ServerReply_Wait2, requestID, machineID, mailbox, -1);
                            }
                            else{
                                if(SLocks->at(lockID)==NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr,RPC_ServerReply_Wait2, requestID, machineID, mailbox, -1);
                                } 

                                else if (SLocks->at(lockID)->owner != outPktHdr->to || (SCVs->at(cvID)->lockIndex != lockNum && SCVs->at(cvID)->lockIndex != -1)){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr,RPC_ServerReply_Wait1, requestID, machineID, mailbox, -1);
                                }else {
                                    SCVs->at(cvID)->useCounter++;
                                    if(SCVs->at(cvID)->lockIndex == -1){
                                        SCVs->at(cvID)->lockIndex = lockNum;
                                    }

                                    PacketHeader *tempOutPkt = SLocks->at(lockID)->packetWaiting->front();
                                    MailHeader *tempOutMail = SLocks->at(lockID)->mailWaiting->front();

                                    if(!(SLocks->at(lockID)->packetWaiting->empty())){
                                        SLocks->at(lockID)->packetWaiting->pop();
                                        SLocks->at(lockID)->mailWaiting->pop();
                                        SLocks->at(lockID)->owner = tempOutPkt->to;
                                        reply << -2;
                                        sendMessage(tempOutPkt, tempOutMail, reply);
                                    }
                                    else{
                                        SLocks->at(lockID)->state = Available;
                                    }

                                  
                                    sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Wait2, requestID, machineID, mailbox, 1);
                                }
                            }
                        }


                    break;
                }
                case RPC_Server_Wait3: {
                    cout<<"Server wait 3, no lock or CV on current server " << endl;
                    ss>>lockID>>cvID;
                    cout << "lockID " << lockID << endl;

                    bool foundLock = false;
                    bool invalidLock = false;
                    int lockNum = lockID;
                    int cvNum = cvID;
                    
                    
                    if(lockID / 100 != netname){
                        cout << "case-1" << endl;
                        foundLock = false;
                    } else {
                        lockID = lockID%100; 
                        foundLock = true; // lock is on server

                        if(lockID < 0 || lockID >= SLocks->size()){
                            cout << "case-2" << endl;
                            sendReplyToClient(machineID, mailbox, -1);
                            invalidLock = true;

                        } else {
                            if(SLocks->at(lockID) == NULL){
                                cout << "case-3" << endl;
                                sendReplyToClient(machineID, mailbox, -1);
                                invalidLock = true;
                            } else if (SLocks->at(lockID)->owner != machineID){
                                cout << "case-4, owner: " << SLocks->at(lockID)->owner << endl;
                                sendReplyToClient(machineID, mailbox, -1);
                                invalidLock = true;
                            }
                        }
                    }
                    //Reply cases: -1 is error, 0 is no lock nor CV, 1 is both lock and CV, 2 is only lock, 3 is only CV
                    if(foundLock && invalidLock){
                        cout << "case 0" << endl;
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, -1);
                    } else{
                        if(cvID / 100 != netname){
                            if(foundLock && !invalidLock){
                                cout << "case 1" << endl;
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, 2); // found lock but not CV 
                            } else if(!foundLock){ // no lock or CV, reply with 0
                                cout << "case 2" << endl;
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, 0); 
                            }
                        } else {
                            cvID = cvID % 100;
                            if (cvID < 0 || cvID >= SCVs->size()){
                                cout << "case 3" << endl;
                                sendReplyToClient(machineID, mailbox, -1);
                            } else {
                                if(SCVs->at(cvID) == NULL || (SCVs->at(cvID)->lockIndex != lockID && SCVs->at(cvID)->lockIndex != -1)) {
                                    cout << "case 5" << endl;
                                    sendReplyToClient(machineID, mailbox, -1);
                                    sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, -1); 
                                } else { //CV exists, check if its on server 

                                    if (lockNum / 100 == netname){
                                        if(SCVs->at(cvID)->lockIndex==-1){
                                            SCVs->at(cvID)->lockIndex = lockNum;
                                        }

                                        PacketHeader *waitOutPkt = new PacketHeader();
                                        MailHeader *waitOutMail = new MailHeader();

                                        waitOutPkt->to = machineID;
                                        waitOutMail->to = mailbox;
                                        waitOutMail->from = netname;

                                        SCVs->at(cvID)->packetWaiting->push(waitOutPkt);
                                        SCVs->at(cvID)->mailWaiting->push(waitOutMail);
                                        PacketHeader *tempOutPkt = SLocks->at(lockID)->packetWaiting->front();
                                        MailHeader *tempOutMail = SLocks->at(lockID)->mailWaiting->front();

                                        if(!(SLocks->at(lockID)->packetWaiting->empty())){
                                            SLocks->at(lockID)->packetWaiting->pop();
                                            SLocks->at(lockID)->mailWaiting->pop();
                                            SLocks->at(lockID)->owner = tempOutPkt->to;
                                            reply << -2;
                                            sendMessage(tempOutPkt, tempOutMail, reply);
                                        } else {
                                            SLocks->at(lockID)->state = Available;
                                        }
                                        cout << "case 6" << endl;
                                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Wait1, requestID, machineID, mailbox, 1);

                                    } else {
                                        //we know CV and lock match, need to verify owner of lock
                                        cout << "case 7" << endl;
                                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal3, requestID, machineID, mailbox, 3);
                                    }
                                }
                            }
                        }
                    } 
                    break;   
                }    
                case RPC_Server_Broadcast1: {
                    ss >> lockID >> cvID; 

                    cout <<"Server signal 1, lock on server but CV on different server " << endl;

                    if (cvID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast1, requestID, machineID, mailbox, 0);
                    } else {
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast1, requestID, machineID, mailbox, 1);

                        cvID = cvID % 100; // index of CV 

                         if (cvID < 0 || cvID >= SCVs->size()){
                                sendReplyToClient(machineID, mailbox, -1);
                        } else {
                            if(SCVs->at(cvID) == NULL){
                            sendReplyToClient(machineID, mailbox, -1);
                            } else if (SCVs->at(cvID)->lockIndex == lockID){ // waiting client
                                if(SCVs->at(cvID)->packetWaiting->empty()){
                                    sendReplyToClient(machineID, mailbox, -1);
                                } else {
                                    reply << -2;
                                    while(!SCVs->at(cvID)->packetWaiting->empty()){
                                    PacketHeader *tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                    SCVs->at(cvID)->packetWaiting->pop();

                                    MailHeader *tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                    SCVs->at(cvID)->mailWaiting->pop();

                                    sendMessage(tempOutPkt, tempOutMail, reply);
                                }
                                    
                                        SCVs->at(cvID)->lockIndex = -1;
                                    
                                     sendReplyToClient(machineID, mailbox, -2);
                                }
                            }
                        }
                    }
                    break;
                }
                case RPC_Server_Broadcast2: {
                    cout<<"Broadcast 2: cv on server but lock on different server"<<endl;
                    //case where cv is on server but lock is on a different server 
                    //Replies:
                    //-1: error in lock
                    //0: not found
                    //1: lock found and valid
                    ss >> lockID >> cvID; 

                    if(lockID/100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Signal2, requestID, machineID, mailbox, 0); 
                    } else {
                        lockID = lockID % 100;


                        if(lockID < 0 || lockID >= SLocks->size()){
                            sendReplyToClient(machineID, mailbox, -1);
                            sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast2, requestID, machineID, mailbox, -1); 

                        } else {
                            if(SLocks->at(lockID) == NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast2, requestID, machineID, mailbox, -1); 
                            } else if (SLocks->at(lockID)->owner != netname){
                                sendReplyToClient(machineID, mailbox, -1);
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast2, requestID, machineID, mailbox, -1); 
                            } else {
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast2, requestID, machineID, mailbox, 1); // client owns lock, send Yes to server 
                            }
                        }
                    }
                }
                case RPC_Server_Broadcast3: {
                    //CV AND LOCK are on different servers 


                    cout<<"Server wait 3, no lock or CV on current server " << endl;
                    ss>>lockID>>cvID;

                    bool foundLock = false;
                    bool invalidLock = false;
                    int lockNum = lockID;
                    
                    if(lockID / 100 != netname){
                        foundLock = false;
                    } else {
                        lockID = lockID%100; 
                        foundLock = true; // lock is on server

                        if(lockID < 0 || lockID >= SLocks->size()){
                            sendReplyToClient(machineID, mailbox, -1);
                            invalidLock = true;

                        } else {
                            if(SLocks->at(lockID) == NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                                invalidLock = true;
                            } else if (SLocks->at(lockID)->owner != netname){
                                sendReplyToClient(machineID, mailbox, 1);
                                invalidLock = true;
                            } else {

                            }
                        }
                    }
                    //Reply cases: -1 is error, 0 is no lock nor CV, 1 is both lock and CV, 2 is only lock, 3 is only CV
                    if(foundLock && invalidLock){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, -1);
                    } else {
                        //check lock ID
                        if(cvID / 100 != netname){
                            if(foundLock && !invalidLock){
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, 2); // found lock but not CV 
                            } else if(!foundLock){ // no lock or CV, reply with 0
                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, 0); 
                                }
                            } else {
                                cvID = cvID % 100;

                                if (cvID < 0 || cvID >= SCVs->size()){
                                    sendReplyToClient(machineID, mailbox, -1);
                                } else {
                                    if(SCVs->at(cvID) == NULL || (SCVs->at(cvID)->lockIndex != lockID && SCVs->at(cvID)->lockIndex != -1)){
                                        sendReplyToClient(machineID, mailbox, -1);
                                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, -1);
                                    } else { //CV exists, get lock

                                        cvID = cvID % 100;
                                        if(cvID < 0 || cvID >= SCVs->size()){
                                            sendReplyToClient(machineID, mailbox, -1); 
                                        } else {
                                            if(SCVs->at(cvID)== NULL){
                                                sendReplyToClient(machineID, mailbox, -1);
                                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, -1); 
                                            } else if(SCVs->at(cvID)->lockIndex == lockNum){
                                                if(lockNum / 100 == netname){
                                                    if(SCVs->at(cvID)->packetWaiting->empty()){
                                                        sendReplyToClient(machineID, mailbox, -1);
                                                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, -1);
                                                    } else {
                                                        reply << -2;
                                                        while(!SCVs->at(cvID)->packetWaiting->empty()){
                                                         PacketHeader *tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                                        SCVs->at(cvID)->packetWaiting->pop();

                                                        MailHeader *tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                                        SCVs->at(cvID)->mailWaiting->pop();

                                                        sendMessage(tempOutPkt, tempOutMail, reply);
                                                    }
                                                        
                                                            SCVs->at(cvID)->lockIndex = -1;
                                                        
                                                        sendReplyToClient(machineID, mailbox, -2);
                                                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, 1);
                                                    }
                                                } else { // lock doesn't exist on server
                                                    sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, 3);
                                                }
                                            } else {
                                                sendReplyToClient(machineID, mailbox, -1);
                                                sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Broadcast3, requestID, machineID, mailbox, -1); 
                                                //lockID doesn't match cvs lock index so send error message back 
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        break;
                   
                }
                case RPC_Server_CreateMV: {
                    string name = "";
                    ss >> name;

                    int existingMV = -1;

                    for(int i =0; i<SMVs->size(); i++){
                        if(SMVs->at(i)->name == name){
                            existingMV = i;
                            break;
                        }
                    }

                    if(existingMV == -1){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_CreateMV, requestID, machineID, mailbox, 0);
                    } else {
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_CreateMV, requestID, machineID, mailbox, 1);
                        sendReplyToClient(machineID, mailbox, existingMV + uniqueID);
                    }
                    break;
                }
                case RPC_Server_GetMV: {
                    cout<<"Server get MV " << endl;
                    ss>>mvID>>mvIndex; 


                    if(mvID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_GetMV, requestID, machineID, mailbox, 0);
                    } else {
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_GetMV, requestID, machineID, mailbox, 1);

                        mvID = mvID % 100;

                        if(mvID<0 || mvID >= SMVs->size() || mvIndex < 0){
                            sendReplyToClient(machineID, mailbox, -1);
                        } else {
                            if(SMVs->at(mvID)==NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                            } else if(mvIndex >= SMVs->at(mvID)->len){
                                sendReplyToClient(machineID, mailbox, -1);
                            } else{
                                sendReplyToClient(machineID, mailbox, SMVs->at(mvID)->values[mvIndex]);
                            }
                        }

                    }
                    break;
                }
                case RPC_Server_SetMV: {

                    ss>>mvID>>mvIndex>>mvVal;

                    if(mvID / 100 != netname){
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_SetMV, requestID, machineID, mailbox, 0);

                    } else {
                        sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_SetMV, requestID, machineID, mailbox, 1);

                        mvID = mvID % 100;
                         if(mvID<0 || mvID >= SMVs->size() || mvIndex < 0){
                            sendReplyToClient(machineID, mailbox, -1);
                        } else {
                            if(SMVs->at(mvID)==NULL){
                                sendReplyToClient(machineID, mailbox, -1);
                            } else if(mvIndex >= SMVs->at(mvID)->len){
                                sendReplyToClient(machineID, mailbox, -1);
                            } else{
                                SMVs->at(mvID)->values[mvIndex]= mvVal;
                                sendReplyToClient(machineID, mailbox, -2);
                            }
                        }


                    }
                    break;
                }
                case RPC_Server_DestroyMV: {
                }
                default: {
                    cout<<"Unknown message" <<endl;
                    continue;
                    break;        
                }
            }
        } else {
            //handle server reply from a request sent out

            ss>>requestID;
            ss>>machineID;
            ss>>mailbox;
            ss>>response;

            cout<<"Request ID: "<<requestID<<endl;
            cout<<"MachineID: "<<machineID<<endl;
            cout<<"resonse: "<<response<<endl;
            ServerRequest* curReq;
            bool yes;

            if(requestID != -1){
                curReq = serverRequests->at(requestID);
                yes = curReq->yes;
            }
            switch(RPCType){
                case RPC_ServerReply_CreateLock:{
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1){
                                cout<<"Creating a new lock "<<endl;
                                 ServerLock *lock = new ServerLock;
                                    cout << "REQUEST NAME: " << curReq->name;
                                    lock->name = curReq->name;
                                    lock->packetWaiting = new queue<PacketHeader *>();
                                    lock->mailWaiting = new queue<MailHeader *>(); 
                                    lock->state = Available; 
                                    lock->toBeDeleted = false;
                                    lock->owner = -1; 
                                    lock->counter = 1;
                                    SLocks->push_back(lock);

                                    curReq->yes = true;
                                    sendReplyToClient(machineID, mailbox, (SLocks->size()-1)+uniqueID); 

                            } else{
                                curReq->noCount++;
                            }
                        } else {
                            curReq->yes = true;
                            cout<<"other server has lock" << endl;
                        }
                    }
                    break;
                }

                case RPC_ServerReply_DestroyLock: {
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                cout<<"Lock doesn't exist"<<endl;


                            } else {
                                cout<<"other server destroyed lock already"<<endl;
                                curReq->yes = true;
                            }
                        }
                    }
                    break;
                }
                case RPC_ServerReply_CreateCV: {
                    cout<<"RPC server reply createCV"<<endl;

                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1){

                                    ServerCV* scv = new ServerCV; 
                                    scv->name = curReq->name;
                                    scv->packetWaiting = new queue<PacketHeader *>();
                                    scv->mailWaiting = new queue<MailHeader *>();
                                    scv->toBeDeleted = false;
                                    scv->lockIndex = -1; 
                                    scv->counter = 1;
                                    scv->useCounter = 0;
                                    SCVs->push_back(scv); 
                                    sendReplyToClient(machineID, mailbox, (SCVs->size()-1)+uniqueID);

                            } else{
                                curReq->noCount++;
                            }
                        } else{
                            curReq->yes = true;
                            cout<<"Other server already created CV"<<endl;
                        }
                    }

                    break;
                }

                case RPC_ServerReply_DestroyCV: {
                    cout<<"RPC server reply destroy CV"<<endl;

                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS -1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                cout<<"CV doesn't exist, cant destroy"<<endl;

                            } else {
                                curReq->noCount++;
                            }
                        } else {
                            curReq->yes = true;
                            cout<<"Other server destroyed CV "<<endl;

                        }
                    }
                    break;
                }

                case RPC_ServerReply_Acquire: {
                    cout<<"RPC server reply acquire"<<endl;

                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS -1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                cout<<"Lock doesnt exist to acquire"<<endl;


                            } else{
                                curReq->noCount++;

                            }
                        } else {
                            curReq->yes = true;
                            cout<<"other server acquired lock"<<endl;
                        }
                    }
                    cout<<"This server thinks it has the lock " << endl;
                    break;
                }

                case RPC_ServerReply_Release: {
                    cout<<"Server reply release"<<endl;

                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                    cout<<"lock doesn't exist"<<endl;
                                } else {
                                    curReq->noCount++;
                                }
                            } else{
                                curReq->yes = true;
                                cout<<"Other server released lock"<<endl;
                            }
                        }
                    
                    break;
                }

                case RPC_ServerReply_Signal1: {
                    cout<<"Serve reply signal 1"<<endl;
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                    cout<<"cv doesn't exist"<<endl;
                             } else {
                                    curReq->noCount++;
                                }
                            } else{
                                curReq->yes = true;
                                cout<<"Other server signaled cv"<<endl;
                            }
                        }
                         break;
                }
                case RPC_ServerReply_Signal2: {
                    cout<<"server reply signal 2"<<endl;
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                    cout<<"cv doesn't exist"<<endl;
                                 } else {
                                    curReq->noCount++;
                                
                                }
                        } else if (response == -1){
                            curReq->yes = true;
                            sendReplyToClient(machineID, mailbox, -1);
                            cout<<"CV is invalid"<< endl;
                        } else if(response == 1){
                            curReq->yes = true;
                            cvID = curReq->arg1 % 100; 

                            if(SCVs->at(cvID)->packetWaiting->empty()){
                                sendReplyToClient(machineID, mailbox, -1);
                            } else{
                               reply<<-2;
                                SCVs->at(cvID)->useCounter--;
                                PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                SCVs->at(cvID)->packetWaiting->pop();
                                MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                SCVs->at(cvID)->mailWaiting->pop();
                                sendMessage(tempOutPkt, tempOutMail, reply);

                            if(SCVs->at(cvID)->packetWaiting->empty()){
                                SCVs->at(cvID)->lockIndex
                                 = -1; 
                            }
                            sendReplyToClient(machineID, mailbox, -2);
                        }

                    }
                    break;
                }
                case RPC_ServerReply_Signal3: {
                    //reply cases:
                    //-1: invalid
                    //0: no lock, no CV
                    //1: both lock and CV 
                    //2: only lock
                    //3: only CV 


                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1 || (curReq->lockFound && curReq->noCount == NUM_SERVERS - 2) || (curReq->cvFound && curReq->noCount == NUM_SERVERS-2)){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                cout << "Lock or CV is missing or they don't exist"<<endl;

                            } else {
                                curReq->noCount++;
                            }
                        } else if (response == -1) {
                            curReq->yes = true;
                            sendReplyToClient(machineID, mailbox, -1);
                            cout<<"Invalid lock or CV "<<endl;
                        } else if (response == 2) {
                            curReq->lockFound = true;
                            if(curReq->cvFound){
                                sendReplyToServer(curReq->replyServerMachineID, curReq->replyServerMailbox, RPC_ServerReply_Signal4, -1, machineID, mailbox, curReq->arg1);
                            } else if(curReq->noCount == NUM_SERVERS -2){
                                sendReplyToClient(machineID, mailbox, -1);
                                curReq->yes = true;
                                cout<<"CV does not exist"<<endl; 
                            }
                        } else if(response == 3) {
                            curReq->cvFound = true;

                            PacketHeader *tempOutPkt = new PacketHeader();
                            MailHeader *tempOutMail = new MailHeader();

                            tempOutPkt->to = inPktHdr->from;
                            tempOutMail->to = inMailHdr->from;
                            tempOutMail->from = netname;

                            curReq->replyServerMachineID = tempOutPkt;
                            curReq->replyServerMailbox = tempOutMail;

                            if(curReq->lockFound){
                                sendReplyToServer(curReq->replyServerMachineID, curReq->replyServerMailbox, RPC_ServerReply_Signal4, -1, machineID, mailbox, curReq->arg1);
                                curReq->yes = true;
                                cout<<"LOck and CV found"<<endl;
                            } else if(curReq->noCount == NUM_SERVERS -2){
                                sendReplyToClient(machineID, mailbox, -1);
                                    curReq->yes = true;
                                    cout<<"Lock doesn't exist"<<endl;
                            }    
                        } else if (response == 1){
                                curReq->yes = true;
                                cout<<"Wait completed"<<endl;
                            }

                        }
                    
                    break;
                }

                case RPC_ServerReply_Signal4: {
                    cvID = response % 100; 

                    if(SCVs->at(cvID)->packetWaiting->empty()){
                        sendReplyToClient(machineID, mailbox, -1);
                    } else {
                        reply << -2;
                        PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                        SCVs->at(cvID)->packetWaiting->pop();
                        MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                        SCVs->at(cvID)->mailWaiting->pop();
                        sendMessage(tempOutPkt, tempOutMail, reply);

                        if(SCVs->at(cvID)->packetWaiting->empty()){
                            SCVs->at(cvID)->lockIndex = -1;
                        }
                        sendReplyToClient(machineID, mailbox, -2);
                    }
                    break;
                }

                case RPC_ServerReply_Wait1: {
                    //lock on server, CV on different server

                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS -1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                cout<<"CV doesn't exist"<<endl;
                            } else {
                                curReq->noCount++;
                            }
                        } else if(response == 1){
                            lockID = curReq->arg1%100; 

                            PacketHeader* waitOutPkt = SLocks->at(lockID)->packetWaiting->front();
                            MailHeader *waitOutMail = SLocks->at(lockID)->mailWaiting->front(); 

                            if(!(waitOutPkt == NULL)){
                                    SLocks->at(lockID)->packetWaiting->pop();
                                    SLocks->at(lockID)->mailWaiting->pop();
                                    SLocks->at(lockID)->owner = waitOutPkt->to;
                                    reply << -2;
                                    sendMessage(waitOutPkt, waitOutMail, reply);
                                } else{
                                    SLocks->at(lockID)->state = Available; 
                                }

                                cout<<"Waiting"<<endl;

                        }
                    }
                    break;
                }
                case RPC_ServerReply_Wait2: {
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS -1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                cout<<"Lock doesn't exist"<<endl;
                            } else {
                                curReq->noCount++;
                            }
                        } else if (response == -1){
                            curReq->yes = true;
                            cout<<"Lock invalid"<<endl;
                        }else if(response == 1){
                            cvID = curReq->arg2%100; 

                            if(SCVs->at(cvID)->lockIndex == -1){
                                SCVs->at(cvID)->lockIndex = curReq->arg1;
                            }

                            PacketHeader *tempOutPkt = new PacketHeader();
                            MailHeader *tempOutMail = new MailHeader();

                            tempOutPkt->to = machineID;
                            tempOutMail->to = mailbox;
                            tempOutMail->from = netname;

                            SCVs->at(cvID)->packetWaiting->push(tempOutPkt);
                            SCVs->at(cvID)->mailWaiting->push(tempOutMail);

                            cout<<"Lock and cv exist, waiting in server reply wait 2"<<endl;
                        }
                    }
                    break;
                }
                case RPC_ServerReply_Wait3: {
                    //Replies
                    //-1 error
                    //0 no lock no CV 
                    //1 both lock and CV 
                    //2 only lock
                    //3 only CV 

                    cout<<"Server reply wait 3"<<endl;

                     if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1 || (curReq->lockFound && curReq->noCount == NUM_SERVERS - 2) || (curReq->cvFound && curReq->noCount == NUM_SERVERS-2)){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                cout << "Lock or CV is missing or they don't exist"<<endl;

                            } else {
                                curReq->noCount++;
                            }
                        } else if (response == -1){
                            curReq->yes = true;
                            sendReplyToClient(machineID, mailbox, -1);
                            cout<<"Invalid lock or CV "<<endl;
                        } else if (response == 2){
                            curReq->lockFound = true;
                            PacketHeader *tempOutPkt = new PacketHeader();
                            MailHeader *tempOutMail = new MailHeader();

                            tempOutPkt->to = inPktHdr->from;
                            tempOutMail->to = inMailHdr->from;
                            tempOutMail->from = netname;

                            curReq->replyServerMachineID_two = tempOutPkt;
                            curReq->replyServerMailbox_two = tempOutMail;

                            if(curReq->cvFound){
                                sendReplyToServer(curReq->replyServerMachineID, curReq->replyServerMailbox, RPC_ServerReply_Wait4, -1, machineID, mailbox, curReq->arg1*1000000 + curReq->arg2);
                            } else if(curReq->noCount == NUM_SERVERS -2){
                                sendReplyToClient(machineID, mailbox, -1);
                                    curReq->yes = true;
                                    cout<<"CV not found"<<endl;
                                
                                }
                        } else if(response == 3){
                            curReq->cvFound = true;

                            PacketHeader *tempOutPkt = new PacketHeader();
                            MailHeader *tempOutMail = new MailHeader();

                            tempOutPkt->to = inPktHdr->from;
                            tempOutMail->to = inMailHdr->from;
                            tempOutMail->from = netname;

                            curReq->replyServerMachineID = tempOutPkt;
                            curReq->replyServerMailbox = tempOutMail;

                            if(curReq->lockFound){
                                sendReplyToServer(curReq->replyServerMachineID, curReq->replyServerMailbox, RPC_ServerReply_Wait4, -1, machineID, mailbox, curReq->arg1*1000000 + curReq->arg2);
                                cout<<"LOck and CV found"<<endl;
                            } else if(curReq->noCount == NUM_SERVERS -2){
                                sendReplyToClient(machineID, mailbox, -1);
                                    curReq->yes = true;
                                    cout<<"Lock doesn't exist"<<endl;
                                }
                            } else if (response == 1){
                                curReq->yes = true;
                                cout<<"Wait completed"<<endl;
                            } else if (response == 4){
                                sendReplyToServer(curReq->replyServerMachineID, curReq->replyServerMailbox, RPC_ServerReply_Wait5, -1, machineID, mailbox, curReq->arg1);
                                curReq->yes = true;
                                cout<<"changed lock ownership"<<endl;
                            }

                        }
                    }
                    break;
                }

                case RPC_ServerReply_Wait4: {
                    lockID = (response / 100000000) % 100;
                    cvID = response % 1000000; 

                    if(SCVs->at(cvID)->lockIndex==-1){
                        SCVs->at(cvID)->lockIndex = lockID;
                    }
                             PacketHeader *tempOutPkt = new PacketHeader();
                            MailHeader *tempOutMail = new MailHeader();

                            tempOutPkt->to = machineID;
                            tempOutMail->to = mailbox;
                            tempOutMail->from = netname;

                            SCVs->at(cvID)->packetWaiting->push(tempOutPkt);
                            SCVs->at(cvID)->mailWaiting->push(tempOutMail);

                            sendReplyToServer(outPktHdr, outMailHdr, RPC_ServerReply_Wait3, requestID, machineID, mailbox, 4);
                            break;
                }

                case RPC_ServerReply_Wait5: {
                    lockID = response % 100; 

                     PacketHeader* waitOutPkt = SLocks->at(lockID)->packetWaiting->front();
                            MailHeader *waitOutMail = SLocks->at(lockID)->mailWaiting->front(); 

                            if(!(waitOutPkt == NULL)){
                                    SLocks->at(lockID)->packetWaiting->pop();
                                    SLocks->at(lockID)->mailWaiting->pop();
                                    SLocks->at(lockID)->owner = waitOutPkt->to;
                                    reply << -2;
                                    sendMessage(waitOutPkt, waitOutMail, reply);
                                } else{
                                    SLocks->at(lockID)->state = Available; 
                                }
                            sendReplyToClient(machineID, mailbox, -2);
                            break;
                }

                case RPC_ServerReply_Broadcast1: {
                    cout<<"Serve reply signal 1"<<endl;
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                    cout<<"cv doesn't exist"<<endl;
                             } else {
                                    curReq->noCount++;
                                }
                            } else{
                                curReq->yes = true;
                                cout<<"Other server broadcasted cv"<<endl;
                            }
                        }
                    break;
                }

                case RPC_ServerReply_Broadcast2: {
                    cout<<"server reply signal 2"<<endl;
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                    cout<<"cv doesn't exist"<<endl;
                                 } else {
                                    curReq->noCount++;
                                
                                }
                        } else if (response == -1){
                            curReq->yes = true;
                            sendReplyToClient(machineID, mailbox, -1);
                            cout<<"CV is invalid"<< endl;
                        } else if(response == 1){
                            curReq->yes = true;
                            cvID = curReq->arg1 % 100; 

                            if(SCVs->at(cvID)->packetWaiting->empty()){
                                sendReplyToClient(machineID, mailbox, -1);
                            } else{
                               reply<<-2;
                               while(!SCVs->at(cvID)->packetWaiting->empty()){
                                SCVs->at(cvID)->useCounter--;
                                PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                SCVs->at(cvID)->packetWaiting->pop();
                                MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                SCVs->at(cvID)->mailWaiting->pop();
                                sendMessage(tempOutPkt, tempOutMail, reply);
                            }
                           
                                SCVs->at(cvID)->lockIndex
                                 = -1; 
                            
                            sendReplyToClient(machineID, mailbox, -2);
                        }
                        cout<<"Condition broadcasted since we own lock"<<endl;
                    }
                }
                    break;
                }

                case RPC_ServerReply_Broadcast3: {
                    //reply cases:
                    //-1: invalid
                    //0: no lock, no CV
                    //1: both lock and CV 
                    //2: only lock
                    //3: only CV 


                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount == NUM_SERVERS-1 || (curReq->lockFound && curReq->noCount == NUM_SERVERS - 2) || (curReq->cvFound && curReq->noCount == NUM_SERVERS-2)){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1);
                                cout << "Lock or CV is missing or they don't exist"<<endl;

                            } else {
                                curReq->noCount++;
                            }
                        } else if (response == -1) {
                            curReq->yes = true;
                            sendReplyToClient(machineID, mailbox, -1);
                            cout<<"Invalid lock or CV "<<endl;
                        } else if (response == 2) {
                            curReq->lockFound = true;
                            if(curReq->cvFound){
                                sendReplyToServer(curReq->replyServerMachineID, curReq->replyServerMailbox, RPC_ServerReply_Broadcast4, -1, machineID, mailbox, curReq->arg1);
                            } else if(curReq->noCount == NUM_SERVERS -2){
                                sendReplyToClient(machineID, mailbox, -1);
                                curReq->yes = true;
                                cout<<"CV does not exist"<<endl; 
                            }
                        } else if(response == 3) {
                            curReq->cvFound = true;

                            PacketHeader *tempOutPkt = new PacketHeader();
                            MailHeader *tempOutMail = new MailHeader();

                            tempOutPkt->to = inPktHdr->from;
                            tempOutMail->to = inMailHdr->from;
                            tempOutMail->from = netname;

                            curReq->replyServerMachineID = tempOutPkt;
                            curReq->replyServerMailbox = tempOutMail;

                            if(curReq->lockFound){
                                sendReplyToServer(curReq->replyServerMachineID, curReq->replyServerMailbox, RPC_ServerReply_Broadcast4, -1, machineID, mailbox, curReq->arg1);
                                curReq->yes = true;
                                cout<<"LOck and CV found"<<endl;
                            } else if(curReq->noCount == NUM_SERVERS -2){
                                sendReplyToClient(machineID, mailbox, -1);
                                    curReq->yes = true;
                                    cout<<"Lock doesn't exist"<<endl;
                            }    
                        } else if (response == 1){
                                curReq->yes = true;
                                cout<<"Wait completed"<<endl;
                            }

                        }
                    
                    break;
                }

                case RPC_ServerReply_Broadcast4: {
                    cvID = response % 100; 

                    if(SCVs->at(cvID)->packetWaiting->empty()){
                        sendReplyToClient(machineID, mailbox, -1);
                    } else {
                        reply << -2;
                        while(!SCVs->at(cvID)->packetWaiting->empty()){
                        PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                        SCVs->at(cvID)->packetWaiting->pop();
                        MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                        SCVs->at(cvID)->mailWaiting->pop();
                        sendMessage(tempOutPkt, tempOutMail, reply);
                    }
                       
                            SCVs->at(cvID)->lockIndex = -1;
                        
                        sendReplyToClient(machineID, mailbox, -2);
                    }
                    break;
                }


                case RPC_ServerReply_CreateMV: {
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++;
                            if(curReq->noCount = NUM_SERVERS -1){
                                mvSize = curReq->arg1; 

                                ServerMV *mv = new ServerMV;
                                mv->name = curReq->name;
                                mv->values = new int[mvSize];
                                        mv->len = mvSize;
                                        mv->index = mvIndex;
                                        for(unsigned int i = 0; i < mvSize; i++){
                                            mv->values[i] = 0;
                                        }
                                        mv->toBeDeleted = false;
                                        SMVs->push_back(mv);

                            curReq->yes = true;
                            sendReplyToClient(machineID, mailbox, (SMVs->size() -1) + uniqueID);
                            cout<<"Creating MV"<<endl;

                            } else {
                                curReq->noCount++;
                            }
                        }
                    }
                    break;
                }

                case RPC_ServerReply_DestroyMV: {

                }

                case RPC_ServerReply_SetMV: {
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++; 
                            if(curReq->noCount == NUM_SERVERS -1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1); 
                            } else {
                                curReq->noCount++;
                            }
                        } else {
                            curReq->yes = true;
                            cout<<"MV found"<<endl;
                        }
                    }
                    break;
                }

                case RPC_ServerReply_GetMV: {
                    if(!yes){
                        if(response == 0){
                            curReq->noCount++; 
                            if(curReq->noCount == NUM_SERVERS -1){
                                curReq->yes = true;
                                sendReplyToClient(machineID, mailbox, -1); 
                            } else {
                                curReq->noCount++;
                            }
                        } else {
                            curReq->yes = true;
                            cout<<"MV found"<<endl;
                        }
                    }
                    break;
                }

                default: {
                    cout<<"Unknown message. " << endl;
                    continue;
                    break;
                }
            }
        }
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
                    } else if(SLocks->at(lockID)->state == Busy){
                        pass= false;
                        SLocks->at(lockID)->packetWaiting->push(outPktHdr);
                        int sizeofPacket = SLocks->at(lockID)->packetWaiting->size();
                        SLocks->at(lockID)->mailWaiting->push(outMailHdr);
                    } else{
                        SLocks->at(lockID)->owner = outPktHdr->to;
                        SLocks->at(lockID)->state = Busy;
                        reply << -2; 
                    }
                }
                if(pass){
                    sendMessage(outPktHdr, outMailHdr, reply);
                }
                lockLock->Release(); 
                break;
            }
            case RPC_Release: {
                lockLock->Acquire(); 
                ss >> lockID;
                cout<<"RPC Release Lock ID: " << lockID << endl; 
                bool pass = true;

                if(lockID < 0 || lockID >= SLocks->size()){
                    reply << -1;
                } else{
                    if(SLocks->at(lockID)== NULL){
                        reply << -1;
                    } else if(SLocks->at(lockID)->state == Available){
                        reply << -1;
                    } else{
                        reply << -2; 
                        if(SLocks->at(lockID)->packetWaiting->empty() && SLocks->at(lockID)->mailWaiting->empty()){ 

                            SLocks->at(lockID)->state = Available; 
                            SLocks->at(lockID)->owner = -1;
                        } else{
                            pass = false;
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
                cout<<"RPC Wait : cv " << cvID << " lock " << lockID << endl; 
                bool pass = true;

                if(lockID < 0 || lockID >= SLocks->size() || cvID < 0 || cvID >= SCVs->size()){
                    reply << -1;
                }
                else{
                    if(SLocks->at(lockID)==NULL || SCVs->at(cvID)==NULL){
                        reply<<-1;
                    } else if ((SCVs->at(cvID)->lockIndex != lockID && SCVs->at(cvID)->lockIndex != -1)){
                        reply << -1;
                    }else {
                        SCVs->at(cvID)->useCounter++;
                        pass = false;
                        if(SCVs->at(cvID)->lockIndex == -1){
                            SCVs->at(cvID)->lockIndex = lockID;
                        }
                        SCVs->at(cvID)->packetWaiting->push(outPktHdr);
                        SCVs->at(cvID)->mailWaiting->push(outMailHdr);

                        PacketHeader* waitOutPkt = SLocks->at(lockID)->packetWaiting->front();
                        MailHeader* waitOutMail = SLocks->at(lockID)->mailWaiting->front();

                        if(!SLocks->at(lockID)->packetWaiting->empty()){
                            SLocks->at(lockID)->packetWaiting->pop();
                            SLocks->at(lockID)->mailWaiting->pop();
                            SLocks->at(lockID)->owner = waitOutPkt->to;
                            reply << -2;
                            sendMessage(waitOutPkt, waitOutMail, reply);
                        } else{
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
                cout<<"RPC Signal: " << lockID << " " << cvID << endl; 

                if(lockID < 0 || lockID >= SLocks->size() || cvID < 0 || cvID >= SCVs->size()){
                    reply << -1;
                } 
                else{
                    if(SLocks->at(lockID)==NULL||SCVs->at(cvID)== NULL){
                        reply << -1;
                    } else if(SCVs->at(cvID)->lockIndex != lockID){
                        cout << SCVs->at(cvID)->lockIndex << "!=" << lockID << endl;
                        reply << -1;
                    } else{

                        if(SCVs->at(cvID)->packetWaiting->empty()){
                            cout << "No waiting threads" << endl;
                            reply<<-1;
                        } else{
                        reply<<-2;
                        SCVs->at(cvID)->useCounter--;
                        PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                        SLocks->at(lockID)->packetWaiting->push(tempOutPkt);
                        SCVs->at(cvID)->packetWaiting->pop();
                        MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                        SLocks->at(lockID)->mailWaiting->push(tempOutMail);
                        SCVs->at(cvID)->mailWaiting->pop();
                        //sendMessage(tempOutPkt, tempOutMail, reply);

                        if(SCVs->at(cvID)->packetWaiting->empty()){
                            SCVs->at(cvID)->lockIndex = -1; 
                        }
                        }
                    }
                }
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
                    } else if(SCVs->at(cvID)->lockIndex != lockID){
                        reply << -1;
                    } else{
                        if(SCVs->at(cvID)->packetWaiting->empty()){
                            reply<<-1;
                        } else{
                            while(!SCVs->at(cvID)->packetWaiting->empty()){
                                reply << -2;
                               PacketHeader* tempOutPkt = SCVs->at(cvID)->packetWaiting->front();
                                SLocks->at(lockID)->packetWaiting->push(tempOutPkt);
                                SCVs->at(cvID)->packetWaiting->pop();
                                MailHeader* tempOutMail = SCVs->at(cvID)->mailWaiting->front();
                                SLocks->at(lockID)->mailWaiting->push(tempOutMail);
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
            MVLock->Acquire(); 
            ss>>name>>mvSize; 
            cout<<"RPC CreateMV: " << name << endl;

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
            MVLock->Release();
            break;
        }
        case RPC_DestroyMV: {
            MVLock->Acquire();
            cout<<"RPC Destroy MV: "<<mvID <<endl;
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
            MVLock->Release();
            break;
        }

        case RPC_GetMV: {
            MVLock->Acquire();
            ss >> mvID >> mvIndex; 
            cout<<"RPC GetMV: " << mvID << " " << SMVs->at(mvID)->name << " at index "<<mvIndex<<endl;
            if(mvID < 0 || mvID >= SMVs->size() || mvIndex < 0){
                reply << -1;
            } else {
                if(SMVs->at(mvID)==NULL){
                    cout << "mvID is NULL" << endl;
                    reply << -1;
                } else if(mvIndex >= SMVs->at(mvID)->len){
                    cout << "err: " << mvIndex << " >= " << SMVs->at(mvID)->len << endl;
                    reply << -1;
                } else{
                    reply << SMVs->at(mvID)->values[mvIndex];
                }
            }
            sendMessage(outPktHdr, outMailHdr, reply);
            MVLock->Release();
            break;
        }

        case RPC_SetMV: {
            MVLock->Acquire();
            ss >> mvID >> mvIndex >> mvVal;
            cout<<"RPC SetMV: " << mvID << " at index "<<mvIndex<< " to value "<<mvVal<<endl;

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
            MVLock->Release();
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
