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
               // 4 bits of error
               std::string error = line.substr(0,4);
               errors.push_back(error);

               std::string message = line.substr(5);
               messages.push_back(message);
           }
       }
       filestream.close();
       return;
}

//void Node::outputFile(int printCase, double eventTime, int NodeId, Message_Base* msg, std::string error,int modifiedBitNumber, int duplicateNumber, )
//{
//    output_file.open("output.txt");
//    if(printCase==0)
//    {
//        std:: string dummy="At time ["+ eventTime+"], Node["+NodeId+"] , Introducing channel error with code =["+error +"]."
//
//
//
//
//    }
//    else if(printCase==1)
//    {
//        std::string dupMsg="";
//        if(error[3])
//        {
//            if(duplicateNumber==1)
//            {
//                dupMsg="1";
//            }
//            else
//            {
//                dupMsg="2";
//            }
//        }
//        else
//        {
//            dupMsg="0";
//        }
//        std:: string dummy="At time ["+ eventTime+"], Node["+NodeId+"]  [sent] frame with seq_num=["+std::to_string(msg->getSeqNum()) +"] and payload=["+msg->getPayload()+" ] and trailer=["+msg->getTrailer()+"] , Modified [ "+ (error[0])?modifiedBitNumber:'-1' +" ] , Lost ["+ (error[1])?"Yes":"No" +"], Duplicate ["+dupMsg+"], Delay ["+(!error[2])?"0":getParentModule()->par("DD").doubleValue()+"]."
//    }
//    else if(printCase==2)
//    {
//        std:: string dummy="Time out event at time  ["+ eventTime+"], at Node["+NodeId+"] for frame with seq_num=["+std::to_string(msg->getSeqNum()) +"] ";
//
//
//    }
//
//}
// Start the PT
void Node::scheduleProcessingTime(double PT) {
    // Start new timeout
    scheduleAt( simTime() + PT, ProcessingTimeEvent);
}
// Cancel the processing time when receiving non ack
void Node::cancelProcessingTime() {
    cancelEvent(ProcessingTimeEvent);
}

void Node::scheduleTimeout(int sequenceNumber) {
    // Schedule a new timeout event for the given sequence number
    cMessage *timeoutEvent = new cMessage("Timeout");
    timeoutEvent->setKind(sequenceNumber);
    scheduleAt(simTime() + TO, timeoutEvent);  // Set timeout duration (adjust as needed)
    // Store the timeout event in the map
    timeoutEvents[sequenceNumber] = timeoutEvent;
}

void Node::cancelTimeout(int sequenceNumber) {
    // Iterate over the timeouts and cancel those less than or equal to the given sequenceNumber
        for (auto it = timeoutEvents.begin(); it != timeoutEvents.end(); ) {
            if (it->first <= sequenceNumber) {
                // Cancel and delete the timeout event
                cancelAndDelete(it->second);
                it = timeoutEvents.erase(it);
            } else {
                ++it;
            }
        }
}

void Node::send_new_line(Message_Base *new_msg, bool is_modification = false) {
    // get the message that should be sent from the messages list
    std::string message_text = messages[next_message_index + windowBeg];
    //modify a bit if the modification error exists
    if(is_modification){
        // Randomly choose an index
        int bit_index = modification_random_number % (8 * message_text.size());

        // Calculate the bit position within the byte
        int byte_index = bit_index / 8;
        int bit_position = bit_index % 8;

        // Retrieve the byte from the message
        char& byte = message_text[byte_index];

        // Flip the selected bit in the byte
        byte ^= (1 << bit_position);  // XOR with a bitmask to flip the bit
    }
    // Apply byte stuffing on the message
    std::string stuffedFrame = byteStuffing(message_text.c_str());
    //Calculate the check sum
    std::bitset<8> parityBitStream = calculateChecksum(stuffedFrame);
    //To set the checksum as char
    char parity = (char) parityBitStream.to_ulong();

    // Prepare the message that will be sent
    new_msg->setPayload(stuffedFrame.c_str());
    new_msg->setParity(parity);
    // data message
    new_msg->setFrameType(2);
    // seq number(range from 0 to WS-1)
    new_msg->setSeqNum(next_message_index%WS);

}

