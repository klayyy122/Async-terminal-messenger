//
// timer.cpp
// ~~~~~~~~~
//
// Copyright (c) 2003-2025 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <thread>
#include <chrono>

std::vector<char> vBuffer(1024);

void GrabSomeData(boost::asio::ip::tcp::socket& socket)
{
  socket.async_read_some(boost::asio::buffer(vBuffer.data(), vBuffer.size()), 
    [&](std::error_code ec, std::size_t lenght)
    {
      if (!ec)
      {
        std::cout << "\n\nRead " << lenght << " bytes\n\n";

        for (int i = 0; i < lenght; i++)
          std::cout << vBuffer[i];

        GrabSomeData(socket);
      }
    });
}

int main()
{
  boost::system::error_code ec;
  
  boost::asio::io_context context;

  auto idleWork = boost::asio::make_work_guard(context);
  
  std::thread thrContext = std::thread([&](){ context.run(); });
  
  boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::make_address("51.38.81.49", ec), 80);
  
  boost::asio::ip::tcp::socket socket(context);

  socket.connect(endpoint, ec);

  if (!ec)
    std::cout << "Connected!" << std::endl;
  else
    std::cout << "Failed to connect:\n" << ec.message() << std::endl;

  if (socket.is_open())
  {
    GrabSomeData(socket); 

    std::string sRequest = 
      "GET /index.html HTTP/1.1\r\n"
      "Host: example.com\r\n"
      "Connection: close\r\n\r\n";

    socket.write_some(boost::asio::buffer(sRequest.data(), sRequest.size()), ec);

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
  }

  system("pause");

  return 0;
}