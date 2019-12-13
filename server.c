
/**
	Lab 1 CSCE 3530
	Proxy server that connects to the website, sends an http requests,
	and forwards that request to the client.

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <assert.h>
#include <regex.h>
#include <stdbool.h>

// Initializes the server and listens to requests on the network.
int initializeServer(int port);

// Prints the error message when a certain condition is met, and prints the message of error.
void printErrorMessage(char condition, const char * error_mgs);

// Returns true if the specified word starts with the second parameter.
bool startsWith(const char* word, const char* startsWith);

// Returns true if the specified word in the first parameter contains the word in the second param.
bool contains(char* word, char* containsWord);

// Returns the content of the webpage that the client requested from us.
void getWebpageContent(char* uri, char* host, char* webpageContent);


// Main function which runs the program.
int main(int argc, char ** argv) {
    // Makes sure that the command line arguments are sufficient for
	// this program.
    if (argc != 2) {
        printf("Invalid command lind arguments; Usage: ./pserver <port>\n");
        return 1;
    }
	
	// Declare our strings which we will use to build the website content
	// and send it back to the client.
	char webpageToSend[128000];
	char urlFromClient[2048];
	char host[512];
	char uri[1536];
   
    // Starts to accept requests from the network.
    int sockfd = initializeServer(atoi(argv[1]));
    printf("Socket established; waiting on client to connect...\n");
	
	// Incoming connections are handled here; any newly connected client will appear
	// and have its own unique connection ID.
	int connectionFD = accept(sockfd, (struct sockaddr*) NULL, NULL);
	printf("Client connected.\n");
	
	// Once the client is connected, we are ready to grab the URL from the client.
	// The server reads the url sent and passes it to 'urlFromClient.'
	bzero(urlFromClient, 2048);
	read(connectionFD, urlFromClient, 2048);
	printf("Received URL from client: %s\n", urlFromClient);
	
	// The two code blocks below parse the URL and pass the host name to
	// the host char array and the URI to the uri char array.
	// URL starts with http:// or https://
	if(contains(urlFromClient, "//")) {
		// Splits up the URL into tokens separated by two slashes.
		char * tokens;
		tokens = strtok(urlFromClient, "//");
		char splitHostName[512];
		tokens = strtok(NULL, "//");
		
		// Host name is found and passed to the host char array.
		strncpy(splitHostName, tokens, 512);
		// Handles whether the URL contains www. or not and skips ahead 4 characters
		// in the host name if it does.
		if(startsWith(splitHostName, "www.")) {
			strncpy(host, &splitHostName[4], 512);
		}
		else {
			strncpy(host, splitHostName, 512);
		}
		tokens = strtok(NULL, "//");
		// No URI exists
		if(tokens == NULL) {
			strncpy(uri, "/", 1536);
		}
		// URI was found after the next slash /
		else {
			strncpy(uri, "/", 1536);
			strcat(uri, tokens);
		}
		printf("Host name: %s\n", host);
		printf("URI: %s\n", uri);
	}
	// Handles if the URL starts with www or not
	else {
		char * tokens;
		// Splits up the URL into tokens separated by one slash.
		tokens = strtok(urlFromClient, "/");
		strncpy(host, startsWith(urlFromClient, "www.") ? &tokens[4] : tokens, 512);
		tokens = strtok(NULL, "/");
		// No URI exists
		if(tokens == NULL) {
			strncpy(uri, "/", 1536);
		}
		// URI was found after the next slash /
		else {
			strncpy(uri, "/", 1536);
			strcat(uri, tokens);
		}
		printf("Host: %s\n", host);
		printf("URI: %s\n", uri);
		
	}
	
	// Sends the webpage back to the client after it requests it from the website.
	while(1) {
		printf("Starting to send webpage back to client...\n");
		
		// Ask for the website from the webpage
		bzero(webpageToSend, 128000);
		getWebpageContent(uri, host, webpageToSend);
		
		// Writes the contents of the webpage to the client.
		write(connectionFD, webpageToSend, strlen(webpageToSend));
		
		// Close our connection once we are done.
		close(connectionFD); 
		break;
    }
	
	// Closes our socket once we are done using it.
    close(sockfd);
	return 0;
}

// Initializes the server and listens to requests on the network.
int initializeServer(int port) {
	
	// Creates our server socket and opens it for connections.
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	struct sockaddr_in serverAddress;

    printErrorMessage(sockfd == -1, "Failed to initialize the server socket.");

	// Sets our server's address here.
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
	// Sets the port to be the same as the client.
    serverAddress.sin_port = htons(port);

    // Binds the socket to the address.
    printErrorMessage(bind(sockfd, (struct sockaddr *) &serverAddress, 
		sizeof(serverAddress)) == -1, "Failed to bind the server.");
    
	int option = 1;
    
	// This enables us to reuse our socket.
    printErrorMessage(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, 
		&option, sizeof(option)) == -1, "Failed on setsocketoption.");
    
	// We can listen to a maximum of 10 connections here.
    printErrorMessage(listen(sockfd, 10) != 0, "Failed to listen to the server.");

    return sockfd;
}

// Prints the error message when a certain condition is met, and prints the message of error.
void printErrorMessage(char condition, const char * error_mgs) {
	// Only prints if the condition is met.
	if (!condition) 
		return;
	perror(error_mgs);
	exit(1);
}

// Returns true if the specified word starts with the second parameter.
bool startsWith(const char* word, const char* startsWith) {
	// Compares the two strings to see if there's a match.
	if(strncmp(word, startsWith, strlen(startsWith)) == 0) 
		return 1;
	return 0;
}

// Returns true if the specified word in the first parameter contains the word in the second param.
bool contains(char* word, char* containsWord) {
	if (strstr(word, containsWord) != NULL)
		return true;
	return false;
}

// Returns the content of the webpage that the client requested from us.
void getWebpageContent(char* uri, char* host, char* webpageContent) {
	
	// The HTTP request includes the 
	char httpRequest[2000];
	// This is the entire content of the website that we are trying to grab.
	char contentGrabbed[128000];
	
	// Connects to the tcp server through port 80.
	int portNum = 80;
	
	// Initialize our socket variables.
	int socketFD;
	struct sockaddr_in serverAddress;
	struct hostent* webServer;
	
	// Sets the server to be the host passed as the parameter.
	webServer = gethostbyname(host);
	
	// Performs a check to verify if the web server is valid or not.
	if(webServer == NULL) {
		printf("Invalid response from the proxy server, try another URL.\n");
		return;
	}
	
	// Creates the webserver socket and opens it for connections
	socketFD = socket(AF_INET, SOCK_STREAM, 0);
	printErrorMessage(socketFD == -1, 
		"Error: failed to start the http request on: socket");
		
	// Zeroes out and sets the address via the specified port.
	bzero((char*) &serverAddress, sizeof(serverAddress));
	serverAddress.sin_family = AF_INET;
	bcopy((char*) webServer->h_addr, (char*) 
		&serverAddress.sin_addr.s_addr, webServer->h_length);
		serverAddress.sin_port = htons(portNum);
	
	// Attempts to connect to the website and returns an error if it can't.
	printErrorMessage(connect(socketFD, (struct sockaddr*) 
		&serverAddress, sizeof(serverAddress)) < 0, "Could not establish a connection to the website.\n");
	
	// This is where we create our HTTP request with the host and URI that
	// we grabbed from the URL sent by the client. This is a GET request
	// which returns the HTML document if the webpage doesn't have problems or hasn't moved.
	sprintf(httpRequest, 
		"GET %s HTTP/1.1\r\nHost: %s\r\n\r\n\r\n", uri, host);
		
	// Writes the request to the website and asks the website for the page content.
	printErrorMessage(write(socketFD, httpRequest, strlen(httpRequest)) < 0, "Could not write a request to the website.\n");
	
	// Zeroes out our content and the request
	bzero(contentGrabbed, 128000);
	bzero(httpRequest, 2000);
	
	// .. And reads the content from the webpage.
	read(socketFD, contentGrabbed, sizeof(contentGrabbed));
	
	// Copies the content that we read from the webpage onto the webpageContent char array
	// This will be sent to the client.
	strcpy(webpageContent, contentGrabbed);
	
	// Printing the same message to the server too to verify that 
	// the client and server both received the same content that
	// was requested.
	printf("\n%s\n", webpageContent);
	
	// Close our socket once we are done using it.
	close(socketFD);
}
