#include <nx/common.hpp>
#include <nx/thread/safe_blocking_queue.hpp>
#include <iostream>
#include <string>
#include <boost/thread/thread.hpp>


#define QUEUESIZE 500000

#include <fstream>
std::ofstream ofs;

//#include <boost/spirit/include/classic_file_iterator.hpp>


class Test : public nx::singleton<Test>, boost::noncopyable
{
    friend class nx::singleton<Test>;
    Test() { }
};
int main(int argc,char*argv[])
{
    Test::instance();
    using nx::thread::static_queue;
    using nx::thread::dynamic_queue;
    using nx::thread::expanding_queue;

    static_queue<int,3,true> objTest;
    objTest.push(1);
    objTest.push(2);
    objTest.push(3);
    objTest.push(4);

    static_queue<int,QUEUESIZE> objCircular;
    dynamic_queue<int> objHeapCircular(QUEUESIZE);
    expanding_queue<int> objQueue;

    int val;
    DWORD dwTime;
    timeBeginPeriod(1);

    for (int j=0;j<10;++j)
    {
        std::cout << "PUSH" << std::endl;
        dwTime=timeGetTime();
        for (int i=0;i<QUEUESIZE;)
            objCircular.push(++i);
        std::cout << (timeGetTime()-dwTime) << " vs. ";
        dwTime=timeGetTime();
        for (int i=0;i<QUEUESIZE;)
            objHeapCircular.push(++i);
        std::cout << (timeGetTime()-dwTime) << " vs. ";
        dwTime=timeGetTime();
        for (int i=0;i<QUEUESIZE;)
            objQueue.push(++i);
        std::cout << (timeGetTime()-dwTime) << std::endl;

        std::cout << "POP" << std::endl;
        dwTime=timeGetTime();
        for (int i=0;i<QUEUESIZE;++i)
            objCircular.pop(val);
        std::cout << (timeGetTime()-dwTime) << " vs. ";
        dwTime=timeGetTime();
        for (int i=0;i<QUEUESIZE;++i)
            objHeapCircular.pop(val);
        std::cout << (timeGetTime()-dwTime) << " vs. ";
        dwTime=timeGetTime();
        for (int i=0;i<QUEUESIZE;++i)
            objQueue.pop(val);
        std::cout << (timeGetTime()-dwTime) << std::endl;
    }


    timeEndPeriod(1);


    return 0;
}
/*
struct callable
{
    void operator()()
    {
        std::string strBla;
        for (;;)
        {
            std::cout << "Popping..." << std::endl;
            objQueue.pop(strBla);
        }
    }
};
int main(int argc,char*argv[])
{
    std::string test;
    callable x;
    boost::thread somethread(x);
    Sleep(5000);
    objQueue.push("hello");
    objQueue.push("there");
    objQueue.push("sally");
    while (true)
    {
        std::cout << "wait..." << std::endl;
        Sleep(5000);
    }
    return 0;
}
*/
