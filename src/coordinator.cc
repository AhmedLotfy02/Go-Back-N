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

#include "coordinator.h"
#include <vector>
#include <fstream>
#include "message_m.h"
using namespace std;

Define_Module(Coordinator);

void Coordinator::readInput(){
        ifstream filestream;
        string line;
       filestream.open("D:/Shozy/Networks/project/Go-Back-N/coordinatorfile.txt", ifstream::in);

       if(!filestream.is_open()) {
           EV<<"error with reading file"<<endl;
           return ;
       } else {
           while ( getline(filestream, line) ) {
                   EV<<line<<endl;
               if (line.find(',')) {
                   int beg = line.find(',');
                   char s=line[beg-1];
                   EV<<s<<endl;
                   chosenNode =  (s=='0')?0:1;
                   startingTime = stod(line.substr(beg+1, line.size()-beg-2));
                   EV<<chosenNode;
                   return ;

               }
           }
       }
       return ;

}

void Coordinator::initialize()
{
    // TODO - Generated method body
    readInput();
    Message_Base *senderMsg = new Message_Base(to_string(startingTime).c_str());
    Message_Base *receiverMsg = new Message_Base("rec");
    if(chosenNode == 0){
        send(senderMsg, "port0$o");
        send(receiverMsg, "port1$o");
    } else{
        send(senderMsg, "port1$o");
        send(receiverMsg, "port0$o");
    }
}



void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
