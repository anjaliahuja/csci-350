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

void sendMessage(PacketHeader* outPktHdr, MailHeader* outMailHdr, stringstream& msg){
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

void serverToServer(stringstream& request) {
    PacketHeader* outPktHdr = new PacketHeader();
    PacketHeader* inPktHdr = new PacketHeader();
    MailHeader* outMailHdr = new MailHeader();
    MailHeader* inMailHdr = new MailHeader(); 

    for (int i = 0; i < 5; ++i)
    {
        if (i != netname)
        {
            outPktHdr->to = i;
            outMailHdr->to = i;
            outMailHdr->from = netname; 
            outMailHdr->length = request.str().size() + 1;
            sendMessage(outPktHdr, outMailHdr, request);
        }
    }
}

void Server(){
    cout << "server " << netname << " has started" << endl;
    vector<ServerLock*>* SLocks = new vector<ServerLock*>;
    vector<ServerCV*>* SCVs = new vector<ServerCV*>;
    vector<ServerMV*>* SMVs = new vector<ServerMV*>;
    map<int, int> requestTable;
    int requestNum = 0;

    int lockID, cvID, mvID, mvVal, mvIndex, mvSize;

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

        if ((inPktHdr->from) < 5 ) {
            //0~4, from another server!

            /*message format:
            -typeOfRequest = 0
                -requestID
                -clientFrom
                -RPCtype
                -found
            -typeOfRequest = 1
                -requestID
                -clientFrom
                -RPCtype
                ...
            */

            int typeOfRequest;
            int RPCType;
            int clientFrom;
            int requestID;
            stringstream ss;
            stringstream reply;
            ss<<buffer;
            ss>>typeOfRequest;
            if (typeOfRequest == 0)  //0 means you're receiving a message responding to a request you sent out
            {
                cout << "receiving type 0 message from other servers! " << endl;
                //if you get 5 replies in your table, then go create a new lock
                ss>>requestID;
                ss>>clientFrom;
                ss>>RPCType;
                bool found = false;
                ss>>found;
                switch(RPCType) {
                    case RPC_CreateLock: {
                        lockLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                string name = "";
                                ss>>name;
                                int index = netname*100+SLocks->size(); 
                                ServerLock *lock = new ServerLock;
                                lock->name = name;
                                lock->index = index;
                                lock->packetWaiting = new queue<PacketHeader *>();
                                lock->mailWaiting = new queue<MailHeader *>(); 
                                lock->state = Available; 
                                lock->toBeDeleted = false;
                                lock->owner = -1; 
                                lock->counter = 1;
                                SLocks->push_back(lock);
                                reply << index;
                                cout << "all servers responded! creating lock with index: " << index << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "CREATELOCK found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        lockLock->Release(); 
                        break;
                    }

                    case RPC_DestroyLock: {
                        lockLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                reply << -1;
                                cout << "all servers responded! couldn't find lock to destroy for: " << requestID << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "DESTROYLOCK found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        lockLock->Release(); 
                        break;
                    }

                    case RPC_Acquire: {
                        lockLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                reply << -1;
                                cout << "all servers responded! couldn't find lock to acquire for: " << requestID << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "ACQUIRE found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        lockLock->Release(); 
                        break;
                    }

                    case RPC_Release: {
                        lockLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                reply << -1;
                                cout << "all servers responded! couldn't find lock to release for: " << requestID << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "RELEASE found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        lockLock->Release(); 
                        break;
                    }

                    case RPC_CreateCV: {
                        CVLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                string name = "";
                                ss>>name;
                                int index = netname*100+SCVs->size();
                                ServerCV* scv = new ServerCV; 
                                scv->name = name;
                                scv->packetWaiting = new queue<PacketHeader *>();
                                scv->mailWaiting = new queue<MailHeader *>();
                                scv->toBeDeleted = false;
                                scv->lockIndex = -1; 
                                scv->counter = 1;
                                scv->useCounter = 0;
                                scv->index = index;
                                SCVs->push_back(scv); 
                                reply << index;
                                cout << "all servers responded! creating cv with index: " << index << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "CREATECV found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        CVLock->Release(); 
                        break;
                    }

                    case RPC_DestroyCV: {
                        CVLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                reply << -1;
                                cout << "all servers responded! couldn't find cv to destroy for: " << requestID << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "DESTROYCV found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        CVLock->Release(); 
                        break;          
                    }

                    case RPC_Wait: {
                        
                    }

                    case RPC_Signal: {
         
                    }

                    case RPC_Broadcast: {
                       
                    }

                    case RPC_CreateMV: {
                        MVLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                string name = "";
                                ss>>name;
                                ss>>mvSize;
                                int index = (netname*100)+(SMVs->size());
                                ServerMV *mv = new ServerMV;
                                mv->name = name;
                                mv->values = new int[mvSize];
                                mv->len = mvSize;
                                mv->index = index;
                                for(unsigned int i = 0; i < mvSize; i++){
                                    mv->values[i] = 0;
                                }
                                mv->toBeDeleted = false;
                                SMVs->push_back(mv);
                                cout << "all servers responded! creating mv with index: " << index << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "CREATELOCK found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        MVLock->Release(); 
                        break;
                    }

                    case RPC_DestroyMV: {
                        MVLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                reply << -1;
                                cout << "all servers responded! couldn't find mv to destroy for: " << requestID << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "DESTROYMV found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        MVLock->Release(); 
                        break;                            
                    }

                    case RPC_GetMV: {
                        MVLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                reply << -1;
                                cout << "all servers responded! couldn't find mv to get for: " << requestID << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "GETMV found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        MVLock->Release(); 
                        break;                         
                    }

                    case RPC_SetMV: {
                        MVLock->Acquire();
                        if (found == false)
                        {
                            /* update Map, if it hits 4 replies of false, create newLock */
                            if(requestTable.find(requestID) != requestTable.end()) {
                                cout << "not found! updating request table for request: " << requestID << endl;
                                requestTable[requestID]++;
                            } else {
                                break;
                            }

                            if (requestTable[requestID] == 4)
                            {   
                                outPktHdr->to = clientFrom;
                                outMailHdr->to = clientFrom;
                                outMailHdr->from = netname;

                                reply << -1;
                                cout << "all servers responded! couldn't find mv to set for: " << requestID << endl;

                                sendMessage(outPktHdr, outMailHdr, reply);
                                requestTable.erase(requestID);
                            }
                        } else {                                
                            cout << "SETMV found by another server for: " << requestID << endl;
                            requestTable.erase(requestID);
                        }
                        ss.clear();
                        reply.str("");
                        MVLock->Release(); 
                        break; 
                    }

                    default: {
                        std::cout<<"Unknown RPC \n" << RPCType << std::endl;
                        continue;
                        break;
                    }
                }
            } else { //1 means it needs you to search your table
                cout << "receiving type 1 message from other servers!" << endl;
                bool found = false;
                ss>>requestID;
                ss>>clientFrom;
                ss>>RPCType;
                //got type, now looking through for it
                switch(RPCType) {
                    case RPC_CreateLock: {
                        lockLock->Acquire();
                        string name;
                        ss>>name;

                        cout<<"CREATELOCK server: " << netname << "looking for: " << name << endl; 
                        int index = -1;
                        for(int i = 0; i< SLocks->size();i++){
                            if(SLocks->at(i)->name.compare(name) == 0){
                                index = i;
                                break;
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            reply << SLocks->at(index)->index;
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_CreateLock << " " << true;
                            cout << "CREATELOCK found! replying to the client: " << clientFrom << " now with! index " << endl;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_CreateLock << " " << false << " " << name;
                            cout << "CREATELOCK not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        lockLock->Release();
                        break;
                    }

                    case RPC_DestroyLock: {
                        lockLock->Acquire();
                        ss>>lockID;

                        cout<<"DESTROYLOCK server: " << netname << "looking for lock: " << lockID << endl; 
                        int index = -1;
                        for(int i = 0; i< SLocks->size();i++){
                            if(SLocks->at(i)->index == lockID){
                                index = i;
                                break;
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            SLocks->at(index)->counter--;
                            reply<<lockID;
                            if(SLocks->at(index)->state == Available){
                                ServerLock *lock = SLocks->at(index);
                                SLocks->at(index) = NULL;
                                delete lock;
                            } else{
                                SLocks->at(index)->toBeDeleted = true;
                            }
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_DestroyLock << " " << true;
                            cout << "DESTROYLOCK found! replying to the client: " << clientFrom << " now with! index " << endl;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_DestroyLock << " " << false << " " << lockID;
                            cout << "DESTROYLOCK not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        lockLock->Release();
                        break;
                    }

                    case RPC_Acquire: {
                        lockLock->Acquire();
                        ss>>lockID;

                        cout<<"Acquire server: " << netname << "looking for lock: " << lockID << endl; 
                        int index = -1;
                        for(int i = 0; i< SLocks->size();i++){
                            if(SLocks->at(i)->index == lockID){
                                index = i;
                                break;
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            bool pass = true; 
                            if(SLocks->at(index)->owner == outPktHdr->to && SLocks->at(index)->state == Busy){
                                reply << -1; 
                            } else if(SLocks->at(index)->state == Busy){
                                pass= false;
                                SLocks->at(index)->packetWaiting->push(outPktHdr);
                                int sizeofPacket = SLocks->at(index)->packetWaiting->size();
                                SLocks->at(index)->mailWaiting->push(outMailHdr);
                            } else{
                                SLocks->at(index)->owner = outPktHdr->to;
                                SLocks->at(index)->state = Busy;
                                reply << -2; 
                            }
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            if(pass){
                                sendMessage(outPktHdr, outMailHdr, reply);
                                reply.str("");
                            }

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_Acquire << " " << true;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_Acquire << " " << false << " " << lockID;
                            cout << "Acquire not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        lockLock->Release();
                        break;
                    }

                    case RPC_Release: {
                        lockLock->Acquire();
                        ss>>lockID;

                        cout<<"Release server: " << netname << "looking for lock: " << lockID << endl; 
                        int index = -1;
                        for(int i = 0; i< SLocks->size();i++){
                            if(SLocks->at(i)->index == lockID){
                                index = i;
                                break;
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            if(SLocks->at(index)->state == Available){
                                reply << -1;
                            } else{
                                reply << -2; 
                                if(SLocks->at(index)->packetWaiting->empty() && SLocks->at(index)->mailWaiting->empty()){ 
                                    SLocks->at(index)->state = Available; 
                                    SLocks->at(index)->owner = -1;
                                } else{
                                    PacketHeader* tempOutPkt = SLocks->at(index)->packetWaiting->front();
                                    SLocks->at(index)->packetWaiting->pop();
                                    MailHeader* tempOutMail = SLocks->at(index)->mailWaiting->front();
                                    SLocks->at(index)->mailWaiting->pop();

                                    SLocks->at(index)->owner = tempOutPkt->to;
                                    sendMessage(tempOutPkt, tempOutMail, reply);
                                    reply.str("");
                                }
                            }
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " "<< RPC_Release << " " << true;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_Release << " " << false << " " << lockID;
                            cout << "Release not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        lockLock->Release();
                        break;
                    }
                    
                    case RPC_CreateCV: {
                        CVLock->Acquire();
                        string name;
                        ss>>name;

                        cout<<"CREATECV server: " << netname << "looking for: " << name << endl; 
                        int index = -1;
                        for(unsigned int i = 0; i<SCVs->size(); i++){
                            if(SCVs->at(i) != NULL){
                                if(SCVs->at(i)->name == name){
                                    index = i;
                                    break;
                                }
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            reply << SCVs->at(index)->index;
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_CreateCV << " " << true;
                            cout << "CREATECV found! replying to the client: " << clientFrom << " now with! index " << endl;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_CreateCV << " " << false  << " "<< name;
                            cout << "CREATECV not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        CVLock->Release();
                        break;
                    }

                    case RPC_DestroyCV: {
                        CVLock->Acquire();
                        ss>>cvID;

                        cout<<"DESTROYCV server: " << netname << "looking for cv: " << cvID << endl; 
                        int index = -1;
                        for(unsigned int i = 0; i<SCVs->size(); i++){
                            if(SCVs->at(i) != NULL){
                                if(SCVs->at(i)->index == cvID){
                                    index = i;
                                    break;
                                }
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            reply << cvID;
                            SCVs->at(index)->counter--;
                            if(SCVs->at(index)->useCounter == 0){
                                ServerCV* scv = SCVs->at(index);
                                SCVs->at(index) = NULL;
                                delete scv; 
                            }
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_DestroyCV << " " << true;
                            cout << "DESTROYCV found! replying to the client: " << clientFrom << " now with! index " << endl;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_DestroyCV << " " << false << " " << cvID;
                            cout << "DESTROYCVnot found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        CVLock->Release();  
                        break;          
                    }

                    case RPC_Wait: {
                        
                    }

                    case RPC_Signal: {
         
                    }

                    case RPC_Broadcast: {
                       
                    }

                    case RPC_CreateMV: {
                        MVLock->Acquire();

                        string name;
                        ss>>name;
                        ss>>mvSize;

                        cout<<"CREATEMV server: " << netname << "looking for: " << name << endl; 
                        int index = -1;
                        for(int i = 0; i < SMVs->size(); i++){
                            if(SMVs->at(i) != NULL){
                                if(SMVs->at(i)->name == name){
                                    index = i;
                                    break;
                                }
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            reply << SMVs->at(index)->index;
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_CreateMV << " " << true;
                            cout << "CREATEMV found! replying to the client: " << clientFrom << " now with! index " << endl;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_CreateMV << " " << false << " " << name << " " << mvSize;
                            cout << "CREATEMV not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        MVLock->Release(); 
                        break; 
                    }

                    case RPC_DestroyMV: {
                        MVLock->Acquire();
                        ss>>mvID;

                        cout<<"DESTROYMV server: " << netname << "looking for mv: " << mvID << endl; 
                        int index = -1;
                        for(int i = 0; i < SMVs->size(); i++){
                            if(SMVs->at(i) != NULL){
                                if(SMVs->at(i)->index == mvID){
                                    index = i;
                                    break;
                                }
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            ServerMV *mv = SMVs->at(index);
                            SMVs->at(index)=NULL;
                            delete mv; 
                            reply << mvID; 
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_DestroyMV << " " << true;
                            cout << "DESTROYMV found! replying to the client: " << clientFrom << " now with! index " << endl;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_DestroyMV << " " << false << " " << mvID;
                            cout << "DESTROYMV not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        MVLock->Release(); 
                        break;                              
                    }

                    case RPC_GetMV: {
                        MVLock->Acquire();
                        ss>>mvID;
                        ss>>mvIndex;

                        cout<<"GETMV server: " << netname << "looking for mv: " << mvID << endl; 
                        int index = -1;
                        for(int i = 0; i < SMVs->size(); i++){
                            if(SMVs->at(i) != NULL){
                                if(SMVs->at(i)->index == mvID){
                                    index = i;
                                    break;
                                }
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            cout<<"RPC GET MV: "<<mvID <<endl;
                            if(mvIndex >= SMVs->at(index)->len){
                                reply << -1;
                            } else{
                                reply << SMVs->at(index)->values[mvIndex];
                            } 
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_GetMV << " " << true;
                            cout << "GETMV found! replying to the client: " << clientFrom << " now with! index " << endl;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_GetMV << " " << false << " " << mvID;
                            cout << "GETMV not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        MVLock->Release();  
                        break;                   
                    }

                    case RPC_SetMV: {
                        MVLock->Acquire();

                        ss>>mvID;
                        ss>>mvIndex;
                        ss>>mvVal;

                        cout<<"SETMV server: " << netname << "looking for mv: " << mvID << endl; 
                        int index = -1;
                        for(int i = 0; i < SMVs->size(); i++){
                            if(SMVs->at(i) != NULL){
                                if(SMVs->at(i)->index == mvID){
                                    index = i;
                                    break;
                                }
                            }
                        }

                        if (index != -1)
                        {
                            /* reply to client with index*/
                            cout<<"RPC SET MV: "<<mvID <<endl;
                            if(mvIndex >= SMVs->at(index)->len){
                                reply << -1;
                            } else{
                                SMVs->at(index)->values[mvIndex] = mvVal; 
                                reply << mvVal; 
                            } 
                            outPktHdr->to = clientFrom;
                            outMailHdr->to = clientFrom;
                            outMailHdr->from = netname;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");

                            /* reply to the server saying it was found! */
                            outPktHdr2->to = inPktHdr->from;
                            outMailHdr2->to = inMailHdr->from;
                            outMailHdr2->from = inPktHdr->to;
                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_SetMV << " " << true;
                            cout << "SETMV found! replying to the client: " << clientFrom << " now with! index " << endl;
                            sendMessage(outPktHdr2, outMailHdr2, reply);
                            reply.str("");
                        } else {
                            /* not found! */
                            outPktHdr->to = inPktHdr->from;
                            outMailHdr->to = inMailHdr->from;
                            outMailHdr->from = inPktHdr->to;

                            reply << 0 << " " << requestID << " " << clientFrom << " " << RPC_SetMV << " " << false << " " << mvID;
                            cout << "SETMV not found << replying to server" << endl;
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                        MVLock->Release(); 
                        break;
                    }

                    default: {
                        std::cout<<"Unknown RPC \n" << RPCType << std::endl;
                        continue;
                        break;
                    }
                }
            }
        } else {
            //From a client
            outPktHdr->to = inPktHdr->from;
            outMailHdr->to = inMailHdr->from;
            outMailHdr->from = inPktHdr->to;

            int type;
            int RPCType;
            stringstream ss;
            ss<<buffer;
            ss>>RPCType; 
            
            string name;
            stringstream reply; 
            switch(RPCType){            
                case RPC_CreateLock: {
                    lockLock->Acquire();
                    requestNum++;
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
                        //not found, send to other servers!
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_CreateLock << " " << name;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "CREATELOCK not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } else{
                        reply << SLocks->at(index)->index;
                        sendMessage(outPktHdr, outMailHdr, reply);
                        cout << "CREATELOCK found in this server: " << netname << " replying to client " << endl;
                        reply.str("");
                    }
                    lockLock->Release();
                    break;
                }

                case RPC_DestroyLock: {
                    lockLock->Acquire();
                    requestNum++;
                    ss >> lockID; 
                    cout<<"RPC Destroy Lock ID: " << lockID << endl; 

                    int index = -1;
                    for(int i = 0; i<SLocks->size();i++){
                        if(SLocks->at(i)->index == lockID){
                            index = i;
                            SLocks->at(i)->counter++;
                            break;
                        }
                    }

                    if(index == -1){
                        stringstream req;
                        req << " " << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_DestroyLock << " "<< lockID;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "DESTROYLOCK not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } 
                    else {
                        SLocks->at(index)->counter--;
                        reply<<lockID;
                        if(SLocks->at(index)->state == Available){
                            ServerLock *lock = SLocks->at(index);
                            SLocks->at(index) = NULL;
                            delete lock;
                        } else{
                            SLocks->at(index)->toBeDeleted = true;
                        }
                        sendMessage(outPktHdr, outMailHdr, reply);
                        reply.str("");
                    }
                    lockLock->Release(); 
                    break;
                }

                case RPC_Acquire: {
                    lockLock->Acquire();
                    requestNum++;
                    ss >> lockID; 
                    cout<<"RPC ACQUIRE Lock ID: " << lockID << endl;
                    bool pass = true;  

                    int index = -1;
                    for(int i = 0; i<SLocks->size();i++){
                        if(SLocks->at(i)->index == lockID){
                            index = i;
                            break;
                        }
                    }

                    if(index == -1){
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_Acquire << " " << lockID;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "ACQUIRE not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } 
                    else {
                        if(SLocks->at(index)->owner == outPktHdr->to && SLocks->at(index)->state == Busy){
                            reply << -1; 
                        } else if(SLocks->at(index)->state == Busy){
                            pass= false;
                            SLocks->at(index)->packetWaiting->push(outPktHdr);
                            int sizeofPacket = SLocks->at(index)->packetWaiting->size();
                            SLocks->at(index)->mailWaiting->push(outMailHdr);
                        } else{
                            SLocks->at(index)->owner = outPktHdr->to;
                            SLocks->at(index)->state = Busy;
                            reply << -2; 
                        }
                        if(pass){
                            sendMessage(outPktHdr, outMailHdr, reply);
                            reply.str("");
                        }
                    }

                    lockLock->Release(); 
                    break;
                }

                case RPC_Release: {
                    lockLock->Acquire();
                    requestNum++;
                    ss >> lockID; 
                    cout<<"RPC RELEASE Lock ID: " << lockID << endl;
                    bool pass = true;  

                    int index = -1;
                    for(int i = 0; i<SLocks->size();i++){
                        if(SLocks->at(i)->index == lockID){
                            index = i;
                            break;
                        }
                    }

                    if(index == -1){
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_Release << " " << lockID;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "RELEASE not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } 
                    else {
                        if(SLocks->at(index)->state == Available){
                            reply << -1;
                        } else{
                            reply << -2; 
                            if(SLocks->at(index)->packetWaiting->empty() && SLocks->at(index)->mailWaiting->empty()){ 

                                SLocks->at(index)->state = Available; 
                                SLocks->at(index)->owner = -1;
                            } else{
                                PacketHeader* tempOutPkt = SLocks->at(index)->packetWaiting->front();
                                SLocks->at(index)->packetWaiting->pop();
                                MailHeader* tempOutMail = SLocks->at(index)->mailWaiting->front();
                                SLocks->at(index)->mailWaiting->pop();

                                SLocks->at(index)->owner = tempOutPkt->to;
                                sendMessage(tempOutPkt, tempOutMail, reply);
                                reply.str("");
                            }
                        }
                    }
                    lockLock->Release(); 
                    break;
                }

                case RPC_CreateCV: {
                    CVLock->Acquire();
                    requestNum++;
                    ss>>name;
                    cout<<"RPC Create CV name: " << name << endl; 

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
                        //not found, send to other servers!
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_CreateCV << " " << name;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "CREATECV not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } else{
                        reply << SCVs->at(index)->index;
                        sendMessage(outPktHdr, outMailHdr, reply);
                        cout << "CREATECV found in this server: " << netname << " replying to client " << endl;
                        reply.str("");
                    }
                    CVLock->Release();
                    break;
                }

                case RPC_DestroyCV: {
                    CVLock->Acquire();                    
                    requestNum++;
                    ss >> cvID; 
                    cout<<"RPC Destroy CV : " << cvID << endl; 

                    int index = -1;
                    for(unsigned int i = 0; i<SCVs->size(); i++){
                        if(SCVs->at(i) != NULL){
                            if(SCVs->at(i)->index == cvID){
                                index = i;
                                break;
                            }
                        }
                    }

                    if(index == -1){
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_DestroyCV << " " << cvID;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "DESTROYCV not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } 
                    else {
                        reply << cvID;
                        SCVs->at(index)->counter--;
                        if(SCVs->at(index)->useCounter == 0){
                            ServerCV* scv = SCVs->at(index);
                            SCVs->at(index) = NULL;
                            delete scv; 
                        }
                        sendMessage(outPktHdr, outMailHdr, reply);
                        reply.str("");
                    }
                    CVLock->Release(); 
                    break;
                }

                /*not implemented yet */
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
                        CVLock->Release();
                        break;       
                }
                /* ----------------- */

                case RPC_CreateMV: {
                    MVLock->Acquire();
                    requestNum++;
                    ss>>name>>mvSize;
                    cout<<"RPC Create MV name: " << name << endl; 

                    int index = -1;
                    for(int i = 0; i < SMVs->size(); i++){
                        if(SMVs->at(i) != NULL){
                            if(SMVs->at(i)->name == name){
                                index = i;
                                break;
                            }
                        }
                    }
                    if(index == -1){
                        //not found, send to other servers!
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_CreateMV << " " << name << " " << mvSize;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "CREATEMV not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } else{
                        reply << SMVs->at(index)->index;
                        sendMessage(outPktHdr, outMailHdr, reply);
                        cout << "CREATEMV found in this server: " << netname << " replying to client " << endl;
                        reply.str("");
                    }
                    MVLock->Release();
                    break;
                }

                case RPC_DestroyMV: {
                    MVLock->Acquire();
                    requestNum++;
                    ss >> mvID; 
                    cout<<"RPC Destroy MV : " << mvID << endl; 

                    int index = -1;
                    for(int i = 0; i < SMVs->size(); i++){
                        if(SMVs->at(i) != NULL){
                            if(SMVs->at(i)->index == mvID){
                                index = i;
                                break;
                            }
                        }
                    }

                    if(index == -1){
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_DestroyMV << " " << mvID;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "DESTROYMV not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } 
                    else {
                        cout<<"RPC Destroy MV: "<<mvID <<endl;
                        ServerMV *mv = SMVs->at(index);
                        SMVs->at(index)=NULL;
                        delete mv; 
                        reply << mvID; 
                        sendMessage(outPktHdr, outMailHdr, reply);
                        reply.str("");
                    }
                    MVLock->Release();
                    break;
                }

                case RPC_GetMV: {
                    MVLock->Acquire();
                    requestNum++;
                    ss >> mvID;
                    ss >> mvIndex; 
                    cout<<"RPC Get MV : " << mvID << endl; 

                    int index = -1;
                    for(int i = 0; i < SMVs->size(); i++){
                        if(SMVs->at(i) != NULL){
                            if(SMVs->at(i)->index == mvID){
                                index = i;
                                break;
                            }
                        }
                    }

                    if(index == -1){
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_GetMV << " " << mvID << " " << mvIndex;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "GETMV not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } 
                    else {
                        cout<<"RPC GET MV: "<<mvID <<endl;
                        if(mvIndex >= SMVs->at(index)->len){
                            reply << -1;
                        } else{
                            reply << SMVs->at(index)->values[mvIndex];
                        }
                        sendMessage(outPktHdr, outMailHdr, reply);
                        reply.str("");
                    }
                    MVLock->Release();
                    break;
                }

                case RPC_SetMV: {
                    MVLock->Acquire();
                    requestNum++;
                    ss >> mvID;
                    ss >> mvIndex; 
                    ss >> mvVal;
                    cout<< "RPC Set MV : " << mvID << endl; 
                    int index = -1;
                    for(int i = 0; i < SMVs->size(); i++){
                        if(SMVs->at(i) != NULL){
                            if(SMVs->at(i)->index == mvID){
                                index = i;
                                break;
                            }
                        }
                    }

                    if(index == -1){
                        stringstream req;
                        req << 1 << " " << requestNum << " " << inPktHdr->from << " " << RPC_GetMV << " " << mvID << " " << mvIndex << " "<< mvVal;
                        requestTable.insert (std::pair<int,int>(requestNum, 0));
                        serverToServer(req);
                        cout << "SETMV not found in this server: " << netname << " sending to other servers " << endl;
                        req.str("");
                    } 
                    else {
                        cout<<"RPC SET MV: "<<mvID <<endl;
                        if(mvIndex >= SMVs->at(index)->len){
                            reply << -1;
                        } else{
                            SMVs->at(index)->values[mvIndex] = mvVal; 
                            reply << mvVal; 
                        }
                        sendMessage(outPktHdr, outMailHdr, reply);
                        reply.str("");
                    }
                    MVLock->Release();
                    break;
                }

                default: {
                    std::cout<<"Unknown RPC \n" << RPCType << std::endl;
                    continue;
                    break;
                }
            }
        }
    }
}


//////////////////////////////////////////////////////////////////////////////////// Mail Test ////////////////////////////////////////////////////////////////////////////////////////////////////


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
