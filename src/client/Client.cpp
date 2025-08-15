#include "Client.hpp"

void Client::read()
{
    boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
        [this](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                std::cout << read_buffer.substr(0, length);
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
        std::string msg = '[' + User_login + "]:";
        std::string write_buffer;
        std::getline(std::cin, write_buffer);
        
        msg += write_buffer + '\n';
        // write_buffer += '\n'; // Add delimiter
        
        boost::system::error_code ec;
        boost::asio::async_write(socket, boost::asio::buffer(msg),
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

void Client::send_login_and_password()
{
    send_login();
}

void Client::send_login()
{
    boost::asio::async_write(socket, boost::asio::buffer(User_login + '\n'),
        [this](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
                send_password();
            else
            {
                ;
            }
        });
}

void Client::send_password()
{
    //костыль, чтобы дать время серверу обработать логин и только потом отправить серверу пароль
    for (int i = 0; i < 1000000; i++);
    boost::asio::async_write(socket, boost::asio::buffer(User_password + '\n'), 
        [this](boost::system::error_code /*ec*/, std::size_t /*length*/)
        {
            std::cout << "Auth success" << std::endl;
            // Запускаем чтение в основном потоке io_context
            read();
            // Запускаем запись в отдельном потоке
            write();
        });
}