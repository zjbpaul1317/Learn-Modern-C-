#include <iostream>
#define LEN 10

int len_foo()
{
    int i = 2;
    return 1;
}

constexpr int len_foo_constexpr()
{
    return 5;
}

constexpr int fibonacci(const int n)
{
    return n == 1 || n == 2 ? 1 : fibonacci(n - 1) + fibonacci(n - 2);
}

int main()
{
    char arr_1[10];
    char arr_2[LEN];

    int len = 10;
    // char arr_3[len];  非法，表达式必须含有常量值

    const int len_2 = len + 1;
    constexpr int len_2_constexpr = 1 + 2 + 3;
    // char arr_4[len_2];  非法，len_2是一个从const常数，而不是一个常量表达式，而C++标准中数组的长度必须是一个常量表达式
    char arr_4[len_2_constexpr];

    // char arr_5[len_foo() + 5];  非法
    char arr_6[len_foo_constexpr() + 1];

    std::cout << fibonacci(10) << std::endl;
    std::cout << fibonacci(10) << std::endl;
    return 0;
}
