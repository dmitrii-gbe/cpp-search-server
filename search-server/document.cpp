#include "document.h"

std::ostream& operator<<(std::ostream& out, const Document& document) {
    out << "{ document_id = ";
    out << document.id;
    out << ", relevance = ";
    out << document.relevance;
    out << ", rating = ";
    out << document.rating;
    out << " }";
       return out;
    }
