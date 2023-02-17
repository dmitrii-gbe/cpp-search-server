// -------- Начало модульных тестов поисковой системы ----------

// Тест проверяет, что поисковая система исключает стоп-слова при добавлении документов
void TestExcludeStopWordsFromAddedDocumentContent() {
    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("in"s);
        ASSERT_EQUAL(found_docs.size(), 1u);
        const Document& doc0 = found_docs[0];
        ASSERT_EQUAL(doc0.id, doc_id);
    }

    {
        SearchServer server;
        server.SetStopWords("in the"s);
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        ASSERT_HINT(server.FindTopDocuments("in"s).empty(),
                    "Stop words must be excluded from documents"s);
    }
}

void TestExcludeDocumentsWithMinusWords() {

    const int doc_id = 42;
    const string content = "cat in the city"s;
    const vector<int> ratings = {1, 2, 3};
    {
        SearchServer server;
        server.AddDocument(doc_id, content, DocumentStatus::ACTUAL, ratings);
        const auto found_docs = server.FindTopDocuments("cat in -the"s);
        ASSERT(found_docs.size() == 0);
        const auto found_docs1 = server.FindTopDocuments("cat in the"s);
        ASSERT(found_docs1.size() == 1);
        const Document& doc2 = found_docs1[0];
        ASSERT_EQUAL(doc2.id, doc_id);
    }
}

void TestMatching() {

        {
            SearchServer server;
            server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
            const auto [words0, status0] = server.MatchDocument("белый модный пес"s, 0);
            vector<string> v0 = {"белый"s, "модный"s};
            ASSERT_EQUAL(words0, v0);
            const auto [words1, status1] = server.MatchDocument("белый модный -ошейник"s, 0);
            ASSERT(words1.empty());
        }

}

void TestCorrectSorting() {

    {
            SearchServer search_server;
            search_server.SetStopWords("и в на"s);
            search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
            search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
            search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
            search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
               vector<double> relevance_v;
               for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
                    relevance_v.push_back(document.relevance);
               }
               int m = 0;
               for (size_t i = 0; i + 1 < relevance_v.size(); ++i){
                if (relevance_v[i] < relevance_v[i + 1]) {
                    ++m;
                }
               }
              ASSERT_EQUAL(m, 0);
     }
}

void TestCorrectRating() {
    {
        SearchServer search_server;
        vector<int> v = {7, 2, 7};
        int doc_id = 1;
        search_server.AddDocument(doc_id, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, v);
        ASSERT_EQUAL(search_server.FindTopDocuments("пушистый ухоженный кот"s).size(), 1);
        int rating = search_server.FindTopDocuments("пушистый ухоженный кот"s)[0].rating; 
        ASSERT_EQUAL(rating, ((7 + 2 + 7) / 3));
    }
}

void TestFilteringByPredicate() {

    {
      SearchServer search_server;
            search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
            search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
            search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
            search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});

                auto documents = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; });
                ASSERT(documents.size() == 2);
                ASSERT(documents[0].id % 2 == 0);
                ASSERT(documents[1].id % 2 == 0);
            
                documents = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id > 2; });
                ASSERT(documents.size() == 1);
                ASSERT(documents[0].id > 2 );
             
                documents = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL; });
                ASSERT(documents.size() == 3);
                ASSERT(documents[0].id != 3);
                ASSERT(documents[1].id != 3);
                ASSERT(documents[2].id != 3);
            
 
                documents = search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return rating > 0;});
                ASSERT(documents.size() == 3);
                ASSERT(documents[0].id != 2);
                ASSERT(documents[1].id != 2);
                ASSERT(documents[2].id != 2);
    }
}

void TestFilteringByStatus() {

    {
      SearchServer search_server;
            search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
            search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
            search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
            search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
            auto documents = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED);
                ASSERT(documents.size() == 1);
                ASSERT(documents[0].id == 3);
            

            documents = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::ACTUAL);
                ASSERT(documents.size() == 3);
                ASSERT(documents[0].id != 3);
                ASSERT(documents[1].id != 3);
                ASSERT(documents[2].id != 3);    
    }
}

void TestCalculatingCorrectRelevance() {

    {
      SearchServer search_server;
            search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
            search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
            search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
            search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
            //Четыре документа добавлены, чтобы релевантность искомого не была нулём. Ноль можно получить по-разному, а ln(2)/3 только одним правильным способом.

           auto document = search_server.FindTopDocuments("пушистый ухоженный кот"s, DocumentStatus::BANNED)[0];
                ASSERT(document.relevance == log(2)/3);
            
         
    }
}





/*
Разместите код остальных тестов здесь
*/

// Функция TestSearchServer является точкой входа для запуска тестов
void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeDocumentsWithMinusWords);
    RUN_TEST(TestMatching);
    RUN_TEST(TestCorrectSorting);
    RUN_TEST(TestCorrectRating);
    RUN_TEST(TestFilteringByPredicate);
    RUN_TEST(TestFilteringByStatus);
    RUN_TEST(TestCalculatingCorrectRelevance);
    // Не забудьте вызывать остальные тесты здесь
}

// --------- Окончание модульных тестов поисковой системы -----------