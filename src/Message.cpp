#include "Message.hpp"

// Получение данных аутентификации
std::pair<std::string, std::string> Message::get_auth_data() const 
{
    if (!is_auth_message())
        throw std::runtime_error("Not an authentication message");

    size_t login_start = 5; // После "AUTH:"
    size_t login_end = msg_.find(':', login_start);
    
    if (login_end == std::string::npos)
        throw std::runtime_error("Invalid auth format");

    std::string login = msg_.substr(login_start, login_end - login_start);
    std::string password = msg_.substr(login_end + 1);
    
    // Удаляем возможный \n в конце
    if (!password.empty() && password.back() == '\n')
        password.pop_back();

    return {login, password};
}

// Получение текста обычного сообщения
std::string Message::get_message_text() const 
{
    if (!is_regular_message())
        throw std::runtime_error("Not a regular message");

    std::string text = msg_.substr(4); // После "MSG:"
    
    // Удаляем возможный \n в конце
    if (!text.empty() && text.back() == '\n')
        text.pop_back();

    return text;
}