#include <iostream>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>

int main(int argc, char** argv)
{
    if (argc != 3)
    {
        std::cerr << "Usage: ./client ip port" << std::endl;
        return 1;
    }

    std::string server_ip(argv[1]), server_port(argv[2]);

    Poco::Net::SocketAddress server_address(server_ip, std::atoi(server_port.c_str()));

    std::cout << "Starting the server..." << std::endl;

    Poco::Net::DatagramSocket udp_socket(server_address);

    while (true)
    {
        char buffer[1024];
        Poco::Net::SocketAddress client_address;
        auto size = udp_socket.receiveFrom(buffer, sizeof(buffer) - 1, client_address);
        buffer[size] = '\0';

        std::cout << "Received \"" << buffer << "\" from the client " << client_address.toString() << std::endl;

        std::cout << "Sending it back..." << std::endl;

        udp_socket.sendTo(buffer, size, client_address);
    }

    return 0;
}