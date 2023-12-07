#include "node.h"
#include<fstream>
#include "message_m.h"
#include <string>
#include <bitset>
Define_Module(Node);

void Node::initialize()
{
     //delays= double(getParentModule()->par("PT"))+double(getParentModule()->par("TD"));
     delays=0.1+10.0;
     //ws=int(getParentModule()->par("ws"));
     ws=3;
     isTimeout=false;
     noErrors=false;
     startingPhase=true;
     index=0;
     finishedFrames=0;
     expected_seq_num=0;
}
// Function for framing with byte stuffing
std::string Node::byteStuffing(std::string f){
    char flag='$';
    char escape='/';
    std::string final_value=std::to_string(flag);
    for(int i=0;i<f.size();i++){
        if(f[i]==flag||f[i]==escape){
            final_value+=escape;
        }
        final_value+=f[i];

    }
    final_value+=flag;
    return final_value;

}

// Function to calculate the 8-bit checksum
std::string calculateChecksum(const std::string& str) {
    unsigned char checksum = 0;

    for (size_t i = 0; i < str.length(); ++i) {
        checksum += static_cast<unsigned char>(str[i]);
    }

    // Convert unsigned char to std::bitset
    std::string bitsetChecksum = std::bitset<8>(checksum).to_string();
    return bitsetChecksum;
}

void Node::readInput(const char *filename){
       std::ifstream filestream;
       std::string line;

       filestream.open(filename, std::ifstream::in);

       if(!filestream) {
           throw cRuntimeError("Error opening file '%s'?", filename);
       } else {
           while ( getline(filestream, line) ) {
               std::string error = line.substr(0,4);
               errors.push_back(error);
               std::string message = line.substr(5);
               messages.push_back(message);
           }
       }
       filestream.close();
       return;

}
void Node::handleMessage(cMessage *msg)
{
    /*
     * some notes:
     * -At timeout sender will send self message to itself
     *
     *
     */
    Message_Base *cmsg=check_and_cast<Message_Base *> (msg);
    EV<<"i'm node "<<getName()<<" ,received :"<<cmsg->getName()<<endl;
    //Initialization
    if(startingPhase&&strcmp(cmsg->getName(),"rec")==0){
        //It's A receiver ^^
        isSender=false;
        startingPhase=false;
        EV<<"i'm in receiver"<<endl;
    }
    else if (startingPhase){
        //It's A sender ^^
        EV<<"i'm in sender"<<endl;
        isSender=true;
        if(strcmp( getName(),"node0")==0){
            index=0;
        }
        else{
            index=1;
        }
        //read the corresponding file
        std::string filename="/home/donia/Desktop/college/networks/NW-Project-F23/Go-Back-N/src/input"+std::to_string(index)+".txt";
        readInput(filename.c_str());
        //test reading
        for(int i=0;i<errors.size();i++){
            EV<<errors[i]<<std::endl;
        }
    }

    if(cmsg->isSelfMessage()){
        isTimeout=true;
    }

    if(isSender){
        //Sender Logic
        //send msg if it's starting phase or receiving an ack or timeout
        if(startingPhase|| cmsg->getFrameType()==1 || isTimeout){

            //if it's an ack and not a timeout (assume ws=3 until reading it from .ini)
            if(!isTimeout&&cmsg->getAckNum()==(windowBeg+1)%3){
                  windowBeg=(windowBeg+1)%3; // increase the begining of window
                  finishedFrames++;
            }
            //for test
            Message_Base *new_msg=new Message_Base(messages[0].c_str());
            new_msg->setPayload(messages[0].c_str());
            EV << new_msg -> getPayload();
            new_msg->setFrameType(0);
            new_msg->setSeqNum(windowBeg%3);
            send(new_msg,"nodeGate$o");
            //Byte stuffing then checksum
        }
    }
    else{
        //Receiver Logic.
        // Access the loss_probability parameter value
        double loss_probability = par("LP").doubleValue();
        // Access the PT parameter value
        double PT = par("PT").doubleValue();
        // Access the PT parameter value
        double TD = par("TD").doubleValue();

        // Get the message data
        int seqNum = cmsg -> getSeqNum();
        std::string payload = cmsg -> getPayload();
        char parity = cmsg -> getParity();
        // Ack and NACK are sent for in order messages only.
        if(seqNum == expected_seq_num){

        }
    }




}
