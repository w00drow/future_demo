#include <iostream>
#include <cmath>
#include <chrono>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>

unsigned long getSumOfAllPrimesBelow(unsigned long number)
{
    auto isPrime = [](unsigned long number)
    {
        auto square = std::sqrt(number);
        for (unsigned long i = 2; i <= square; ++i)
        {
            if (number % i == 0)
            {
                return false;
            }
        }
        return true;
    };
    unsigned long sum = 0;
    for (unsigned long i = 1; i <= number; ++i)
    {
        if(isPrime(i))
        {
            sum += i;
        }
    }
    return sum;
}

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
        if (udp_socket.poll(Poco::Timespan(1, 0), Poco::Net::Socket::SelectMode::SELECT_READ))
        {
            char buffer[1024];
            Poco::Net::SocketAddress client_address;
            auto size = udp_socket.receiveFrom(buffer, sizeof(buffer) - 1, client_address);
            buffer[size] = '\0';

            std::cout << "Received \"" << buffer << "\" from the client " << client_address.toString() << std::endl;

            std::cout << "Calculating the result ... " << std::endl;

            auto sum = getSumOfAllPrimesBelow(std::atol(buffer));

            std::cout << "Sending result \"" << sum << "\" back..." << std::endl;

            auto sum_string = std::to_string(sum);
            udp_socket.sendTo(sum_string.c_str(), sum_string.size(), client_address);
        }
        else
        {
            std::cout << "Doing service actions..." << std::endl;
        }
    }

    return 0;
}