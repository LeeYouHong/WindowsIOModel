# WindowsIOModel
Windows常用的4种套接字 I/O 模型：
1. Blocking
2. Select
3. WSAEventSelect
4. Completion port

## The blocking Model
*blocking* 模型用于阻塞套接字，使用这种模型，套接字的recv操作会阻塞，所以一般都会为每个连接创建一个独立的线程。它的优势就是，它看起来非常简单。对于一些很简单的程序，这个模型就非常有用了。但是该模型的缺点就是连接数越多线程数也越来越多，非常占用系统资源。


## The select Model
之所以叫*select*模型，是因为该模型主要使用了*select*函数，*select*函数可以用于探查套接字是否有数据可读，是否可以往套接字里写数据。

## The WSAEventSelect Model
Winsock 提供了一个有用的异步I/O模型，允许程序在一个套接字上接收基于Windows消息的网络事件通知。这是通过创建一个套接字后调用WSAAsyncSelect函数来实现的。该模型提供了对套接字事件的异步通知，但是没有提供异步传输数据。

## The Completion port Model
当你的程序需要负载很多连接时，比如游戏服务器、Webserver等程序。这时候使用*完成端口*模型是最高效的（在Linux平台使用epoll模型最高效）；在你获得它的高效的同时，当然也要付出一定的代价，那就是，该模型对于新手来说非常难理解；首先需要创建一个完成端口对象，然后把相关的套接字与该对象绑定，然后投递相应的套接字事件，然后监听该对象，当相应的套接字事件完成时，便执行对应的处理逻辑；该模型实现了异步读取、发送数据，是一个异步非阻塞的模型。
