// Name: lru.h
// Copyright 2022 Sean.
// License:
// Author: Sean (eppesh@163.com)
// Time: 06/01/2022
// Description: LRU Cache Algorithm

#ifndef LRU_H_    // NOLINT
#define LRU_H_    // NOLINT

#include <unordered_map>

#include <list>

struct Node
{
    int key;
    int value;

    Node() : key(0), value(0)
    {
    }

    Node(int k, int v) : key(k), value(v)
    {
    }
};

class LRUCache
{
public:
    LRUCache() = default;
    explicit LRUCache(int capacity);
    ~LRUCache() = default;

    // disable copying and moving
    LRUCache(const LRUCache &) = delete;
    LRUCache &operator=(const LRUCache &) = delete;
    LRUCache(LRUCache &&) = delete;
    LRUCache &operator=(LRUCache &&) = delete;

    int Get(const int &key);
    void Put(const int &key, const int &value);

private:
    int capacity_;
    std::list<Node> cache_;
    std::unordered_map<int, std::list<Node>::iterator> key_index_;
};

#endif    // NOLINT