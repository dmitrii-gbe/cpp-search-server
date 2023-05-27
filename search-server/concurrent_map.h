#pragma once
#include <algorithm>
#include <cstdlib>
#include <map>
#include <numeric>
#include <string>
#include <vector>
#include <mutex>


template <typename Key, typename Value>
class ConcurrentMap {
public:
    static_assert(std::is_integral_v<Key>, "ConcurrentMap supports only integer keys");

    struct Access {
    
        Access(std::mutex& m, std::map<Key, Value>& sub_map, const Key& key) : guard(m), ref_to_value(sub_map[key]){
        } 

        std::lock_guard<std::mutex> guard;
        Value& ref_to_value;
    };

    explicit ConcurrentMap(size_t bucket_count) : storage_(bucket_count), mutexes_(bucket_count), count_(bucket_count) {}

    Access operator[](const Key& key){
        auto index = static_cast<uint64_t>(key) % count_;
        return Access(mutexes_[index], storage_[index], key);
    }

    std::map<Key, Value> BuildOrdinaryMap(){
        std::map<Key, Value> result;
        for (size_t i = 0; i < storage_.size(); ++i){
            std::lock_guard<std::mutex> guard(mutexes_[i]);
            result.merge(storage_[i]);
        }        
        return result;
    }
    
    void Erase(const Key& key){
        auto index = static_cast<uint64_t>(key) % count_;
        storage_[index].erase(key);
    }

private:
    std::vector<std::map<Key, Value>> storage_;
    std::vector<std::mutex> mutexes_;
    int count_ = 0;
};
