# httpserver
In this code, I did my best at creating an httpserver per the assignment spec. This code is written by me however i did take inspiration for some functionality from other places. In the spirit of transparency, I have linked those source(s) here:

stackoverflow.com/questions/41286260/parse-http-request-line-in-c

# High-level design
This code essential checks for a TCP connection and returns an error if a connection isn't found and exits. Also does the same if a port isn't given. Once we establish a connection, we read the request from the client until we hit a \r\n\r\n. From here, I parse the request to get the request_type and the URI as well as the content-length header. Unfortunetely I didn't have time to properly implement any functionality involving headers. If the request line doesn't follow HTTP 1.1 protocol, client gets a 400 error.

Once we have these key pieces of information, I check if it's a GET or HEAD or PUT request. If it's a PUT, I make sure it has a content-length header and the message isn't empty. If they are, I send a 403 error to the client. I then check if the file doesn't exist and if it doesn't, i'll create one and note down that I need to send a 201 status. If the file exists, I check if it's a regular file and if so, start reading from the cleint again (the message) and write it to the file. I then send a 200 OK response to the client. if the file can't be opened, it also gets a 403 error.

For GET requests, I make sure the file isn't a directory and send a 403 error if it is. If the file exists, I open it and send a 403 if i cant. Once opened, I parse it's contents and put it into a buffer which i send to the client with a 200 status.

For HEAD, I do the same as GET except i dont send a message.

Some interesting design decisions I made were to use regex to check if the request line follows HTTP 1.1 protocol as well as used regex to verify the content length exists for PUT. Once these things are verified, i was able to use string pointers and subtracting them to extract the necessary details (information thanks to stackoverflow). I used a lot of arrays as buffers and because it's a program that continuously loops, i need to reset the buffers with memset() after every iteration of the loop.
