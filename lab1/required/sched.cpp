/*
 *	DKU Operating System Lab (2026)
 *	    Lab1 (Scheduler Algorithm Simulator)
 *	    Student id : 32224626
 *	    Student name : 최범연
 */

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <algorithm>
#include "sched.h"

class FCFS : public Scheduler
{
    private:
    /*
     *  1. job_queue_ :아직 도착하지 않은 작업들                (상위 클래스(Scheduler) 정의)
     *  2. ready_queue : 도착했지만 아직 실행 대기 중인 작업들  (FCFS 클래스 신규 정의)
     *  3. current_job_ — 현재 CPU에서 실행 중인 작업 (단일)    (상위 클래스 정의)
     *  4. end_jobs_ — 실행 완료된 작업들                       (상위 클래스 정의)
     */
    
    std::queue<Job> ready_queue;

public:
    FCFS(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "FCFS";
    }
    /*
     * FCFS 목차 (주요 로직)
     * 1. 현재 시간까지 도착한 작업을 ready_queue에 추가
     * 2. 현재 작업이 없으면 다음 작업 선택
     * 2-1. 다음 작업 선택 - FCFS
     * 2-2. 문맥 교환
     * 3. 1초 실행
     * 4. 작업 완료 처리
     */
    int run() override 
    {
        // 1. 현재 시간까지 도착한 작업을 ready_queue에 추가
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) { // job_queue에 현재 시간 기준 도착한 작업이 있는지 확인
            ready_queue.push(job_queue_.front()); // push: 뒤에 추가
            job_queue_.pop(); // pop: 앞의 것을 pop
        }

        // 2. 현재 작업이 없으면 다음 작업 선택
        if (current_job_.name == 0) { // 현재 작업이 없는 상태
            if (ready_queue.empty()) { 
                if (job_queue_.empty()) 
                    return -1; // 모든 작업이 끝났다고 판단. (ready, job 큐 다 empty)
                current_time_ += 1; // 다음 작업 도착까지 시간 경과
                return 0; // 아직 도착하지 않은 작업 대기 (CPU 유휴)
            }
            
            // 2-1. ready_queue에 가장 먼저 도착한 다음 작업 선택 - FCFS
            current_job_ = ready_queue.front(); // ready큐에서 수행할 현재 작업을 선택
            ready_queue.pop(); // 수행할 현재 작업 ready큐에서 삭제

            // 2-2. 문맥 교환
            if (!end_jobs_.empty()) {  // 완료된 작업이 있으면 문맥 교환 수행(첫 작업은 교환 불필요)
                current_time_ += switch_time_;

                // 수행할 다른 작업이 남아있고 && 그 작업의 도착시간이 현재 시간 이하 
                // -> 현재 기준 도착한 작업 추가
                while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) { 
                    ready_queue.push(job_queue_.front()); // ready 큐에 추가
                    job_queue_.pop();
                }
            }

            current_job_.first_run_time = current_time_; // 시작 시간 기록
        }

        // 3. 1초 실행 - 최소 시간단위 수행 (하나의 클럭 사이클을 추상화)
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        // 4. 작업 완료 처리
        if (current_job_.remain_time == 0) { // 지정한 시간이 다 지나면 수행
            current_job_.completion_time = current_time_; // completion_time 기록
            end_jobs_.push_back(current_job_); // end_jobs 큐에 완료된 작업 추가
            current_job_.name = 0; // 현재 작업 일시적으로 없다고 설정
        }

        return running; // 현재 실행 중인 작업 이름 반환
    }
};

