#pragma once
#include "document.h"
#include "string_processing.h"
#include <map>
#include <set>
#include <deque>
#include <stdexcept>
#include <tuple>
#include <algorithm>
#include <vector>
#include <execution>
#include <utility>
#include <string_view>
#include <execution>
#include "concurrent_map.h"
#include "log_duration.h"



using MatchedWordsAndStatus = std::tuple<std::vector<std::string_view>, DocumentStatus>;

constexpr int MAX_RESULT_DOCUMENT_COUNT = 5;

constexpr int SUB_MAPS_COUNT = 16;

const double DELTA = 1e-6;

class SearchServer {
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words)
        : stop_words_(MakeUniqueNonEmptyStrings(stop_words))  // Extract non-empty stop words
    {
        if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord)) {
            throw std::invalid_argument("Some of stop words are invalid");
        }
    }

    explicit SearchServer(std::string_view stop_words_text);
    
    explicit SearchServer(const std::string& stop_words_text);
    
    void AddDocument(int document_id, const std::string_view document, DocumentStatus status, const std::vector<int>& ratings);

    template <typename DocumentPredicate, typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentPredicate document_predicate) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentStatus status) const;

    template <typename ExecutionPolicy>
    std::vector<Document> FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query) const;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentPredicate document_predicate) const;
    std::vector<Document> FindTopDocuments(const std::string_view raw_query, DocumentStatus status) const;
    std::vector<Document> FindTopDocuments(const std::string_view raw_query) const;

    int GetDocumentCount() const;

    MatchedWordsAndStatus MatchDocument(const std::string_view raw_query, int document_id) const;
    
    MatchedWordsAndStatus MatchDocument(const std::execution::sequenced_policy&, const std::string_view raw_query, int document_id) const;
    
    template <typename ExecutionPolicy>
    MatchedWordsAndStatus MatchDocument(ExecutionPolicy&& policy, const std::string_view raw_query, int document_id) const;

    std::set<int>::iterator begin();
    std::set<int>::iterator end();

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;
    
    void RemoveDocument(int document_id);

    template <typename ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id);

private:

    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    const std::set<std::string, std::less<>> stop_words_;
    std::map<std::string_view, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::set<int> document_ids_;
    std::map<int, std::map<std::string_view, double>> document_id_to_word_frequency_;
    std::deque<std::string> document_text_;
 
    bool IsStopWord(const std::string_view word) const;

    static bool IsValidWord(const std::string_view word);

    std::vector<std::string_view> SplitIntoWordsNoStop(const std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);

    struct QueryWord {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(const std::string_view text) const;

    struct Query {
        std::vector<std::string_view> plus_words;
        std::vector<std::string_view> minus_words;
    };

    Query ParseQuery(const std::string_view text, bool need_sorting = true) const;
    
    double ComputeWordInverseDocumentFreq(const std::string_view word) const;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::sequenced_policy, const Query& query, DocumentPredicate document_predicate) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(std::execution::parallel_policy, const Query& query, DocumentPredicate document_predicate) const;
    
    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, DocumentPredicate document_predicate) const;
    
    template <typename T, typename ExecutionPolicy>
    void MakeSortedVectorWithUniqueElements(std::vector<T>& object, ExecutionPolicy&& policy) const;
};

template <typename DocumentPredicate, typename ExecutionPolicy>
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentPredicate document_predicate) const {
        
        const auto query = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(policy, query, document_predicate);

        std::sort(policy, matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (std::abs(lhs.relevance - rhs.relevance) < DELTA) {
                     return lhs.rating > rhs.rating;
                 } else {
                     return lhs.relevance > rhs.relevance;
                 }
             });
        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }

        return matched_documents;
    }

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(const std::string_view raw_query, DocumentPredicate document_predicate) const {
    return FindTopDocuments(std::execution::seq, raw_query, document_predicate);
}
    
