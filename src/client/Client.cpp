#include "Client.hpp"

void Client::read()
{
    boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::cout << "Received: " << read_buffer.substr(4, length);
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
        Message msg;
        std::string write_buffer;
        std::getline(std::cin, write_buffer);
        
        write_buffer += '\n'; // Add delimiter

        msg.Make_MSG_message(write_buffer);
        
        boost::system::error_code ec;
        boost::asio::async_write(socket, boost::asio::buffer(msg.get_raw_message()),
            [this](boost::system::error_code ec, std::size_t /*length*/)
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

void Client::input_login_and_password()
{
    do
    {
        User_login.erase(User_login.begin(), User_login.end());
        std::cout << "Введите логин: ";
        std::getline(std::cin, User_login);
    } while (User_login.size() == 0 ||  User_login.size() > 32);
    
    
    do
    {
        User_password.erase(User_password.begin(), User_password.end());
        std::cout << "Введите пароль: ";
        std::getline(std::cin, User_password);
    } while (User_password.size() == 0 ||  User_password.size() > 32);
}