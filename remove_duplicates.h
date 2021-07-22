#pragma once
#include "search_server.h"


void RemoveDuplicates(SearchServer& search_server);

//template<class T>
//void RemoveDuplicates(T, SearchServer& search_server)
//{
//    std::set<std::set<std::string>> dec{ {} };
//    std::set<std::string> s_temp1, s_temp2;
//    std::set<int> v_temp;
//    for (const int document_id : search_server)
//    {
//        s_temp2.clear();
//
//        for (auto it = search_server.GetWordFrequencies(document_id).begin();
//            it != search_server.GetWordFrequencies(document_id).end(); ++it)
//        {
//            s_temp2.emplace(it->first);
//        }
//
//        if (s_temp1 == s_temp2)
//        {
//            v_temp.emplace(document_id);
//        }
//
//        std::set<std::set<std::string>> dec1 = dec;
//        for (const auto& temp : dec1)
//        {
//            if (temp == s_temp2)
//            {
//                v_temp.emplace(document_id);
//            }
//            else
//            {
//                dec.emplace(s_temp2);
//                s_temp1 = s_temp2;
//            }
//        }
//    }
//
//    for (const int& id : v_temp)
//    {
//        search_server.RemoveDocument(T, id);
//        std::cout << "Found duplicate document id "s << id << std::endl;
//    }
//}
