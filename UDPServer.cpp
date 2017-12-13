#include <iostream>
#include <sys/types.h>
#include <cstdlib>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <netdb.h>
#include<fstream>
#include<map>
#include <sstream>
#include <sys/time.h> 

using namespace std;

/**
 * This class is an implementation of connectionless, unreliable UDP server 
 * used to service multiple client requests.  
 * @author Shruti Rachh
 *
 */
class UDPServer
{
	private:
	int serverSocket;
   	socklen_t clientSockLen;
   	struct sockaddr_in serverAddress;
   	struct sockaddr_in clientAddress;
   	char buff[1024];	
	map<string,string> httpStatusCodes;
	char msgContent[1009584];
	
	public:
  	void displayError(const char *errorMsg);
	void setHttpStatusCodes();	
	void createSocket();
	void bindAddress(int portNumber);
	void setClientSockLength();
	int receiveRequest();
	string* processRequest();
	string getRequestedContent(string filename);	
	string setStatusLine(string statusCode,string httpVersion);	
	string* setResponseHeaders(string fileExt);	
	string createResponse(string statusCode,string httpVersion,string fileExt);
	int sendResponse(string response);
};

/**
 * This method is used to display the error message.
 * @param
 * 	errorMsg: The error message that is to be displayed.
*/
void UDPServer::displayError(const char *errorMsg)
{
	cerr<<"Error: "<<errorMsg<<endl;
	exit(1);
}

/**
 * This method is used to set the various HTTP status codes and 
 * their corresponding HTTP messages in a map.
 */
void UDPServer::setHttpStatusCodes()
{
	httpStatusCodes["200"]="OK";
	httpStatusCodes["404"]="Page Not Found";
	httpStatusCodes["400"]="Bad Request";	
}

/**
 * This method is used to create the server socket and sets the 
 * serverSocket class variable.
 */
void UDPServer::createSocket()
{
	serverSocket=socket(AF_INET, SOCK_DGRAM, 0);
   	if (serverSocket < 0) 
	{
		displayError("The server socket could not be opened!");
	}
}

/**
 * This method is used to bind the server socket to an address.
 * @param
 * 	portNumber: The port number on which the server will listen for incoming connections.
 */
void UDPServer::bindAddress(int portNumber)
{
	int leng=sizeof(serverAddress);
	bzero(&serverAddress,leng);
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(portNumber);     	
	serverAddress.sin_addr.s_addr = INADDR_ANY;
     	
	if (bind(serverSocket, (struct sockaddr *) &serverAddress,leng) < 0) 
  	{      
		displayError("There is some problem while binding the server socket to an address!");
	}
}

void UDPServer::setClientSockLength()
{
	clientSockLen = sizeof(struct sockaddr_in);
}

int UDPServer::receiveRequest()
{
       	int noOfCharacters = recvfrom(serverSocket,buff,1024,0,(struct sockaddr *)&clientAddress,&clientSockLen);	       	
	if (noOfCharacters < 0) 
	{
		displayError("There is some problem in receiving the request!");
	}
	return noOfCharacters;
}

/**
 * This method is used to process the client request. 
 * It extracts the HTTP version, file name and the connection
 * @return
 *	a string array consisting of the file name, HTTP version and connection type. 
 */
string* UDPServer::processRequest()
{
	char *filename;
	string httpVersion;
	string request=string(buff);
	string httpVersionStr;
	string statusCode;
	char *req=strtok(buff,"/");
	if(req)
	{
		
		req=strtok(NULL," ");
		if(req)
		{
			filename=req;
			req=strtok(NULL,"\n");
			if(req)
			{
				httpVersionStr=string(req);
				httpVersion=httpVersionStr.substr(4,(httpVersionStr.size()-5));	
			}
		}	
	}
	if(strcmp(httpVersionStr.c_str(),"HTTP/1.1")==0 || strcmp(request.substr(0,3).c_str(),"GET")==0)
	{
		statusCode="200";
	}else
	{
		statusCode="400";
	}
	string* values=new string[3];
	values[0]=filename;
	values[1]=httpVersion;
	values[2]=statusCode;
	return values;
}

/**
 * This method gets the requested file contents.
 * It sets the status code to either 200 or 404 depending 
 * on whether the file is present or not on the server's 
 * directory.
 * @param
 * 	filename: The name of the file requested.
 * @return 
 * 	the HTTP status code
 */
string UDPServer::getRequestedContent(string filename)
{
	int iteration=0;
	string statusCode;
	bzero(msgContent,1009584);
	ifstream readFile;
	readFile.open(filename.c_str());		
	if(readFile.is_open())
	{
		while(!readFile.eof())
		{
			readFile.get(msgContent[iteration++]);
		}
		statusCode="200";
	}
	else
	{
		statusCode="404";
	}
	return statusCode;
}

