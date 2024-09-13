#include <iostream>
#include <sstream>
#include <vector>
#include <thread>
#include <atomic>
#include <cstring>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/epoll.h>

// Порт, на котором будет работать сервер
constexpr int PORT = 8080;
// Размер буфера для чтения данных
constexpr size_t BUFFER_SIZE = 1024;
// Максимальное количество одновременно обрабатываемых событий
constexpr int MAX_EVENTS = 10000;

// Функция для обработки клиентского соединения
void handle_client(int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);
    if (bytes_read > 0) {
        buffer[bytes_read] = '\0';

        // Простая проверка на GET запрос
        if (strncmp(buffer, "GET /", 5) == 0) {
            std::ostringstream response;
            response << "HTTP/1.1 200 OK\r\n"
                     << "Content-Type: text/plain\r\n"
                     << "Content-Length: 13\r\n"
                     << "\r\n"
                     << "Hello, World!";
            std::string response_str = response.str();
            write(client_socket, response_str.c_str(), response_str.size());
        }
    }
    close(client_socket); // Закрываем соединение с клиентом
}

// Основная функция сервера, где создается и настраивается сервер
void server() {
    int server_fd, epoll_fd;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Создание сокета
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Настройка адреса для связки
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Связывание сокета с адресом
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Прослушивание порта
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Создание epoll
    if ((epoll_fd = epoll_create1(0)) == -1) {
        perror("epoll_create1");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = server_fd;

    // Добавление серверного сокета в epoll
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1) {
        perror("epoll_ctl");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    std::vector<std::thread> threads;
    std::atomic<bool> running(true);

    auto worker = [&]() { // Рабочий поток epoll
        struct epoll_event events[MAX_EVENTS];

        while (running) {
            int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
            for (int i = 0; i < n; i++) {
                if (events[i].data.fd == server_fd) {
                    // Новое клиентское соединение
                    int client_socket;
                    client_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen);
                    if (client_socket < 0) {
                        perror("accept");
                        continue;
                    }

                    // Запуск нового потока для обработки клиента
                    std::thread(handle_client, client_socket).detach();
                }
            }
        }
    };

    // Запуск рабочих потоков в количестве, равном числу ядер процессора
    for (int i = 0; i < std::thread::hardware_concurrency(); ++i) {
        threads.emplace_back(worker);
    }

    for (auto& t : threads) {
        t.join();
    }

    close(server_fd);
}

// Точка входа в программу
int main() {
    std::cout << "Starting server on port " << PORT << std::endl;
    server();
    return 0;
}
