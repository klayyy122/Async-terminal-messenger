#include <iostream>
#include <vector>
#include <memory>
#include <deque>
#include <boost/asio.hpp>

#include "ChatServer.hpp"

using boost::asio::ip::tcp;

int main(int argc, char* argv[])
{
    try
    {
        if (argc != 2)
        {
            std::cerr << "Usage: chat_server <port>\n";
            return 1;
        }

        boost::asio::io_context io_context;
        ChatServer server(io_context, std::atoi(argv[1]));

        io_context.run();

        std::thread t([&io_context](){ io_context.run(); });

        std::cout << "Server running on port " << argv[1] << std::endl;
        std::cout << "Press Enter to exit..." << std::endl;
        std::cin.get();

        io_context.stop();
        t.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}