void Node::initialize()
{
     isTimeout=false;
     // will be set false after recieving from the coordinator and start the sending between the sender and the receiver
     from_coordinator=true;
     // to indicate whether to resend the frames
     is_non_ack = false;
     // to indicate the timeout
     is_time_out = false;
     //wheteher the current node is sender or receiver
     isSender= true;
     // Index of the input file
     sender_file_index=0;
     // Index of the ack number received from the receiver
     last_acked_frame = 0;
     // the true seq number expected in the receiver side
     expected_seq_num=0;
     // Index of the message to be sent
     next_message_index = 0;
     // the beginning of the window now
     windowBeg = 0;
     // Get the parameters to use in sending and receiving
     TD = getParentModule()->par("TD").doubleValue();
     PT = getParentModule()->par("PT").doubleValue();
     ED = getParentModule()->par("ED").doubleValue();
     DD = getParentModule()->par("DD").doubleValue();
     TO = getParentModule()->par("TO").doubleValue();
     WS = getParentModule()->par("WS").intValue();
     modification_random_number = getParentModule()->par("modification_random_number").intValue();

     // initialize messages
     timeoutEvent = new cMessage();
     ProcessingTimeEvent = new cMessage();
}

void Node::handleMessage(cMessage *msg)
{
    // Check if the received message is of type cMessage (message from another nodes)
    if (dynamic_cast<Message_Base*>(msg) != nullptr) {
        // Received a Message_Base message
        Message_Base *cmsg=check_and_cast<Message_Base *> (msg);

            //Initialization
            if(from_coordinator && strcmp(cmsg->getName(),"rec")==0){
                //It's A receiver
                isSender=false;
                // false as the next messages won't be from coordinator
                from_coordinator=false;
            }
            // sender
            else if (from_coordinator){
                //It's A sender
                isSender=true;
                from_coordinator = false;
                // set the correct input file index (input0 ro input1)
                if(strcmp( getName(),"node0")==0){
                    sender_file_index=0;
                }
                else{
                    sender_file_index=1;
                }
                //read the corresponding file
                std::string filename="";
                std::string name="Donia";
                if(name=="heba")
                    filename="E:/CMP4/Networks/Go-Back-N/src/input"+std::to_string(sender_file_index)+".txt";
                else if(name=="shaza")
                    filename="D:/Shozy/Networks/project/Go-Back-N/src/input"+std::to_string(sender_file_index)+".txt";
                else if(name=="ahmed")
                    filename="C:/Users/LP-7263/Documents/CMP4/Networks/Project/Go-Back-N/src/input"+std::to_string(sender_file_index)+".txt";
                else if(name=="Donia")
                    filename="/home/donia/Desktop/college/networks/Go-Back-N/src/input"+std::to_string(sender_file_index)+".txt";
                readInput(filename.c_str());

                // get time of the first message
                // Convert const char* to double
                char* endPtr;
                double time_first_message = std::strtod(cmsg->getName(), &endPtr);
                if (*endPtr != '\0') {
                    EV<<"error converting to double"<<endl;
                }
                else{
                    //printing of the first message
                    EV<<"AT ["<<simTime().dbl()<<"], Node["<<sender_file_index<<"], ";
                    EV<<"Introducing channel error with code=["<<errors[0]<<"]"<<endl;
                    //the starting node should start reading its messages from its file on the specified starting time
                    //PT is the time between start reading and sending
                    //so it will send after the specified starting time + the processing time
                    scheduleProcessingTime(time_first_message + PT);
                }
            }
            // sender logic
            if(isSender){
                // if ack/not ack is received
                if(cmsg->getFrameType()==1){
                    last_acked_frame = cmsg->getAckNum();
                    // Cancel the timeout of the last message before the ack number
                    //accumulative cancel
                    cancelTimeout(windowBeg + last_acked_frame - 1);
                    // increment the windowBeg by the number of frames acked
                    windowBeg = (windowBeg + last_acked_frame) % WS;
                    // set the first index to send according to the new window
                    next_message_index = (next_message_index - last_acked_frame) % WS;
                }
                // if non ack ==> resend the frame with non ack
                else if (cmsg->getFrameType()==0){
                    last_acked_frame = cmsg->getAckNum();
                    // Cancel the timeout of the last message before the ack number
                    //accumulative cancel
                    cancelTimeout(windowBeg + last_acked_frame - 1);
                    // increment the windowBeg by the number of frames acked
                    windowBeg = (windowBeg + last_acked_frame) % WS;
                    //the first message in the new window
                    next_message_index = 0;
                    //to send the errored message again “error free this time”
                    is_non_ack = true;
                    //upon receiving a NACK , the sender will stop what he is processing
                    cancelProcessingTime();
                    //the message is sent after the processing time +0.001 (to break any possible ties).
                    scheduleProcessingTime(PT + 0.001);
                    }
            }
            //Receiver logic
            else{
                //It isn't the message from the coordinator
                if(strcmp(cmsg->getName(),"rec")!=0){
                    //Receiver Logic.
                    // Access the loss_probability parameter value
                    double loss_probability = getParentModule()->par("LP").doubleValue();
                    //default is true ==> if loss ==> upate it to false
                    bool isLoss = true;
                    int frameType;
                    // Get the message data
                    int seqNum = cmsg -> getSeqNum();
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
                         if(std::bitset<8>(checksum) == std::bitset<8>("11111111")){
                             expected_seq_num = (expected_seq_num + 1)%WS;
                             //Frame type:ACK=1
                             frameType = 1;
                             //Deframing
                             std::string destuffedFrame = byteDestuffing(payload);
                             EV<<"Uploading payload=["<< destuffedFrame<<"] and seq_num =["<<seqNum<<"] to the network layer"<<endl;
                        }
                        //Else
                        else{
                            //Frame type: NACK=0
                            frameType = 0;
                            }
                          EV<<"At time["<< simTime().dbl() + PT <<"], Node["<<(sender_file_index==0?"1":"0")<<"]";
                          EV<<"Sending ["<<(frameType==0?"NACK":"ACK")<<"] with number ["<<expected_seq_num<<"]";
                         //The ACK/NACK number is set as the sequence number of the next correct expected frame.
                          cmsg ->setAckNum(expected_seq_num);
                          cmsg ->setFrameType(frameType);
                          //Loss or not
                          //Loss with probability = LP
                          volatile float val = uniform(0, 1);
                          if (val >= loss_probability) {
                              EV<<" , loss [No]"<<endl;
                              double delay = TD + PT;
                              //NO loss
                              isLoss= false;
                              //Send Ack.
                              sendDelayed(cmsg, delay, "nodeGate$o");
                          }
                          else{
                              EV<<" , loss [Yes]"<<endl;
                          }
                              //Print #4
                        }
                }
            }
    }
    // generic message (a message from the same node)
    else{
        //cancel the current PT
        cancelProcessingTime();
        // Check if the message is a timeout alarm
        if (std::strcmp(msg->getName(),"Timeout")==0) {
            // Delete any remaining timeout events
            for (auto &entry : timeoutEvents) {
                cancelAndDelete(entry.second);
            }
            // time out handling
            // Handle timeout event
            expiredSequenceNumber = msg->getKind();
            EV<<"Time out event at time ["<<simTime().dbl()<<"], at Node["<<sender_file_index<<"] for frame with seq_num=["<<expiredSequenceNumber<<"]";
            EV << "Timeout occurred for message with sequence number " << expiredSequenceNumber << "\n";
            //read again the first message time out but error free this time
            //index of the first message of the window to be sent
            next_message_index = 0;
            //send the message
            //error free
            is_time_out = true;
            // send after 0.001 (to break any possible ties).
            scheduleAt( simTime() + 0.001, ProcessingTimeEvent);
        }
        else if (msg == ProcessingTimeEvent){
            std::string error_bits;

            //check don't exceed window size
            //check messages doesn't end
            if(next_message_index <= WS && sent_frames < messages.size()){
                // if the sender received non ack ==> resend the last message
                //error free
                if(is_non_ack){
                    //error free
                    error_bits = "0000";
                    //finished frames
                    sent_frames = last_acked_frame;
                }
                else if(is_time_out){
                    //error free
                    error_bits = "0000";
                    //finished frames
                    sent_frames = expiredSequenceNumber;
                }
                else{
                    //read next message and corresponding errors
                    error_bits = errors[next_message_index + windowBeg];
                    }
                EV<<"At time ["<<simTime().dbl()<<"], Node["<<sender_file_index<<"], [sent] frame with seq_num=[";

                Message_Base *new_msg = new Message_Base();
                //error_bits[0] indicates whether there is a modification error or not
                send_new_line(new_msg,error_bits[0] == '1');
                //printing
                EV << new_msg->getSeqNum() << "] and payload=[" << new_msg->getPayload() << "] and trailer=[" << new_msg->getParity() << "], ";
                EV<<"Modified ["<< (error_bits[0]=='0'?"-1":"1")<<"],";
                EV<<"Lost ["<< (error_bits[1]=='0'?"NO":"Yes")<<"],";
                EV<<"Duplicate ["<< error_bits[2]<<"],";
                EV<<"Delay ["<< (error_bits[3]=='0' ?0:ED)<<"]."<<endl;
                // TD delay for all messages
                double delay = TD;
                //check delay
                if(error_bits[3] == '1'){
                    //increase the delay
                    delay += ED;
                }
                //check loss error
                if(error_bits[1]!= '1'){
                    //no loss
                    sendDelayed(new_msg, delay, "nodeGate$o");
                    }
                else{
                    cancelAndDelete(new_msg);
                }
                //check duplicate error bit
                if(error_bits[2]== '1'){
                    //duplicate printing
                    EV<<"At time ["<<simTime().dbl() + DD<<"], Node["<<sender_file_index<<"], [sent] frame with seq_num=[";
                    EV << new_msg->getSeqNum() << "] and payload=[" << new_msg->getPayload() << "] and trailer=[" << new_msg->getParity() << "], ";
                    EV<<"Modified ["<< (error_bits[0]=='0'?"-1":"1")<<"],";
                    EV<<"Lost ["<< (error_bits[1]=='0'?"NO":"Yes")<<"],";
                    EV<<"Duplicate [2],";
                    EV<<"Delay ["<< (error_bits[3]=='0' ?0:ED)<<"]."<<endl;
                    //check loss error
                    if(error_bits[1]!= '1'){
                        //send duplicate version of the message
                        sendDelayed(new_msg->dup(), delay + DD, "nodeGate$o");
                    }
                }
                //else ==> loss
                //increment the next_message_index after sending
                next_message_index = (next_message_index + 1) % WS;
                //number of frames already sent
                sent_frames+=1;
                //print the following message prepare
                EV<<"AT ["<<simTime().dbl()<<"], Node["<<sender_file_index<<"], ";
                EV<<"Introducing channel error with code=["<<errors[next_message_index + windowBeg]<<"]"<<endl;
                //set processing time delay
                scheduleProcessingTime(PT);
                // Schedule the timeout event for the sent message
                scheduleTimeout(sent_frames);
                }
            //terminate condition
            //all message are sent ==> terminate
            else if(sent_frames == messages.size()){
                endSimulation();
            }

        }
    }
}
