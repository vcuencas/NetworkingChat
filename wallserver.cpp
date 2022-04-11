//#pragma comment(lib, "ws2_32.lib")

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <vector>
//#include <netdb.h>
using namespace std;

int main(int argc, char* argv[]) {

    int n, newsockfd, sockfd, queueSize, portNum;
    socklen_t clilen;
    char inputCommand[255], message[255], name[255];
    string iCommand = "",  nameCont = "", messageCont = "";
    struct sockaddr_in serverAddress, clientAddress;
    vector<pair<string,string>> content;

    if (argc < 2) {
        cout << "Wall server running on port 5514 with queue size 20." << endl;
        queueSize = 20;
        portNum = 5514;
    } else if (argc < 3) {
	queueSize = atoi(argv[1]);
        cout <<"Wall server running on port 5514 with queue size " << queueSize << "." << endl;
        portNum = 5514;
    } else {
	queueSize = atoi(argv[1]);
	portNum = atoi(argv[2]);
        cout <<"Wall server running on port " << portNum << " with queue size " << queueSize << "." << endl;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0) {
        cout << "Error opening socket." << endl;
        return 0;
    }

    bzero((char *) &serverAddress, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(portNum);

    if (bind(sockfd, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
       cout << "Binding failed." << endl;
       return 0;
    }

    listen(sockfd, 5);
    clilen = sizeof(clientAddress);

    newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &clilen);

    if (newsockfd < 0) {
        cout << "Error on Accept" << endl;
    }

    int i = 1;
    //bool longInput = false;
    while (i != 0) {

        n = write(newsockfd, "Wall Contents\n", 14);
        //n = write(newsockfd, "\n", 2);
        n = write(newsockfd, "-------------\n", 14);
        if (content.empty()) {
            n = write(newsockfd, "[NO MESSAGES - WALL EMPTY]\n\n", 28);
        } else {
            if (content.size() > queueSize) {
                content.erase(content.begin());
            }
            for (int a = 0; a < content.size(); a++) {
                const char *first = content[a].first.c_str();
                const char *second = content[a].second.c_str();
                //cout << "first pair: " << content[a].first << "second pair: " << content[a].second << endl;
                n = write(newsockfd, first, content[a].first.size());
                n = write(newsockfd, second, content[a].second.size());
            }
            n = write(newsockfd, "\n", 1);
        }
	    n = write(newsockfd, "Enter command: ", 15);

        bzero(inputCommand, 255);
        n = read(newsockfd, inputCommand, 255);

        iCommand += inputCommand;
        bzero(inputCommand, 255);
        if (iCommand.find("clear") != string::npos) {
            n = write(newsockfd, "Wall cleared.\n\n", 15);
            content.clear();
        } else if (iCommand.find("post") != string::npos) {
            int maxSize = 80;
            nameCont.clear();
            messageCont.clear();
            n = write(newsockfd, "Enter name: ", 12);
            n = read(newsockfd, name, 255);

            for (int i = 0; i < maxSize; i++) {
               if (name[i] != '\n') {
                   nameCont += name[i];
               } else {
                   break;
               }
            }

            bzero(name, 255);

            nameCont.append(": ");

            maxSize -= nameCont.size();
            string postMessageInput = "Post [Max Length " + to_string(maxSize) + "]: ";

            const char *inputMes = postMessageInput.c_str();
            // update the max char count to work properly
            n = write(newsockfd, inputMes, strlen(inputMes));
            n = read(newsockfd, message, 255);

            //cout << "message: " << message << endl;
            messageCont += message;

            bzero(message, 255);
            //cout << "messageCont.size(): " << messageCont.size() << endl;
            if (messageCont.size() > maxSize) {
                write(newsockfd, "Error: message is too long!\n\n", 29);
//                longInput = true;
            } else {
                n = write(newsockfd, "Successfully tagged the wall.\n\n", 31);
                //cout << "name: " << nameCont << "message " << messageCont << endl;
                content.push_back(make_pair(nameCont, messageCont));
            }
        } else if (iCommand.find("kill") != string::npos) {
            n = write(newsockfd, "Closing socket and terminating server. Bye!", 44);
            i = 0;
        } else if (iCommand.find("quit") != string::npos) {
            n = write(newsockfd, "Come back soon. Bye!\n", 22);
		    close(newsockfd);
            newsockfd = accept(sockfd, (struct sockaddr *) &clientAddress, &clilen);
        }
        iCommand.clear();
        //int i = strncmp("Bye", inputCommand, 3);

        if (i == 0)
            break;

    }
    close(newsockfd);
    close(sockfd);
    return 0;
}
