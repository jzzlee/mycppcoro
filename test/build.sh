#!/bin/bash

cd ../lib
g++ -std=c++2a -c lightweight_manual_reset_event.cpp lightweight_manual_reset_event.o -I ../include
g++ -std=c++2a -c async_manual_reset_event.cpp async_manual_reset_event.o -I ../include
g++ -std=c++2a -c async_mutex.cpp async_mutex.o -I ../include

ar crv libcppcoro_fundation.a lightweight_manual_reset_event.o async_manual_reset_event.o async_mutex.o

cd ../test
g++ -std=c++2a task_tests.cpp -o task_tests ../lib/libcppcoro_fundation.a -I ../include
g++ -std=c++2a when_all_test.cpp -o when_all_test ../lib/libcppcoro_fundation.a -I ../include
g++ -std=c++2a test_awaitable.cpp -o test_awaitable -I ../include
g++ -std=c++2a test_awaiter.cpp -o test_awaiter -I ../include
g++ -std=c++2a test_awaitable_traits.cpp -o test_awaitable_traits -I ../include
