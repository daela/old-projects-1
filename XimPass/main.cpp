#include <windows.h>
#include "XIMCore.h"

int main(int argc,char*argv[])
{
    XIMConnect();
    XIMSetMode(XIMModeAutoAnalogDisconnect);
    system("pause");
    XIMDisconnect();
    return 0;
}
