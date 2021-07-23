#include "document.h"
#include "log_duration.h" 


void PrintDocument(const Document& document)
{
    std::cout << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s 
        << std::endl;
}

void PrintMatchDocumentResult(int document_id, const std::vector<std::string_view>& words, DocumentStatus status)
{
    std::cout << "{ "s
        << "document_id = "s << document_id << ", "s
        << "status = "s << static_cast<int>(status) << ", "s
        << "words ="s;
    for (std::string_view word : words)
    {
        std::cout << ' ' << word;
    }
    std::cout << "}"s << std::endl;
}

void AddDocument(SearchServer& search_server, int document_id, const std::string& document, DocumentStatus status,
    const std::vector<int>& ratings) 
{
    try
    {
        search_server.AddDocument(document_id, document, status, ratings);
    }
    catch (const std::invalid_argument& e) 
    {
        std::cerr << "Ошибка добавления документа "s << document_id << ": "s << e.what() << std::endl;
    }
}


void FindTopDocuments(const SearchServer& search_server,  const std::string& raw_query)
{
    LOG_DURATION_STREAM("Operation time"s, std::cerr);
    std::cout << "Результаты поиска по запросу: "s << raw_query << std::endl;
    try 
    {
        for (const Document& document : search_server.FindTopDocuments( raw_query)) 
        {
            PrintDocument(document);
            std::cout << std::endl;
        }
    }
    catch (const std::invalid_argument& e) 
    {
        std::cerr << "Ошибка поиска: "s << e.what() << std::endl;
    }
}

void MatchDocuments(const SearchServer& search_server, const std::string& query) 
{
    LOG_DURATION_STREAM("Operation time"s, std::cerr);
    try
    {
        std::cout << "Матчинг документов по запросу: "s << query << std::endl;
        const int document_count = search_server.GetDocumentCount();
        for (int index = 0; index < document_count; ++index) 
        {
            const auto it = find(search_server.begin(), search_server.end(), index);
            if (it != search_server.end())
            {
                const int document_id = *it;
                //const int document_id = search_server.GetDocumentId(index);
                const auto [words, status] = search_server.MatchDocument(query, document_id);
                PrintMatchDocumentResult(document_id, words, status);
            }
        }
    }
    catch (const std::invalid_argument& e) 
    {
        std::cerr << "Ошибка матчинга документов на запрос "s << query << ": "s << e.what() << std::endl;
    }
}

std::ostream& operator <<(std::ostream& out, const Document& document)
{
    out << "{ "s
        << "document_id = "s << document.id << ", "s
        << "relevance = "s << document.relevance << ", "s
        << "rating = "s << document.rating << " }"s;
    return out;
}

