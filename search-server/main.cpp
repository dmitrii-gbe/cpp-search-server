#include <algorithm>
#include <iostream>
#include <set>
#include <string>
#include <utility>
#include <vector>
#include <map>
#include <cmath>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        } else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }

    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> words = SplitIntoWordsNoStop(document);
        ++document_count_;
        double frequency = 1.0 / words.size();
              for (const string& word : words){
                      vocabulary_[word][document_id] += frequency;
        }
    }



    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query_words);

       sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 return lhs.relevance > rhs.relevance;
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    int document_count_ = 0;

    map<string, map<int, double>> vocabulary_;

    set<string> stop_words_;

    struct Query {
      set<string> plus_words;
      set<string> minus_words;
    };

    bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
    }

    Query ParseQuery(const string& text) const {
      Query plus_minus_words;
      //vector<string> query_words = SplitIntoWordsNoStop(text);
      for (const string& word : SplitIntoWordsNoStop(text)) {
        if (word[0] == '-'){
            plus_minus_words.minus_words.insert(word.substr(1));
          }
        else {
          plus_minus_words.plus_words.insert(word);
          }
      }
      return plus_minus_words;
  }

 double CalculateIDF(const string& text) const {
      double idf = log (static_cast<double>(document_count_) / static_cast<double>(vocabulary_.at(text).size()));

   return idf;
 }

     vector<Document> FindAllDocuments(const Query& query_words) const {
       vector<Document> matched_documents;
       map<int, map<string, double>> matched_documents1;
       map<int, double> relevance;
       //set<string> minus_words = ParseMinusWords(query_words);

        for (const string& word : query_words.plus_words){
            if (vocabulary_.count(word)){
              double word_idf = CalculateIDF(word);
              for (const auto& [key1, value1] : vocabulary_.at(word)){
                                relevance[key1] += value1 * word_idf;

                      }
                  }
              }

      for (const auto& word : query_words.minus_words) {
      if (vocabulary_.count(word)){
           for (const auto& [docid, frequency] : vocabulary_.at(word)){
                    relevance.erase(docid);
                  }
              }
          }


        for (const auto& [key, value] : relevance){
            matched_documents.push_back({key, value});
        }
        return matched_documents;
    }
};

SearchServer CreateSearchServer() {
    SearchServer search_server;
    search_server.SetStopWords(ReadLine());

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        search_server.AddDocument(document_id, ReadLine());
    }

    return search_server;
}

int main() {
    const SearchServer search_server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto& [document_id, relevance] : search_server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", "
             << "relevance = "s << relevance << " }"s << endl;
    }
}
