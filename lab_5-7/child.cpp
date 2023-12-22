#include "zmq.hpp"
#include <sstream>
#include <string>
#include <iostream>
#include <zconf.h>
#include <vector>
#include <signal.h>
#include <fstream>
#include <algorithm>
#include <map>




void send_message(std::string message_string, zmq::socket_t& socket){
    zmq::message_t message_back(message_string.size());
    memcpy(message_back.data(), message_string.c_str(), message_string.size());
    if(!socket.send(message_back)){
        std::cout << "Error: can't send message from node with pid " << getpid() << std::endl;
    }
}


int main(int argc, char * argv[]){
    std::string adr = argv[0];
    zmq::context_t context(1);
    zmq::socket_t main_socket(context, ZMQ_REQ);

    main_socket.connect(argv[0]);

    send_message("OK: " + std::to_string(getpid()), main_socket);

    int id_proc = std::stoi(argv[1]);

std::vector<std::string>adreses;
    std::map<std::string, int> m;
    std::string adrs = "tcp://127.0.0.1:300";
    std::vector<int> child_id;
    std::vector<zmq::socket_t>list_of_scokets;
    while(1){
        zmq::message_t message_main;
        main_socket.recv(&message_main);
        std::string recieved_message(static_cast<char*>(message_main.data()), message_main.size());
        std::string command;
        for(int i = 0; i < recieved_message.size(); ++i){
            if(recieved_message[i] != ' '){
                command += recieved_message[i];
            } else {
                break;
            }
        }
        if(command == "exec"){
            int id,n,temp,val;
            std::string id_proc_;
            std::string name,  for_return;
            int flag = 0;
            std::string answer;
            for(int i = 5; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    temp=i;
                    break;
                }
            }
            id = std::stoi(id_proc_);
             id_proc_="";
             for(int i = temp+1; i < recieved_message.size(); ++i){
                    if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    temp=i;
                    break;
                }
             }
                n=std::stoi(id_proc_);
                id_proc_="";
                for(int i = temp+1; i < recieved_message.size(); ++i){
                    if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    temp=i;
                    break;
                }
                }
                name=static_cast<std::string>(id_proc_);
                id_proc_="";
                if(n==2){
                     for(int i = temp+1; i < recieved_message.size(); ++i){
                    if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    temp=i;
                    break;
                }
                }
val=std::stoi(id_proc_);
             }

            if(id_proc == id) {
                if(n==2){
                    m[name]=val;
                    answer="";
                }
                else{
                    if(m.find(name)!=m.end()){
                        answer=std::to_string(m[name]);
                    }
                    else answer ="not found";
                }


                for_return = "OK:" + std::to_string(id_proc) + ":"+name+" " + answer;
                send_message(for_return, main_socket);
            } else if(list_of_scokets.empty()){
                send_message("Error:id: Not found",main_socket);
            }else {
            std::string message_string = command + " " + std::to_string(id)+" "+std::to_string(n) + " " + name + " " + std::to_string(val);
            std::string to_user_message="";
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(message_string.size());
                memcpy(message.data(), message_string.c_str(), message_string.size());
                if(!list_of_scokets.at(i).send(message)){
                      std::cout << "Error: can't send message from child node in node with pid:" << getpid() << std::endl;
                };
                if(!list_of_scokets.at(i).recv(&message)){
                        std::cout << "Error: can't receive message from child node in node with pid:" << getpid() << std::endl;
                };
                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                to_user_message=recieved_message;
                 if(recieved_message[0]!='E'){
                    break;
                 }
                }

                send_message(to_user_message,main_socket);


            }
        } else if(command == "create"){
            int id,parent_id,temp;
            std::string id_proc_,parent_id_;
            for(int i = 7; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    temp=i;
                    break;
                }
            }
            for(int i = temp+1; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    parent_id_ += recieved_message[i];
                } else {
                    temp=i;
                    break;
                }
            }
            id = std::stoi(id_proc_);
            parent_id=std::stoi(parent_id_);
