// Name: lru.cpp
// Copyright 2022 Sean.
// License:
// Author: Sean (eppesh@163.com)
// Time: 06/01/2022
// Description: LRU Cache Algorithm

#include "lru.h"    // NOLINT

LRUCache::LRUCache(int capacity) : capacity_(capacity)
{
}

int LRUCache::Get(const int &key)
{
    if (key_index_.find(key) == key_index_.end())
    {
        return -1;
    }
    else
    {
        cache_.splice(cache_.begin(), cache_, key_index_[key]);    // update the hit block's location
        key_index_[key] = cache_.begin();
        return key_index_[key]->value;
    }
}

void LRUCache::Put(const int &key, const int &value)
{
    if (key_index_.find(key) == key_index_.end())
    {
        if (cache_.size() >= capacity_)
        {
            key_index_.erase(cache_.back().key);
            cache_.pop_back();
        }
        cache_.emplace_front(Node(key, value));
        key_index_[key] = cache_.begin();
    }
    else
    {
        key_index_[key]->value = value;
        cache_.splice(cache_.begin(), cache_, key_index_[key]);
        key_index_[key] = cache_.begin();
    }
}
