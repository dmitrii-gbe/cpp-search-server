#pragma once
#include <utility>
#include <algorithm>
#include <vector>
#include <iostream>

template <typename Iterator>
class IteratorRange {
   public:
    IteratorRange(Iterator& first, Iterator& last){
        range_ = std::make_pair(first, last);
    }
    
    Iterator begin() const {
        return range_.first;
    }
        
    Iterator end() const {
        return range_.second;
    }
        
    size_t size() const {
        return std::distance(range_.first, range_.second);
    }
    
  private:
      std::pair<Iterator, Iterator> range_;
    
};


template <typename Iterator>
class Paginator {
    public:
     Paginator(const Iterator& range_begin, const Iterator& range_end, size_t& page_size){
         auto it = range_begin;
         while (it < range_end){
                 auto it1 = it;
                if (std::distance(it, range_end) > page_size){
                    std::advance(it, page_size);
                 }
                 else {
                    it = range_end;
                 }
                 IteratorRange range(it1, it);
                 sections_.push_back(range);
                 
        }
     }
          
    
    
    auto begin() const {
            return sections_.begin();
    }
            
    auto end() const {
            return sections_.end();
    }
            
    size_t size() const {
            return std::distance(sections_.begin(), sections_.end());
    }



private:

     std::vector<IteratorRange<Iterator>> sections_;  
};

template <typename Iterator>
std::ostream& operator<<(std::ostream& out, const IteratorRange<Iterator>& range){
    for (auto it = range.begin(); it != range.end(); ++it){
    out << *it;        
    }
  return out;
}

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
         return Paginator(begin(c), end(c), page_size);
}