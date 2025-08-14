#pragma once
#include <iostream>
#include <vector>
#include <memory>
#include <deque>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class ChatSession : public std::enable_shared_from_this<ChatSession>
{
public:
    ChatSession(tcp::socket socket, std::vector<std::shared_ptr<ChatSession>>& sessions)
        : socket_(std::move(socket)), sessions_(sessions){}

    void start() {read_message();}

    void deliver(const std::string& message);

private:
    void read_message();
    void write_message();   

    tcp::socket socket_;
    std::string read_buffer_;
    std::deque<std::string> write_msgs_;
    std::vector<std::shared_ptr<ChatSession>>& sessions_;

    std::string User_login;
    std::string User_password;
};