std::cout<<id<<" "<<parent_id<<std::endl;

            if(id_proc == id){
                send_message("Error: Already exists", main_socket);
            } else if(id_proc == parent_id){
                    int id_tmp = id - 1;
                std::string new_adr = adrs+ std::to_string(id_tmp);
                char* adr_ = new char[new_adr.size() + 1];
                memcpy(adr_, new_adr.c_str(), new_adr.size() + 1);
                char* id_ = new char[std::to_string(id).size() + 1];
                memcpy(id_, std::to_string(id).c_str(), std::to_string(id).size() + 1);
                char* args[] = {adr_, id_, NULL};
                    int f = fork();
                    if(f == 0){
                        execv("./child",args);
                    } else if (f == -1){
                        std::cout << "Error in forking in node with pid: " << getpid() << std::endl;
                    } else {
                    child_id.push_back(id);
                    list_of_scokets.emplace_back(context, ZMQ_REP);
                    list_of_scokets.back().bind(new_adr);
                    adreses.push_back(new_adr);

                        zmq::message_t message_from_node;
                        if(!list_of_scokets.back().recv(&message_from_node)){
                            std::cout << "Error: can't receive message from child node in node with pid:" << getpid() << std::endl;
                        }
                        std::string recieved_message_from_node(static_cast<char*>(message_from_node.data()), message_from_node.size());

                        if(!main_socket.send(message_from_node)){
                            std::cout << "Error: can't send message to main node from node with pid:" << getpid() << std::endl;
                        }
                    }
                    delete [] adr_;
                    delete [] id_;
                } else {
                    std::string to_user_message="";
                    if(child_id.empty()){
                    to_user_message="Error: Parent is not found ";
                    zmq::message_t message(to_user_message.size());
                    memcpy(message.data(), to_user_message.c_str(), to_user_message.size());
                    if(!main_socket.send(message)){
                        std::cout << "Error: can't send message to main node from node with pid: " << getpid() << std::endl;
                    }
                    continue;
                }
                std::string message_string = command + " " + std::to_string(id)+ " "+std::to_string(parent_id);

                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(message_string.size());
                memcpy(message.data(), message_string.c_str(), message_string.size());
                if(!list_of_scokets.at(i).send(message)){
                        if(child_id[i]!=id)
                      std::cout << "Error: can't send message from child node in node with pid:" << getpid() << std::endl;
                else{
                    send_message("Error:Parent is unavailible",main_socket);
                }
                };
                if(!list_of_scokets.at(i).recv(&message)){
                        if(child_id[i]!=id)
                        std::cout << "Error: can't receive message from child node in node with pid:" << getpid() << std::endl;
                else{
                    send_message("Error:Parent is unavailible",main_socket);
                }
                };
                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                to_user_message=recieved_message;
                if(recieved_message[0]!='E'||recieved_message=="Error: Already exists"||recieved_message=="Error: Parent is unavailable"){
                    break;
                }

                }
                zmq::message_t message(to_user_message.size());
                memcpy(message.data(), to_user_message.c_str(), to_user_message.size());
                if(!main_socket.send(message)){
                        std::cout << "Error: can't send message to main node from node with pid: " << getpid() << std::endl;
                    }

        } }else if(command == "ping") {
            int id;
            std::string id_proc_;
            for(int i = 5; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    break;
                }
            }
            id = std::stoi(id_proc_);
            if(id_proc == id){
                send_message("OK: 1", main_socket);
            } else if(child_id.empty()) {
                send_message("Error: not found", main_socket);
            }else{
                std::string message_string = command + " " + std::to_string(id);
            std::string to_user_message="";
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(message_string.size());
                memcpy(message.data(), message_string.c_str(), message_string.size());
                 if(!list_of_scokets.at(i).send(message)){
                      if(child_id[i]!=id){
                      std::cout << "Error: can't send message from child node in node with pid:" << getpid() << std::endl;
                      }
                      else {
                        send_message("OK:0",main_socket);
                      }
                };
                if(!list_of_scokets.at(i).recv(&message)){
                        if(child_id[i]!=id){
                        std::cout << "Error: can't receive message from child node in node with pid:" << getpid() << std::endl;
                        }
                         else {
                        send_message("OK:0",main_socket);
                      }
                };
                std::string recieved_message(static_cast<char*>(message.data()), message.size());
                to_user_message=recieved_message;
                 if(recieved_message[0]!='E'){
                    break;
                 }
                }
                send_message(to_user_message,main_socket);
            }
        } else if(command == "kill") {
            int id;
            std::string id_proc_;
            for(int i = 5; i < recieved_message.size(); ++i){
                if(recieved_message[i] != ' '){
                    id_proc_ += recieved_message[i];
                } else {
                    break;
                }
            }
            id = std::stoi(id_proc_);
           if(child_id.empty()) {
                send_message("Error: no find in child nodes", main_socket);
            }else if(count(child_id.begin(),child_id.end(),id)){
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
                 send_message("OK: It was killed", main_socket);
                        }
                }
            }
             else {
                std::string kill_message = command + " " + std::to_string(id);
                std::string to_user_message="";
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(kill_message.size());
                memcpy(message.data(), kill_message.c_str(), kill_message.size());
                if(!list_of_scokets.at(i).send(message)){
                      std::cout << "Error: can't send message from child node in node with pid:" << getpid() << std::endl;
                };
                if(!list_of_scokets.at(i).recv(&message)){
                        std::cout << "Error: can't receive message from child node in node with pid:" << getpid() << std::endl;
                };
                std::string received_message(static_cast<char*>(message.data()), message.size());
                to_user_message=received_message;
                 if(received_message[0]!='E'){
                    break;
                 }
                }

                 send_message(to_user_message,main_socket);
            }
        } else if (command == "DIE") {
            if(!child_id.empty()){
                std::string kill_message = "DIE";
                for(size_t i=0;i<list_of_scokets.size();++i){
                zmq::message_t message(kill_message.size());
                memcpy(message.data(), kill_message.c_str(), kill_message.size());
                list_of_scokets.at(i).send(message);
                list_of_scokets.at(i).unbind(adreses.at(i));
                list_of_scokets.at(i).close();
                }
            }

            context.close();
            return 0;
        }
    }
}
