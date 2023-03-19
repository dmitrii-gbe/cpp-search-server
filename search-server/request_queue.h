#pragma once
#include "search_server.h"
#include <vector>
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);
    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status); 

    std::vector<Document> AddFindRequest(const std::string& raw_query);
     
    int GetNoResultRequests() const;

private:
    struct QueryResult {
        QueryResult() = default;
        
        QueryResult(const bool& value){
            is_result_empty = value;
        }
        
        bool is_result_empty = 0;
    };
    
    std::deque<QueryResult> requests_;
    const static int min_in_day_ = 1440;
    int requests_with_no_result_;
    const SearchServer& server_;

};