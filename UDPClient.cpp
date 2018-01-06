#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <cstdlib>
#include <unistd.h>
#include <string.h>
#include<fstream>
#include <sstream>

using namespace std;

/**
 * The UDPClient class is used to send requests for files to the server.  
 * @author Shruti Rachh
 *
 */
class UDPClient
{
	private:
	int clientSocket;
   	unsigned int len;
   	struct sockaddr_in serverAddress,clientAddress;
  	struct hostent *host;
   	char buffer[1009629];
		
	public:
	void displayError(const char *errorMsg);
	void createSocket();
	void getServerInfo(char* hostname);
	void setServerAddress(int portNo);
	string* setRequestHeaders();
	string createRequest(string filename);
	int sendRequest(string request);
	int readResponse();
	void displayResponse(string response);	
};

/**
 * This method is used to display the error message.
 * @param
 * 	errorMsg: The error message that is to be displayed.
*/
void UDPClient::displayError(const char *errorMsg)
{
	cerr<<"Error: "<<errorMsg<<endl;
	exit(1);
}

/**
 * This method is used to create the client socket and sets the 
 * clientSocket class variable.
 */
void UDPClient::createSocket()
{
	clientSocket=socket(AF_INET,SOCK_DGRAM,0);
	if (clientSocket < 0)
  	{
		displayError("The server socket could not be opened!");
	}
}

/**
 * This method is used to get the IP address of the server
 * based on the hostname provided by the client.
 * @param
 *	hostname: the server hostname
 */
void UDPClient::getServerInfo(char* hostname)
{
	host=gethostbyname(hostname);
	if(host==NULL)
	{
		displayError("There is no such host!");
	}
}

/**
 * This method is used to set the server address.
 * @param
 *	portNo: the port number on which server is listening.
 */
void UDPClient::setServerAddress(int portNo)
{
	bzero((char *) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	bcopy((char *)host->h_addr,(char *)&serverAddress.sin_addr.s_addr,host->h_length);
	serverAddress.sin_port = htons(portNo);
}

/**
 * This method is used by the createRequest method in order to set the
 * headers of the request. 
 * @param
 *	connectionType: The connection type will be either close for non-persistent connection
 *			or keep-alive for persistent connection.
 * @return
 *	string array consisting of all the request headers
 */
string* UDPClient::setRequestHeaders()
{
	string* headers=new string[4];
	char tmpServerAddr[32];
	const char* dottedDecimalStr=inet_ntop(AF_INET,(void *)&serverAddress.sin_addr.s_addr,tmpServerAddr,sizeof(tmpServerAddr));
	stringstream serverPort;
	serverPort<<ntohs(serverAddress.sin_port);	
	headers[0]="Host: "+string(dottedDecimalStr)+":"+serverPort.str()+"\r\n";
	headers[1]="Accept: text/plain; charset='UTF-8'\r\n";
	headers[2]="Accept-Encoding: gzip, deflate, sdch\r\n";
	headers[3]="Accept-Language: en-US,en;q=0.8";
	return headers;
}

/**
 * This method is used to create the request.
 * @param
 *	filename: the name of the file that is to be requested.
 * @param
 *	connectionType: The connection type will be either close for non-persistent connection
 *			or keep-alive for persistent connection.
 * @return
 *	the request string to be sent
 */
string UDPClient::createRequest(string filename)
{
	string* headers=setRequestHeaders();
	string requestLine="GET /"+filename+" HTTP/1.1";
	string request=requestLine+"\r\n";
	
	for(int it=0;it<4;it++)
	{
		request=request+headers[it];
	}
	return request;
}

/**
 * This method is used to send the request to the server.
 * @param
 *	request: the request to be sent.
 * @return
 * 	the number of characters written.
 */
int UDPClient::sendRequest(string request)
{
	bzero(buffer,1009629);
	strncpy(buffer,request.c_str(),1009629);
	len=sizeof(struct sockaddr_in);
	int no=sendto(clientSocket,buffer,strlen(buffer),0,(const struct sockaddr *)&serverAddress,len);	
	if (no<0)
   	{
		displayError("There is problem while sending request!");
	}
	return no;
}

/**
 * This method reads the server response into a buffer.
 * @return 
 * 	the number of characters read.
 */
int UDPClient::readResponse()
{
	bzero(buffer,1009629);
	int no=recvfrom(clientSocket,buffer,1009629,0,(struct sockaddr *)&clientAddress, &len);	
	string resp=string(buffer);
	displayResponse(resp);	
	string contentLengthHeader="Content-Length: ";	
	size_t contentLengthStartPos=resp.find(contentLengthHeader.c_str());
	size_t contentLengthEndPos=resp.find("\r\n");
	string contentLenStr=resp.substr(contentLengthStartPos+contentLengthHeader.length(),contentLengthEndPos);
	int contentLen=atoi(contentLenStr.c_str());
	while(contentLen>1009564)
	{
		bzero(buffer,1009629);
		no+=recvfrom(clientSocket,buffer,1009629,0,(struct sockaddr *)&clientAddress, &len);
		if(no<=0)
			break;				
		resp+=string(buffer);	
		cout<<resp<<endl;
		contentLen=contentLen-1009629;	
	}
	
  	if (no<0)
    	{
		displayError("There is problem while reading from socket");
	}
	cout<<"Number of bytes received: "<<no<<endl;
	return no; 
}

/**
 * This method is used by readResponse to display the requested file content.
 * @param
 *	response: the response containing the header and the file content.
 */
void UDPClient::displayResponse(string response)
{
	size_t fileContentStartPos=response.find("\r\n\r\n");
	string fileContent=response.substr(fileContentStartPos+4);
	cout<<fileContent<<endl;
}

int main(int noOfArguments,char *argumentList[])
{
	UDPClient client;
	/*
     * It checks if all the command-line arguments are provided.
     */	
	if(noOfArguments<4)
	{
		client.displayError("Invalid arguments!");
	}  
	client.createSocket();
	int portNo=atoi(argumentList[2]);
	client.getServerInfo(argumentList[1]);
	client.setServerAddress(portNo);
	string request=client.createRequest(argumentList[3]);	
	int size=client.sendRequest(request);
	size=client.readResponse(); 
	return 0;
}