/**
 * This method is used by the createResponse method in order to set the
 * status line of the response.
 * @param
 *	statusCode: The HTTP status code of the response.
 * @param
 * 	httpVersion: The HTTP version of the response.
 * @return
 *	the status line string
 */
string UDPServer::setStatusLine(string statusCode,string httpVersion)
{
	string statusLine="HTTP"+httpVersion+" "+statusCode+" "+httpStatusCodes[statusCode];
	return statusLine;
}

/**
 * This method is used by the createResponse method in order to set the
 * headers of the response.
 * @param
 *	fileExt: The file extension used to determine whether the content is of text/html or not.
 * @return
 * 	the response headers
 */
string* UDPServer::setResponseHeaders(string fileExt)
{
	string* headers=new string[2];
	int len=strlen(msgContent);
	stringstream contentLen;
	contentLen<<len;
	headers[0]="Content-Length: "+contentLen.str();	
	if(fileExt==".html")
	{
		headers[1]="Content-Type: text/html";
	}
	else
	{
		headers[1]="Content-Type: text/plain";
	}
	
	return headers;
}

/**
 * This method is used to create the complete HTTP response.
 * @param
 *	statusCode: The HTTP status code of the response.
 * @param
 * 	httpVersion: The HTTP version of the response.
 * @param
 *	fileExt: The file extension used to determine whether the content is of text/html or not.
 * @return
 *	the HTTP response
 */
string UDPServer::createResponse(string statusCode,string httpVersion,string fileExt)
{		
	string response;	
	string statusLine=setStatusLine(statusCode,httpVersion);
	string* headers=setResponseHeaders(fileExt);	
	if(statusCode=="200")
	{
		response=statusLine+"\r\n"+headers[0]+"\r\n"+headers[1]+"\r\n\r\n"+msgContent;
	}else if(statusCode=="404")
	{
		if(fileExt==".html")
		{		
			response=statusLine+"\r\n"+headers[1]+"\r\n\r\n<html>404 Page Not Found</html>";
		}
		else
		{
			response=statusLine+"\r\n"+headers[1]+"\r\n\r\n404 File Not Found";
		}
	}else
	{
		response=statusLine+"\r\n"+headers[1]+"\r\n\r\n400 Bad Request";
	}				
	return response;		
}

/**
 * This method is used to send the response to the client.
 * @param
 *	response: the response to be sent
 * @param
 * 	clientSock: the client socket
 * @return
 *	the number of characters written
 */
int UDPServer::sendResponse(string response)
{
	int no=0;
	struct timeval t1, t2;
	double elapsedTime;
	gettimeofday(&t1, NULL);
	if(response.length()<=1009629)
	{
		no=sendto(serverSocket,(const void *)response.c_str(),response.length(),0,(struct sockaddr *)&clientAddress,clientSockLen);   			
	  if (no < 0)
		{ 
			displayError("There is problem while writing to socket!");
		}
	}
	else
	{
		int start=0,end=1009627;
		string resp="";
		int responseLen=response.length();
		
		while(start<responseLen)
		{
			if(end>responseLen)
			{
				resp=response.substr(start,responseLen-1);
				no+=sendto(serverSocket,(const void *)resp.c_str(),(responseLen-start),0,(struct sockaddr *)&clientAddress,clientSockLen);
				break;
			}
			else
			{
				resp=response.substr(start,end);
			}
			no+=sendto(serverSocket,(const void *)resp.c_str(),(end-start+1),0,(struct sockaddr *)&clientAddress,clientSockLen);
			start=end+1;
			end=end+start; 
		}
		gettimeofday(&t2, NULL);
	  	
	  	elapsedTime = (t2.tv_sec - t1.tv_sec) * 1000.0;      
	  	elapsedTime += (t2.tv_usec - t1.tv_usec) / 1000.0;   
	  	cout << "The time when the last byte was sent: "<<elapsedTime << " ms\n";
		cout<<"No. of bytes sent: "<<no<<endl;
		
	}
	return no;
}

int main(int noOfArguments,char *argumentList[])
{
	UDPServer server;
	/*
         * It checks if all the command-line arguments are provided.
         */	
	if(noOfArguments<2)
	{
		server.displayError("The client must provide a port number!");
	}
	server.createSocket();
	int portNo=atoi(argumentList[1]);	
	server.bindAddress(portNo);
	server.setClientSockLength();	
	while(1)
	{
		server.receiveRequest();
		string* values=server.processRequest();
		size_t dotPos=values[0].find(".");
    		string fileExt=values[0].substr(dotPos);
		server.setHttpStatusCodes();	
		string statusCode;	
		if(strcmp(values[2].c_str(),"400")!=0)
		{    		
			statusCode=server.getRequestedContent(values[0]);
		}else
		{	
			statusCode=values[2];
		}	
    		string response=server.createResponse(statusCode,values[1],fileExt);
		int no=server.sendResponse(response);
	}
	return 0;	
}
