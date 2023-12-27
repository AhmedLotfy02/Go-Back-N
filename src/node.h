#ifndef __GOBACKN_NODE_H_
#define __GOBACKN_NODE_H_

#include <omnetpp.h>
#include <vector>
#include "message_m.h"
#include <string>
#include <bitset>
#include <fstream>
using namespace omnetpp;

class Node : public cSimpleModule
{
  protected:
    // handling timeout Event
    cMessage *timeoutEvent;
    //to check if the node is sender or not
    bool isSender;
    //The beginning of the window.
    int windowBeg;
    //Messages and their corresponding Errors
    std::vector<std::string>messages,errors;
    //to indicate the index of the node to read the file
    int sender_file_index;
    //the received ack number
    int acked_frame;
    //the previous received ack number
    int previous_acked_frame;
    //keep track of sent messages
    int next_message_index;
    // the seq number of time out
    int expiredSequenceNumber;
    // log output file
    std::ofstream output_file;
    //Reciever data
    int expected_seq_num;
    //number of frames already sent
    int sent_frames;
    //squence number of the next message
    int next_seq;
    //to keep track all the window is sent to avoid resending before the timeout
    bool is_window_ended;

    int timing_idx;
    // Map to store timeout events for each message
    std::map<int, cMessage *> timeoutEvents;
    // Parameters to use in sending and receiving
    double TD;
    double PT;
    double ED;
    double DD;
    double TO;
    int WS;
    int modification_random_number;

    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;

    // function to read input file
    void readInput(std::ifstream filestream);
    // function to apply byteStuffing in sender side
    std::string byteStuffing(std::string frame);
    // function to apply byteDestuffing in receiver side
    std::string byteDestuffing(const std::string& stuffedFrame);
    // function to apply check sum in sender side
    std::bitset<8> calculateChecksum(const std::string& str);
    // function to check sum at the receiver side
    bool checksum(char parity,std::string payload);
    // Function to prepare new message to send
    void prepare_new_message(Message_Base *new_msg, bool is_modification);
    // Function to send new message
    void my_message_handling(bool is_non_ack, bool is_time_out, double time_to_add);
    // start waiting for time_out timee after each message
    virtual void scheduleTimeout(int sequenceNumber, double sending_time);
    // stop all the time_out timers after an ack/non-ack/time_out occured
    virtual void cancelTimeout(int sequenceNumber);
    // to print char as bits
    std::string charToBinary(char c);

};

#endif
