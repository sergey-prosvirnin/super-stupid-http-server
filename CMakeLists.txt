# Минимальная версия CMake
cmake_minimum_required(VERSION 3.10)

# Имя проекта
project(super-stupid-http-server)

# Установка стандарта C++
set(CMAKE_CXX_STANDARD 17)

# Добавление исполняемого файла
add_executable(super-stupid-http-server main.cpp)

# Установка исполняемого файла
install(TARGETS super-stupid-http-server DESTINATION /usr/local/bin)

# Создание и установка man страницы
install(FILES super-stupid-http-server.1 DESTINATION /usr/share/man/man1)

# Создание скрипта post-installation для обновления PATH и регенерации man страниц
file(WRITE ${CMAKE_BINARY_DIR}/postinst
        "#!/bin/bash
echo 'export PATH=/usr/local/bin:\$PATH' >> /etc/profile.d/super_stupid_http_server.sh
mandb -q
")
file(COPY ${CMAKE_BINARY_DIR}/postinst DESTINATION ${CMAKE_BINARY_DIR}/DEBIAN/)

# Настройка пакета DEB
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "super-stupid-http-server")
set(CPACK_PACKAGE_VERSION "1.0")
set(CPACK_PACKAGE_CONTACT "Sergei Prosvirnin <prosvirninsd@ya.ru>")
set(CPACK_DEBIAN_PACKAGE_SECTION "base")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_PACKAGE_DESCRIPTION "Очень простой и глупый HTTP-сервер, который, однако, решает проблему 10K одновременных соединений и делает это хорошо.")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA ${CMAKE_BINARY_DIR}/postinst)

include(CPack)

# Добавление цели для деинсталляции пакета
add_custom_target(uninstall
        COMMAND ${CMAKE_COMMAND} -P ${CMAKE_BINARY_DIR}/cmake_uninstall.cmake)

configure_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
        "${CMAKE_BINARY_DIR}/cmake_uninstall.cmake"
        IMMEDIATE @ONLY
)

