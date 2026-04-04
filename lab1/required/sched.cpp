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
    //  1. job_queue_ :아직 도착하지 않은 작업들                (상위 클래스 정의)
    //  2. ready_queue : 도착했지만 아직 실행 대기 중인 작업들  (신규 정의(FCFS))
    //  3. current_job_ — 현재 CPU에서 실행 중인 작업 (단일)    (상위 클래스 정의)
    //  4. end_jobs_ — 실행 완료된 작업들                       (상위 클래스 정의)
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
     * 2-1. 문맥 교환
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

            current_job_ = ready_queue.front(); // ready큐에서 수행할 현재 작업을 선택
            ready_queue.pop(); // 수행할 현재 작업 ready큐에서 삭제

            // 2-1. 문맥 교환
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

        // 3. 1초 실행
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        // 4. 작업 완료 처리
        if (current_job_.remain_time == 0) { // 지정한 시간이 다 지나면 수행
            current_job_.completion_time = current_time_; // completion_time 기록
            end_jobs_.push_back(current_job_); // end_jobs 큐에 완료된 작업 추가
            current_job_.name = 0; // 현재 작업 일시적으로 없다고 설정
        }

        return running; // 현재 실행 중인 작업 이름 반환 반환
    }
};

class SPN : public Scheduler
{
private:
    std::vector<Job> ready_queue;

public:
    SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "SPN";
    }

    int run() override
    {
        // 도착한 작업을 ready_queue에 추가
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
            ready_queue.push_back(job_queue_.front());
            job_queue_.pop();
        }

        // 현재 작업이 없으면 다음 작업 선택
        if (current_job_.name == 0) { // 현재 작업이 없는 상태.
            if (ready_queue.empty()) {
                if (job_queue_.empty()) 
                    return -1; // 모든 작업이 끝났다고 판단. (ready, job 큐 다 poped)
                current_time_ += 1; // 다음 작업 도착까지 시간 경과
                return 0; // 아직 도착하지 않은 작업 대기 (CPU 유휴)
            }

            // service_time이 가장 짧은 작업 선택 (동일하면 이름순)
            auto shortest = std::min_element(ready_queue.begin(), ready_queue.end(),
                [](const Job& a, const Job& b) {
                    if (a.service_time == b.service_time) return a.name < b.name;
                    return a.service_time < b.service_time;
                });
            current_job_ = *shortest;
            ready_queue.erase(shortest);

            // 문맥 교환 (첫 작업 제외)
            if (!end_jobs_.empty()) {
                current_time_ += switch_time_; // 현재 시간에 문맥 교환 비용 시간 추가
                while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) { // 
                    ready_queue.push_back(job_queue_.front());
                    job_queue_.pop();
                }
            }

            current_job_.first_run_time = current_time_;
        }

        // 1초 실행
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        // 작업 완료 처리
        if (current_job_.remain_time == 0) {
            current_job_.completion_time = current_time_;
            end_jobs_.push_back(current_job_);
            current_job_.name = 0;
        }

        return running;
    }
};

class RR : public Scheduler
{
private:
    int time_slice_;
    int left_slice_;
    std::queue<Job> waiting_queue;
    /* 
    * 구현 (멤버 변수/함수 추가 및 삭제 가능)
    */

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

    int run() override
    {
        /* 
        * 구현
        */
        return -1;
    }
};


// FeedBack 스케줄러 (queue 개수 : 4 / boosting 없음)
class FeedBack : public Scheduler
{
private:
    /*
    * 구현 (멤버 변수/함수 추가 및 삭제 가능)
    */

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
    }

    int run() override {
        /*
        * 구현
        */
        return -1;
    }
};
