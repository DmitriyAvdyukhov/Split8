#pragma once
#include "search_server.h"
#include "remove_duplicates.h"



template<class S>
std::ostream& operator << (std::ostream& os, const std::set<S>& s) {
    os << "{";
    bool first = true;
    for (const auto& s_ : s) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << s_;
    }
    return os << "}";
}

template<class K, class V>
std::ostream& operator << (std::ostream& os, const std::map<K, V>& m) {
    os << "{";
    bool first = true;
    for (const auto& m_ : m) {
        if (!first) {
            os << ", ";
        }
        first = false;
        os << m_.first << ": " << m_.second;
    }
    return os << "}";
}

template <typename T, typename U>
void AssertEqual(const T& t, const U& u, const std::string& hint) {
    if (t != u) {
        std::cerr << std::boolalpha;
        std::cerr << "ASSERT_EQUAL"s /*<< t << ", "s << u */ << " failed: "s;
        std::cerr << t << " != "s << u << "."s;
        if (!hint.empty()) {
            std::cerr << " Hint: "s << hint;
        }
        std::cerr << std::endl;
        abort();
    }
}

template<class TestFunc>
void RunTestImpl(TestFunc func, const std::string& name_test) {
    func();
    std::cerr << name_test << " Ok"s << std::endl;
}

void Assert(bool value, const std::string& hint);

void TestExcludeStopWordsFromAddedDocumentContent();

void TestExcludeMinusWords();

void TestCorrectMatching();

void TestCorrectSort();

void TestCorrectRating();

void TestResultFromPredicate();

void TestSearchDocumentByStatus();

void TestCorrectRelevance();

void TestRemoveDocument();

void TestRemoveDuplicates();

void TestSearchServer();

#define RUN_TEST(func)  RunTestImpl(func, #func)
