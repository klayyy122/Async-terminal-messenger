#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>
#include "../Message.hpp"

using boost::asio::ip::tcp;

class Client
{
private:
    tcp::socket socket;
    std::string read_buffer;
    std::string User_login;
    std::string User_password;

    void read();
    void write();
    void input_login_and_password();

public:
    Client(boost::asio::io_context& io_context, tcp::socket&& sock)
        : socket(std::move(sock))
    {
        input_login_and_password();

        // Запускаем чтение в основном потоке io_context
        read();
        // Запускаем запись в отдельном потоке
        write();
    }

    ~Client()
    {socket.close();}
};