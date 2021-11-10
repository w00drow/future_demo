#include <iostream>
#include <cmath>
#include <chrono>
#include <queue>
#include <mutex>
#include <future>
#include <list>
#include <Poco/Net/DatagramSocket.h>
#include <Poco/Net/SocketAddress.h>

using namespace std::literals::chrono_literals;

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

std::mutex mutex;
std::condition_variable condition_variable;
std::queue<std::pair<std::promise<int>, std::function<unsigned long()>>> tasks;

void processor()
{
    while (true)
    {
        std::unique_lock<std::mutex> lock(mutex);
        condition_variable.wait(lock, []{return !tasks.empty();});
        if (!tasks.empty())
        {
            auto task = std::move(tasks.front());
            tasks.pop();

            lock.unlock();

            auto value = task.second();
            task.first.set_value(value);
        }
    }
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

    std::thread processor_tread(processor);

    std::list<std::pair<std::future<int>, Poco::Net::SocketAddress>> results;
    while (true)
    {
        if (udp_socket.poll(Poco::Timespan(1, 0), Poco::Net::Socket::SelectMode::SELECT_READ))
        {
            char buffer[1024];
            Poco::Net::SocketAddress client_address;
            auto size = udp_socket.receiveFrom(buffer, sizeof(buffer) - 1, client_address);
            buffer[size] = '\0';

            std::cout << "Received \"" << buffer << "\" from the client " << client_address.toString() << std::endl;
            std::cout << "Schedule a task for processing ... " << std::endl;

            auto number = std::atol(buffer);

            std::promise<int> promise;
            results.emplace_back(promise.get_future(), client_address);

            std::lock_guard<std::mutex> lock(mutex);
            tasks.emplace(std::move(promise), [number]{return getSumOfAllPrimesBelow(number);});
            condition_variable.notify_one();
        }
        else
        {
            auto it = results.begin();
            while (it != results.end())
            {
                auto& future = it->first;
                if (future.wait_for(1ms) == std::future_status::ready)
                {
                    auto sum_string = std::to_string(future.get());

                    std::cout << "Sending result \"" << sum_string << "\" back to " << it->second.toString() << std::endl;

                    udp_socket.sendTo(sum_string.c_str(), sum_string.size(), it->second);

                    results.erase(it++);
                }
                else
                {
                    ++it;
                }
            }
        }
    }

    processor_tread.join();

    return 0;
}