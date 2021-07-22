#include "paginator.h"
#include "document.h"
#include "search_server.h"
#include "read_input_functions.h"
#include "request_queue.h"
#include "string_processing.h"
#include "remove_duplicates.h"
#include "test_example_functions.h"
#include "process_queries.h"







int main() {

    setlocale(LC_ALL, "Russian");
    {
        SearchServer search_server1("and with"s);

        int id = 0;
        for (
             const std::string& text : {
                "funny pet and nasty rat"s,
                "funny pet with curly hair"s,
                "funny pet and not very nasty rat"s,
                "pet with rat and rat and rat"s,
                "nasty rat with curly hair"s,
            }
            ) {
            search_server1.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
        }

        const std::string query = "curly and funny -not"s;

        {
            const auto [words, status] = search_server1.MatchDocument(query, 1);
            std::cout << words.size() << " words for document 1"s << std::endl;
            // 1 words for document 1
        }

        {
            const auto [words, status] = search_server1.MatchDocument(std::execution::seq, query, 2);
            std::cout << words.size() << " words for document 2"s << std::endl;
            // 2 words for document 2
        }

        {
            const auto [words, status] = search_server1.MatchDocument(std::execution::par, query, 3);
            std::cout << words.size() << " words for document 3"s << std::endl;
            // 0 words for document 3
        }
    }

   

    TestSearchServer();
    std::cout << "Search server testing finished"s << std::endl; 
    {
        SearchServer search_server1("and with"s);

        int id = 0;
        for (const std::string& text :
            {
                "funny pet and nasty rat"s,
                "funny pet with curly hair"s,
                "funny pet and not very nasty rat"s,
                "pet with rat and rat and rat"s,
                "nasty rat with curly hair"s,
            })
        {
            search_server1.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
        }

        const std::vector<std::string> queries =
        {
            "nasty rat -not"s,
            "not very funny nasty pet"s,
            "curly hair"s
        };
        id = 0;
        for (const auto& documents : ProcessQueries(search_server1, queries))
        {
            std::cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << std::endl;
        }

        for (const Document& document : ProcessQueriesJoined(search_server1, queries))
        {
            std::cout << "Document "s << document.id << " matched with relevance "s << document.relevance << std::endl;
        }

    }
    {
        SearchServer search_server("and with"s);

        int id = 0;
        for (
            const std::string& text : {
                "funny pet and nasty rat"s,
                "funny pet with curly hair"s,
                "funny pet and not very nasty rat"s,
                "pet with rat and rat and rat"s,
                "nasty rat with curly hair"s,
            }
            ) {
            search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
        }

        const std::string query = "curly and funny"s;

        auto report = [&search_server, &query] {
            std::cout << search_server.GetDocumentCount() << " documents total, "s
                << search_server.FindTopDocuments(query).size() << " documents for query ["s << query << "]"s << std::endl;
        };

        report();
        // однопоточная версия
        search_server.RemoveDocument(5);
        report();
        // однопоточная версия
        search_server.RemoveDocument(std::execution::seq, 1);
        report();
        // многопоточная версия
        search_server.RemoveDocument(std::execution::par, 2);
        report();
    }
    {
        SearchServer search_server2("and with"s);

        AddDocument(search_server2, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
        AddDocument(search_server2, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
        AddDocument(search_server2, 3, "funny pet with curly hair rat"s, DocumentStatus::ACTUAL, { 1, 2 });
        AddDocument(search_server2, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
        AddDocument(search_server2, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
        AddDocument(search_server2, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
        AddDocument(search_server2, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });
        AddDocument(search_server2, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });
        AddDocument(search_server2, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });


        std::cout << "Before duplicates removed: "s << search_server2.GetDocumentCount() << std::endl;
        RemoveDuplicates(search_server2);

        std::cout << "After duplicates removed: "s << search_server2.GetDocumentCount() << std::endl;
    }
    
    SearchServer search_server3("and with"s);

    int id = 0;
    for (const std::string& text :
        {
            "funny pet and nasty rat"s,
            "funny pet with curly hair"s,
            "funny pet and not very nasty rat"s,
            "pet with rat and rat and rat"s,
            "nasty rat with curly hair"s,
        })
    {
        search_server3.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
    }

    const std::vector<std::string> queries =
    {
        "nasty rat -not"s,
        "not very funny nasty pet"s,
        "curly hair"s
    };
    id = 0;
    for (const auto& documents : ProcessQueries(search_server3, queries))
    {
        std::cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << std::endl;
    }

    for (const Document& document : ProcessQueriesJoined(search_server3, queries))
    {
        std::cout << "Document "s << document.id << " matched with relevance "s << document.relevance << std::endl;
    }

    SearchServer search_server4("and in at"s);

    RequestQueue request_queue(search_server4);

    search_server4.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server4.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    search_server4.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    search_server4.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    search_server4.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });


    for (int i = 0; i < 1439; ++i)
    {
        request_queue.AddFindRequest("empty request"s);
    }

    request_queue.AddFindRequest("curly dog"s);

    request_queue.AddFindRequest("big collar"s);

    request_queue.AddFindRequest("sparrow"s);
    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;


    return 0;
}



