#!/bin/bash

cd ../lib
rm *.o
rm *.a

cd ../test
rm task_tests 
rm when_all_test
rm test_awaitable
rm test_awaiter
rm test_awaitable_traits

echo "done"
