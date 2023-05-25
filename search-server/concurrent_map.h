#pragma once
#include <algorithm>
#include <cstdlib>
#include <map>
#include <numeric>
#include <string>
#include <vector>
#include <mutex>


using namespace std::string_literals;


template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys"s);

    struct Access {
    
        Access(ConcurrentMap& p, const Key& key) : parent(p), index(static_cast<uint64_t>(key) % parent.count_), ref_to_value(parent.storage_[index][key]){
        } 
        
        ~Access(){
          parent.mutexes_[index].unlock();
        }
        ConcurrentMap& parent;
        int index = 0;
        Value& ref_to_value;
        
    };

    explicit ConcurrentMap(size_t bucket_count) : storage_(bucket_count), mutexes_(bucket_count), count_(bucket_count) {}

    Access operator[](const Key& key){
        mutexes_[static_cast<uint64_t>(key) % count_].lock();
        return Access(*this, key);
    }

    std::map<Key, Value> BuildOrdinaryMap(){
        for (size_t i = 0; i < mutexes_.size(); ++i){
            mutexes_[i].lock();
        }
        
        std::map<Key, Value> result;
        for (size_t i = 0; i < storage_.size(); ++i){
            result.merge(storage_[i]);
        }
        
        for (size_t i = 0; i < mutexes_.size(); ++i){
            mutexes_[i].unlock();
        }
        
        return result;
    }

private:
    std::vector<std::map<Key, Value>> storage_;
    std::vector<std::mutex> mutexes_;
    int count_ = 0;
};