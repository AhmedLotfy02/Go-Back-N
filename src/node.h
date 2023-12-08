#ifndef __GOBACKN_NODE_H_
#define __GOBACKN_NODE_H_

#include <omnetpp.h>
#include <vector>
#include "message_m.h"
#include <string>
#include <bitset>
using namespace omnetpp;

class Node : public cSimpleModule
{
  protected:
    //to check if the node is sender or not
    bool isSender;
    //window size
    int ws;
    //default delays PT+TD
    double delays=0.0;
    //The beginning of the window.
    int windowBeg = 0;
    //Messages and their corresponding Errors
    std::vector<std::string>messages,errors;
    //is there timeout or not
    bool isTimeout;
    //in case of timeout the first frame will be transmitted with no errors
    bool noErrors ;
    //to indicate the configuration phase
    bool startingPhase;
    //to indicate the index of the node to read the file
    int index;
    //to indicate the number of frames send and acknowledged to compare it with the number of messagse
    int finishedFrames;

    //Reciever data
    int expected_seq_num;
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void readInput(const char *filename);
    std::string byteStuffing(std::string f);
    std::bitset<8> calculateChecksum(const std::string& str);
};

#endif
