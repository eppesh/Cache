#include <iostream>
#include <vector>
#include "test.h"
#include "indicators.h"

using namespace sh_cache;
using namespace indicators;

int main()
{
    // 多个缓存大小测试
    std::vector<int> cache_size = { 25,50,100,200,300,400,500,600,700,800,900,1000,1100,1200,1300,1400,
                1500,1600,1700,1800,1900,2000,2100,2200,2300,2400 };
    BlockProgressBar bar
    {
        option::BarWidth{50},
        option::ForegroundColor{Color::white},
        option::FontStyles{std::vector<FontStyle>{FontStyle::bold}},
        option::ShowElapsedTime{true},
        option::ShowPercentage{true},
        option::ShowRemainingTime{true},
        option::MaxProgress{cache_size.size()}
    };
    indicators::show_console_cursor(false);

    std::cout << "Start parsing ...\n";
    for (int i = 0; i < cache_size.size(); ++i)
    {        
        size_t llirs = std::floor(0.99 * cache_size[i]);
        size_t lhirs = std::ceil(0.01 * cache_size[i]);
        Test test(llirs, lhirs);
        test.RunTest();

        // 进度条控制        
        bar.set_option(option::PostfixText{ 
            std::to_string(i + 1) + "/" + std::to_string(cache_size.size())
            + " Size=" + std::to_string(cache_size[i]) });
        bar.tick();
    }
    bar.mark_as_completed();
    indicators::show_console_cursor(true);
    std::cout << "Finish parsing ...\n";

    /*
    // 单个缓存大小测试
    size_t cache_size = 25;
    size_t llirs = std::floor(0.99 * cache_size);
    size_t lhirs = std::ceil(0.01 * cache_size);
    Test test(llirs, lhirs);
    test.RunTest();*/


    /*
    // 模拟论文Table1中例子
    size_t llirs = 2;
    size_t lhirs = 1;
    Test test(llirs, lhirs);
    test.RunTest();
    test.RunExample();*/

    system("pause");
    return 0;
}