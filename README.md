# Async Webserver in C

An asynchronous web server implementation written in C. The goal is to make this server implementation blazingly fast by using non-blocking sockets and I/O multiplexing provived by epoll syscall in linux. Currently, the server implements GET requests, capable of sending html, css, javscripts, images. More functionality is to be added at a later date. 

The asynchronous server runs in a event loop, polling for sockets that are ready to be read, and instantly responding the GET requests. Non-blocking sockets and epoll combined gives us another way to handle multiple client connections without creating more threads, thus the code is single-threaded and doesn't have any overhead of multiple threads. 

### Requirements 
  There aren't a lot of requirement. Just ensure that cmake is installed and you are using linux. 
  
### Installation 
  1. Clone this repository 
  2. Create a build folder. This is where you'll run the cmake and make commands.
    ```mkdir build```
  3. move into the build folder and execute cmake.
    ``` cd build && cmake .. ```
  4. run make 
    ``` make ```
  5. Copy the main executable to the folder which has your html, css, js files
     and run the command by giving the port as the argument.
    ``` ./main 9999 ```

## To-do 
  1. Add support for POST request
  2. Validation of HTTP request (it currently doesn't check if your request is in a valid format)
  3. Better support for sending large files
  4. Increase memory efficiency ( possibly by using circular buffers )
 
