#include "ChatSession.hpp"

void ChatSession::deliver(const std::string& message)
{
    bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(message);
        if (!write_in_progress)
            write_message();
}

void ChatSession::read_message()
{
    auto self(shared_from_this());
    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                // Извлекаем сообщение (с удалением разделителя)
                std::string msg = read_buffer_.substr(0, length);
                read_buffer_.erase(0, length);

                // Рассылаем всем клиентам
                for (auto& session : sessions_)
                    if (session.get() != this)
                        session->deliver(msg);

                // Читаем следующее сообщение
                read_message();
            }
            else
            {
                // Удаляем сессию при ошибке
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                    sessions_.erase(it);    
            }
        });
}

void ChatSession::write_message()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front()),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
            {
                write_msgs_.pop_front();
                if (!write_msgs_.empty())
                    write_message();
            }
            else
            {
                // Удаляем сессию при ошибке
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                    sessions_.erase(it);
            }
        });
}

void ChatSession::authorization()
{
    read_login();
}

void ChatSession::read_login()
{
    auto self(shared_from_this());

    // read login
    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec) 
            {
                User_login = read_buffer_.substr(0, length-1);
                read_buffer_.clear();

                std::cout << User_login << std::endl;

                read_password();
            } 
            else 
            {
                // Удаляем сессию при ошибке
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                    sessions_.erase(it);
            }            
        });
}

void ChatSession::read_password()
{
    auto self(shared_from_this());

    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec)
            {
                User_password = read_buffer_.substr(0, length);
                read_buffer_.clear();
                std::cout << "User " << User_login << " is here with password " << User_password << std::endl;

                read_message();
            }
            else
            {
                // Удаляем сессию при ошибке
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                    sessions_.erase(it);
            }
        });
}