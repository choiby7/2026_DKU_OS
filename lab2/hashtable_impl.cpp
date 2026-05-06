/*
*  DKU Operating System Lab (2026)
*      Lab2 (Concurrency Data Structure: Hash Table)
*      Student id : 32224626
*      Student name : 최범연
*      Date: 2026-05-04
*/


#include "hashtable_impl.h"
#include <cstdlib>
#include <vector>
#include <algorithm>

// DefaultHashTable
// DefaultHashTable 생성자
DefaultHashTable::DefaultHashTable(int num_buckets) : num_buckets_(num_buckets) {
    // 버킷 배열 할당 및 초기화 (모두 nullptr)
    buckets_ = new HTNode*[num_buckets_];
    for (int i = 0; i < num_buckets_; i++) {
        buckets_[i] = nullptr;
    }
}

// DefaultHashTable 소멸자
DefaultHashTable::~DefaultHashTable() {
    // 각 버킷의 체인 노드 전부 해제
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) {
            HTNode* tmp = node->next;
            delete node;
            node = tmp;
        }
    }
    delete[] buckets_;
}

// 해시 함수 : key를 버킷 수로 나눈 나머지
int DefaultHashTable::hash_func(int key) {
    return key % num_buckets_;
}

// 순회 함수 : 전체 버킷을 순회하여 key 오름차순으로 정렬 후 arr에 저장
// Hash Table은 삽입 순서를 보장하지 않으므로, std::sort로 정렬한다.
void DefaultHashTable::traversal(KVC* arr) {
    std::vector<KVC> tmp;
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) {
            KVC kvc;
            kvc.key     = node->key;
            kvc.value   = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);
            node = node->next;
        }
    }
    // key 기준 오름차순 정렬 (std::map 순회와 동일한 순서 보장)
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) {
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) {
        arr[i] = tmp[i];
    }
}

// HashTable (without lock)
// HashTable 생성자
HashTable::HashTable(int num_buckets) : DefaultHashTable(num_buckets) {}

// HashTable 소멸자
HashTable::~HashTable() {}

void HashTable::insert(int key, int value) {
    int hash = hash_func(key);

    // 기존 key가 있는지 탐색
    HTNode* node = buckets_[hash];
    while (node) {
        if (node->key == key) {
            // 이미 존재하면 value를 더하고 upd_cnt 증가
            node->value += value;
            node->upd_cnt++;
            return;
        }
        node = node->next;
    }

    // 존재하지 않으면 새 노드를 버킷 헤드에 삽입
    HTNode* new_node = new HTNode();
    new_node->key = key;
    new_node->value = value;
    new_node->upd_cnt = 0;
    new_node->next = buckets_[hash];
    buckets_[hash] = new_node;
}

int HashTable::lookup(int key) {
    int hash = hash_func(key);
    HTNode* node = buckets_[hash];
    while (node) {
        if (node->key == key) {
            return node->value;
        }
        node = node->next;
    }
    return 0;
}

void HashTable::remove(int key) {
    int hash = hash_func(key);
    HTNode* prev = nullptr;
    HTNode* node = buckets_[hash];
    while (node) {
        if (node->key == key) {
            if (prev) {
                prev->next = node->next;
            } else {
                buckets_[hash] = node->next;
            }
            delete node;
            return;
        }
        prev = node;
        node = node->next;
    }
}

void HashTable::traversal(KVC* arr) {
    std::vector<KVC> tmp;
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) {
            KVC kvc;
            kvc.key = node->key;
            kvc.value = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);
            node = node->next;
        }
    }
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) {
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) {
        arr[i] = tmp[i];
    }
}

// CoarseHashTable (coarse-grained lock)
// CoarseHashTable 생성자
CoarseHashTable::CoarseHashTable(int num_buckets) : DefaultHashTable(num_buckets) {
    pthread_mutex_init(&mutex_lock, nullptr);
}

// CoarseHashTable 소멸자
CoarseHashTable::~CoarseHashTable() {
    pthread_mutex_destroy(&mutex_lock);
}

void CoarseHashTable::insert(int key, int value) {
    pthread_mutex_lock(&mutex_lock);

    int hash = hash_func(key);
    // 기존 key가 있는지 탐색
    HTNode* node = buckets_[hash];
    while (node) {
        if (node->key == key) {
            // 이미 존재하면 value를 더하고 upd_cnt 증가
            node->value += value;
            node->upd_cnt++;
            pthread_mutex_unlock(&mutex_lock);
            return;
        }
        node = node->next;
    }

    // 존재하지 않으면 새 노드를 버킷 헤드에 삽입
    HTNode* new_node = new HTNode();
    new_node->key = key;
    new_node->value = value;
    new_node->upd_cnt = 0;
    new_node->next = buckets_[hash];
    buckets_[hash] = new_node;
    pthread_mutex_unlock(&mutex_lock);
}

int CoarseHashTable::lookup(int key) {
    pthread_mutex_lock(&mutex_lock);
    int hash = hash_func(key);
    HTNode* node = buckets_[hash];
    while (node) {
        if (node->key == key) {
            pthread_mutex_unlock(&mutex_lock);
            return node->value;
        }
        node = node->next;
    }
    pthread_mutex_unlock(&mutex_lock);
    return 0;
}

void CoarseHashTable::remove(int key) {
    pthread_mutex_lock(&mutex_lock);
    int hash = hash_func(key);
    HTNode* prev = nullptr;
    HTNode* node = buckets_[hash];
    while (node) {
        if (node->key == key) {
            if (prev) {
                prev->next = node->next;
            } else {
                buckets_[hash] = node->next;
            }
            delete node;
            pthread_mutex_unlock(&mutex_lock);
            return;
        }
        prev = node;
        node = node->next;
    }
    pthread_mutex_unlock(&mutex_lock);
}

void CoarseHashTable::traversal(KVC* arr) {
    pthread_mutex_lock(&mutex_lock);
    std::vector<KVC> tmp;
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) {
            KVC kvc;
            kvc.key = node->key;
            kvc.value = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);
            node = node->next;
        }
    }
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) {
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) {
        arr[i] = tmp[i];
    }
    pthread_mutex_unlock(&mutex_lock);
}
// FineHashTable (fine-grained lock)
// FineHashTable 생성자
FineHashTable::FineHashTable(int num_buckets) : DefaultHashTable(num_buckets) {
    // 버킷 수만큼 뮤텍스 배열 할당 및 초기화
    bucket_locks = new pthread_mutex_t[num_buckets_];
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_init(&bucket_locks[i], nullptr);
    }
}

// FineHashTable 소멸자
FineHashTable::~FineHashTable() {
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_destroy(&bucket_locks[i]);
    }
    delete[] bucket_locks;
}

void FineHashTable::insert(int key, int value) {

}

int FineHashTable::lookup(int key) {
    // 구현
}

void FineHashTable::remove(int key) {
    // 구현
}

void FineHashTable::traversal(KVC* arr) {
    // 구현
}
