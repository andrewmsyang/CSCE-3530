Compile: 

run 'make'

Execute:

First start the server on the server machine like below:
- ./dserver 6701 <port number>
- network address: <enter the IP address>
- subnet_part: <enter the number of bits>

Then start the client on the client machine like below:
-./dclient 6701 <port number>

NOTE: Multiple clients are allowed to run at the same time.

Test:

As is required in the assignment spec, any sent segment will be printed in the 
console. During the DHCP requesting process, there should be two segments 
output in the client's console and two segment output in the server's console.
The discover packet and request packet are expected from the client's console
and the offer packet and ack packet are expected from the server's console. 
Besides, multiple clients can collaborate with the same server, the server should
allocate IP addresses from the pool starting from *.*.*.1. If all the segments 
are well printed in both consoles, and IP addresses allocated to clients are 
increased one by one as expected, then I believe the simulation works well.
