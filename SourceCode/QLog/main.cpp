#include "QSimpleLog.h"
#include <iostream>
#include <string>



int main(int argc, char *argv[])
{
    QSimpleLog Test;
    Test.SetLogFile("log.txt");
    Test.WriteDebug("Debug test, number = %d, msg = %s", LL_DEBUG, "debug");
    Test.WriteInfo("Info test, number = %d, msg = %s", LL_INFO, "info");
    Test.WriteWarn("Warn test, number = %d, msg = %s", LL_WARN, "warn");
    Test.WriteError("Error test, number = %d, msg = %s", LL_ERROR, "error");

#ifdef _WIN32
    std::string test;
    std::cin >> test;
#endif // _WIN32

    return 0;
}
