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
// ջS��ջQ������LRUջ,��һЩ��ͬ����
class StackBase
{
public:
    // ��ȡջ��С
    int Size();
    // ��������,�ҵ�����λ��,���򷵻�-1
    int Find(const int &key);    

    // ��ȡջ������
    EntryPointer GetTop();
    // ��ȡջ������
    EntryPointer GetBottom();
    // ��ȡָ��λ������
    EntryPointer GetEntry(const int &pos);

    // ��ջ����������
    void EmplaceTop(EntryPointer entry);
    // ��ջ��ɾ������
    EntryPointer EraseBottom();
    // ɾ��ָ��λ������
    void Erase(const int &pos);
    //  ��ָ��λ����������ջ��
    void MoveToTop(const int &pos);

    // ջSջ��֦
    void Pruning();

    // ��ӡջ��Ԫ��
    void Print();

private:
    // ��std::vectorģ��ջ,�������ջ������;
    // ע��,ʹ��emplace_back��"ջ��"�������;"ջ��"ָstd::prev(stack_.end());"ջ��"ָbegin();
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

    // ��̬ά��LIR��HIR����
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