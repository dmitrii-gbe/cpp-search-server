#pragma once
#include "search_server.h"
#include <vector>
#include <deque>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server);
    
    template <typename DocumentPredicate>
    std::vector<Document> AddFindRequest(const std::string_view& raw_query, DocumentPredicate document_predicate) {
        auto result = server_.FindTopDocuments(raw_query, document_predicate);
        bool is_there_result = !result.empty();
        QueryResult query_result(is_there_result);
        requests_.push_back(query_result);
        if (requests_.size() > min_in_day_){
            requests_.pop_front();
        }
        int empty_requests = 0;
        for (size_t i = 0; i < requests_.size(); ++i){
            if (requests_[i].is_result_empty == false){
                ++empty_requests;
            }
        }
        requests_with_no_result_ = empty_requests;
        return result;
    }

    std::vector<Document> AddFindRequest(const std::string_view& raw_query, DocumentStatus status); 

    std::vector<Document> AddFindRequest(const std::string_view& raw_query);
     
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