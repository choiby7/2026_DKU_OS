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
    buckets_ = new HTNode*[num_buckets_]; // 생성장 인자만큼의 bucket 첫 루트 노드 생성
    for (int i = 0; i < num_buckets_; i++) {
        buckets_[i] = nullptr; // nullptr로 초기화
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
    std::vector<KVC> tmp;                   // KVC의 임시 벡터
    for (int i = 0; i < num_buckets_; i++) {// for: 모든 버킷 순회
        HTNode* node = buckets_[i];         // 이번 순회의 버킷 설정
        while (node) {                      // while: node가 nullptr가 아닐 때 까지 순회. (bucket 연결리스트 모든 노드 순회)
            KVC kvc;                        // 각 순회에서 사용하는 kvc 구조체 인스턴스
            kvc.key     = node->key;        // 이하 각 k,v,c 값 할당
            kvc.value   = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);             // tmp에 하나씩 넣음
            node = node->next;              // 다음 노드로 이동 
        }
    }
    // key 기준 오름차순 정렬 (std::map 순회와 동일한 순서 보장)
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) { // tmp 벡터의 구조체를 key값에 따라 오름차순 정렬
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) { // tmp값을 arr에 복사
        arr[i] = tmp[i];
    }
}

// HashTable (without lock)
// HashTable 생성자
HashTable::HashTable(int num_buckets) : DefaultHashTable(num_buckets) {}

// HashTable 소멸자
HashTable::~HashTable() {}

void HashTable::insert(int key, int value) {
    int hash = hash_func(key); // hash 값 계산

    // 기존 key가 있는지 탐색
    HTNode* node = buckets_[hash];  // bucket의 node 받아오기
    while (node) {                  // while: node가 nullptr까지 순회 (삽입 위치 찾기)
        if (node->key == key) {         // 동일한 값이 이미 존재하면 value를 더하고 upd_cnt 증가
            node->value += value;   
            node->upd_cnt++;
            return;
        }
        node = node->next;          // 조건 부합 X: 다음 노드 이동
    }

    // 동일한 값이 존재하지 않으면 새 노드를 버킷 헤드에 삽입
    HTNode* new_node = new HTNode(); 
    new_node->key = key;                // KVC 형식에 맞게 구조체 초기화
    new_node->value = value;
    new_node->upd_cnt = 0;
    new_node->next = buckets_[hash];    // 기존 헤드를 새 노드의 next로
    buckets_[hash] = new_node;          // 새로 만든 노드를 헤드에 삽입
}

int HashTable::lookup(int key) {
    int hash = hash_func(key);          // 해시 값 계산
    HTNode* node = buckets_[hash];      // 해당 해시에 해당하는 버킷의 헤드 노드 추출
    while (node) {                      // while: 해당 버킷 모든 노드 순회
        if (node->key == key) {             // 만약 같은 값이 존재하면, 해당 키의 value 반환
            return node->value;
        }
        node = node->next;                  // 다음 노드 이동
    }
    return 0;                           // 없다면 0 반환
}

void HashTable::remove(int key) {
    int hash = hash_func(key);              // 해시 값 계산
    HTNode* prev = nullptr;                 // 제거를 위해 이전 노드 포인터 준비
    HTNode* node = buckets_[hash];          // 해시에 해당하는 버킷의 헤드노드 추출
    while (node) {                          // while: 해당 버킷 모든 노드 순회
        if (node->key == key) {                 // 만약 같은 값이 존재하면
            if (prev) {                             // 이전 노드가 존재하면,
                prev->next = node->next;                // 이전 노드의 next를 현재 노드의 next로 설정 
            } else {
                buckets_[hash] = node->next;        // 이전노드가 존재하지 않으면(헤드라면) 다음 노드를 헤드로 설정
            }
            delete node;                        // node 삭제
            return;
        }
        prev = node;                            // 다음 순회를 위해 prev에 현재 노드를, 현재 노드에 next 노드를 할당
        node = node->next;
    }
}

