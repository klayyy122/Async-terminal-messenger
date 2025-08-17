#include "ChatSession.hpp"
#include "ChatRoom.hpp"
// Глобальный набор для хранения занятых логинов
std::unordered_set<std::string> logins_;
std::unordered_map<std::string, std::shared_ptr<ChatRoom>> Rooms_list;

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
                std::string msg = read_buffer_.substr(0, length);
                read_buffer_.erase(0, length);

                std::cout << msg << std::endl;
                if (msg[User_login.size() + 3] == '/') 
                    handle_command(msg.substr(User_login.size() + 3, msg.size() - 1));
                else 
                {
                    if (auto room = current_room_.lock())
                        room->Broadcast(getLogin() + ": " + msg, shared_from_this());
                    else
                        deliver("You must join a room first (/join_room <name>)\n");
                }
                read_message();
            } else {
                // При отключении выходим из комнаты
                if (auto room = current_room_.lock()) {
                    room->Disconnect(shared_from_this());
                }
                
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end()) {
                    sessions_.erase(it);
                    logins_.erase(User_login);
                }
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
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end())
                {
                    sessions_.erase(it);
                    
                    logins_.erase(User_login); // Освобождаем логин при отключении
                    
                }
            }
        });
}

void ChatSession::handle_command(const std::string& command) 
{
    std::cout << "Debug - Received command: '" << command << "'" << std::endl;

    // Удаляем возможные пробелы и переносы строк
    std::string trimmed_cmd = command;
    trimmed_cmd.erase(trimmed_cmd.find_last_not_of(" \n\r\t") + 1);

    if (trimmed_cmd == "/room_list") {
        list_rooms();
    }
    else if (trimmed_cmd.find("/join_room ") == 0) {
        join_room(trimmed_cmd.substr(10)); // Берем все после "/join_room "
    }
    else if (trimmed_cmd.find("/create_room ") == 0) {
        create_room(trimmed_cmd.substr(12)); // Берем все после "/create_room "
    }
    else if (trimmed_cmd == "/leave_room") {
        leave_room();
    }
    else {
        deliver("Unknown command. Available commands:\n"
               "/room_list\n"
               "/join_room <name>\n"
               "/create_room <name>\n"
               "/leave_room\n");
    }
}

void ChatSession::create_room(const std::string& room_name) {
    // Удаляем возможные пробелы и переносы строки
    std::string trimmed_name = room_name;
    trimmed_name.erase(trimmed_name.find_last_not_of(" \n\r\t") + 1);

    if (trimmed_name.empty()) {
        deliver("Error: Room name cannot be empty\n");
        return;
    }

    // Проверяем, существует ли уже комната
    if (Rooms_list.find(trimmed_name) != Rooms_list.end()) {
        deliver("Error: Room '" + trimmed_name + "' already exists\n");
        return;
    }

    // Создаем новую комнату
    Rooms_list[trimmed_name] = std::make_shared<ChatRoom>(trimmed_name);
    deliver("Room '" + trimmed_name + "' created successfully\n");

    // Автоматически присоединяемся к созданной комнате
    join_room(trimmed_name);
}

void ChatSession::join_room(const std::string& room_name) {
    std::string trimmed_name = room_name;
    trimmed_name.erase(trimmed_name.find_last_not_of(" \n\r\t") + 1);

    if (trimmed_name.empty()) {
        deliver("Error: Room name cannot be empty\n");
        return;
    }

    if (auto room = current_room_.lock()) {
        deliver("You are already in room: " + room->GetName() + "\n");
        return;
    }

    auto it = Rooms_list.find(trimmed_name);
    if (it == Rooms_list.end()) {
        deliver("Error: Room '" + trimmed_name + "' does not exist\n");
        return;
    }

    current_room_ = it->second;
    it->second->Connect(shared_from_this());
    deliver("Joined room: " + trimmed_name + "\n");
}

void ChatSession::leave_room() {
    if (auto room = current_room_.lock()) {
        room->Disconnect(shared_from_this());
        current_room_.reset();
        deliver("You left the room\n");
    } else {
        deliver("You are not in any room\n");
    }
}

void ChatSession::list_rooms() {
    std::string response = "Available rooms:\n";
    for (const auto& room_pair : Rooms_list) {
        const std::string& name = room_pair.first;
        const std::shared_ptr<ChatRoom>& room = room_pair.second;
        response += "- " + name + " (" + std::to_string(room->GetUserCount()) + " users)\n";
    }
    deliver(response);
}

void ChatSession::authorization()
{
    read_login();
}

void ChatSession::read_login()
{
    auto self(shared_from_this());

    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length)
        {
            if (!ec) 
            {
                std::string received_login = read_buffer_.substr(0, length-1); // Убираем \n
                read_buffer_.clear();
                
                if (logins_.find(received_login) == logins_.end())
                {
                    // Логин свободен
                    User_login = received_login;
                    logins_.insert(User_login);
                       
                    // Отправляем подтверждение клиенту
                    send_confirm_login();
                }
                else
                {
                    // Логин занят
                    send_login_taken();
                }
            } 
            else 
            {
                socket_.close();
            }            
        });
}

void ChatSession::send_confirm_login()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, 
        boost::asio::buffer("LOGIN_OK\n"),
        [this, self](boost::system::error_code error, std::size_t){
            if (!error)
                read_password();
            else 
            {
                logins_.erase(User_login);
                socket_.close();
            }
        });
}

void ChatSession::send_login_taken()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer("LOGIN_TAKEN\n"),
        [this, self](boost::system::error_code error, std::size_t)
        {
            if (!error)
                // Даем клиенту еще попытку
                read_login();
            else 
                socket_.close();
        });
}

void ChatSession::send_confirm_password()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer("Welcome, " + self->getLogin() + "!\n"),
        [this, self](boost::system::error_code error, std::size_t)
        {
            if (!error)
                read_message();
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
                if (User_login.empty()) 
                {
                    // Если вдруг нет логина (не должно происходить)
                    socket_.close();
                    return;
                }
                
                User_password = read_buffer_.substr(0, length-1);
                read_buffer_.clear();
                std::cout << User_login << " connected successfully\n";
                
                // Отправляем подтверждение авторизации
                send_confirm_password();
            }
            else
            {
                logins_.erase(User_login);
                
                socket_.close();
            }
        });
}