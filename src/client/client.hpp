#pragma once

#include "./../../include/some_libs.hpp"

class Client
{
    boost::asio::io_context io;
    boost::system::error_code ec;
    boost::asio::ip::tcp::socket socket;

public:
    Client()
    {
        socket
    }

};