#pragma once
#include <map>
#include <set>
#include <string>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <deque>
#include <execution> 
#include <string_view>
#include <string_view>


#include "read_input_functions.h"
#include "string_processing.h"


using namespace std::string_literals;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

class SearchServer
{
public:
    template <typename StringContainer>
    explicit SearchServer(const StringContainer& stop_words);

    explicit SearchServer( const std::string& stop_words_text);     

    void AddDocument(int document_id, std::string_view document,
        DocumentStatus status, const std::vector<int>& ratings);   

    template <typename DocumentPredicate>
    std::vector<Document> FindTopDocuments(std::string_view raw_query,
        DocumentPredicate document_predicate) const;   

    std::vector<Document> FindTopDocuments(std::string_view raw_query,
        DocumentStatus status) const;   

    std::vector<Document> FindTopDocuments(std::string_view raw_query) const;

    int GetDocumentCount() const; 

    const std::vector<int>::const_iterator begin() const;

    const std::vector<int>::const_iterator end() const; 
    
    template<class ExecutionPolicy>
    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(ExecutionPolicy&& policy, std::string_view raw_query, int document_id) const;

    std::tuple<std::vector<std::string_view>, DocumentStatus> MatchDocument(std::string_view raw_query, int document_id) const;

    const std::map<std::string_view, double>& GetWordFrequencies(int document_id) const;

    template<class ExecutionPolicy>
    void RemoveDocument(ExecutionPolicy&& policy, int document_id);

    void RemoveDocument(int document_id);
   
    

private:
    struct DocumentData
    {
        int rating;
        DocumentStatus status;
    };
        
    const std::set<std::string> stop_words_;
    std::map<std::string, std::map<int, double>> word_to_document_freqs_;
    std::map<int, DocumentData> documents_;
    std::vector<int> document_ids_;
    std::map<int, std::map<std::string_view, double>> id_document_ { {0, {}} };

    bool IsStopWord(std::string_view word) const;

    static bool IsValidWord(std::string_view word);

    std::vector<std::string_view> SplitIntoWordsNoStop(std::string_view text) const;

    static int ComputeAverageRating(const std::vector<int>& ratings);    

    struct QueryWord
    {
        std::string_view data;
        bool is_minus;
        bool is_stop;
    };

    QueryWord ParseQueryWord(std::string_view text) const;    

    struct Query
    {
        std::set<std::string_view> plus_words;
        std::set<std::string_view> minus_words;
    };

    Query ParseQuery(std::string_view text) const;
   
    double ComputeWordInverseDocumentFreq(std::string_view word) const;

    template <typename DocumentPredicate>
    std::vector<Document> FindAllDocuments(const Query& query, 
        DocumentPredicate document_predicate) const;    
};

template <typename StringContainer>
SearchServer::SearchServer(const StringContainer& stop_words)
    : stop_words_(MakeUniqueNonEmptyStrings(stop_words))
{
    if (!all_of(stop_words_.begin(), stop_words_.end(), IsValidWord))
    {
        throw std::invalid_argument("Some of stop words are invalid"s);
    }
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query,
    DocumentPredicate document_predicate) const
{   
    const auto query = ParseQuery(raw_query);

    auto matched_documents = FindAllDocuments(query, document_predicate);

    const double eps = 1e-6;

    sort(matched_documents.begin(), matched_documents.end(), [&](const Document& lhs, const Document& rhs)
        {
            if (std::abs(lhs.relevance - rhs.relevance) < eps)
            {
                return lhs.rating > rhs.rating;
            }
            else
            {
                return lhs.relevance > rhs.relevance;
            }
        });

    if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT)
    {
        matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
    }

    return matched_documents;
}

template <typename DocumentPredicate>
std::vector<Document> SearchServer::FindAllDocuments(const Query& query, 
    DocumentPredicate document_predicate) const
{
    std::map<int, double> document_to_relevance;
    for (std::string_view word : query.plus_words)
    {
        std::string temp{ word };
        if (word_to_document_freqs_.count(temp) == 0)
        {
            continue;
        }
        const double inverse_document_freq = ComputeWordInverseDocumentFreq(word);
        for (const auto [document_id, term_freq] : word_to_document_freqs_.at(temp))
        {
            const auto& document_data = documents_.at(document_id);
            if (document_predicate(document_id, document_data.status, document_data.rating))
            {
                document_to_relevance[document_id] += term_freq * inverse_document_freq;
            }
        }
    }

    for (std::string_view word : query.minus_words)
    {
        std::string temp{ word };
        if (word_to_document_freqs_.count(temp) == 0)
        {
            continue;
        }
        for (const auto [document_id, _] : word_to_document_freqs_.at(temp))
        {
            document_to_relevance.erase(document_id);
        }
    }

    std::vector<Document> matched_documents;
    for (const auto [document_id, relevance] : document_to_relevance)
    {
        matched_documents.push_back({ document_id, relevance, documents_.at(document_id).rating });
    }
    return matched_documents;
}

template<class ExecutionPolicy>
void SearchServer::RemoveDocument(ExecutionPolicy&& policy, int document_id)
{
    if (documents_.count(document_id) == 0)
    {
        return;
    }
    if (documents_.count(document_id))
    {
        for (size_t i = 0; i < document_ids_.size(); ++i)
        {
            auto it = find(policy, document_ids_.begin(), document_ids_.end(), document_id);
            if (it != document_ids_.end())
            {
                document_ids_.erase(it);
            }
        }

        const auto it = documents_.find(document_id);
        if (it != documents_.end())
        {
            documents_.erase(it);
        }

        const auto it1 = id_document_.find(document_id);
        if (it1 != id_document_.end())
        {
            id_document_.erase(it1);
        }

        for (auto it = word_to_document_freqs_.begin(); it != word_to_document_freqs_.end(); ++it)
        {
            auto& temp = *it;
            temp.second.erase(document_id);

        }
    }
}

template<class ExecutionPolicy>
std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(ExecutionPolicy&& policy, std::string_view raw_query, int document_id) const
{
    return MatchDocument(raw_query, document_id);
}