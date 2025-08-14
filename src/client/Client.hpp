#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>

using boost::asio::ip::tcp;

class Client
{
private:
    tcp::socket socket;
    std::string read_buffer;

    void read();
    void write();

public:
    Client(boost::asio::io_context& io_context, tcp::socket&& sock)
        : socket(std::move(sock))
    {
        // Запускаем чтение в основном потоке io_context
        read();
        // Запускаем запись в отдельном потоке
        write();
    }

    ~Client()
    {socket.close();}
};