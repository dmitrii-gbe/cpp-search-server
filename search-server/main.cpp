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
                if ((relevance_v[i] - relevance_v[i+1]) < 0) {
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
        int sum = 0;
        search_server.AddDocument(doc_id, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, v);
        for (const auto& v_ : v){
            sum += v_;
        }
        int average = sum / static_cast<int>(v.size());
                int rating;
               for (const Document& document : search_server.FindTopDocuments("пушистый ухоженный кот"s)) {
                   rating = document.rating;
               }
            ASSERT_EQUAL(rating, average);
    }
}

void TestFilter() {

    {
      SearchServer search_server;
            search_server.AddDocument(0, "белый кот и модный ошейник"s,        DocumentStatus::ACTUAL, {8, -3});
            search_server.AddDocument(1, "пушистый кот пушистый хвост"s,       DocumentStatus::ACTUAL, {7, 2, 7});
            search_server.AddDocument(2, "ухоженный пёс выразительные глаза"s, DocumentStatus::ACTUAL, {5, -12, 2, 1});
            search_server.AddDocument(3, "ухоженный скворец евгений"s,         DocumentStatus::BANNED, {9});
            int y = 0;
            for (const auto& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id % 2 == 0; })){
                if (document.id % 2 != 0){
                    ++y;
                }

            }
           ASSERT_EQUAL(y, 0);

           y = 0;
             for (const auto& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return document_id > 2; })){
                if (document.id <= 2 ){
                    ++y;
                }
            }
           ASSERT_EQUAL(y, 0);

                      y = 0;
             for (const auto& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return status == DocumentStatus::ACTUAL;})){
                if (document.id == 3){
                    ++y;
                }
            }
           ASSERT_EQUAL(y, 0);

                                 y = 0;
             for (const auto& document : search_server.FindTopDocuments("пушистый ухоженный кот"s, [](int document_id, DocumentStatus status, int rating) { return rating > 0;})){
                if (document.id == 2){
                    ++y;
                }
            }
           ASSERT_EQUAL(y, 0);

    }
}


void TestSearchServer() {
    RUN_TEST(TestExcludeStopWordsFromAddedDocumentContent);
    RUN_TEST(TestExcludeDocumentsWithMinusWords);
    RUN_TEST(TestMatching);
    RUN_TEST(TestCorrectSorting);
    RUN_TEST(TestCorrectRating);
    RUN_TEST(TestFilter);
}

// --------- Окончание модульных тестов поисковой системы -----------