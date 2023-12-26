#include "coordinator.h"
#include <vector>
#include <fstream>
#include "message_m.h"
using namespace std;

Define_Module(Coordinator);

void Coordinator::readInput(){
        ifstream filestream("../src/coordinator.txt");
        string line;
       if(!filestream.is_open()) {
           EV<<"error with reading file"<<endl;
           return ;
       } else {
           // Read all lines
           while ( getline(filestream, line) ) {
               if (line.find(',')) {
                   int separator = line.find(',');
                   // The sender node index
                   char sender_idx = line[separator-1];
                   sender_node = (sender_idx=='0')?0:1;
                   startingTime = stod(line.substr(separator+1, line.size()-separator-2));
                   output_file.open("output.txt");
               }
           }
       }
       filestream.close();
}

void Coordinator::initialize()
{
    // TODO - Generated method body
    readInput();
    Message_Base *senderMsg = new Message_Base(to_string(startingTime).c_str());
    senderMsg->setFrameType(2);
    Message_Base *receiverMsg = new Message_Base("rec");
    if(sender_node == 0){
        send(receiverMsg, "port1");
        send(senderMsg, "port0");
    } else{
        send(receiverMsg, "port0");
        send(senderMsg, "port1");
    }
}



void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
