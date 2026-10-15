cmake_minimum_required(VERSION 3.15)

project(jogo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(jogo
    main.c
    listaEncadeada.c
    )

target_link_libraries(jogo PRIVATE raylib)