class SPN : public Scheduler
{
private:
    //  1. job_queue_ :아직 도착하지 않은 작업들                (상위 클래스 정의)
    //  2. ready_queue : 도착했지만 아직 실행 대기 중인 작업들  (신규 정의(SPN))
    //  3. current_job_ — 현재 CPU에서 실행 중인 작업 (단일)    (상위 클래스 정의)
    //  4. end_jobs_ — 실행 완료된 작업들                       (상위 클래스 정의)
    std::vector<Job> ready_queue; 

public:
    SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "SPN";
    }

    /*
     * SPN 목차 (주요 로직)
     * 1. 현재 시간까지 도착한 작업을 ready_queue에 추가
     * 2. 현재 작업이 없으면 다음 작업 선택
     * 2-1. service_time이 가장 짧은 작업 선택 - SPN
     * 2-2. 문맥 교환
     * 3. 1초 실행
     * 4. 작업 완료 처리
     */
    int run() override
    {
        // 1. 현재 시간까지 도착한 작업을 ready_queue에 추가
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) { // job_queue에 현재 시간 기준 도착한 작업이 있는지 확인
            ready_queue.push_back(job_queue_.front()); // push_back: 뒤에 추가
            job_queue_.pop(); // pop: 앞의 것을 pop
        }

        // 2. 현재 작업이 없으면 다음 작업 선택
        if (current_job_.name == 0) { // 현재 작업이 없는 상태
            if (ready_queue.empty()) {
                if (job_queue_.empty())
                    return -1; // 모든 작업이 끝났다고 판단. (ready, job 큐 다 empty)
                current_time_ += 1; // 다음 작업 도착까지 시간 경과
                return 0; // 아직 도착하지 않은 작업 대기 (CPU 유휴)
            }

            // 2-1. service_time이 가장 짧은 작업 선택 - SPN (동일하면 이름순)
            // min_element - 범위 내 가장 작은 요소를 찾아 그 위치(iterator)를 반환하는 STL 함수
            auto shortest = std::min_element(ready_queue.begin(), ready_queue.end(), // ready_queue에서 가장 짧은 작업의 iterator 반환
                [](const Job& a, const Job& b) {
                    if (a.service_time == b.service_time) return a.name < b.name; // service_time 동일하면 이름순
                    return a.service_time < b.service_time; // service_time이 짧은 순
                });
            current_job_ = *shortest; // 가장 짧은 작업을 현재 작업으로 선택
            ready_queue.erase(shortest); // 선택한 작업을 ready_queue에서 삭제

            // 2-2. 문맥 교환
            if (!end_jobs_.empty()) { // 완료된 작업이 있으면 문맥 교환 수행 (첫 작업은 교환 불필요)
                current_time_ += switch_time_;
                // 문맥 교환 시간 동안 도착한 작업 추가
                while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                    ready_queue.push_back(job_queue_.front()); // ready 큐에 추가
                    job_queue_.pop();
                }
            }

            current_job_.first_run_time = current_time_; // 시작 시간 기록
        }

        // 3. 1초 실행 - 최소 시간단위 수행 (클럭 추상화)
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        // 4. 작업 완료 처리
        if (current_job_.remain_time == 0) { // 지정한 시간이 다 지나면 수행
            current_job_.completion_time = current_time_; // completion_time 기록
            end_jobs_.push_back(current_job_); // end_jobs 큐에 완료된 작업 추가
            current_job_.name = 0; // 현재 작업 일시적으로 없다고 설정
        }

        return running; // 현재 실행 중인 작업 이름 반환
    }
};

class RR : public Scheduler
{
private:
    //  1. job_queue_ :아직 도착하지 않은 작업들                (상위 클래스 정의)
    //  2. ready_queue : 도착했지만 아직 실행 대기 중인 작업들  (신규 정의(RR))
    //  3. current_job_ — 현재 CPU에서 실행 중인 작업 (단일)    (상위 클래스 정의)
    //  4. end_jobs_ — 실행 완료된 작업들                       (상위 클래스 정의)

    std::queue<Job> ready_queue; // 2. ready_queue: 도착했지만 ready 상태의 작업들

    // 추가된 변수들
    int time_slice_; // 5. 할당된 time_slice의 크기
    int left_slice_; // 6. 남은 time_slice의 크기
    int last_job_name_ = 0; // 7.마지막으로 실행한 작업 이름 (문맥 교환 판단용)

public:
    RR(std::queue<Job> jobs, double switch_overhead, int time_slice) : Scheduler(jobs, switch_overhead)
    {
        name = "RR_" + std::to_string(time_slice);
        /*
         * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
         * 나머지는 자유롭게 수정 및 작성 가능 (아래 코드 수정 및 삭제 가능)
         */
        time_slice_ = time_slice;
        left_slice_ = time_slice;
    }

    /*
     * RR 목차 (주요 로직)
     * 1. 현재 시간까지 도착한 작업을 ready_queue에 추가
     * 2. 현재 작업이 없으면 다음 작업 선택
     * 2-1. ready_queue에서 다음 작업 선택 - RR (FCFS 순서)
     * 2-2. 문맥 교환
     * 3. 1초 실행
     * 4. 작업 완료 또는 타임슬라이스 만료 처리
     */
    int run() override
    {
        // 1. 현재 시간까지 도착한 작업을 ready_queue에 추가
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) { // job_queue에 현재 시간 기준 도착한 작업이 있는지 확인
            ready_queue.push(job_queue_.front()); // push: 뒤에 추가
            job_queue_.pop(); // pop: 앞의 것을 pop
        }

