#include <iostream>
#include <set>
#include <string>
#include <vector>
#include <map>
#include <algorithm>

using namespace std;



int main() {
    map<string, int> shelter = {{"landseer"s, 1}, {"otterhound"s, 2}, {"pekingese"s, 2}, {"pointer"s, 3}};
  const  map<string, int> max_amount = {{"landseer"s, 2}, {"otterhound"s, 3}, {"pekingese"s, 4}, {"pointer"s, 7}};
    vector<string> new_dogs = {"landseer"s, "otterhound"s, "otterhound"s, "otterhound"s, "pointer"s};
//
int i = 0;
for (const auto& [key, value] : shelter){
i = i + value;
}
    cout << i;
//     for (const string& dog : new_dogs){
//         if (shelter[dog] < max_amount.at(dog)){
//             shelter[dog] += 1;
//             ++i;
//           }
//         }
// for (const auto& [key, value] : shelter){
//     cout << key << "-"s << value << endl;
//
// }
// cout << i << endl;
cout << count_if(new_dogs.begin(), new_dogs.end(), [&max_amount, &shelter](const string& dog){
  int i = 0;
if (shelter[dog] < max_amount.at(dog)){
    shelter[dog] += 1;
    }
    for (const auto& [key, value] : shelter){
  i = i + value;
}
        return i;
    });
}
