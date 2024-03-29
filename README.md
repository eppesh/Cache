[toc]

该目录主要记录一些常见缓存置换算法的学习笔记。

# LRU

`LRU( Least Recently Used)`最近最少使用算法：将最近**最少使用**（即最近一直没有被使用）的内容作为替换对象。

利用一个队列，元素从队首入队列，从队尾出队列。每当元素被命中（即查询/更新的元素已存在队列中时）则将该元素放到队首位置，当队列满且需要插入新元素时，将队尾元素出队列。LRU认为队列尾部的元素就是**最近最少使用**的元素。

## 优缺点

优点：简单；

缺点：

- **突然访问不常使用的**数据，可能使缓存中经常引用的数据被替换；如：偶发的批量操作-批量查询历史数据，由于是批量，大量不常会用到的历史数据（冷数据）把缓存中的热数据替换掉了，会导致后面的缓存命中率下降，拖慢了对正常数据的查询速度。
- 对于比缓存大小稍大的**循环**访问情形，命中率会非常低；如：缓存大小为3，有4个数据块：`1,2,3,4`；循环访问该数据块，即`1,2,3,4,1,2,3,4,1...`，会发现从`4`开始每一个数据都无法命中。
- **不同访问概率**下的表现不佳。如，数据集`{1,2}`和数据集`{a,b,c,d,e}`，访问按照两个数据集交替的方式：`1,a,2,b,1,c,2,d,1,e,2,a...`；假定缓存大小为3，可以发现，虽然`{1,2}`被访问的概率较高，但依然会被频繁地替换出去。

## 实现

