#pragma once
#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>

class Message 
{
public:

    explicit Message() {}
    explicit Message(const std::string& text) 
    {Make_MSG_message(text);}
    Message(const std::string& login, const std::string& password) 
    {Make_AUTH_message(login, password);}


    void Make_AUTH_message(const std::string& login, const std::string& password)
    {
        if (login.empty() || password.empty())
            throw std::invalid_argument("Login and password cannot be empty");

        msg_ = "AUTH:" + login + ":" + password;
    }

    void Make_MSG_message(const std::string& text)
    {
        msg_ = "MSG:" + text;
    }
    
    void Make_MSG_message_with_username(const std::string& username, const std::string& text)
    {
        if (username.empty())
            throw std::invalid_argument("Username and text cannot be empty");
            
        msg_ = "MSG:[" + username + "]: " + text;
    }

    // Получение данных аутентификации
    // Получение данных аутентификации
    std::pair<std::string, std::string> get_auth_data() const 
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
    std::string get_message_text() const 
    {
        if (!is_regular_message())
            throw std::runtime_error("Not a regular message");

        std::string text = msg_.substr(4); // После "MSG:"
        
        // Удаляем возможный \n в конце
        if (!text.empty() && text.back() == '\n')
            text.pop_back();

        return text;
    }
    
    // Получение исходного сообщения
    const std::string& get_raw_message() const 
    {return msg_;}

    Message& operator=(const std::string& str) 
    {
        if (str.empty())
            throw std::invalid_argument("Message text cannot be empty");

        msg_ = str;
        return *this;
    }

    bool is_auth_message() const 
    {
        // Проверяем что сообщение начинается с "AUTH:" и содержит хотя бы один ':'
        return (msg_.size() >= 5) && 
               (msg_.substr(0, 5) == "AUTH:") && 
               (msg_.find(':', 5) != std::string::npos);
    }

    // Проверка на обычное сообщение (формат: "MSG:текст")
    bool is_regular_message() const 
    {
        // Проверяем что сообщение начинается с "MSG:"
        return (msg_.size() >= 4) && 
               (msg_.substr(0, 4) == "MSG:");
    }


private:

    std::string msg_;
};