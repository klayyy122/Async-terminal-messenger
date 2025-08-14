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
    
    // Получение данных аутентификации
    std::pair<std::string, std::string> get_auth_data() const;
    
    // Получение текста обычного сообщения
    std::string get_message_text() const ;
    
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