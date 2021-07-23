#include "paginator.h"
#include "document.h"
#include "search_server.h"
#include "read_input_functions.h"
#include "request_queue.h"
#include "string_processing.h"
#include "remove_duplicates.h"
#include "test_example_functions.h"
#include "process_queries.h"
#include "log_duration.h"


#include <random>

using namespace std;

string GenerateWord(mt19937& generator, int max_length) 
{
    const int length = uniform_int_distribution(1, max_length)(generator);
    string word;
    word.reserve(length);
    for (int i = 0; i < length; ++i) 
    {
        word.push_back(static_cast<char>(uniform_int_distribution(static_cast<int>('a'), static_cast<int>('z'))(generator)));
    }
    return word;
}

vector<string> GenerateDictionary(mt19937& generator, int word_count, int max_length)
{
    vector<string> words;
    words.reserve(word_count);
    for (int i = 0; i < word_count; ++i)
    {
        words.push_back(GenerateWord(generator, max_length));
    }
    words.erase(unique(words.begin(), words.end()), words.end());
    return words;
}

string GenerateQuery(mt19937& generator, const vector<string>& dictionary, int word_count, double minus_prob = 0)
{
    string query;
    for (int i = 0; i < word_count; ++i) 
    {
        if (!query.empty()) {
            query.push_back(' ');
        }
        if (uniform_real_distribution<>(0, 1)(generator) < minus_prob)
        {
            query.push_back('-');
        }
        query += dictionary[uniform_int_distribution<int>(0, dictionary.size() - 1)(generator)];
    }
    return query;
}

vector<string> GenerateQueries(mt19937& generator, const vector<string>& dictionary, int query_count, int max_word_count)
{
    vector<string> queries;
    queries.reserve(query_count);
    for (int i = 0; i < query_count; ++i) 
    {
        queries.push_back(GenerateQuery(generator, dictionary, max_word_count));
    }
    return queries;
}

template <typename ExecutionPolicy>
void Test(string_view mark, const SearchServer& search_server, const vector<string>& queries, ExecutionPolicy&& policy)
{
    LOG_DURATION("mark"s);
    double total_relevance = 0;
    for (const string_view query : queries)
    {
        for (const auto& document : search_server.FindTopDocuments(policy, query)) 
        {
            total_relevance += document.relevance;
        }
    }
    cout << total_relevance << endl;
}

#define TEST(policy) Test(#policy, search_server, queries, execution::policy)


