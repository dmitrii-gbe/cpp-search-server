#include "remove_duplicates.h"
#include <set>
#include <vector>

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {
    set<set<string>> words_of_documents;
    vector<int> documents_to_remove;
    for (const auto& document_id : search_server){
        set<string> words;
        for (const auto& [word, frequency] : search_server.SearchServer::GetWordFrequencies(document_id)){
                words.insert(word);
        }
        if (words_of_documents.count(words) == 0){
                words_of_documents.insert(words);
        }
        else {
            documents_to_remove.push_back(document_id);
        }
    }  
    for (const auto& id : documents_to_remove){
        cout << "Found duplicate document id "s << id << endl;
        search_server.SearchServer::RemoveDocument(id);
    } 
}
        



