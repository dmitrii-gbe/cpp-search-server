#include <iostream>
#include <string>
using namespace std;

int main() {
    int y = 0;
    int x = 0;
    for (int i = 0; i <= 1000; ++i){
        string s = to_string(i);
        //cout << s << endl;
        for (int g = 0; g < s.size(); ++g){
            if (s[g] == '3'){
                ++y;
            }
        }
            if (y > 0){
            ++x;
            y = 0;
        }
    }
    cout << x << endl;
    return 0;
}

// Решите загадку: Сколько чисел от 1 до 1000 содержат как минимум одну цифру 3?
// Напишите ответ здесь: 271

// Закомитьте изменения и отправьте их в свой репозиторий.