// 이미 부모(DefaultHashTable)에 동일한 구현이 존재하나, 일관성을 위해 오버라이드함.
// 순회 함수 : 전체 버킷을 순회하여 key 오름차순으로 정렬 후 arr에 저장
// Hash Table은 삽입 순서를 보장하지 않으므로, std::sort로 정렬한다.
void HashTable::traversal(KVC* arr) {
    std::vector<KVC> tmp;                   // KVC의 벡터 임시 자료구조
    for (int i = 0; i < num_buckets_; i++) {
        HTNode* node = buckets_[i];
        while (node) { // node가 nullptr가 아닐 때 까지 순회.
            KVC kvc;                        // 각 순회에서 사용하는 kvc 구조체 인스턴스
            kvc.key     = node->key;        // 이하 각 k,v,c 값 할당
            kvc.value   = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);             // tmp
            node = node->next;
        }
    }
    // key 기준 오름차순 정렬 (std::map 순회와 동일한 순서 보장)
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) { // tmp 벡터의 구조체를 key값에 따라 오름차순 정렬
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) { // tmp값을 arr에 복사
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
    pthread_mutex_lock(&mutex_lock);                         // 함수의 처음부터 끝까지 lock을 설정

    int hash = hash_func(key);                              // hash 값 계산
    // 기존 key가 있는지 탐색
    HTNode* node = buckets_[hash];                          // bucket의 node 받아오기
    while (node) {                                          // while: node가 nullptr까지 순회 (삽입 위치 찾기)
        if (node->key == key) {                                 // 동일한 값이 이미 존재하면 value를 더하고 upd_cnt 증가
            node->value += value;
            node->upd_cnt++;
            pthread_mutex_unlock(&mutex_lock);              // lock 해제 분기1: 이미 값이 존재하는 경우
            return;
        }
        node = node->next;                                  // 조건 부합 X: 다음 노드 이동
    }

    // 동일한 값이 존재하지 않으면 새 노드를 버킷 헤드에 삽입
    HTNode* new_node = new HTNode();
    new_node->key = key;                                    // KVC 형식에 맞게 구조체 초기화
    new_node->value = value;
    new_node->upd_cnt = 0;
    new_node->next = buckets_[hash];                        // 기존 헤드를 새 노드의 next로
    buckets_[hash] = new_node;                              // 새로 만든 노드를 헤드에 삽입
    pthread_mutex_unlock(&mutex_lock);                      // lock 해제 분기2: 값이 존재하지 않아 새로운 노드 삽입하는 경우
}

int CoarseHashTable::lookup(int key) {
    pthread_mutex_lock(&mutex_lock);                        // 함수 전체적으로 lock 설정
    int hash = hash_func(key);                              // 해시 값 계산
    HTNode* node = buckets_[hash];                          // 해당 해시에 해당하는 버킷의 헤드 노드 추출
    while (node) {                                          // while: 해당 버킷 모든 노드 순회
        if (node->key == key) {                                 // 만약 같은 값이 존재하면, 해당 키의 value 반환
            int result = node->value;                       // 락 보유 중 값을 지역변수에 복사 (use-after-unlock 방지)
            pthread_mutex_unlock(&mutex_lock);              // lock 해제 분기1: 찾는 값 존재, 값 반환.
            return result;
        }
        node = node->next;                                  // 다음 노드 이동
    }
    pthread_mutex_unlock(&mutex_lock);                      // lock 해제 분기2: 찾는 값이 없으면, 락 해제하고 0 반환
    return 0;                                               // 없다면 0 반환
}

void CoarseHashTable::remove(int key) {
    pthread_mutex_lock(&mutex_lock);                        // 함수 전체적으로 lock 설정
    int hash = hash_func(key);                              // 해시 값 계산
    HTNode* prev = nullptr;                                 // 제거를 위해 이전 노드 포인터 준비
    HTNode* node = buckets_[hash];                          // 해시에 해당하는 버킷의 헤드노드 추출
    while (node) {                                          // while: 해당 버킷 모든 노드 순회
        if (node->key == key) {                                 // 만약 같은 값이 존재하면
            if (prev) {                                             // 이전 노드가 존재하면,
                prev->next = node->next;                                // 이전 노드의 next를 현재 노드의 next로 설정
            } else {
                buckets_[hash] = node->next;                        // 이전노드가 존재하지 않으면(헤드라면) 다음 노드를 헤드로 설정
            }
            delete node;                                        // node 삭제
            pthread_mutex_unlock(&mutex_lock);              // 락 해제 분기1: 삭제할 노드 존재, 삭제후 락 해제.
            return;
        }
        prev = node;                                            // 다음 순회를 위해 prev에 현재 노드를, 현재 노드에 next 노드를 할당
        node = node->next;
    }
    pthread_mutex_unlock(&mutex_lock);                      // 락 해제 분기2: 삭제할 노드 존재하지 않음. 모든 노드 순회후 락 해제.
}

