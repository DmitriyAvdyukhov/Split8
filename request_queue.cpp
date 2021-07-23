#include "request_queue.h"

 RequestQueue::RequestQueue(const SearchServer& search_server) : server(search_server)
{}

 std::vector<Document>  RequestQueue::AddFindRequest(const std::string& raw_query)
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
     result.found_documents = server.FindTopDocuments(raw_query);
     if (result.found_documents.empty())
     {
         ++num_of_empty;
         result.empty_docs = true;
     }
     requests_.push_back(result);
     return result.found_documents;
 }
 
 int RequestQueue::GetNoResultRequests() const
 {
        return num_of_empty;
 }

