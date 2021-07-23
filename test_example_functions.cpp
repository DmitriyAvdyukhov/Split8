#include <iostream>

#include "test_example_functions.h"

void Assert(bool value, const std::string& hint) {
    if (!value) {
        std::cerr << "Assertion failed. "s;
        if (!hint.empty()) {
        std::cerr << "Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const std::string content = "cat in the city"s;
    const std::vector<int> ratings = { 1, 2, 3 };
    {
        SearchServer server(" the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        AssertEqual(found_docs.size(), 1, "Search doc by word method size()"s);
        const Document& doc0 = found_docs[0];
        Assert((doc0.id == doc_id), "Search doc by word in struct Document"s);
    }
    {
        SearchServer server ("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        Assert((server.FindTopDocuments("in"s).empty()), "Search by stop word = null"s);
    }
}

void TestExcludeMinusWords() {
    SearchServer search_server("in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document> document = search_server.FindTopDocuments("-пушистый -ухоженный -кот"s);
    Assert((document.empty()), "Search doc by minus word return 0"s);

    document = search_server.FindTopDocuments("-пушистый -ухоженный кот"s);
    Assert((document[0].id == 0 && (document.size() == 1)), "Search doc by minus word return 0"s);

    document = search_server.FindTopDocuments("пушистый ухоженный кот"s);
    Assert((document.size() == 3), "Search doc by minus word return 0"s);
}

void TestCorrectMatching() {
    SearchServer search_server("in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::string query = "-пушистый -ухоженный кот"s;
    std::vector<Document> document = search_server.FindTopDocuments("-пушистый -ухоженный кот"s);

    auto a = search_server.MatchDocument(query, 1);
    std::vector < std::string_view> vect = std::get<std::vector<std::string_view>>(a);

    Assert((vect.empty()), "For minus word Return vector matchdocument empty() for id = 1"s);

    auto b = search_server.MatchDocument(query, 0);
    vect = std::get<std::vector<std::string_view>>(b);

    Assert((!vect.empty() && vect.size() == 1), "For minus word Return vector matchdocument empty() for id = 0"s);
    AssertEqual(count(vect.begin(), vect.end(), "кот"s), 1, "number of words in Document"s);
}

void TestCorrectSort() {
    SearchServer search_server("in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document> documents = search_server.FindTopDocuments("пушистый ухоженный кот"s);

    double relevance(0);
    for (const auto& document : documents) {
        if (relevance != 0) Assert((document.relevance <= relevance), "Sort by relevance"s);
        relevance = document.relevance;
    }
    Assert((documents[0].relevance > documents[1].relevance && documents[0].relevance > documents[2].relevance), "Sort by relevance  id0 is greater than id1, id2 "s);
    Assert((documents[1].relevance > documents[2].relevance && documents[0].relevance > documents[1].relevance), "Sort by relevance id1 is greater than id2 and less than id0"s);
}

void TestCorrectRating() {
    SearchServer search_server("in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document> documents = search_server.FindTopDocuments("пушистый ухоженный кот"s);
    Assert((documents[0].rating != documents[1].rating
        && documents[0].rating != documents[2].rating
        && documents[1].rating != documents[2].rating), "ratings are unequal"s);

    Assert(documents[0].rating > documents[1].rating, "rating id1 is graeter than rating id2"s);
    AssertEqual(documents[0].rating, 5, "For id = 0"s);
    AssertEqual(documents[1].rating, -1, "For id = 1"s);
    AssertEqual(documents[2].rating, 2, "For id = 2"s);

}

void TestResultFromPredicate() {

    SearchServer search_server("in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    auto predicate = [](int id, DocumentStatus status, int rating) { return id == 1; };
    std::vector<Document> documents = search_server.FindTopDocuments(std::execution::seq, "пушистый ухоженный кот"s, predicate);

    AssertEqual(documents[0].id, 1, "documents[0] has id = 1");
    AssertEqual(documents.size(), 1, "documents has a size = 1"s);
    Assert(!(documents[0].id == 2), "documents[0] has no id = 2");
}

void TestSearchDocumentByStatus() {
    SearchServer search_server("in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::REMOVED, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document> documents = search_server.FindTopDocuments(std::execution::seq, "пушистый ухоженный кот"s, DocumentStatus::REMOVED);

    AssertEqual(documents.size(), 1, "documents has a size = 1 with DocumentStatus::REMOVED"s);
    AssertEqual(documents[0].id, 1, "document with DocumentStatus::REMOVED has id = 1");
    Assert(!(documents[0].id == 2), "document with DocumentStatus::REMOVED has no id = 2");

    std::vector<Document> documents1 = search_server.FindTopDocuments(std::execution::par, "пушистый ухоженный кот"s, DocumentStatus::ACTUAL);

    AssertEqual(documents1.size(), 2, "documents has a size = 2 with DocumentStatus::ACTUAL"s);
    Assert((documents1[0].id == 2 && documents1[1].id == 0), "document with DocumentStatus::ACTUAL has  id = 2 and d = 0"s);
    Assert(!(documents1[0].id == 1 && documents1[1].id == 3), "document with DocumentStatus::ACTUAL has no id = 1 and id = 3"s);
}

void TestCorrectRelevance() {
    SearchServer search_server("in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    std::vector<Document> documents = search_server.FindTopDocuments("пушистый ухоженный кот"s);

    Assert((std::abs(documents[2].relevance) == std::abs(0.13862943611198905)), "relevance for document.id=0"s);
    Assert((std::abs(documents[0].relevance) == std::abs(0.86643397569993164)), "relevance for document.id=1"s);
    Assert((std::abs(documents[1].relevance) == std::abs(0.17328679513998632)), "relevance for document.id=2"s);

    const double eps = 1e-6;
    Assert((std::abs(documents[2].relevance - 0.138629) < eps), "correct relevance for document.id=0"s);
    Assert((std::abs(documents[0].relevance - 0.866434) < eps), "correct relevance for document.id=1"s);
    Assert((std::abs(documents[1].relevance - 0.173287) < eps), "correct relevance for document.id=2"s);
}

void TestRemoveDocument()
{
    SearchServer search_server("in the"s);
    search_server.AddDocument(0, "белый кот и модный ошейник"s, DocumentStatus::ACTUAL, { 8, -3 });
    search_server.AddDocument(1, "пушистый кот пушистый хвост"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, { 5, -12, 2, 1 });
    search_server.AddDocument(3, "ухоженный скворец евгений"s, DocumentStatus::BANNED, { 9 });

    search_server.RemoveDocument(1);
    Assert((search_server.GetDocumentCount() == 3), "size map == 3"s);
    std::map<std::string_view, double> test = search_server.GetWordFrequencies(1);
    Assert((test.size() == 0), "return empty map.second id_document_"s);
    test = search_server.GetWordFrequencies(3);
    Assert((test["ухоженный"s] == 0.33333333333333331), "before remove"s);
    search_server.RemoveDocument(3);
    test = search_server.GetWordFrequencies(3);
    Assert((test.count("ухоженный"s) == 0), "after remove document id3"s);
    Assert((distance(search_server.begin(), search_server.end()) == 2), "size vector document_ids_ is equai 3"s);
}

void TestRemoveDuplicates()
{
    SearchServer search_server("and with"s);
    search_server.AddDocument( 1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, { 7, 2, 7 });
    search_server.AddDocument( 2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });   
    search_server.AddDocument( 3, "funny pet with curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });   
    search_server.AddDocument( 4, "funny pet and curly hair"s, DocumentStatus::ACTUAL, { 1, 2 });    
    search_server.AddDocument( 5, "funny funny pet and nasty nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });    
    search_server.AddDocument( 6, "funny pet and not very nasty rat"s, DocumentStatus::ACTUAL, { 1, 2 });   
    search_server.AddDocument( 7, "very nasty rat and not very funny pet"s, DocumentStatus::ACTUAL, { 1, 2 });

    std::map<std::string_view, double>test = search_server.GetWordFrequencies(5);
    std::map<std::string_view, double>test1 = search_server.GetWordFrequencies(1);
    Assert((test.size() == test1.size()), "size id1 and id5 are the same"s);

    test = search_server.GetWordFrequencies(2);
    test = search_server.GetWordFrequencies(3);
    Assert((test.size() == test1.size()), "size id2 and id3 are the same"s);

    RemoveDuplicates(search_server);

    Assert((search_server.GetDocumentCount() == 3), "size documents_ids_ is equal 5"s);
    test = search_server.GetWordFrequencies(3);
    Assert((test.size() == 0), "return empty map.second id_document_"s);
    test = search_server.GetWordFrequencies(2);
    Assert((test["funny"s] == 0.25), "document id2 present"s);
    
}

void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeMinusWords);
    RUN_TEST(TestCorrectMatching);
    RUN_TEST(TestCorrectSort);
    RUN_TEST(TestCorrectRating);
    RUN_TEST(TestResultFromPredicate);
    RUN_TEST(TestSearchDocumentByStatus);
    RUN_TEST(TestCorrectRelevance);
    RUN_TEST(TestRemoveDocument);
    RUN_TEST(TestRemoveDuplicates);
}

