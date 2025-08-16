#include "ChatSession.hpp"

void ChatSession::deliver(const std::string& message)
{
    bool write_in_progress = !write_msgs_.empty();
        write_msgs_.push_back(message);
        if (!write_in_progress)
            write_message();
}

//чтение сообщения от пользователя и отправка его всем остальным
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

//отправка сообщения пользователю
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

//чтение логина отправляемого клиентом
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

                send_confirm_login();
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

//отправка подтверждения того что логин пришёл
void ChatSession::send_confirm_login()
{
    auto self(shared_from_this());
    std::string msg = "All good\n";

    boost::asio::async_write(socket_, boost::asio::dynamic_buffer(msg),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
                read_password();
            else
            {
                // Удаляем сессию при ошибке
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                    sessions_.erase(it);
            }
        });
}

//отправка подтверждения что пароль пришёл (пока не используется)
void ChatSession::send_confirm_password()
{
    auto self(shared_from_this());
    std::string msg = "All good\n";

    boost::asio::async_write(socket_, boost::asio::dynamic_buffer(msg),
        [this, self](boost::system::error_code ec, std::size_t /*length*/)
        {
            if (!ec)
                read_message();
            else
            {
                // Удаляем сессию при ошибке
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                    sessions_.erase(it);
            }
        });
}

//чтение пароля отправленного пользователем
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

                //send_confirm_password();
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