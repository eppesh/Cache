// Name: lirs.h
// Copyright 2022 SH Inc. All rights reserved.
// License:
// Author: Sean (eppesh@163.com)
// Time: 09/15/2021
// Description: An implementation of the LIRS algorithm

#ifndef LIRS_H_
#define LIRS_H_

#include <iostream>
#include <memory>
#include <vector>

namespace sh_cache
{
// The status of a block/entry.
enum class Status
{
    kLir = 0,
    kResidentHir,
    kNonResidentHir,
    kInvalid
};

// A entry means a block
class Entry
{
public:
    explicit Entry(const int &key, const int &value, Status status = Status::kInvalid);

    int GetKey();
    Status GetStatus();
    void SetStatus(Status status);

private:
    int key_;
    int value_;
    Status status_;
};

//////////////////////////////////////////////////////////////////////////

using EntryPointer = std::shared_ptr<Entry>;

// The base class of StackS and StackQ.
// 栈S和栈Q类似于LRU栈,有一些共同特征
class StackBase
{
public:
    // 获取栈大小
    int Size();
    // 查找数据,找到返回位置,否则返回-1
    int Find(const int &key);    

    // 获取栈顶数据
    EntryPointer GetTop();
    // 获取栈底数据
    EntryPointer GetBottom();
    // 获取指定位置数据
    EntryPointer GetEntry(const int &pos);

    // 向栈顶插入数据
    void EmplaceTop(EntryPointer entry);
    // 从栈底删除数据
    EntryPointer EraseBottom();
    // 删除指定位置数据
    void Erase(const int &pos);
    //  将指定位置数据移至栈顶
    void MoveToTop(const int &pos);

    // 栈S栈剪枝
    void Pruning();

    // 打印栈中元素
    void Print();

private:
    // 用std::vector模拟栈,用来存放栈中数据;
    // 注意,使用emplace_back从"栈顶"添加数据;"栈顶"指std::prev(stack_.end());"栈底"指begin();
    std::vector<EntryPointer> stack_;
};

class IsEqual
{
public:
    explicit IsEqual(int key) :key_(key)
    {
    }
    bool operator() (const std::pair<int, int> &obj) const
    {
        return key_ == obj.first;
    }
private:
    const int key_;
};

// The core class of LIRS
class LIRS
{
public:
    LIRS(size_t llirs, size_t lhirs);
    ~LIRS() = default;

    // Access <key,value>; return true when hitting, or false when missing
    bool Put(const int &key, const int &value);

    size_t GetHitCount();
    size_t GetMissCount();

    // get the stack S/Q
    StackBase GetS();
    StackBase GetQ();

    // print the element in cache
    void PrintCache();

private:
    // Insert a new data to cache
    void Insert(const int &key, const int &value);

    // Update the existed data in cache
    void Update(const int &key, const int &value);

    // 动态维护LIR和HIR集合
    void DynamicRegulation(const int &pos);

private:
    size_t llirs_;                  // the size of LIR set in cache
    size_t lhirs_;                  // the size of resident HIR set in cache

    size_t hit_count_;
    size_t miss_count_;

    std::vector<std::pair<int,int>> cache_;        // use vector as the cache
    //std::vector<int> cache_;        // use vector as the cache

    StackBase stack_s_;
    StackBase stack_q_;
};
} // namespace sh_cache

#endif //LIRS_H_