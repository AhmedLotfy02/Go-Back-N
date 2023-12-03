//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

#ifndef __GOBACKN_NODE_H_
#define __GOBACKN_NODE_H_

#include <omnetpp.h>
#include <vector>
#include "message_m.h"
#include <string>
using namespace omnetpp;

/**
 * TODO - Generated class
 */
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
    virtual void initialize() override;
    virtual void handleMessage(cMessage *msg) override;
    void readInput(const char *filename);
    std::string byteStuffing(std::string f);
};

#endif