template <typename ExecutionPolicy>    
std::vector<Document> SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query, DocumentStatus status) const {
        return FindTopDocuments(
            policy, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
}
template <typename ExecutionPolicy>
std::vector<Document> SearchServer::SearchServer::FindTopDocuments(ExecutionPolicy&& policy, const std::string_view raw_query) const {
        return FindTopDocuments(policy, raw_query, DocumentStatus::ACTUAL);
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(std::execution::parallel_policy, const Query& query,
                                      DocumentPredicate document_predicate) const {
        ConcurrentMap<int, double> document_to_relevance_tmp(SUB_MAPS_COUNT); 
        std::for_each(
            std::execution::par,
            query.plus_words.begin(),
            query.plus_words.end(),
            [&](const auto& word){
                if (word_to_document_freqs_.count(word) == 0) {
                    return;
                }   
               const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
               for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                    const auto& document_data = documents_.at(document_id);
                    if (document_predicate(document_id, document_data.status, document_data.rating)) {
                        document_to_relevance_tmp[document_id].ref_to_value += term_freq * inverse_document_freq;
                    }
                };
            }
        );
    
        
         std::for_each(
            std::execution::par,
            query.minus_words.begin(),
            query.minus_words.end(),
            [&](const auto& word){
                if (word_to_document_freqs_.count(word) == 0) {
                    return;
                }
                for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                    document_to_relevance_tmp.Erase(document_id);
                }
            }
         );
    
    
        std::map<int, double> document_to_relevance = document_to_relevance_tmp.BuildOrdinaryMap();
        
        
        std::vector<Document> matched_documents;

        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(std::execution::sequenced_policy, const Query& query,
                                      DocumentPredicate document_predicate) const {
        std::map<int, double> document_to_relevance;
        for (const std::string_view word : query.plus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
            for (const auto [document_id, term_freq] : word_to_document_freqs_.at(word)) {
                const auto& document_data = documents_.at(document_id);
                if (document_predicate(document_id, document_data.status, document_data.rating)) {
                    document_to_relevance[document_id] += term_freq * inverse_document_freq;
                }
            }
        }

        for (const std::string_view word : query.minus_words) {
            if (word_to_document_freqs_.count(word) == 0) {
                continue;
            }
            for (const auto [document_id, _] : word_to_document_freqs_.at(word)) {
                document_to_relevance.erase(document_id);
            }
        }

        std::vector<Document> matched_documents;
        for (const auto [document_id, relevance] : document_to_relevance) {
            matched_documents.push_back(
                {document_id, relevance, documents_.at(document_id).rating});
        }
        return matched_documents;
    }

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query,
                                      DocumentPredicate document_predicate) const {
    return FindAllDocuments(std::execution::seq, query, document_predicate);
}
    

template <typename ExecutionPolicy> 
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id){
    documents_.erase(document_id);
    document_ids_.erase(document_id);
    std::vector<const std::string_view*> list_vector;
    for (const auto& element : SearchServer::GetWordFrequencies(document_id)){
        list_vector.push_back(&element.first);
    }
    for_each(policy, list_vector.begin(), list_vector.end(), [this, document_id](const std::string_view* word){ 
        word_to_document_freqs_.at(*word).erase(document_id); } );
        document_id_to_word_frequency_.erase(document_id);
}

template <typename T, typename ExecutionPolicy>
void SearchServer::MakeSortedVectorWithUniqueElements(std::vector<T>& object, ExecutionPolicy&& policy) const {
        std::sort(policy, object.begin(), object.end());
        auto it = std::unique(object.begin(), object.end());
        object.erase(it, object.end());
    }

template <typename ExecutionPolicy> 
MatchedWordsAndStatus SearchServer::MatchDocument(ExecutionPolicy&& policy, const std::string_view raw_query, int document_id) const {

        const auto query = ParseQuery(raw_query, false);
         
        if (std::any_of(policy, query.minus_words.begin(), query.minus_words.end(), [&](const std::string_view word){ return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id); })){
        return {{}, documents_.at(document_id).status};
        }
        
        std::vector<std::string_view> matched_words(query.plus_words.size());
        
        auto it = std::copy_if(policy, query.plus_words.begin(), query.plus_words.end(), matched_words.begin(), [&](const std::string_view word){ return word_to_document_freqs_.count(word) && word_to_document_freqs_.at(word).count(document_id); } );
        
        matched_words.resize(it - matched_words.begin());
        
        MakeSortedVectorWithUniqueElements(matched_words, policy);

        return {matched_words, documents_.at(document_id).status};
}