        // 2. 현재 작업이 없으면 다음 작업 선택
        if (current_job_.name == 0) { // 현재 작업이 없는 상태
            if (ready_queue.empty()) {
                if (job_queue_.empty())
                    return -1; // 모든 작업이 끝났다고 판단. (ready, job 큐 다 empty)
                current_time_ += 1; // 다음 작업 도착까지 시간 경과
                return 0; // 아직 도착하지 않은 작업 대기 (CPU 유휴)
            }

            // 2-1. ready_queue에서 다음 작업 선택 - RR (FCFS 순서)
            current_job_ = ready_queue.front(); // ready큐 에서 수행할 현재 작업을 선택
            ready_queue.pop(); // 선택한 작업을 ready_queue에서 삭제
            left_slice_ = time_slice_; // 타임슬라이스 초기화

            // 2-2. 문맥 교환 (다른 작업으로 전환되었을 때만 수행)
            if (last_job_name_ != 0 && last_job_name_ != current_job_.name) {
                // 이전 작업과 다른 작업이면 문맥 교환 수행
                current_time_ += switch_time_;
                // 문맥 교환 시간 동안 도착한 작업 추가
                while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                    ready_queue.push(job_queue_.front()); // ready 큐에 추가
                    job_queue_.pop();
                }
            }

            // 첫 실행인 경우 first_run_time 기록
            if (current_job_.remain_time == current_job_.service_time) {
                current_job_.first_run_time = current_time_; // 시작 시간 기록
            }
        }

        // 3. 1초 실행 - 최소 시간단위 수행 (클럭 추상화)
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;
        left_slice_--;

        // 4. 작업 완료 또는 타임슬라이스 만료 처리
        // 작업 완료
        if (current_job_.remain_time == 0) { // 지정한 시간이 다 지나면 수행
            current_job_.completion_time = current_time_; // completion_time 기록
            last_job_name_ = current_job_.name; // 마지막 실행 작업 기록
            end_jobs_.push_back(current_job_); // end_jobs 큐에 완료된 작업 추가
            current_job_.name = 0; // 현재 작업 없음으로 설정
        }
        // 타임슬라이스 만료 -> 도착한 작업 먼저 추가 후 현재 작업을 ready_queue 뒤로
        else if (left_slice_ == 0) {
            while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                ready_queue.push(job_queue_.front()); // ready 큐에 추가
                job_queue_.pop();
            }
            last_job_name_ = current_job_.name; // 마지막 실행 작업 기록
            ready_queue.push(current_job_); // 현재 작업을 ready_queue 뒤에 추가
            current_job_.name = 0; // 현재 작업 없음으로 설정
        }

        return running; // 현재 실행 중인 작업 이름 반환
    }
};


// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler
{
private:
    static const int NUM_QUEUES = 4;
    bool is_2i_;
    int left_slice_;
    int current_level_ = NUM_QUEUES - 1;
    int last_job_name_ = 0;
    std::queue<Job> ready_queues_[NUM_QUEUES]; // Q3=최고, Q0=최저 우선순위

    int get_time_slice(int level) {
        if (is_2i_) {
            return (1 << (NUM_QUEUES - 1 - level)); // NUM_QUEUES=4 기준, Q3=1, Q2=2, Q1=4, Q0=8
        }
        return 1; // FeedBack_1: 항상 1
    }

    int find_highest_queue() {
        for (int i = NUM_QUEUES - 1; i >= 0; i--) {
            if (!ready_queues_[i].empty()) return i;
        }
        return -1;
    }

public:
    FeedBack(std::queue<Job> jobs, double switch_overhead, bool is_2i) : Scheduler(jobs, switch_overhead) {
        if (is_2i) {
            name = "FeedBack_2i";
        }
        else {
            name = "FeedBack_1";
        }
        /*
         * 위 생성자 선언 및 이름 초기화 코드 수정하지 말것.
         * 나머지는 자유롭게 수정 및 작성 가능
         */
        is_2i_ = is_2i;
        left_slice_ = get_time_slice(NUM_QUEUES - 1);
    }

    /*
     * FeedBack 목차 (주요 로직)
     * 1. 현재 시간까지 도착한 작업을 최상위 Priority 큐에 추가
     * 2. 현재 작업이 없으면 다음 작업 선택
     * 2-1. 가장 높은 우선순위 큐에서 다음 작업 선택 - FeedBack
     * 2-2. 문맥 교환
     * 3. 1초 실행
     * 4. 작업 완료 또는 타임슬라이스 만료 처리
     */
    int run() override {
        // 1. 현재 시간까지 도착한 작업을 최상위 큐(레벨 3)에 추가
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) { // job_queue에 현재 시간 기준 도착한 작업이 있는지 확인
            Job j = job_queue_.front();
            job_queue_.pop(); // pop: 앞의 것을 pop
            ready_queues_[NUM_QUEUES - 1].push(j); // 최상위 큐에 추가
        }

        // 1-1. 우선순위 선점 검사 — 현재 작업보다 높은 레벨 큐에 작업이 있으면 선점
        if (current_job_.name != 0) {
            int hq = find_highest_queue();
            if (hq > current_level_) {
                // 현재 작업을 자기 레벨 큐로 되돌림 (잔여 quantum 은 단순화상 폐기)
                ready_queues_[current_level_].push(current_job_);
                last_job_name_ = current_job_.name; // 다음 run()에서 문맥 교환 트리거
                current_job_.name = 0; // 현재 작업 비움 → 아래 2.에서 상위 큐 작업 선택
            }
        }

        // 2. 현재 작업이 없으면 다음 작업 선택
        if (current_job_.name == 0) { // 현재 작업이 없는 상태
            int qi = find_highest_queue();
            if (qi == -1) {
                if (job_queue_.empty())
                    return -1; // 모든 작업이 끝났다고 판단. (ready, job 큐 다 empty)
                current_time_ += 1; // 다음 작업 도착까지 시간 경과
                return 0; // 아직 도착하지 않은 작업 대기 (CPU 유휴)
            }

            // 2-1 가장 높은 우선순위 큐에서 다음 작업 선택 - FeedBack
            current_job_ = ready_queues_[qi].front(); // 가장 높은 우선순위 큐에서 수행할 현재 작업을 선택
            ready_queues_[qi].pop(); // 선택한 작업을 ready_queue에서 삭제
            current_level_ = qi; // 현재 작업의 큐 레벨 기록
            left_slice_ = get_time_slice(qi); // 타임슬라이스 초기화

            // 2-2. 문맥 교환 (다른 작업으로 전환할 때만)
            if (last_job_name_ != 0 && last_job_name_ != current_job_.name) {
                // 이전 작업과 다른 작업이면 문맥 교환 수행
                current_time_ += switch_time_;
                // 문맥 교환 시간 동안 도착한 작업 추가
                while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                    Job j = job_queue_.front();
                    job_queue_.pop();
                    ready_queues_[NUM_QUEUES - 1].push(j); // 최상위 큐에 추가
                }
            }

            // 첫 실행인 경우 first_run_time 기록
            if (current_job_.remain_time == current_job_.service_time) {
                current_job_.first_run_time = current_time_; // 시작 시간 기록
            }
        }

        // 3. 1초 실행 - 최소 시간단위 수행 (클럭 추상화)
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;
        left_slice_--;

        // 4. 작업 완료 또는 타임슬라이스 만료 처리
        // 작업 완료
        if (current_job_.remain_time == 0) { // 지정한 시간이 다 지나면 수행
            current_job_.completion_time = current_time_; // completion_time 기록
            last_job_name_ = current_job_.name; // 마지막 실행 작업 기록
            end_jobs_.push_back(current_job_); // end_jobs 큐에 완료된 작업 추가
            current_job_.name = 0; // 현재 작업 없음으로 설정
        }
        // 타임슬라이스 만료
        else if (left_slice_ <= 0) {
            // 신규 작업 추가
            while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                Job j = job_queue_.front();
                job_queue_.pop();
                ready_queues_[NUM_QUEUES - 1].push(j); // 최상위 큐에 추가
            }
            last_job_name_ = current_job_.name; // 마지막 실행 작업 기록
            // 표준 MLFQ Rule 4: allotment 소진 시 무조건 강등 (대기 작업 유무와 무관)
            // I/O 양보가 없는 시뮬레이터이므로 1 time slice = 해당 레벨의 allotment.
            // 따라서 매 슬라이스 만료마다 한 단계 강등하면 표준 allotment 기반 강등과 등가.
            int next_level = std::max(current_level_ - 1, 0);
            ready_queues_[next_level].push(current_job_); // 한 단계 낮은 큐에 추가
            current_job_.name = 0; // 현재 작업 없음으로 설정
        }

        return running; // 현재 실행 중인 작업 이름 반환
    }
};
