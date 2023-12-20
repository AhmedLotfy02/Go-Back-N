#include "coordinator.h"
#include <vector>
#include <fstream>
#include "message_m.h"
using namespace std;

Define_Module(Coordinator);

void Coordinator::readInput(){
        ifstream filestream;
        string line;
        //CHange the file path according to the team member
       std::string file_path="";
       std::string name="Donia";
       if(name=="heba")
           file_path="E:/CMP4/Networks/Go-Back-N/";
       else if(name=="shaza")
           file_path="D:/Shozy/Networks/project/Go-Back-N/";
       else if(name=="ahmed")
           file_path="C:/Users/LP-7263/Documents/CMP4/Networks/Project/Go-Back-N/";
       else if(name == "Donia")
           file_path = "/home/donia/Desktop/college/networks/Go-Back-N/";

       filestream.open(file_path+"coordinatorfile.txt", ifstream::in);


       if(!filestream.is_open()) {
           EV<<"error with reading file"<<endl;
           return ;
       } else {
           // Read all lines
           while ( getline(filestream, line) ) {
                   EV<<line<<endl;
               if (line.find(',')) {
                   int beg = line.find(',');
                   // The sender node inde (0 o)
                   char sender_idx = line[beg-1];
                   EV<<"sender_idx "<<sender_idx<<endl;
                   sender_node =  (sender_idx=='0')?0:1;
                   startingTime = stod(line.substr(beg+1, line.size()-beg-2));
                   EV<<"startingTime "<<startingTime<<endl;
                   output_file.open("output.txt");
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
    if(sender_node == 0){
        send(receiverMsg, "port1$o");
        send(senderMsg, "port0$o");
    } else{
        send(receiverMsg, "port0$o");
        send(senderMsg, "port1$o");
    }
}



void Coordinator::handleMessage(cMessage *msg)
{
    // TODO - Generated method body
}
