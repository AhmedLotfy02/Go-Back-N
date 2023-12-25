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
    //if we setted the message outside ProcessingTimeEvent handling
    Message_Base *setted_message;
    // handling timeout Event
    cMessage *timeoutEvent;
    // Handling processing time after reading a line
    cMessage *ProcessingTimeEvent;
    //to check if the node is sender or not
    bool isSender;
    //The beginning of the window.
    int windowBeg;
    //Messages and their corresponding Errors
    std::vector<std::string>messages,errors;
    //is there timeout or not
    bool isTimeout;
    //to indicate the configuration phase
    bool from_coordinator;
    //to indicate the index of the node to read the file
    int sender_file_index;
    //the received ack number
    int acked_frame;
    //the previous received ack number
    int previous_acked_frame;
    //keep track of sent messages
    int next_message_index;
    // to indicate whether to resend the frames
    bool is_non_ack;
    // to indicate timeout occured
    bool is_time_out;
    // the seq number of time out
    int expiredSequenceNumber;
    // log output file
    std::ofstream output_file;
    //Receiver data
    int expected_seq_num;
    //number of frames already sent
    int sent_frames = 0;
    //to keep track all the window is sent to avoid resending before the timeout
    bool is_window_ended;
    //indicator that we are handling ack
    bool is_ack;
    // indicator that we set the message data outside ProcessingTimeEvent handling
    bool is_already_set;

    int last_sent_index = 0;
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
    void readInput(const char *filename);

    // function to apply byteStuffing in sender side
    std::string byteStuffing(std::string frame);
    // function to apply byteDestuffing in receiver side
    std::string byteDestuffing(const std::string& stuffedFrame);

    // function to apply check sum in sender side
    std::bitset<8> calculateChecksum(const std::string& str);

    // functions to deal with PT wait
    void scheduleProcessingTime(double PT);
    void cancelProcessingTime();
    // Fumction to send new message from the sender
    void send_new_line(Message_Base *new_msg, bool is_modification);
  //  void outputFile(int printCase, double eventTime, int NodeId, Message_Base* msg, std::string error );

    virtual void scheduleTimeout(int sequenceNumber);
    virtual void cancelTimeout(int sequenceNumber);
};

#endif
