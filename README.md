# EE542_Lab4

# Setting MTU Size
To set the MTU size in the receiver.cpp and sender.cpp one needs to change the payload/maxbufferwindow constancs for each. There is a definition at the top of the file for 1500 mtu and 9000 mtu. Comment/uncomment the desired. 

# Compiling Files
to compile the c++ files run the following commands
	g++ -o sender sender.cpp -pthread
	g++ -o receiver receiver.cpp -pthread

#Executing Bash Script

to execute the bash script run the following commands

	Run the File transfer sender (FTPS) on the sender side where the file you want to send is
		./FTPS.sh -f <filename> -I <ip address of receiver>
	Run the File Transfer Receiver (FTPR) on the receiver side where you want the file to download. (If no filename is specified the file will default to data.bin)
	./FTPR.sh -f <filename>
