#include "node.h"
#include <fstream>
#include "message_m.h"
#include <string>
#include <bitset>

Define_Module(Node);

// to print parity char as bits
std::string Node::charToBinary(char c) {
    // Use std::bitset to get the binary representation
    std::bitset<8> bits(c);

    // Convert the bitset to a string
    return bits.to_string();
}

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

//check  sum aht the receiver side
bool Node::checksum(char parity, std::string payload){
    //Calclate the check sum and check it
    char checksum = parity;
    for (size_t i = 0; i < payload.length(); ++i) {
         checksum += static_cast<unsigned char>(payload[i]);
     }
    //If it is correct
     if(std::bitset<8>(checksum) == std::bitset<8>("11111111")){
         return true;
     }
     else{
         return false;
     }
}
//Function to read input file
void Node::readInput(std::ifstream filestream){
       std::string line;
       if(!filestream) {
           EV<<"error with reading file"<<endl;
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

void Node::scheduleTimeout(int sequenceNumber, double sending_time) {
    // Schedule a new timeout event for the given sequence number
    cMessage *timeoutEvent = new cMessage("Timeout");
    timeoutEvent->setKind(sequenceNumber);
    scheduleAt(simTime() + sending_time +  TO, timeoutEvent);  // Set timeout duration (adjust as needed)
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

void Node::prepare_new_message(Message_Base *new_msg, bool is_modification = false) {
    // get the message that should be sent from the messages list
    std::string message_text = messages[next_message_index + windowBeg];
    // Apply byte stuffing on the message
    std::string stuffedFrame = byteStuffing(message_text.c_str());
    //Calculate the check sum
    std::bitset<8> parityBitStream = calculateChecksum(stuffedFrame);//To set the checksum as char
    char parity = (char) parityBitStream.to_ulong();
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
        // Apply byte stuffing on the message after error
        stuffedFrame = byteStuffing(message_text.c_str());
    }
    // Prepare the message that will be sent
    new_msg->setPayload(stuffedFrame.c_str());
    new_msg->setParity(parity);
    // data message
    new_msg->setFrameType(2);
    // seq number(range from 0 to WS-1)
    new_msg->setSeqNum(next_seq%WS);

}
void Node::my_message_handling(bool is_non_ack, bool is_time_out, double time_to_add){
    if(next_message_index <= WS && sent_frames < messages.size() && !is_window_ended){
        Message_Base *new_msg = new Message_Base();
        std::string error_bits = errors[next_message_index + windowBeg];
        //the first frame after receiving a non ack
        if(is_non_ack && timing_idx == -1){
            //error free
            error_bits = "0000";
            //modify number os sent frames, as we will resend frames starting from the ack number
            sent_frames = next_message_index + windowBeg;
        }
        else if(is_time_out && timing_idx == -1){
            //error free
            error_bits = "0000";
            //modify number os sent frames, as we will resend frames starting from the ack number
            sent_frames = expiredSequenceNumber;
        }
        else{
            //console printing
            EV<<"AT ["<<simTime().dbl() + (timing_idx)*PT<<"], Node["<<sender_file_index<<"], ";
            EV<<"Introducing channel error with code=["<<errors[next_message_index + windowBeg]<<"]"<<endl;
            // file printing
            output_file.open("output.txt",std::fstream::app);
            output_file<<"AT ["<<simTime().dbl() + (timing_idx)*PT<<"], Node["<<sender_file_index<<"], ";
            output_file<<"Introducing channel error with code=["<<errors[next_message_index + windowBeg]<<"]"<<endl;
            output_file.close();
        }

        //error_bits[0] indicates whether there is a modification error or not
        prepare_new_message(new_msg,error_bits[0] == '1');
        //console printing
        EV<<"At time ["<<simTime().dbl() + timing_idx*PT + PT<<"], Node["<<sender_file_index<<"], [sent] frame with seq_num=[";
        EV << new_msg->getSeqNum() << "] and payload=[" << new_msg->getPayload() << "] and trailer=[" << charToBinary(new_msg->getParity()) << "], ";
        EV<<"Modified ["<< (error_bits[0]=='0'?"-1":"1")<<"],";
        EV<<"Lost ["<< (error_bits[1]=='0'?"NO":"Yes")<<"],";
        EV<<"Duplicate ["<< error_bits[2]<<"],";
        EV<<"Delay ["<< (error_bits[3]=='0' ?0:ED)<<"]."<<endl;
        //file printing
        output_file.open("output.txt",std::fstream::app);
        output_file<<"At time ["<<simTime().dbl() + timing_idx*PT + PT<<"], Node["<<sender_file_index<<"], [sent] frame with seq_num=[";
        output_file << new_msg->getSeqNum() << "] and payload=[" << new_msg->getPayload() << "] and trailer=[" << charToBinary(new_msg->getParity()) << "], ";
        output_file<<"Modified ["<< (error_bits[0]=='0'?"-1":"1")<<"],";
        output_file<<"Lost ["<< (error_bits[1]=='0'?"NO":"Yes")<<"],";
        output_file<<"Duplicate ["<< error_bits[2]<<"],";
        output_file<<"Delay ["<< (error_bits[3]=='0' ?0:ED)<<"]."<<endl;
        output_file.close();
        // TD delay for all messages
        //initial PT will be added to all messages

        double delay = TD + PT;

        //check delay
        if(error_bits[3] == '1'){
            //increase the delay
            delay += ED;
        }

        //check loss error
        if(error_bits[1]!= '1'){
            //no loss
            sendDelayed(new_msg, delay + PT*timing_idx + time_to_add, "nodeGate$o");
            }
        else{
            cancelAndDelete(new_msg);
        }

        //check duplicate error bit
        if(error_bits[2]== '1'){
            //duplicate printing
            //console printing
            EV<<"At time ["<<simTime().dbl() + DD + PT*(timing_idx)<<"], Node["<<sender_file_index<<"], [sent] frame with seq_num=[";
            EV << new_msg->getSeqNum() << "] and payload=[" << new_msg->getPayload() << "] and trailer=[" << charToBinary(new_msg->getParity()) << "], ";
            EV<<"Modified ["<< (error_bits[0]=='0'?"-1":"1")<<"],";
            EV<<"Lost ["<< (error_bits[1]=='0'?"NO":"Yes")<<"],";
            EV<<"Duplicate [2],";
            EV<<"Delay ["<< (error_bits[3]=='0' ?0:ED)<<"]."<<endl;
            //file printing
            output_file.open("output.txt",std::fstream::app);
            output_file<<"At time ["<<simTime().dbl() + DD + PT*(timing_idx)<<"], Node["<<sender_file_index<<"], [sent] frame with seq_num=[";
            output_file << new_msg->getSeqNum() << "] and payload=[" << new_msg->getPayload() << "] and trailer=[" << charToBinary(new_msg->getParity()) << "], ";
            output_file<<"Modified ["<< (error_bits[0]=='0'?"-1":"1")<<"],";
            output_file<<"Lost ["<< (error_bits[1]=='0'?"NO":"Yes")<<"],";
            output_file<<"Duplicate [2],";
            output_file<<"Delay ["<< (error_bits[3]=='0' ?0:ED)<<"]."<<endl;
            output_file.close();
            //check loss error
            if(error_bits[1]!= '1'){
                //send duplicate version of the message
                sendDelayed(new_msg->dup(), delay + DD + PT*timing_idx + time_to_add, "nodeGate$o");
            }
        }
        // Schedule the timeout event for the sent message
        scheduleTimeout(sent_frames, timing_idx*PT + PT);
        if(next_message_index == WS-1){
            //the first time this if condition is met
            if(!is_window_ended){
                //number of frames already sent
                sent_frames+=1;
            }
            //the window ends
            //stop sending until ack, non ack or timeout
            is_window_ended=true;
            // index will be updated in the new window
            next_message_index = WS;
            next_seq = next_seq + 1;
    }

    else{
        //increment the next_message_index after sending
        next_message_index = (next_message_index + 1) % WS;
        next_seq = next_seq + 1;
        //number of frames already sent
        sent_frames+=1;

    }
        timing_idx = timing_idx+1;
    }
    //terminate condition
    //all message are sent ==> terminate
    else if(sent_frames == messages.size()){
        endSimulation();
    }
}
void Node::initialize()
{
     //wheteher the current node is sender or receiver
     isSender= true;
     // Index of the input file
     sender_file_index=0;
     // Index of the ack number received from the receiver
     acked_frame = 0;
     // Index of the previous ack number received from the receiver
     previous_acked_frame = 0;
     // the true seq number expected in the receiver side
     expected_seq_num=0;
     // Index of the message to be sent
     next_message_index = 0;
     // the beginning of the window now
     windowBeg = 0;
     //will be true if the window ended
     //will be false if the winoe beginning changed or resending
     is_window_ended= false;
     //number of frames already sent
     sent_frames = 0;
     //squence number of the next message
     next_seq = 0;
     // Get the parameters to use in sending and receiving
     TD = getParentModule()->par("TD").doubleValue();
     PT = getParentModule()->par("PT").doubleValue();
     ED = getParentModule()->par("ED").doubleValue();
     DD = getParentModule()->par("DD").doubleValue();
     TO = getParentModule()->par("TO").doubleValue();
     WS = getParentModule()->par("WS").intValue();
     modification_random_number = getParentModule()->par("modification_random_number").intValue();

     // initialize time out messages
     timeoutEvent = new cMessage();
}

void Node::handleMessage(cMessage *msg)
{
    // Check if the received message is of type cMessage (message from another nodes)
    if (dynamic_cast<Message_Base*>(msg) != nullptr) {
        // Received a Message_Base message
        Message_Base *cmsg=check_and_cast<Message_Base *> (msg);
        // Check the arrival gate of the received message
        //coordinator message
            if (msg->getArrivalGate() == gate("coordinatorGate")) {
                if(strcmp(cmsg->getName(),"rec")==0){
                    //It's A receiver
                    isSender=false;
                }
                else{
                    //It's A sender
                    isSender=true;
                    // set the correct input file index (input0 ro input1)
                    if(strcmp( getName(),"node0")==0){
                        sender_file_index=0;
                    }
                    else{
                        sender_file_index=1;
                    }
                    //read the corresponding file
                    std::string filename="../src/input"+std::to_string(sender_file_index)+".txt";
                    std::ifstream filestream(filename);
                    if (filestream.is_open()) {
                        readInput(std::move(filestream));  // Use std::move to transfer ownership
                    } else {
                        std::cerr << "Error opening file." << std::endl;
                    }
                    // get time of the first message
                    // Convert const char* to double
                    char* endPtr;
                    double time_first_message = std::strtod(cmsg->getName(), &endPtr);
                    if (*endPtr != '\0') {
                        EV<<"error converting to double"<<endl;
                    }
                    else{
                        //the starting node should start reading its messages from its file on the specified starting time
                        //PT is the time between start reading and sending
                        cMessage * self_msg = new cMessage("start_sending");
                        scheduleAt(simTime() + time_first_message, self_msg);
                    }
                }
            }
            //ack / non ack
            else if(msg->getArrivalGate() == gate("nodeGate$i")){
                // ACK/ non ack
                if(isSender){
                    // if ack/not ack is received
                    if(cmsg->getFrameType()==1 || cmsg->getFrameType()==0){
                        //number of frames acked with this accumulative ack
                        int number_of_frames;
                        // number of received ack
                        acked_frame = cmsg->getAckNum();
                        //acked_frame - previous_acked_frame ==> to get number of frames acked
                        //2 acks within the same window
                        if(acked_frame >= previous_acked_frame){
                            number_of_frames = acked_frame - previous_acked_frame;
                        }
                        //ack in the new window
                        else{
                            number_of_frames = (WS - windowBeg) + acked_frame;
                        }
                        // increment the windowBeg by the number of frames acked
                        windowBeg = windowBeg + number_of_frames;
                        next_message_index = next_message_index - number_of_frames;
                        //if non ack ==> resend from the first index in the new window
                        if(cmsg->getFrameType()==0){
                            next_message_index = 0;
                        }
                        // Cancel the timeout of the last message before the ack number
                        //accumulative cancel
                        cancelTimeout(windowBeg - 1);
                        // update previous_acked_frame
                        previous_acked_frame = acked_frame;
                        is_window_ended = false;
                        int time_to_add;
                        //from the next message to send to the end of the new window
                        if(cmsg->getFrameType()==0){
                            timing_idx = -1;
                            time_to_add = 0.001;
                        }
                        else{
                            timing_idx = 0;
                            time_to_add = 0;
                        }
                        for(int i = next_message_index; i< windowBeg+ WS; i++){
                            my_message_handling(cmsg->getFrameType()==0, false, time_to_add);
                        }

                    }
                }
                //Receiver logic
                else{
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
                        //If it is correct
                         if(checksum(parity, payload)){
                             expected_seq_num = (expected_seq_num + 1)%WS;
                             //Frame type:ACK=1
                             frameType = 1;
                             //Deframing
                             std::string destuffedFrame = byteDestuffing(payload);
                             EV<<"Uploading payload=["<< destuffedFrame<<"] and seq_num =["<<seqNum<<"] to the network layer"<<endl;
                             output_file.open("output.txt",std::fstream::app);
                             output_file<<"Uploading payload=["<< destuffedFrame<<"] and seq_num =["<<seqNum<<"] to the network layer"<<endl;
                             output_file.close();
                        }
                        //Else
                        else{
                            //Frame type: NACK=0
                            frameType = 0;
                            }
                          EV<<"At time["<< simTime().dbl() + PT <<"], Node["<<(sender_file_index==0?"1":"0")<<"]";
                          EV<<"Sending ["<<(frameType==0?"NACK":"ACK")<<"] with number ["<<expected_seq_num<<"]";
                          output_file.open("output.txt",std::fstream::app);
                          output_file<<"At time["<< simTime().dbl() + PT <<"], Node["<<(sender_file_index==0?"1":"0")<<"]";
                          output_file<<"Sending ["<<(frameType==0?"NACK":"ACK")<<"] with number ["<<expected_seq_num<<"]";
                          output_file.close();
                         //The ACK/NACK number is set as the sequence number of the next correct expected frame.
                          cmsg ->setAckNum(expected_seq_num);
                          cmsg ->setFrameType(frameType);
                          //Loss or not
                          //Loss with probability = LP
                          volatile float val = uniform(0, 1);
                          if (val >= loss_probability) {
                              EV<<" , loss [No]"<<endl;
                              output_file.open("output.txt",std::fstream::app);
                              output_file<<" , loss [No]"<<endl;
                              output_file.close();
                              double delay = TD + PT;
                              //NO loss
                              isLoss= false;
                              //Send Ack.
                              sendDelayed(cmsg, delay, "nodeGate$o");
                          }
                          else{
                              EV<<" , loss [Yes]"<<endl;
                              output_file.open("output.txt",std::fstream::app);
                              output_file<<" , loss [Yes]"<<endl;
                              output_file.close();
                          }
                        }
                }
            }
    }
    //the beginning of sending
    else if(strcmp(msg->getName(), "start_sending") == 0){
        timing_idx = 0;
        //from the beginning of the message to the end of the first window
        for (int i = 0; i< WS ; i++){
            my_message_handling(false, false, 0);
        }
    }
    // Check if the message is a timeout alarm
    else if (std::strcmp(msg->getName(),"Timeout")==0) {
        // Delete any remaining timeout events
        for (auto &entry : timeoutEvents) {
            cancelAndDelete(entry.second);
        }
        // time out handling
        // Handle timeout event
        expiredSequenceNumber = msg->getKind();
        EV<<"Time out event at time ["<<simTime().dbl()<<"], at Node["<<sender_file_index<<"] for frame with seq_num=["<<expiredSequenceNumber%WS<<"]"<<endl;
        output_file.open("output.txt",std::fstream::app);
        output_file<<"Time out event at time ["<<simTime().dbl()<<"], at Node["<<sender_file_index<<"] for frame with seq_num=["<<expiredSequenceNumber%WS<<"]"<<endl;
        output_file.close();
        //read again the first message time out but error free this time
        //index of the first message of the window to be sent
        next_message_index = 0;
        //send the message
        //to resend
        is_window_ended= false;
        cancelTimeout(expiredSequenceNumber);
        timing_idx = -1;
        for(int i = next_message_index; i< windowBeg+ WS; i++){
            my_message_handling(false, true, 0.001);
        }

    }

}