void CoarseHashTable::traversal(KVC* arr) {
    pthread_mutex_lock(&mutex_lock);                        // 함수 전체적으로 락 설정
    std::vector<KVC> tmp;                                   // KVC의 임시 벡터
    for (int i = 0; i < num_buckets_; i++) {                // for: 모든 버킷 순회
        HTNode* node = buckets_[i];                         // 이번 순회의 버킷 설정
        while (node) {                                      // while: node가 nullptr가 아닐 때 까지 순회. (bucket 연결리스트 모든 노드 순회)
            KVC kvc;                                        // 각 순회에서 사용하는 kvc 구조체 인스턴스
            kvc.key = node->key;                            // 이하 각 k,v,c 값 할당
            kvc.value = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);                             // tmp에 하나씩 넣음
            node = node->next;                              // 다음 노드로 이동
        }
    }
    // key 기준 오름차순 정렬 (std::map 순회와 동일한 순서 보장)
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) { // tmp 벡터의 구조체를 key값에 따라 오름차순 정렬
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) {             // tmp값을 arr에 복사
        arr[i] = tmp[i];
    }
    pthread_mutex_unlock(&mutex_lock);                      // 다 순회 완료 후 락 해제.
}
// FineHashTable (fine-grained lock)
// FineHashTable 생성자
// Fine-Grained 적용 지점
// 1. bucket 마다 락 객체 하나씩 지정
// 2. 임계 영역에만 lock-unlock 구간 설정
FineHashTable::FineHashTable(int num_buckets) : DefaultHashTable(num_buckets) {
    // 버킷 수만큼 뮤텍스 배열 할당 및 초기화
    bucket_locks = new pthread_mutex_t[num_buckets_];       // 모든 버킷마다 각각 하나의 락을 가지게 함. (fine-grained 적용 방법 1)
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
    int hash = hash_func(key);                              // hash 값 계산

    pthread_mutex_lock(&bucket_locks[hash]);                // 해당 버킷에 해당하는 락만 설정 (fine-grained 적용 방법 2: 임계 영역에만 lock)
    // ============================== 임계 영역 시작 ==============================
    // 기존 key가 있는지 탐색
    HTNode* node = buckets_[hash];                          // bucket의 node 받아오기
    while (node) {                                          // while: node가 nullptr까지 순회 (삽입 위치 찾기)
        if (node->key == key) {                                 // 동일한 값이 이미 존재하면 value를 더하고 upd_cnt 증가
            node->value += value;
            node->upd_cnt++;
            pthread_mutex_unlock(&bucket_locks[hash]);      // lock 해제 분기1: 이미 값이 존재하는 경우
            // ============================== 임계 영역 종료 ==============================
            return;
        }
        node = node->next;                                  // 조건 부합 X: 다음 노드 이동
    }

    // 동일한 값이 존재하지 않으면 새 노드를 버킷 헤드에 삽입
    HTNode* new_node = new HTNode();
    new_node->key = key;                                    // KVC 형식에 맞게 구조체 초기화
    new_node->value = value;
    new_node->upd_cnt = 0;
    new_node->next = buckets_[hash];                        // 기존 헤드를 새 노드의 next로
    buckets_[hash] = new_node;                              // 새로 만든 노드를 헤드에 삽입
    pthread_mutex_unlock(&bucket_locks[hash]);              // lock 해제 분기2: 값이 존재하지 않아 새로운 노드 삽입하는 경우
    // ============================== 임계 영역 종료 ==============================
}

