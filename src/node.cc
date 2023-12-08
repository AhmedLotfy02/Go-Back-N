#include "node.h"
#include<fstream>
#include "message_m.h"
#include <string>
#include <bitset>
Define_Module(Node);

// Function for framing with byte stuffing
std::string Node::byteStuffing(std::string frame){
    char flag='$';
    char escape='/';
    std::string final_value="";
    final_value+=flag;
    for(int i=0;i<frame.size();i++){
        if(frame[i]==flag||frame[i]==escape){
            final_value+=escape;
        }
        final_value+=frame[i];

    }
    final_value+=flag;
    return final_value;
}
// Function for destuffing
std::string Node::byteDestuffing(const std::string& stuffedFrame) {
    char flag = '$';
    char escape = '/';
    std::string destuffedFrame;

    // Skip the initial flag
    size_t i = 1;

    while (i < stuffedFrame.size() - 1) { // Skip the final flag
        if (stuffedFrame[i] == escape) {
            // If the current character is an escape character,
            // the next character is the actual data, including flag or escape
            destuffedFrame += stuffedFrame[i + 1];
            i += 2; // Skip the escape character and the next character
        } else {
            destuffedFrame += stuffedFrame[i];
            i++;
        }
    }

    return destuffedFrame;
}
// Function to calculate the 8-bit checksum
std::bitset<8> Node::calculateChecksum(const std::string& str) {
    //1 Byte checksum
    unsigned char checksum = 0;
    //Add all chracters
    for (size_t i = 0; i < str.length(); ++i) {
        checksum += static_cast<unsigned char>(str[i]);
    }

    // Calculate one's complement
   unsigned char onesComplement = ~checksum;

   // Convert unsigned char to 8-bit bitstream
   return std::bitset<8>(onesComplement);
}
//Function to read input file
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
        std::string filename="/home/donia/Desktop/college/networks/Go-Back-N/src/input"+std::to_string(index)+".txt";
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
            std::string stuffedFrame = byteStuffing(messages[0].c_str());
            new_msg->setPayload(stuffedFrame.c_str());
            EV << new_msg -> getPayload() <<endl;
            //To set the checksum as char
            std::bitset<8> parityBitStream = calculateChecksum(stuffedFrame);
            char parity = (char) parityBitStream.to_ulong();
            new_msg->setParity(parity);
            EV << "Parity: " <<new_msg->getParity()<<endl;
            new_msg->setFrameType(0);
            new_msg->setSeqNum(windowBeg%3);
            EV<< "seq_num "<< new_msg -> getSeqNum() <<endl;
            send(new_msg,"nodeGate$o");
        }
    }
    else{
        //It isn't the message from the coordinator
        if(strcmp(cmsg->getName(),"rec")!=0){
            //Receiver Logic.
            // Access the loss_probability parameter value
            double loss_probability = par("LP").doubleValue();
            // Access the PT parameter value
            double PT = par("PT").doubleValue();
            // Access the PT parameter value
            double TD = par("TD").doubleValue();
            //default is true ==> if loss ==> upate it to false
            bool isLoss = true;
            int frameType;
            // Get the message data
            int seqNum = cmsg -> getSeqNum();
            EV<< "seq_num "<< seqNum <<endl;
            std::string payload = cmsg -> getPayload();
            char parity = cmsg -> getParity();
            // Ack and NACK are sent for in order messages only.
            if(seqNum == expected_seq_num){
                //Calclate the check sum and check it
                char checksum = parity;
                for (size_t i = 0; i < payload.length(); ++i) {
                     checksum += static_cast<unsigned char>(payload[i]);
                 }
                //If it is correct
                 if(std::bitset<8>(checksum) == std::bitset<8>("11111110")){
                     EV <<"correct checksum"<<endl;
                     expected_seq_num+=1;
                     //Frame type:ACK=1
                     frameType = 1;
                     //Deframing
                     std::string destuffedFrame = byteDestuffing(payload);
                     //Print #5
                }
                //Else
                else{
                    //Frame type: NACK=0
                    frameType = 0;
                    }
                      //Print #4
                }
                 //The ACK/NACK number is set as the sequence number of the next correct expected frame.
                 cmsg ->setAckNum(expected_seq_num);
                 cmsg ->setFrameType(frameType);
                 //Loss or not
                 //Loss woth probability = LP
                 volatile float val = uniform(0, 1);
                 EV << "Random value: " << val << endl;
                 if (val >= loss_probability) {
                     //NO loss
                     isLoss= false;
                     //Send Ack.
                     send(cmsg,"nodeGate$o");
                 }
                 //Print #4
        }
    }
}
