#include "request_queue.h"
#include "document.h"
#include "search_server.h"

RequestQueue::RequestQueue(const SearchServer& search_server) 
    : server_(search_server)
    {}

std::vector<Document> RequestQueue::AddFindRequest(const std::string_view raw_query, DocumentStatus status) {
        return AddFindRequest(raw_query, [status](int document_id, DocumentStatus document_status, int rating) { return document_status == status; });

}

std::vector<Document> RequestQueue::AddFindRequest(const std::string_view raw_query) {
        return  AddFindRequest(raw_query, DocumentStatus::ACTUAL);
}
int RequestQueue::GetNoResultRequests() const {
           return requests_with_no_result_;
}