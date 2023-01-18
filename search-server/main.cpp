#include <algorithm>
#include <iostream>
#include <string>
#include <vector>
#include <set>

using namespace std;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result;
    cin >> result;
    ReadLine();
    return result;
}

int main() {
    cout << "Hello"s << endl;
    const int queryCount = ReadLineWithNumber();

    vector<string> queries(queryCount);
    for (string& query : queries) {
        query = ReadLine();
    }
    const string buzzword = ReadLine();

    cout << count_if(queries.begin(), queries.end(), [buzzword](const string& query) {
        set<string> query_set;
        string word;
        for (const char& c : query){
        if (c != ' '){
            word += c;
        }
        else {
          if (!word.empty() && word != " "s){
            query_set.insert(word);
           }
            word.clear();
           }
                    }

     if (!word.empty() && word != " "s){
         query_set.insert(word);
     }
        return query_set.count(buzzword);
        // Реализуйте эту лямбда-функцию
        // Верните true, если query содержит слово buzzword
    });
    cout << endl;
}
