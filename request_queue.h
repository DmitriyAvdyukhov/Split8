#pragma once
#include <stack>

#include "read_input_functions.h"
#include "search_server.h"

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server); 

    template <typename DocumentPredicate, class ExecutionPolicy>
    std::vector<Document> AddFindRequest(ExecutionPolicy&& policy, const std::string& raw_query, DocumentPredicate document_predicate);

    template <class ExecutionPolicy>
    std::vector<Document>  AddFindRequest(ExecutionPolicy&& policy, const std::string& raw_query, DocumentStatus status);

   
    std::vector<Document>  AddFindRequest( const std::string& raw_query);

    int GetNoResultRequests() const;
    

private:
    struct QueryResult
    {
        std::vector<Document> found_documents;
        bool empty_docs = false;
    };

    std::deque<QueryResult> requests_;
    const static int sec_in_day_ = 1440;
    const SearchServer& server;
    int num_of_empty = 0;
};

template <typename DocumentPredicate, class ExecutionPolicy>
std::vector<Document> RequestQueue::AddFindRequest(ExecutionPolicy&& policy, const std::string& raw_query, DocumentPredicate document_predicate)
{
    if (!requests_.empty()) 
    {
        while (requests_.size() >= sec_in_day_)
        {
            if (requests_.front().empty_docs)
            {
                num_of_empty -= 1;
            }
            requests_.pop_front();
        }
    }

    QueryResult result;
    result.found_documents = server.FindTopDocuments(policy, raw_query, document_predicate);
    if (result.found_documents.empty())
    {
        ++num_of_empty;
        result.empty_docs = true;
    }
    requests_.push_back(result);
    return result.found_documents;
}

template <class ExecutionPolicy>
std::vector<Document>  RequestQueue::AddFindRequest(ExecutionPolicy&& policy, const std::string& raw_query, DocumentStatus status)
{
    if (!requests_.empty())
    {
        while (requests_.size() >= sec_in_day_)
        {
            if (requests_.front().empty_docs)
            {
                num_of_empty -= 1;
            }
            requests_.pop_front();
        }
    }
    QueryResult result;
    result.found_documents = server.FindTopDocuments(policy, raw_query, status);
    if (result.found_documents.empty())
    {
        ++num_of_empty;
        result.empty_docs = true;
    }
    requests_.push_back(result);
    return result.found_documents;
}



