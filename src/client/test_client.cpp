#include <iostream>
#include <boost/asio.hpp>
#include <thread>
#include <atomic>

using boost::asio::ip::tcp;

class Client
{
private:
    tcp::socket socket;
    std::string read_buffer;
    std::atomic<bool> running{true};

    void async_read()
    {
        boost::asio::async_read_until(socket, boost::asio::dynamic_buffer(read_buffer), '\n',
            [this](boost::system::error_code ec, std::size_t length)
            {
                if (!ec)
                {
                    std::cout << "Received: " << read_buffer.substr(0, length);
                    read_buffer.erase(0, length);
                    async_read(); // Continue reading
                }
                else
                {
                    std::cerr << "Read error: " << ec.message() << "\n";
                    running = false;
                    socket.close();
                }
            });
    }

    void async_write()
    {
        std::string write_buffer;
        while (running)
        {
            std::getline(std::cin, write_buffer);
            if (!running) break;
            
            write_buffer += '\n'; // Add delimiter
            
            boost::system::error_code ec;
            boost::asio::write(socket, boost::asio::buffer(write_buffer), ec);
            
            if (ec)
            {
                std::cerr << "Write error: " << ec.message() << "\n";
                running = false;
                socket.close();
                break;
            }
        }
    }

public:
    Client(boost::asio::io_context& io_context, tcp::socket&& sock)
        : socket(std::move(sock))
    {
        // Запускаем чтение в основном потоке io_context
        async_read();
        
        // Запускаем запись в отдельном потоке
        std::thread([this]() { async_write(); }).detach();
    }

    ~Client()
    {
        running = false;
        socket.close();
    }
};

int main() 
{
    try 
    {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        socket.connect(tcp::endpoint(boost::asio::ip::make_address("127.0.0.1"), 1111));

        Client c(io_context, std::move(socket));
        
        io_context.run(); // Запускаем обработку асинхронных операций
        
    } 
    catch (std::exception& e) 
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}