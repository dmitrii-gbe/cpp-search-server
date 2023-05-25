#include "process_queries.h"

using namespace std;

vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const vector<string>& queries){
    vector<vector<Document>> result(queries.size());
    vector<Document> flat_result;
    transform(
        execution::par,
        queries.begin(), queries.end(), result.begin(),
        [&search_server](string query){ return search_server.SearchServer::FindTopDocuments(query); }
        );
    for (const auto& entry : result){
        flat_result.insert(flat_result.end(), entry.begin(), entry.end());
    }
    
    return flat_result;
}

vector<vector<Document>> ProcessQueries(const SearchServer& search_server, const vector<string>& queries){
    vector<vector<Document>> result(queries.size());
    transform(
        execution::par,
        queries.begin(), queries.end(),
        result.begin(),
        [&search_server](string query){ return search_server.SearchServer::FindTopDocuments(query); }
        );
    return result;
}