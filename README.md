# Async Webserver in C

An asynchronous web server implementation written in C. The goal is to make this server implementation blazingly fast by using non-blocking sockets and I/O multiplexing provived by epoll syscall in linux. Currently, the server implements GET requests, capable of sending html, css, javscripts, images. More functionality is to be added at a later date. 

The asynchronous server runs in a event loop, polling for sockets that are ready to be read, and instantly responding the GET requests. Non-blocking sockets and epoll combined gives us another way to handle multiple client connections without creating more threads, thus the code is single-threaded and doesn't have any overhead of multiple threads. 


## To-do 
  1. Add support for POST request
  2. Better support for sending large files
  3. Increase memory efficiency ( possibly by using circular buffers )
 
