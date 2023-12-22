#include "zmq.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <zconf.h>
#include <vector>
#include <signal.h>
#include <sstream>
#include <set>
#include <algorithm>




int main(){
    zmq::context_t context(1);
    zmq::socket_t main_socket(context, ZMQ_REP);
    std::vector<zmq::socket_t>list_of_scokets;
    std::string adr = "tcp://127.0.0.1:300";
    std::string command;
    std::vector<int> child_id;
    std::vector<std::string>adreses;
    while(1){
        std::cout << "command:";
        std::cin >> command;
        if(command == "create") {
                int id,parent_id;
                std::cin >> id>>parent_id;
            if(parent_id == -1){

                int id_tmp = id - 1;

                std::string new_adr = adr + std::to_string(id_tmp);

                char* adr_ = new char[new_adr.size() + 1];
                memcpy(adr_, new_adr.c_str(), new_adr.size() + 1);
                char* id_ = new char[std::to_string(id).size() + 1];
                memcpy(id_, std::to_string(id).c_str(), std::to_string(id).size() + 1);
                char* args[] = { adr_, id_, NULL};
                int id2 = fork();
                if (id2 == -1) {
                    std::cout << "Error:Unable to create  worker node" << std::endl;
                    id = 0;
                    exit(1);
                } else if(id2 == 0){
                    execv("./child",args);
                } else {
                    child_id.push_back(id);
                    list_of_scokets.emplace_back(context, ZMQ_REP);


list_of_scokets.back().bind(new_adr);
adreses.push_back(new_adr);
                }
                zmq::message_t message;


                if(!list_of_scokets.back().recv(&message)){
                            std::cout << "Error: can't receive message from child node in node with pid:" << getpid() << std::endl;
                        }


                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                std::cout << recieved_message << std::endl;
                delete [] adr_;
                delete [] id_;
            } else {
                if(child_id.empty()){
                    std::cout<<"Error: Parent not found";
                    continue;
                }
                std::string message_string = command + " " + std::to_string(id)+ " "+std::to_string(parent_id);
                std::string to_user_message="";
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(message_string.size());
                memcpy(message.data(), message_string.c_str(), message_string.size());
                if(!list_of_scokets.at(i).send(message)){
                        if(child_id[i]!=id)
                      std::cout << "Error: can't send message from child node in node with pid:" << getpid() << std::endl;
                else{
                    to_user_message="Error:Parent is unavailible";
                    break;
                }
                };
                if(!list_of_scokets.at(i).recv(&message)){
                        if(child_id[i]!=id)
                        std::cout << "Error: can't receive message from child node in node with pid:" << getpid() << std::endl;
                else{
                    to_user_message="Error:Parent is unavailible";
                    break;
                }
                };
                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                to_user_message=recieved_message;
                if(recieved_message[0]!='E'||recieved_message=="Error: Already exists"||recieved_message=="Error: Parent is unavailable"){
                    break;
                }

                }
                std::cout << to_user_message << std::endl;
            }
        } else if (command == "exec") {
            int id,n;
            std::string name, val;
            val="";
            std::cin >> id>>n;
            if(n==2){
            std::cin >> name >> val;
            }
            else if(n==1){
                 std::cin >> name;
            }
            else{
                std::cout << "Error: incorrect command - number of arguments\n";
                continue;
            }
            std::string message_string = command + " " + std::to_string(id)+" "+std::to_string(n) + " " + name + " " + val;
            std::string to_user_message="";
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(message_string.size());
                memcpy(message.data(), message_string.c_str(), message_string.size());
                list_of_scokets.at(i).send(message);
                list_of_scokets.at(i).recv(&message);
                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                to_user_message=recieved_message;
                 if(recieved_message[0]!='E'){
                    break;
                 }
                }

                std::cout << to_user_message << std::endl;
        } else if (command == "ping") {
            int id;
            std::cin >> id;
            std::string message_string = command + " " + std::to_string(id);
            std::string to_user_message="";
            if(child_id.empty()){
                std::cout <<"Error: not found"<<std::endl;
                continue;
            }
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(message_string.size());
                memcpy(message.data(), message_string.c_str(), message_string.size());
                if(!list_of_scokets.at(i).send(message)){
                      if(child_id[i]!=id){
                      std::cout << "Error: can't send message from child node in node with pid:" << getpid() << std::endl;
                      }
                      else {
                        to_user_message="OK:0";
                        break;
                      }
                };
                if(!list_of_scokets.at(i).recv(&message)){
                        if(child_id[i]!=id){
                        std::cout << "Error: can't receive message from child node in node with pid:" << getpid() << std::endl;
                        }
                         else {
                        to_user_message="OK:0";
                        break;
                      }
                };
                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                to_user_message=recieved_message;
                 if(recieved_message[0]!='E'){
                    break;
                 }
                }

                std::cout << to_user_message << std::endl;
        } else if(command == "kill"){
            int id;
            std::cin >> id;
            if(child_id.empty()){
                std::cout << "Error: there isn't nodes" << std::endl;
            } else if(id==-1){
                std::cout<<"Error: Use exit command to kill this node with all nodes";
            } else if(count(child_id.begin(),child_id.end(),id)){
                std::string kill_message = "DIE";
                for(size_t i=0;i<list_of_scokets.size();++i){
                        if(child_id[i]==id){
                zmq::message_t message(kill_message.size());
                memcpy(message.data(), kill_message.c_str(), kill_message.size());
                if(!list_of_scokets.at(i).send(message)){
                      std::cout << "Error: can't send message from child node in node with pid:" << getpid() << std::endl;
                };
                list_of_scokets.at(i).unbind(adreses.at(i));
                list_of_scokets.at(i).close();
                list_of_scokets.erase(list_of_scokets.begin()+i);
                child_id.erase(child_id.begin()+i);
                adreses.erase(adreses.begin()+i);
                 std::cout << "OK: It was killed" << std::endl;
                        }
                }
            }
             else {
                std::string kill_message = command + " " + std::to_string(id);
                std::string to_user_message="";
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(kill_message.size());
                memcpy(message.data(), kill_message.c_str(), kill_message.size());
                list_of_scokets.at(i).send(message);
                list_of_scokets.at(i).recv(&message);
                std::string received_message(static_cast<char*>(message.data()), message.size());
                to_user_message=received_message;
                 if(received_message[0]!='E'){
                    break;
                 }
                }

                std::cout << to_user_message << std::endl;
            }
        } else if(command == "exit"){
            if(!child_id.empty()){
                std::string kill_message = "DIE";
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(kill_message.size());
                memcpy(message.data(), kill_message.c_str(), kill_message.size());
                list_of_scokets.at(i).send(message);
                list_of_scokets.at(i).unbind(adreses.at(i));
                list_of_scokets.at(i).close();
                }
                std::cout << "Tree was deleted" << std::endl;
            }
            main_socket.close();
            context.close();
            return 0;
        } else {
            std::cout << "Error: incorrect command\n";
        }
    }
}
