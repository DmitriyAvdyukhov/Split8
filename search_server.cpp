#include  <cmath>
#include <utility>

#include "search_server.h"


SearchServer::SearchServer(const std::string& stop_words_text)
    : SearchServer( SplitIntoWords(stop_words_text) )
    { }

void  SearchServer::AddDocument(int document_id, std::string_view document, DocumentStatus status, const std::vector<int>& ratings) {
    if ((document_id < 0) || (documents_.count(document_id) > 0)) {
        throw std::invalid_argument("Invalid document_id"s);
    }
    const auto words = SplitIntoWordsNoStop(document);

    const double inv_word_count = 1.0 / words.size();
    for (std::string_view word : words)
    {
        std::string temp  { word };
        word_to_document_freqs_[temp][document_id] += inv_word_count;
    }
    documents_.emplace(document_id, DocumentData{ ComputeAverageRating(ratings), status });
    document_ids_.push_back(document_id);

    std::map<std::string_view, double>temp{};
    double result(0.0);   

    for (const auto&[word, value] : word_to_document_freqs_)
    {       
        auto it = std::find(std::execution::par,words.begin(), words.end(), word);
        if (it != words.end())
        {
            double counts = count(words.begin(), words.end(), word);
            result = counts / static_cast<double>(words.size());
            temp.emplace(word, result);
        }
    }
    id_document_.emplace(document_id, std::move(temp));
}

std::vector<Document> SearchServer::FindTopDocuments(std::string_view raw_query) const
{
    return FindTopDocuments(std::execution::seq, raw_query, DocumentStatus::ACTUAL);
}


std::vector<Document> SearchServer::FindTopDocuments( std::string_view raw_query, DocumentStatus status) const
{
    return FindTopDocuments(raw_query, [status](int document_id, DocumentStatus document_status, int rating)
        {
            return document_status == status;
        });
}

int SearchServer::GetDocumentCount() const
{
        return documents_.size();
}

const std::vector<int>::const_iterator  SearchServer::begin() const
{
    return  document_ids_.begin();
}

const std::vector<int>::const_iterator  SearchServer::end() const
{
    return  document_ids_.end();
}

std::tuple<std::vector<std::string_view>, DocumentStatus> SearchServer::MatchDocument(std::string_view raw_query, int document_id) const
{    
    const auto query = ParseQuery(raw_query);
    std::vector<std::string_view> matched_words;
    for (std::string_view word : query.plus_words)
    {
        std::string temp{ word };
        if (word_to_document_freqs_.count(temp) == 0)
        {
            continue;
        }
        if (word_to_document_freqs_.at(temp).count(document_id))
        {
            matched_words.push_back(std::move(word));
        }
    }
    for (std::string_view word : query.minus_words)
    {
        std::string temp{ word };
        if (word_to_document_freqs_.count(temp) == 0)
        {
            continue;
        }
        if (word_to_document_freqs_.at(temp).count(document_id))
        {
            matched_words.clear();
            break;
        }
    }
    return { matched_words, documents_.at(document_id).status };
}

bool SearchServer::IsStopWord(std::string_view word) const
{
    std::string temp{ word };
    return stop_words_.count(temp) > 0;
}

bool SearchServer::IsValidWord(std::string_view word)
{    
    return std::none_of(word.begin(), word.end(), [](char c)
        {
            return c >= '\0' && c < ' ';
        });
}

std::vector<std::string_view> SearchServer::SplitIntoWordsNoStop(const std::string_view text) const
{
    std::vector<std::string_view> words;
    for (std::string_view word : SplitIntoWords(text))
    {
        if (!IsValidWord(word))
        {
            std::string temp{ word };
            throw std::invalid_argument("Word "s + temp + " is invalid"s);
        }
        if (!IsStopWord(word))
        {
            words.push_back(std::move(word));
        }
    }
    return words;
}

int SearchServer::ComputeAverageRating(const std::vector<int>& ratings)
{
    if (ratings.empty())
    {
        return 0;
    }
    int rating_sum = 0;
    for (const int rating : ratings)
    {
        rating_sum += rating;
    }
    return rating_sum / static_cast<int>(ratings.size());
}

SearchServer::QueryWord SearchServer::ParseQueryWord(std::string_view text) const
{
    if (text.empty())
    {
        throw std::invalid_argument("Query word is empty"s);
    }
    std::string_view word = text;
    bool is_minus = false;
    if (word[0] == '-')
    {
        is_minus = true;
        word = word.substr(1);
    }
    if (word.empty() || word[0] == '-' || !IsValidWord(word))
    {
        std::string temp{ word };
        throw std::invalid_argument("Word "s + temp + " is invalid"s);
    }
    return { word, is_minus, IsStopWord(word) };
}

SearchServer::Query SearchServer::ParseQuery(std::string_view text) const
{
    Query result;
    for (std::string_view word : SplitIntoWords(text))
    {
        auto query_word = ParseQueryWord(word);
        if (!query_word.is_stop)
        {
            if (query_word.is_minus)
            {
                result.minus_words.insert(std::move(query_word.data));
            }
            else
            {
                result.plus_words.insert(std::move(query_word.data));
            }
        }
    }
    return result;
}

double SearchServer::ComputeWordInverseDocumentFreq(std::string_view word) const
{
    std::string temp{ word };
    return std::log(GetDocumentCount() * 1.0 / word_to_document_freqs_.at(temp).size());
}

    
const std::map<std::string_view, double>& SearchServer::GetWordFrequencies(int document_id) const
{
    const auto it = id_document_.find(document_id);
    if (it == id_document_.end())
    {
        const auto it1 = id_document_.begin();
        return it1->second;
    }
    return it->second;
}

void SearchServer::RemoveDocument(int document_id)
{
    if (!documents_.count(document_id))
    {
        return;
    }
    if (documents_.count(document_id))
    {   
        auto it = std::find(std::execution::par, document_ids_.begin(), document_ids_.end(), document_id);
        if (it != document_ids_.end())
        {
            document_ids_.erase(it);
        }        

        const auto it1 = documents_.find(document_id);
        if (it1 != documents_.end())
        {
            documents_.erase(it1);
        }

        const auto it2 = id_document_.find(document_id);
        if (it2 != id_document_.end())
        {
            id_document_.erase(it2);
        }

        for (auto it = word_to_document_freqs_.begin(); it != word_to_document_freqs_.end(); ++it)
        {
            auto &temp = *it;
            temp.second.erase(document_id);
            
        }
    }
}



