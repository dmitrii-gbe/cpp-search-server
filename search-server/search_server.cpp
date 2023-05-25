#include "search_server.h"
#include <cmath>
#include <numeric>


using namespace std;


SearchServer::SearchServer(string_view stop_words_text)
        : SearchServer::SearchServer(
            SplitIntoWords(stop_words_text))  // Invoke delegating constructor from string container
{
}

SearchServer::SearchServer(const string& stop_words_text)
        : SearchServer::SearchServer(string_view(stop_words_text))
{
}

void SearchServer::AddDocument(int document_id, const string_view& document, DocumentStatus status,
                     const vector<int>& ratings) {
        if ((document_id < 0) || (documents_.count(document_id) > 0)) {
            throw invalid_argument("Invalid document_id"s);
        }
        const auto it = document_text_.insert(static_cast<string>(document));
        const auto words = SplitIntoWordsNoStop(*(it.first));

        const double inv_word_count = 1.0 / words.size();
        for (const string_view& word : words) {
            word_to_document_freqs_[word][document_id] += inv_word_count;
            document_id_to_word_frequency_[document_id][word] += inv_word_count;
        }
        documents_.emplace(document_id, DocumentData{ComputeAverageRating(ratings), status});
        document_ids_.insert(document_id);
}
    
int SearchServer::GetDocumentCount() const {
        return documents_.size();
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const std::execution::sequenced_policy&, const string_view& raw_query, int document_id) const {
        return SearchServer::MatchDocument(raw_query, document_id);
}

tuple<vector<string_view>, DocumentStatus> SearchServer::MatchDocument(const string_view& raw_query, int document_id) const {
 
        const auto query = ParseQuery(raw_query);
        vector<string_view> matched_words;    
        for (const string_view& word : query.minus_words) {
            if (word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id)) {
                return {matched_words, documents_.at(document_id).status};
            }
        }    
        for (const string_view& word : query.plus_words) {
            if (word_to_document_freqs_.count(word) != 0 && word_to_document_freqs_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }   
        return {matched_words, documents_.at(document_id).status};
}


vector<Document> SearchServer::FindTopDocuments(const string_view& raw_query, DocumentStatus status) const {
        return FindTopDocuments(
            execution::seq, raw_query, [status](int document_id, DocumentStatus document_status, int rating) {
                return document_status == status;
            });
}

vector<Document> SearchServer::SearchServer::FindTopDocuments(const std::string_view& raw_query) const {
        return FindTopDocuments(execution::seq, raw_query, DocumentStatus::ACTUAL);
}


bool SearchServer::IsStopWord(const string_view& word) const {
        return stop_words_.count(word) > 0;
}

bool SearchServer::IsValidWord(const string_view& word) {
        // A valid word must not contain special characters
        return none_of(word.begin(), word.end(), [](char c) {
            return c >= '\0' && c < ' ';
        });
}

vector<string_view> SearchServer::SplitIntoWordsNoStop(const string_view& text) const {
        vector<string_view> words;
        for (const string_view& word : SplitIntoWords(text)) {
            if (!IsValidWord(word)) {
                throw invalid_argument("Word "s + static_cast<string>(word) + " is invalid"s);
            }
            if (!IsStopWord(word)) {
                words.push_back(word);
            }
        }
        return words;
}

int SearchServer::ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.empty()) {
            return 0;
        }
        int rating_sum = accumulate(ratings.begin(), ratings.end(), 0);

        return rating_sum / static_cast<int>(ratings.size());
}


SearchServer::QueryWord SearchServer::ParseQueryWord(const string_view& text) const {
        if (text.empty()) {
            throw invalid_argument("Query word is empty"s);
        }
        string_view word = text;
        bool is_minus = false;
        if (word[0] == '-') {
            is_minus = true;
            word = word.substr(1);
        }
        if (word.empty() || word[0] == '-' || !IsValidWord(word)) {
            throw invalid_argument("Query word "s + static_cast<string>(text) + " is invalid");
        }

        return {word, is_minus, IsStopWord(word)};
}


SearchServer::Query SearchServer::ParseQuery(const string_view& text, bool need_sorting) const {
        Query result;
        for (const string_view& word : SplitIntoWords(text)) {
            const auto query_word = ParseQueryWord(word);
            if (!query_word.is_stop) {
                if (query_word.is_minus) {
                    result.minus_words.push_back(query_word.data);
                } else {
                    result.plus_words.push_back(query_word.data);
                }
            }            
        }
    if (need_sorting){
        MakeSortedVectorWithUniqueElements(result.plus_words, std::execution::seq);
        MakeSortedVectorWithUniqueElements(result.minus_words, std::execution::seq);
        return result;
    }
    else {
        return result;
    }
}

double SearchServer::ComputeWordInverseDocumentFreq(const string_view& word) const {
        return log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(word).size());
}

set<int>::iterator SearchServer::begin(){
    return  document_ids_.begin();
}

set<int>::iterator SearchServer::end(){
    return  document_ids_.end();

}

const map<string_view, double>& SearchServer::GetWordFrequencies(int document_id) const {
    if (!document_id_to_word_frequency_.count(document_id)){
        static map<string_view, double> empty_map;
        return empty_map;
    }
    else {
        return document_id_to_word_frequency_.at(document_id);
    }
}

void SearchServer::RemoveDocument(int document_id){
    documents_.erase(document_id);
    document_ids_.erase(document_id);
    std::vector<const std::string_view*> list_vector;
    for (const auto& element : SearchServer::GetWordFrequencies(document_id)){
        list_vector.push_back(&element.first);
    }
    for_each(std::execution::seq, list_vector.begin(), list_vector.end(), [this, document_id](const std::string_view* word){ 
    word_to_document_freqs_.at(*word).erase(document_id); } );
    document_id_to_word_frequency_.erase(document_id);
}