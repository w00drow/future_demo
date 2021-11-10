#include <iostream>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        std::cerr << "Usage: ./client ip port number" << std::endl;
        return 1;
    }

    std::string server_ip(argv[1]), server_port(argv[2]), number(argv[3]);

    Poco::Net::SocketAddress server_address(server_ip, std::atoi(server_port.c_str()));

    Poco::Net::DatagramSocket udp_socket;
    udp_socket.connect(server_address);

    std::cout << "Sending number to the server..." << std::endl;

    udp_socket.sendBytes(number.c_str(), number.size());

    std::cout << "Waiting for an answer..." << std::endl;

    char buffer[1024];
    auto size = udp_socket.receiveBytes(buffer, sizeof(buffer) - 1);
    buffer[size] = '\0';

    std::cout << "Received result from the server: " << buffer << std::endl;

    return 0;
}