参考：[Leetcode-LRU Cache](https://leetcode.cn/problems/lru-cache/); [Leetcode-LRU Cache LCCI](https://leetcode.cn/problems/lru-cache-lcci/); [Leetcode-LRU](https://leetcode.cn/problems/OrIXps/); 

为了提高查询的效率可以借助`map`，为了提高插入/删除的效率可以借助**链表**，将这两者结合起来进行实现。

方式一：利用标准库中的`std::list`和`std::unordered_map`；

```c++
class LRUCache 
{
public:
    struct Node
    {
        int key;
        int value;
        Node():key(0),value(0){}
        Node(int k, int v):key(k),value(v){}
    };

    LRUCache(int capacity) : capacity_(capacity)
    {

    }

    int get(int key) 
    {
        if(map_.find(key) != map_.end())
        {
            list_.splice(list_.begin(), list_, map_[key]);  // 更新该元素在缓存中的位置
            map_[key] = list_.begin();                      // 更新该元素在map_中的值
            return map_[key]->value;
        }
        else
        {
            return -1;
        }
    }
    
    void put(int key, int value) 
    {
        if(map_.find(key) != map_.end())    // 已存在
        {
            map_[key]->value = value;                       // 更新map_中key的值
            list_.splice(list_.begin(), list_, map_[key]);  // map_[key]是该key在缓存(list_)中的位置,将其转移到“队首”
            map_[key] = list_.begin();
        }
        else
        {
            if(list_.size() < capacity_)   // 缓存内还有空间
            {
                list_.emplace_front(Node(key,value));   // 在缓存头部插入key-value
                map_[key] = list_.begin();              // 将缓存中该元素位置更新到map_中
            }
            else
            {
                map_.erase(list_.back().key);
                list_.pop_back();           // 删除缓存中“队尾”元素,它就是最近最少使用的元素
                
                list_.emplace_front(Node(key,value));
                map_[key] = list_.begin();
            }
        }
    }

private:
    std::list<Node> list_; // 用双向链表来表示缓存,因为其插入/删除操作可以O(1)
    std::unordered_map<int, std::list<Node>::iterator> map_;   // 用map当做索引,使查询可以O(1)
    int capacity_;  // 容量
};

/**
 * Your LRUCache object will be instantiated and called as such:
 * LRUCache* obj = new LRUCache(capacity);
 * int param_1 = obj->get(key);
 * obj->put(key,value);
 */
```

方式二：手写双向链表+`std::unordered_map`；

```c++
class LRUCache {
public:
    struct DoubleLinkedList     // 定义一个双向链表,用来模拟缓存
    {
        int key;
        int value;
        DoubleLinkedList *prev; // 前驱节点
        DoubleLinkedList *next; // 后置节点

        DoubleLinkedList():key(0),value(0),prev(nullptr),next(nullptr){}
        DoubleLinkedList(int k,int v):key(k),value(v),prev(nullptr),next(nullptr){}
    };

    LRUCache(int capacity) : capacity_(capacity), current_size_(0)
    {
        dummy_head = new DoubleLinkedList();
        dummy_tail = new DoubleLinkedList();
        dummy_head->next = dummy_tail;
        dummy_tail->prev = dummy_head;
    }
    
    int get(int key) 
    {
        if(map_.find(key) == map_.end())
        {
            return -1;
        }
        else
        {
            // 更新key在缓存中的位置以及在索引中的内容,并返回
            MoveToHead(map_[key]);
            return map_[key]->value;
        }
    }
    
    void put(int key, int value) 
    {
        if(map_.find(key) == map_.end())
        {
            DoubleLinkedList *node = new DoubleLinkedList(key,value);            
            if(current_size_ < capacity_)
            {
                InsertHead(node);
                map_.insert(std::pair<int,DoubleLinkedList *>(key,node));
            }
            else
            {
                // 超过缓存容量时先删除尾部节点,再向头部插入新节点
                map_.erase(dummy_tail->prev->key);
                DeleteTail();
                InsertHead(node);
                map_.insert(std::pair<int,DoubleLinkedList *>(key,node));
            }
        }
        else
        {
            map_[key]->value = value;
            MoveToHead(map_[key]);
        }
    }

    // 以下是辅助方法; 暂不考虑异常参数
    
    // 将key对应节点移动到链表头部
    void MoveToHead(DoubleLinkedList *node)
    {
        if(node->prev == dummy_head)
        {
            return; // 该节点就在头部时无须移动
        }
        node->prev->next = node->next;  // 移除node结点
        node->next->prev = node->prev;

        node->next = dummy_head->next;
        dummy_head->next->prev = node;
        node->prev = dummy_head;
        dummy_head->next = node;
    }

    // 在链表头部插入新节点
    void InsertHead(DoubleLinkedList *node)
    {        
        node->next = dummy_head->next;
        dummy_head->next->prev = node;
        node->prev = dummy_head;
        dummy_head->next = node;
        current_size_++;        
    }

    // 删除链表尾部节点
    void DeleteTail()
    {
        DoubleLinkedList *tail_node = dummy_tail->prev;
        tail_node->prev->next = dummy_tail;
        dummy_tail->prev = tail_node->prev;
        delete tail_node;
        tail_node = nullptr;
        current_size_--;        
    }
private:
    int capacity_;                  // 缓存容量
    int current_size_;              // 缓存当前大小
    std::unordered_map<int, DoubleLinkedList *> map_;   // 用map当作索引,指示key在缓存中的位置
    DoubleLinkedList *dummy_head;   // 表头哑节点
    DoubleLinkedList *dummy_tail;   // 表尾哑节点
};

/**
 * Your LRUCache object will be instantiated and called as such:
 * LRUCache* obj = new LRUCache(capacity);
 * int param_1 = obj->get(key);
 * obj->put(key,value);
 */
```

# LFU

`LFU(Least Frequently Used)`最少使用（最不常使用）算法；

记录缓存中每个元素的使用**次数**（频次）。每次插入、查询/更新的元素存在，则频次+1。插入新元素且缓存已满时，将频次最小的置换出去。

LFU与LRU区别：

- LFU将频次最低的淘汰掉，LRU将最近未用过的淘汰；（前者基于访问次数，后者基于访问时间）

  对于缓存容量为3，输入为`1-2-1-2-1-3-4`的例子，输入3后缓存满，输入4时需要发生置换：

  LFU会将频次最低的3淘汰出去；LRU则会将队列底部的2淘汰。

- LFU需要额外记录各元素的频次；

LFU的缺点：对于缓存容量为3，输入为`1-1-1-2-2-3-4-3-4-3-4...`（后面是`3-4-3-4`的循环交替）的例子，遇到3时缓存满，遇到4时将3淘汰掉，再遇到3时将4淘汰掉，由于后面一直是`3-4`，理应把3、4留下来，但却会导致频繁的缺页中断，而1虽然一直不会用到，但一直淘汰不了。

## 实现

参考[Leetcode- LFU Cache](https://leetcode.cn/problems/lfu-cache/); 

思路：利用`std::unordered_map<int,Node>`来模拟缓存，用红黑树`std::set<Node>`作为辅助来查找最少频次（频次相同时再按LRU思路找最少未使用）的节点。

时间复杂度：`O(logN)`; 由于`get/put`操作涉及对二叉树的插入/删除，因此时间复杂度为`O(logN)`; 

空间复杂度：`O(n)`; 此时`n`是缓存容量。

```c++
class LFUCache {
public:
    struct Node
    {
        int key;
        int value;
        int count;  // 频次
        int time;   // 用int型数表示时间;该值越大说明越新

        Node():key(0),value(0),count(0),time(0){}
        Node(int k,int v,int c,int t):key(k),value(v),count(c),time(t){}

        // 重载小于符号:按频次count从小到大排序,频次相等时按时间time从小到大排序
        bool operator < (const Node &node) const
        {
            return count==node.count ? time<node.time : count<node.count;
        }
    };
    LFUCache(int capacity) : capacity_(capacity), time(0)
    {

    }
    
    int get(int key) 
    {
        if(capacity_<=0)
        {
            return -1;
        }
        time++;
        if(cache_.find(key) == cache_.end())
        {
            return -1;
        }
        else
        {
            // set中查询到值是const不能修改,因此需要先删除旧的,再插入新的
            Node node = cache_[key];
            tools.erase(node);
            node.count++;
            node.time = time;
            tools.insert(node);
            cache_[key] = node;
            return cache_[key].value;
        }
    }
    
    void put(int key, int value) 
    {
        if(capacity_<=0)
        {
            return;
        }
        time++;
        if(cache_.find(key) == cache_.end())
        {
            if(cache_.size() >= capacity_)
            {
                // 缓存已满需先删除频次最小的元素
                cache_.erase(tools.begin()->key);
                tools.erase(tools.begin());
            }
            // 插入新元素
            Node node(key,value,1,time);
            cache_.insert(std::make_pair(key,node));
            tools.insert(node);
        }
        else
        {
            Node node = cache_[key];
            tools.erase(node);
            node.count++;
            node.time = time;
            node.value = value;
            tools.insert(node);
            cache_[key] = node;
        }
    }

private:
    int capacity_;
    int time;   // 时间：每次get/put操作时间都会+1
    std::unordered_map<int, Node> cache_;   // 使用unordered_map模拟缓存
    std::set<Node> tools;   // 辅助工具，因为重载了小于符号，该容器会按频次从小到大排序
};

/**
 * Your LFUCache object will be instantiated and called as such:
 * LFUCache* obj = new LFUCache(capacity);
 * int param_1 = obj->get(key);
 * obj->put(key,value);
 */
```

# LIRS

详见LIRS文件夹。



