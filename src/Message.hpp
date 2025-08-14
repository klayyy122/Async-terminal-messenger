#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>

class Message 
{
public:
    // Конструктор
    explicit Message(const std::string& msg) : msg_(msg) {
        if (msg.empty())
            throw std::invalid_argument("Message cannot be empty");
    }

    // Проверка на сообщение аутентификации (формат: "AUTH:login:password")
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

    // Получение данных аутентификации
    std::pair<std::string, std::string> get_auth_data() const;

    // Получение текста обычного сообщения
    std::string get_message_text() const ;

    // Получение исходного сообщения
    const std::string& get_raw_message() const {
        return msg_;
    }

private:
    std::string msg_;
};