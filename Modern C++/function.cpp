#include <iostream>
#include <functional>
using namespace std;

int g_Minus(int i, int j)
{
    return i - j;
}

int main()
{
    function<int(int, int)> f = g_Minus;
    cout << f(1, 2) << endl;
    return 0;
}