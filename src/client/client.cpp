#include "client.hpp"

void Client::write()
{
    boost::asio::post([this]()
    {
        std::string msg = '[' + User_login + "]:";
        std::string write_buffer;
        std::getline(std::cin, write_buffer);
        
        msg += write_buffer + '\n';
        
        boost::system::error_code ec;
        boost::asio::async_write(socket, boost::asio::buffer(msg),
            [this](boost::system::error_code ec, std::size_t)
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

void Client::input_login(){
    do  
    {
        User_login.erase(User_login.begin(), User_login.end());
        std::cout << "Enter login: ";
        std::getline(std::cin, User_login);
    } while (User_login.size() == 0 ||  User_login.size() > 32);
}

void Client::input_password(){
 do
    {
        User_password.clear();
        std::cout << "Enter password: ";
        std::getline(std::cin, User_password);
    } while (User_password.size() == 0 ||  User_password.size() > 32);
}

void Client::send_login_and_password()
{
    // i dont remember what it does)
    send_login();
}

void Client::send_login()
{
    boost::asio::async_write(socket, boost::asio::buffer(User_login + '\n'),
        [this](boost::system::error_code ec, std::size_t )
        {
            if (!ec)
            {
                read(); // wait the data from server
            }
            else
            {
                std::cerr << "Send login error: " << ec.message() << "\n";
                socket.close();
            }
        });
}
void Client::read()
{
    boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
        [this](boost::system::error_code ec, std::size_t length)
        {  
            if (!ec)
            {
                std::string response = read_buffer.substr(0, length);
                read_buffer.erase(0, length);
                
                //std::cout << "Received from server: " << response; for debug
                
                // Обрабатываем ответ сервера
                if (response == "LOGIN_ALREADY_IN_USE\n") 
                {
                    std::cout << "This login is already in use. Please choose another one.\n";
                    input_login();
                    send_login();
                }
                else if (response == "LOGIN_EXISTING\n") 
                {
                    std::cout << "Enter password for existing account: ";
                    input_password();
                    send_password();
                }
                else if (response == "LOGIN_NEW\n") 
                {
                    std::cout << "Create password for new account: ";
                    input_password();
                    send_password();
                }
                else if (response == "WRONG_PASSWORD\n") 
                {
                    std::cout << "Wrong password. Try again: ";
                    input_password();
                    send_password();
                }
                else if (response == "Unknown command.\n"){
                    std::cout << "Unknown command. Available commands:\n/room_list\n/join_room <name>\n/create_room <name>\n/leave_room\n";
                }
                else if (response == "REGISTRATION_SUCCESS\n" || response == "LOGIN_SUCCESS\n") 
{
    std::cout << "Authentication successful! You can start chatting now.\n";
    start_chat(); 
}
                else 
                {
                    // if no one response is not acceped
                    std::cout << response;
                    read(); 
                }
            }
            else
            {   
                std::cerr << "Server is not working, please try again later!\n";
                socket.close();
            }
        });
}
void Client::start_chat()
{
    //read the data from server and send data to the server
    read();
    
    write();
}

// sending password
void Client::send_password()
{
    boost::asio::async_write(socket, boost::asio::buffer(User_password + '\n'), 
        [this](boost::system::error_code ec, std::size_t )
        {
            if (!ec)
            {
                
                std::cout << "Password sent, waiting for response...\n";
                read();
            }
            else
            {
                std::cerr << "Write error: " << ec.message() << "\n";
                socket.close();
            }
        });
}

