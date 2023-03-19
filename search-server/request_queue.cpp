#include "request_queue.h"
#include "document.h"
#include "search_server.h"

using namespace std;


   RequestQueue::RequestQueue(const SearchServer& search_server) 
    : server_(search_server)
    {}

    template <typename DocumentPredicate>
    vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentPredicate document_predicate) {
        auto result = server_.FindTopDocuments(raw_query, document_predicate);
        bool is_there_result = !result.empty();
        QueryResult query_result(is_there_result);
        requests_.push_back(query_result);
        if (requests_.size() > min_in_day_){
            requests_.pop_front();
        }
        int empty_requests = 0;
        for (int i = 0; i < requests_.size(); ++i){
            if (requests_[i].is_result_empty == false){
                ++empty_requests;
            }
        }
        requests_with_no_result_ = empty_requests;
        return result;
    }

    vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
        return AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating) { return document_status == status; });

    }

   vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
        return  AddFindRequest(raw_query, DocumentStatus::ACTUAL);
    }

       int RequestQueue::GetNoResultRequests() const {
           return requests_with_no_result_;
    }