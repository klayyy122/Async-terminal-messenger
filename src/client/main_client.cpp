#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>

#include "client.hpp"



int main() 
{
    try 
    {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 1111));

        Client c(io_context, std::move(socket));
        
        io_context.run();
    } 
    catch (std::exception& e) 
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}