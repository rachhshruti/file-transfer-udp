# Web server and client implementation using UDP

Developed a connectionless web server which accepts file request connections from web client using User Datagram Protocol (UDP).

# Running the code

## Creating and configuring virtual machines
	
	vagrant up
   
   This will boot both the server and client machines

## SSH into virtual machines
	
	vagrant ssh server
	vagrant ssh client
   
## Compile the code
	
	make

## Run the web server

	./Server port_number(any number between 1025 and 65535)

## Run the web client
	
	./Client server_IP/hostname server_portNo filename
	
# Screenshots

<img src="https://github.com/rachhshruti/file-transfer-udp/blob/master/images/file-transfer-udp-output.png" width="1000" height="600" align="center"/>