int main() {

    //setlocale(LC_ALL, "Russian");
    //{
    //    SearchServer search_server("and with"s);

    //    int id = 0;
    //    for (
    //         const std::string& text : {
    //            "funny pet and nasty rat"s,
    //            "funny pet with curly hair"s,
    //            "funny pet and not very nasty rat"s,
    //            "pet with rat and rat and rat"s,
    //            "nasty rat with curly hair"s,
    //        }
    //        ) {
    //        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
    //    }

    //    const std::string query = "curly and funny -not"s;

    //    {
    //        const auto [words, status] = search_server.MatchDocument(query, 1);
    //        std::cout << words.size() << " words for document 1"s << std::endl;
    //        // 1 words for document 1
    //    }

    //    {
    //        const auto [words, status] = search_server.MatchDocument(std::execution::seq, query, 2);
    //        std::cout << words.size() << " words for document 2"s << std::endl;
    //        // 2 words for document 2
    //    }

    //    {
    //        const auto [words, status] = search_server.MatchDocument(std::execution::par, query, 3);
    //        std::cout << words.size() << " words for document 3"s << std::endl;
    //        // 0 words for document 3
    //    }
    //}

   

    //TestSearchServer();
    //std::cout << "Search server testing finished"s << std::endl; 
    //{
    //    SearchServer search_server("and with"s);

    //    int id = 0;
    //    for (const std::string& text :
    //        {
    //            "funny pet and nasty rat"s,
    //            "funny pet with curly hair"s,
    //            "funny pet and not very nasty rat"s,
    //            "pet with rat and rat and rat"s,
    //            "nasty rat with curly hair"s,
    //        })
    //    {
    //        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
    //    }

    //    const std::vector<std::string> queries =
    //    {
    //        "nasty rat -not"s,
    //        "not very funny nasty pet"s,
    //        "curly hair"s
    //    };
    //    id = 0;
    //    for (const auto& documents : ProcessQueries(search_server, queries))
    //    {
    //        std::cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << std::endl;
    //    }

    //    for (const Document& document : ProcessQueriesJoined(search_server, queries))
    //    {
    //        std::cout << "Document "s << document.id << " matched with relevance "s << document.relevance << std::endl;
    //    }

    //}


    //{
    //    SearchServer search_server("and with"s);

    //    int id = 0;
    //    for (
    //        const std::string& text : {
    //            "funny pet and nasty rat"s,
    //            "funny pet with curly hair"s,
    //            "funny pet and not very nasty rat"s,
    //            "pet with rat and rat and rat"s,
    //            "nasty rat with curly hair"s,
    //        }
    //        ) {
    //        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
    //    }

    //    const std::string query = "curly and funny"s;

    //    auto report = [&search_server, &query] {
    //        std::cout << search_server.GetDocumentCount() << " documents total, "s
    //            << search_server.FindTopDocuments(query).size() << " documents for query ["s << query << "]"s << std::endl;
    //    };

    //    report();
    //    // однопоточная версия
    //    search_server.RemoveDocument(5);
    //    report();
    //    // однопоточная версия
    //    search_server.RemoveDocument(std::execution::seq, 1);
    //    report();
    //    // многопоточная версия
    //    search_server.RemoveDocument(std::execution::par, 2);
    //    report();
    //}


    //{
    //    SearchServer search_server("and with"s);

    //    AddDocument(search_server, 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    //    AddDocument(search_server, 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
    //    AddDocument(search_server, 3, "funny pet with curly hair rat"s, DocumentStatus::ACTUAL, { 1, 2 });
    //    AddDocument(search_server, 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });
    //    AddDocument(search_server, 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
    //    AddDocument(search_server, 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });
    //    AddDocument(search_server, 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });
    //    AddDocument(search_server, 8, "pet with rat and rat and rat"s, DocumentStatus::ACTUAL, { 1, 2 });
    //    AddDocument(search_server, 9, "nasty rat with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });


    //    std::cout << "Before duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
    //    RemoveDuplicates(search_server);

    //    std::cout << "After duplicates removed: "s << search_server.GetDocumentCount() << std::endl;
    //}


    //{
    //    SearchServer search_server("and with"s);

    //    int id = 0;
    //    for (const std::string& text :
    //        {
    //            "funny pet and nasty rat"s,
    //            "funny pet with curly hair"s,
    //            "funny pet and not very nasty rat"s,
    //            "pet with rat and rat and rat"s,
    //            "nasty rat with curly hair"s,
    //        })
    //    {
    //        search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
    //    }

    //    const std::vector<std::string> queries =
    //    {
    //        "nasty rat -not"s,
    //        "not very funny nasty pet"s,
    //        "curly hair"s
    //    };
    //    id = 0;
    //    for (const auto& documents : ProcessQueries(search_server, queries))
    //    {
    //        std::cout << documents.size() << " documents for query ["s << queries[id++] << "]"s << std::endl;
    //    }

    //    for (const Document& document : ProcessQueriesJoined(search_server, queries))
    //    {
    //        std::cout << "Document "s << document.id << " matched with relevance "s << document.relevance << std::endl;
    //    }
    //}

    //{
    //    SearchServer search_server("and in at"s);

    //    RequestQueue request_queue(search_server);

    //    search_server.AddDocument(1, "curly cat curly tail"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    //    search_server.AddDocument(2, "curly dog and fancy collar"s, DocumentStatus::ACTUAL, { 1, 2, 3 });
    //    search_server.AddDocument(3, "big cat fancy collar "s, DocumentStatus::ACTUAL, { 1, 2, 8 });
    //    search_server.AddDocument(4, "big dog sparrow Eugene"s, DocumentStatus::ACTUAL, { 1, 3, 2 });
    //    search_server.AddDocument(5, "big dog sparrow Vasiliy"s, DocumentStatus::ACTUAL, { 1, 1, 1 });


    //    for (int i = 0; i < 1439; ++i)
    //    {
    //        request_queue.AddFindRequest("empty request"s);
    //    }

    //    request_queue.AddFindRequest("curly dog"s);

    //    request_queue.AddFindRequest("big collar"s);

    //    request_queue.AddFindRequest("sparrow"s);
    //    std::cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << std::endl;
    //}

    {
        SearchServer search_server("and with"s);
        int id = 0;
        for (
            const std::string& text : {
                "white cat and yellow hat"s,
                "curly cat curly tail"s,
                "nasty dog with big eyes"s,
                "nasty pigeon john"s,
            }
            ) {
            search_server.AddDocument(++id, text, DocumentStatus::ACTUAL, { 1, 2 });
        }
        std::cout << "ACTUAL by default:"s << std::endl;
        // последовательная версия
        for (const Document& document : search_server.FindTopDocuments("curly nasty cat"s)) {
            PrintDocument(document);
        }
        std::cout << "BANNED:"s << std::endl;
        // последовательная версия
        for (const Document& document : search_server.FindTopDocuments(std::execution::seq, "curly nasty cat"s, DocumentStatus::BANNED)) {
            PrintDocument(document);
        }
        std::cout << "Even ids:"s << std::endl;
        // параллельная версия
        for (const Document& document : search_server.FindTopDocuments(std::execution::par, "curly nasty cat"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })) {
            PrintDocument(document);
        }
    }


    /*{
        mt19937 generator;

        const auto dictionary = GenerateDictionary(generator, 1000, 10);
        const auto documents = GenerateQueries(generator, dictionary, 10'000, 70);

        SearchServer search_server(dictionary[0]);
        for (size_t i = 0; i < documents.size(); ++i) {
            search_server.AddDocument(i, documents[i], DocumentStatus::ACTUAL, { 1, 2, 3 });
        }

        const auto queries = GenerateQueries(generator, dictionary, 100, 70);

        TEST(seq);
        TEST(par);
    }*/
    return 0;
}



