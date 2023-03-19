#include "document.h"


using namespace std;

ostream& operator<<(ostream& out, const Document& document) {
    out << "{ document_id = "s;
    out << document.id;
    out << ", relevance = "s;
    out << document.relevance;
    out << ", rating = "s;
    out << document.rating;
    out << " }"s;
       return out;
    }
