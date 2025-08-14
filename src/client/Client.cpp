#include "Client.hpp"

void Client::read()
{
    boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::cout << "Received: " << read_buffer.substr(0, length);
                read_buffer.erase(0, length);
                read(); // Continue reading
            }
            else
            {
                std::cerr << "Read error: " << ec.message() << "\n";
                socket.close();
            }
        });
}

void Client::write()
{
    boost::asio::post([this]()
    {
        std::string write_buffer;
        std::getline(std::cin, write_buffer);
        
        write_buffer += '\n'; // Add delimiter
        
        boost::system::error_code ec;
        boost::asio::async_write(socket, boost::asio::buffer(write_buffer), 
            [this](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                    write();
                else
                {
                    std::cerr << "Write error: " << ec.message() << "\n";
                    socket.close();
                }
            });
    });
}
