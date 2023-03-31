#include "remove_duplicates.h"
#include <algorithm>
#include <utility>

using namespace std;

void RemoveDuplicates(SearchServer& search_server) {

    auto are_duplicates = [](map<string, double>& first_map, map<string, double>& second_map){ return first_map.size() == second_map.size() && 
         equal(first_map.begin(), first_map.end(), second_map.begin(), second_map.end(), 
             [](pair<string, double> left, pair<string, double> right){ return left.first == right.first; } ); };
    for (const auto& document_id_query : search_server){
        map<string, double> document_words_query = search_server.SearchServer::GetWordFrequencies(document_id_query);
        for (const auto& document_id : search_server){
            map<string, double> document_words = search_server.SearchServer::GetWordFrequencies(document_id);
            if (are_duplicates(document_words_query, document_words) && document_id_query != document_id){
                cout << "Found duplicate document id "s << max(document_id_query, document_id) << endl;
                search_server.SearchServer::RemoveDocument(max(document_id_query, document_id));
            }
        }
    }
}