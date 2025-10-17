cmake_minimum_required(VERSION 3.16)
project(host_mock C)
set(CMAKE_C_STANDARD 11)

add_executable(ge_receiver
  receiver_main.c
  mock_kermit.c
  ../common/framing.c
)
target_include_directories(ge_receiver PRIVATE ../common)

add_executable(ge_sender
  sender_main.c
  mock_kermit.c
  ../common/framing.c
)
target_include_directories(ge_sender PRIVATE ../common)
