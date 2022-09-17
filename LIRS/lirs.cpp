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
    // ��ɾ���ٴ�ջ������
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
    
    // ��ջ����ջ�״�ӡ����
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
        // ����ջS
        EntryPointer entry = std::make_shared<Entry>(key, value, Status::kLir);
        stack_s_.EmplaceTop(entry);
    }
    else if (lhirs_ > 0)
    {
        lhirs_--;
        cache_.emplace_back(std::make_pair(key, value));
        // ����ջS��ջQ
        EntryPointer entry = std::make_shared<Entry>(key, value, Status::kResidentHir);
        stack_s_.EmplaceTop(entry);
        stack_q_.EmplaceTop(entry);
    }
    else // ��������,����ɾ��һ�����ݺ������
    {
        // ɾ��Qջ������,��victim;��ͬʱ��ջS��,״̬��Ϊnon-resident
        EntryPointer victim = stack_q_.EraseBottom();
        int pos_in_s = stack_s_.Find(victim->GetKey());
        if (pos_in_s != -1)
        {
            stack_s_.GetEntry(pos_in_s)->SetStatus(Status::kNonResidentHir);
        }

        // ������ɾ��victim,������������(��ֱ���������ݸ���victim)
        auto pos_in_cache = std::find_if(cache_.begin(), cache_.end(), IsEqual(victim->GetKey()));
        if (pos_in_cache != cache_.end())
        {
            /*cache_.erase(pos_in_cache);
            cache_.emplace_back(std::make_pair(key, value));*/
            *pos_in_cache = std::make_pair(key, value);
        }        

        auto pos = stack_s_.Find(key);
        if (pos == -1)  // ���ʿ鲻��ջS��
        {
            // ����ջS��ջQ        
            EntryPointer entry = std::make_shared<Entry>(key, value, Status::kResidentHir);
            stack_s_.EmplaceTop(entry);
            stack_q_.EmplaceTop(entry);
        }
        else // ���ʿ���ջS��
        {
            DynamicRegulation(pos);
        }
    }
}

// ���е����ݿ�Ҫô��LIR״̬,Ҫô��resident HIR״̬;ǰ��һ��λ��ջS��,������һ����ջS��(���ܱ�ջ��֦����
void LIRS::Update(const int &key, const int &value)
{
    int pos = stack_s_.Find(key);
    if (pos != -1)  // ��ջS��
    {
        Status status = stack_s_.GetEntry(pos)->GetStatus();
        if (status == Status::kLir)
        {
            stack_s_.MoveToTop(pos);
            stack_s_.Pruning();
        }
        else
        {
            // ջQɾ����¼
            int position = stack_q_.Find(key);
            stack_q_.Erase(position);

            DynamicRegulation(pos);
        }
    }
    else  // ���е�����ջS��,ֻ����λ��ջQ�е�resident HIR��
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

// ���ʿ�λ��ջS��ʱ,��Ҫ��̬ά��LIR���Ϻ�HIR����
// ��:����Sջ��+״̬��ΪLIR+Sջ��LIR���ΪHIR�鲢����Qջ��
void LIRS::DynamicRegulation(const int &pos)
{
    stack_s_.GetEntry(pos)->SetStatus(Status::kLir);
    stack_s_.MoveToTop(pos);

    stack_s_.GetBottom()->SetStatus(Status::kResidentHir);
    stack_q_.EmplaceTop(stack_s_.GetBottom());

    stack_s_.Pruning(); // ջSջ�׵�LIR�Ѿ������resident HIR��,��ջ��֦ʱ�ᱻ�Զ�ɾ��
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