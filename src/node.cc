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

#include "node.h"
#include<fstream>
Define_Module(Node);

void Node::initialize()
{
    // TODO - Generated method body
}
void Node::readInput(char *filename){

        std::ifstream filestream;
       std::string line;

       filestream.open(filename, std::ifstream::in);

       if(!filestream) {
           throw cRuntimeError("Error opening file '%s'?", filename);
       } else {
           while ( getline(filestream, line) ) {
               std::string error = line.substr(0,4);
               errors.push_back(err);
               std::string message = line.substr(5);
               messages.push_back(mes);
           }
       }
       filestream.close();
       return;

}
void Node::handleMessage(cMessage *msg)
{
    // TODO - Generated method body




    EV<<"i'm node "<<getName()<<" ,received :"<<msg->getName()<<endl;
}
