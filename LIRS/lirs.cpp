#include "lirs.h"
namespace sh_cache
{
Entry::Entry(const int &key, const int &value, Status status /* = Status::kInvalid */) :
    key_(key), value_(value), status_(status)
{

}
int Entry::GetKey()
{
    return key_;
}
Status Entry::GetStatus()
{
    return status_;
}
void Entry::SetStatus(Status status)
{
    status_ = status;
}

//////////////////////////////////////////////////////////////////////////

int StackBase::Size()
{
    return stack_.size();
}

int StackBase::Find(const int &key)
{
    int result = -1;
    int position = 0;
    for (const auto entry : stack_)
    {
        if (entry->GetKey() == key)
        {
            result = position;
            break;
        }
        position++;
    }
    return result;
}

EntryPointer StackBase::GetTop()
{
    return stack_[stack_.size() - 1];
}

EntryPointer StackBase::GetBottom()
{
    return stack_[0];
}

EntryPointer StackBase::GetEntry(const int &pos)
{
    if (pos < 0 || pos >= stack_.size())
    {
        return nullptr;
    }
    return stack_[pos];
}

void StackBase::EmplaceTop(EntryPointer entry)
{
    stack_.emplace_back(entry);
}

EntryPointer StackBase::EraseBottom()
{
    EntryPointer entry = stack_[0];
    stack_.erase(stack_.begin());
    return entry;
}

void StackBase::Erase(const int &pos)
{
    if (pos <0 || pos >= stack_.size())
    {
        return;
    }
    stack_.erase(stack_.begin() + pos);
}

void StackBase::MoveToTop(const int &pos)
{
    if (pos < 0 || pos >= stack_.size())
    {
        return;
    }
    // 先删除再从栈顶插入
    EntryPointer entry = stack_[pos];
    stack_.erase(stack_.begin() + pos);
    stack_.emplace_back(entry);
}

void StackBase::Pruning()
{
    while (!stack_.empty() && GetBottom()->GetStatus() != Status::kLir &&
        GetBottom()->GetStatus() != Status::kInvalid)
    {
        EraseBottom();
    }
}

std::string StatusToString(const Status &status)
{
    if (status == Status::kInvalid)
    {
        return "Invalid";
    }
    else if (status == Status::kLir)
    {
        return "LIR";
    }
    else if (status == Status::kResidentHir)
    {
        return "resident HIR";
    }
    else if (status == Status::kNonResidentHir)
    {
        return "non-resident HIR";
    }
    return "Invalid --";
}

void StackBase::Print()
{
    std::vector<std::string> hash = { "","A","B","C","D","E" };
    std::cout << "Size=" << stack_.size() << std::endl;
    
    // 从栈顶到栈底打印出来
    for (int i = stack_.size() - 1; i >= 0; --i)
    {
        std::cout << hash[stack_[i]->GetKey()] << " - " << StatusToString(stack_[i]->GetStatus()) << std::endl;
    }
}

//////////////////////////////////////////////////////////////////////////

LIRS::LIRS(size_t llirs, size_t lhirs) :llirs_(llirs), lhirs_(lhirs), hit_count_(0), miss_count_(0)
{
    cache_.reserve(llirs_ + lhirs_);
}



bool LIRS::Put(const int &key, const int &value)
{
    auto result = std::find_if(cache_.begin(), cache_.end(), IsEqual(key));
    if (result == cache_.end())
    {
        miss_count_++;
        Insert(key, value);
        return false;
    }
    else
    {
        hit_count_++;
        Update(key, value);
        return true;
    }
}

size_t LIRS::GetHitCount()
{
    return hit_count_;
}

size_t LIRS::GetMissCount()
{
    return miss_count_;
}

void LIRS::Insert(const int &key, const int &value)
{
    if (llirs_ > 0)     // Llirs is not full
    {
        llirs_--;
        cache_.emplace_back(std::make_pair(key, value));
        // 更新栈S
        EntryPointer entry = std::make_shared<Entry>(key, value, Status::kLir);
        stack_s_.EmplaceTop(entry);
    }
    else if (lhirs_ > 0)
    {
        lhirs_--;
        cache_.emplace_back(std::make_pair(key, value));
        // 更新栈S和栈Q
        EntryPointer entry = std::make_shared<Entry>(key, value, Status::kResidentHir);
        stack_s_.EmplaceTop(entry);
        stack_q_.EmplaceTop(entry);
    }
    else // 缓存已满,需先删除一个数据后再添加
    {
        // 删除Q栈底数据,即victim;若同时在栈S中,状态改为non-resident
        EntryPointer victim = stack_q_.EraseBottom();
        int pos_in_s = stack_s_.Find(victim->GetKey());
        if (pos_in_s != -1)
        {
            stack_s_.GetEntry(pos_in_s)->SetStatus(Status::kNonResidentHir);
        }

        // 缓存中删除victim,并插入新数据(或直接用新数据覆盖victim)
        auto pos_in_cache = std::find_if(cache_.begin(), cache_.end(), IsEqual(victim->GetKey()));
        if (pos_in_cache != cache_.end())
        {
            /*cache_.erase(pos_in_cache);
            cache_.emplace_back(std::make_pair(key, value));*/
            *pos_in_cache = std::make_pair(key, value);
        }        

        auto pos = stack_s_.Find(key);
        if (pos == -1)  // 访问块不在栈S中
        {
            // 更新栈S、栈Q        
            EntryPointer entry = std::make_shared<Entry>(key, value, Status::kResidentHir);
            stack_s_.EmplaceTop(entry);
            stack_q_.EmplaceTop(entry);
        }
        else // 访问块在栈S中
        {
            DynamicRegulation(pos);
        }
    }
}

// 命中的数据块要么是LIR状态,要么是resident HIR状态;前者一定位于栈S中,后者则不一定在栈S中(可能被栈剪枝掉）
void LIRS::Update(const int &key, const int &value)
{
    int pos = stack_s_.Find(key);
    if (pos != -1)  // 在栈S中
    {
        Status status = stack_s_.GetEntry(pos)->GetStatus();
        if (status == Status::kLir)
        {
            stack_s_.MoveToTop(pos);
            stack_s_.Pruning();
        }
        else
        {
            // 栈Q删除记录
            int position = stack_q_.Find(key);
            stack_q_.Erase(position);

            DynamicRegulation(pos);
        }
    }
    else  // 命中但不在栈S中,只能是位于栈Q中的resident HIR块
    {
        int position = stack_q_.Find(key);
        if (position != -1)
        {
            stack_s_.EmplaceTop(stack_q_.GetEntry(position));
            stack_q_.MoveToTop(position);
        }
        else
        {
            std::cerr << "key=" << key << " is hit but not in S and Q!\n";
        }
    }
}

// 访问块位于栈S中时,需要动态维护LIR集合和HIR集合
// 即:移至S栈顶+状态改为LIR+S栈底LIR块变为HIR块并移至Q栈顶
void LIRS::DynamicRegulation(const int &pos)
{
    stack_s_.GetEntry(pos)->SetStatus(Status::kLir);
    stack_s_.MoveToTop(pos);

    stack_s_.GetBottom()->SetStatus(Status::kResidentHir);
    stack_q_.EmplaceTop(stack_s_.GetBottom());

    stack_s_.Pruning(); // 栈S栈底的LIR已经变成了resident HIR块,在栈剪枝时会被自动删除
}

StackBase LIRS::GetS()
{
    return stack_s_;
}

StackBase LIRS::GetQ()
{
    return stack_q_;
}

void LIRS::PrintCache()
{
    std::cout << "Cache:";
    std::vector<std::string> hash = { "","A","B","C","D","E" };
    for (const auto it : cache_)
    {
        std::cout << hash[it.first] << ";";
    }
    std::cout << "\n" << std::endl;
}

} // namespace sh_cache