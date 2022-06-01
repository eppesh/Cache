// Name: main.cpp
// Copyright 2022 Sean.
// License:
// Author: Sean (eppesh@163.com)
// Time: 06/01/2022
// Description: LRU Cache Algorithm test

#include "lru.h"    // NOLINT

#include <iostream>

int main()
{
    int capacity = 2;
    LRUCache *lru_cache = new LRUCache(2);
    lru_cache->Put(1, 1);                           // cache is {1=1}
    lru_cache->Put(2, 2);                           // cache is {1=1, 2=2}
    std::cout << lru_cache->Get(1) << std::endl;    // return 1
    lru_cache->Put(3, 3);                           // LRU key was 2, evicts key 2, cache is {1=1, 3=3}
    std::cout << lru_cache->Get(2) << std::endl;    // returns -1 (not found)
    lru_cache->Put(4, 4);                           // LRU key was 1, evicts key 1, cache is {4=4, 3=3}
    std::cout << lru_cache->Get(1) << std::endl;    // return -1 (not found)
    std::cout << lru_cache->Get(3) << std::endl;    // return 3
    std::cout << lru_cache->Get(4) << std::endl;    // return 4

    delete lru_cache;
    lru_cache = nullptr;
    return 0;
}
