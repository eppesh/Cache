#include "test.h"
#include "indicators.h"

using namespace indicators;

namespace sh_cache
{
Test::Test(int llirs, int lhirs) :llirs_(llirs), lhirs_(lhirs)
{
    lirs_ = new LIRS(llirs, lhirs);
}

Test::~Test()
{
    delete lirs_;
    lirs_ = nullptr;
}

void Test::RunTest()
{
    std::ifstream ifs("data.txt");        // 测试数据
    std::ofstream ofs("result.csv", std::ios::app);            // 测试结果
    //std::ofstream ofs_hit_info("hit_info.txt"); // 用于调bug时查看命中详情

    if (!ifs.is_open() || !ofs.is_open())
    {
        std::cout << "File open failed!\n";
        return;
    }

    std::string tmp = "";
    // 将文件指针状态清除并将文件指针重新指向文件流开头
    ifs.clear();
    ifs.seekg(std::ios::beg);
    int index = 0;// 测试数据总个数

    int no = 0; // 测试用
    
    while (std::getline(ifs, tmp))
    {
        if (tmp.empty())
        {
            continue;
        }
        index++;
        int key = std::atoi(tmp.c_str());
        bool res = lirs_->Put(key, -1);
        /*if (res)
        {
            ofs_hit_info << "hit[" << ++no << "], index[" << index << "], key[" << key << "]\n";
        }*/
    }
    
    if (index != 0)
    {
        double hit_ratio = (lirs_->GetHitCount() * 1.0 / index) * 100;        
        ofs << "LIRS," << llirs_ +lhirs_ << "," << hit_ratio  << "," << lirs_->GetHitCount()
            << "," << lirs_->GetMissCount() << "," << index << "," << llirs_ << "," << lhirs_ << std::endl;
    }    
    ifs.close();
    ofs.close();
    //ofs_hit_info.close();
}

void Test::RunExample()
{
    std::vector<int> input = { 1,4,2,3,2,1,4,1,5 };
    std::vector<std::string> hash = { "","A","B","C","D","E" };
    for (size_t i = 0; i < input.size(); ++i)
    {
        lirs_->Put(input[i], -1);
        std::cout << "------ t" << i + 1 << "=" << hash[input[i]] << " ------\n";
        lirs_->PrintCache();
        lirs_->GetS().Print();
        lirs_->GetQ().Print();
    }
}
}   // namespace sh_cache