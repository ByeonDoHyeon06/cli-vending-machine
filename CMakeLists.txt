cmake_minimum_required(VERSION 4.0)
project(FinalExam C)

set(CMAKE_C_STANDARD 99)

add_executable(FinalExam main.c
        vmcd.h
        vmcd_core.c
        vmcd_ui.c
        vmcd_file.c
)

