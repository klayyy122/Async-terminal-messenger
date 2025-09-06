#include "chat_session.hpp"
#include "room.hpp"

std::unordered_set<std::string> ChatSession::logins_;
std::unordered_map<std::string, std::shared_ptr<ChatRoom>> ChatSession::Rooms_list;
std::unordered_map<std::string, std::string> ChatSession::registered_users_;
std::unordered_set<std::string> ChatSession::active_users_;


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

                ProcessingMessage(msg);
                read_message();
            } 
            else 
            {
                
                if (auto room = current_room_.lock())
                    room->Disconnect(shared_from_this());
                
                auto it = std::find(sessions_.begin(), sessions_.end(), self);
                if (it != sessions_.end()) 
                {
                    sessions_.erase(it);
                    logins_.erase(User_login);
                }
            }
        });
}

void ChatSession::ProcessingMessage(const std::string& msg)
{
    if (msg[User_login.size() + 3] == '/') 
        handle_command(msg.substr(User_login.size() + 3, msg.size() - 1));
    else 
    {
        if (auto room = current_room_.lock())
            room->Broadcast(msg, shared_from_this());
        else
            deliver("You must join a room first (/join_room <name>)\n");
    }
}

void ChatSession::write_message()
{
    auto self(shared_from_this());

    boost::asio::async_write(socket_, boost::asio::buffer(write_msgs_.front()),
        [this, self](boost::system::error_code ec, std::size_t )
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
                    logins_.erase(User_login); 
                }
            }
        });
}

void ChatSession::handle_command(const std::string& command) 
{
    std::cout << "Debug - Received command: " << command;

    
    std::string trimmed_cmd = command;
    trimmed_cmd.erase(trimmed_cmd.find_last_not_of(" \n\r\t") + 1);

    if (trimmed_cmd == "/room_list")    list_rooms();
    else if (trimmed_cmd.find("/join_room ") == 0)  join_room(trimmed_cmd.substr(11)); 
    else if (trimmed_cmd.find("/create_room ") == 0)    create_room(trimmed_cmd.substr(13)); 
    else if (trimmed_cmd == "/leave_room")  leave_room();
    else deliver("Unknown command.\n");
}

//creating a room
void ChatSession::create_room(const std::string& room_name) 
{
    
    std::string name = room_name;
    name.erase(name.find_last_not_of(" \n\r\t") + 1);

    if (name.empty()) 
    {
        deliver("Error: Room name cannot be empty\n");
        return;
    }

    //check if room is already exists
    if (Rooms_list.find(name) != Rooms_list.end()) 
    {
        deliver("Error: Room '" + name + "' already exists\n");
        return;
    }

    // creating a new room
    Rooms_list[name] = std::make_shared<ChatRoom>(name);
    deliver("Room '" + name + "' created successfully\n");

    //connecting to new room
    join_room(name);
}

void ChatSession::join_room(const std::string& room_name) 
{
    std::string name = room_name;
    name.erase(name.find_last_not_of(" \n\r\t") + 1);

    if (name.empty()) 
    {
        deliver("Error: Room name cannot be empty\n");
        return;
    }

    if (auto room = current_room_.lock()) 
    {
        deliver("You are already in room: " + room->GetName() + "\n");
        return;
    }

    auto it = Rooms_list.find(name);
    if (it == Rooms_list.end()) 
    {
        deliver("Error: Room '" + name + "' does not exist\n");
        return;
    }

    current_room_ = it->second;
    it->second->Connect(shared_from_this());
    deliver("Joined room: " + name + "\n");
}

void ChatSession::leave_room() 
{
    if (auto room = current_room_.lock()) 
    {
        room->Disconnect(shared_from_this());
        current_room_.reset();
        deliver("You left the room\n");
    } 
    else 
    {
        deliver("You are not in any room\n");
    }
}

void ChatSession::list_rooms()
{
    std::string response = "Available rooms:\n";

    for (const auto& room_pair : Rooms_list) 
    {
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

void ChatSession::read_login() {
    auto self(shared_from_this());

    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) 
            {
                std::string received_login = read_buffer_.substr(0, length-1); 
                read_buffer_.clear();
                
                // checking the using of login
                if (active_users_.find(received_login) != active_users_.end()) {
                    deliver("LOGIN_ALREADY_IN_USE\n");
                    read_login(); // one more chance
                    return;
                }
                
                // check registration
                auto it = registered_users_.find(received_login);
                if (it != registered_users_.end()) {
                    //waid password
                    User_login = received_login;
                    deliver("LOGIN_EXISTING\n");
                    read_password();
                } else {
                    // wait password too( but new)
                    User_login = received_login;
                    deliver("LOGIN_NEW\n");
                    read_new_password();
                }
            } else {
                socket_.close();
            }
        });
}



void ChatSession::send_confirm_password()
{
    auto self(shared_from_this());

    // welcome message
    std::string welcome_msg = "Welcome, " + User_login + "!\n";
    
    boost::asio::async_write(socket_, boost::asio::buffer(welcome_msg),
        [this, self](boost::system::error_code error, std::size_t)
        {
            if (!error) {
                //waiting client's messages
                read_message();
            } else {
                socket_.close();
            }
        });
}

void ChatSession::read_new_password() {
    auto self(shared_from_this());

    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string new_password = read_buffer_.substr(0, length-1);
                read_buffer_.clear();
                
                // registration of new client
                registered_users_[User_login] = new_password;
                active_users_.insert(User_login);
                logins_.insert(User_login);
                
                deliver("REGISTRATION_SUCCESS\n");
                send_confirm_password();
                std::cout << getLogin() + " connected to server\n";
            } else {
                socket_.close();
            }
        });
}

void ChatSession::read_password() {
    auto self(shared_from_this());

    boost::asio::async_read_until(socket_, boost::asio::dynamic_buffer(read_buffer_), '\n',
        [this, self](boost::system::error_code ec, std::size_t length) {
            if (!ec) {
                std::string password = read_buffer_.substr(0, length-1);
                read_buffer_.clear();
                
               // checking password
                if (registered_users_[User_login] == password) {
                    // password is correct
                    active_users_.insert(User_login);
                    logins_.insert(User_login);
                    deliver("LOGIN_SUCCESS\n");
                    send_confirm_password();
                } else {
                    // password isn't  correct
                    deliver("WRONG_PASSWORD\n");
                    read_password(); 
                }
            } else {
                socket_.close();
            }
        });
}

ChatSession::~ChatSession() {
    std::cout << User_login + " disconnected\n";
    if (!User_login.empty()) {
        active_users_.erase(User_login);
        logins_.erase(User_login);
    }
    socket_.close();
}