int FineHashTable::lookup(int key) {
    int hash = hash_func(key);                              // 해시 값 계산
    pthread_mutex_lock(&bucket_locks[hash]);                // 해당 버킷에 해당하는 락만 설정 (fine-grained 적용 방법 2: 임계 영역에만 lock)
    // ============================== 임계 영역 시작 ==============================
    HTNode* node = buckets_[hash];                          // 해당 해시에 해당하는 버킷의 헤드 노드 추출
    while (node) {                                          // while: 해당 버킷 모든 노드 순회
        if (node->key == key) {                                 // 만약 같은 값이 존재하면, 해당 키의 value 반환
            int result = node->value;                       // 락 보유 중 값을 지역변수에 복사 (use-after-unlock 방지)
            pthread_mutex_unlock(&bucket_locks[hash]);      // lock 해제 분기1: 찾는 값 존재, 값 반환.
            // ============================== 임계 영역 종료 ==============================
            return result;
        }
        node = node->next;                                  // 다음 노드 이동
    }
    pthread_mutex_unlock(&bucket_locks[hash]);              // lock 해제 분기2: 찾는 값이 없으면, 락 해제하고 0 반환
    // ============================== 임계 영역 종료 ==============================
    return 0;                                               // 없다면 0 반환
}

void FineHashTable::remove(int key) {
    int hash = hash_func(key);                              // 해시 값 계산
    HTNode* prev = nullptr;                                 // 제거를 위해 이전 노드 포인터 준비
    pthread_mutex_lock(&bucket_locks[hash]);                // 해당 버킷에 해당하는 락만 설정 (fine-grained 적용 방법 2: 임계 영역에만 lock)
    // ============================== 임계 영역 시작 ==============================
    HTNode* node = buckets_[hash];                          // 해시에 해당하는 버킷의 헤드노드 추출
    while (node) {                                          // while: 해당 버킷 모든 노드 순회
        if (node->key == key) {                                 // 만약 같은 값이 존재하면
            if (prev) {                                             // 이전 노드가 존재하면,
                prev->next = node->next;                                // 이전 노드의 next를 현재 노드의 next로 설정
            } else {
                buckets_[hash] = node->next;                        // 이전노드가 존재하지 않으면(헤드라면) 다음 노드를 헤드로 설정
            }
            delete node;                                        // node 삭제
            pthread_mutex_unlock(&bucket_locks[hash]);      // lock 해제 분기1: 값을 찾아 노드 삭제 완료
            // ============================== 임계 영역 종료 ==============================
            return;
        }
        prev = node;                                            // 다음 순회를 위해 prev에 현재 노드를, 현재 노드에 next 노드를 할당
        node = node->next;
    }
    pthread_mutex_unlock(&bucket_locks[hash]);              // lock 해제 분기2: 값을 찾지 못해 변경 없이 종료
    // ============================== 임계 영역 종료 ==============================
    return;
}

void FineHashTable::traversal(KVC* arr) {
    std::vector<KVC> tmp;                                   // KVC의 임시 벡터
    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_lock(&bucket_locks[i]);               // 전체 순회 일관성 보장 위해 모든 버킷 락을 순서대로 획득
    }
    // ============================== 임계 영역 시작 ==============================
    for (int i = 0; i < num_buckets_; i++) {                // for: 모든 버킷 순회
        HTNode* node = buckets_[i];                         // 이번 순회의 버킷 설정
        while (node) {                                      // while: node가 nullptr가 아닐 때 까지 순회. (bucket 연결리스트 모든 노드 순회)
            KVC kvc;                                        // 각 순회에서 사용하는 kvc 구조체 인스턴스
            kvc.key     = node->key;                        // 이하 각 k,v,c 값 할당
            kvc.value   = node->value;
            kvc.upd_cnt = node->upd_cnt;
            tmp.push_back(kvc);                             // tmp에 하나씩 넣음
            node = node->next;                              // 다음 노드로 이동
        }
    }

    for (int i = 0; i < num_buckets_; i++) {
        pthread_mutex_unlock(&bucket_locks[i]);             // 순회 완료 후 모든 버킷 락 해제
    }
    // ============================== 임계 영역 종료 ==============================
    // key 기준 오름차순 정렬 (std::map 순회와 동일한 순서 보장)
    std::sort(tmp.begin(), tmp.end(), [](const KVC& a, const KVC& b) { // tmp 벡터의 구조체를 key값에 따라 오름차순 정렬
        return a.key < b.key;
    });
    for (int i = 0; i < (int)tmp.size(); i++) {             // tmp값을 arr에 복사
        arr[i] = tmp[i];
    }

}
