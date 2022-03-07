#!/bin/bash

cd ../lib
g++ -std=c++20 -c lightweight_manual_reset_event.cpp lightweight_manual_reset_event.o -I ../include
ar crv liblightweight_manual_reset_event.a lightweight_manual_reset_event.o

cd ../test
g++ -std=c++20 task_tests.cpp -o task_tests ../lib/liblightweight_manual_reset_event.a -I ../include
g++ -std=c++20 test_awaitable.cpp -o test_awaitable -I ../include
g++ -std=c++20 test_awaiter.cpp -o test_awaiter -I ../include
g++ -std=c++20 test_awaitable_traits.cpp -o test_awaitable_traits -I ../include
