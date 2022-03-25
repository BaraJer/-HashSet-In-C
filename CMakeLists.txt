cmake_minimum_required(VERSION 3.20)
project(ex4_gfat C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(ex4_gfat
        HashSet.c
        Node.c
        Hash.h
        Node.h
        Utils.h
        test_runner.h
        test_runner.c
        tests.c)
