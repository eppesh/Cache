// Name: test.h
// Copyright 2022 SH Inc. All rights reserved.
// License:
// Author: Sean (eppesh@163.com)
// Time: 09/15/2021
// Description: class for testing

#include <fstream>
#include <iostream>
#include <string>

#include "lirs.h"

namespace sh_cache
{
class Test
{
public:
    Test(int llirs, int lhirs);
    ~Test();

    // �����������,��ȡ�����ʵ���Ϣ
    void RunTest();

    // ģ������Table1������
    void RunExample();

private:
    size_t llirs_;
    size_t lhirs_;
    LIRS *lirs_;
};
} // namespace sh_cache
