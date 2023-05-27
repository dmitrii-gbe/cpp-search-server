#include "process_queries.h"

std::vector<Document> ProcessQueriesJoined(const SearchServer& search_server, const std::vector<std::string>& queries){
    std::vector<std::vector<Document>> result = ProcessQueries(search_server, queries);
    std::vector<Document> flat_result;
    for (const auto& entry : result){
        flat_result.insert(flat_result.end(), entry.begin(), entry.end());
    }
    return flat_result;
}

std::vector<std::vector<Document>> ProcessQueries(const SearchServer& search_server, const std::vector<std::string>& queries){
    std::vector<std::vector<Document>> result(queries.size());
    std::transform(
        std::execution::par,
        queries.begin(), queries.end(),
        result.begin(),
        [&search_server](std::string query){ return search_server.SearchServer::FindTopDocuments(query); }
        );
    return result;
}