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
    std::vector<Job> all_jobs;
    std::queue<Job> ready_queue;
    size_t next_job_idx = 0;

public:
    FCFS(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "FCFS";
        // job_queue_는 이름순 정렬이므로, 도착시간 기준으로 재정렬
        while (!job_queue_.empty()) {
            all_jobs.push_back(job_queue_.front());
            job_queue_.pop();
        }
        std::sort(all_jobs.begin(), all_jobs.end(), [](const Job& a, const Job& b) {
            if (a.arrival_time == b.arrival_time) return a.name < b.name;
            return a.arrival_time < b.arrival_time;
        });
    }

    int run() override
    {
        // 도착한 작업을 ready_queue에 추가
        while (next_job_idx < all_jobs.size() && all_jobs[next_job_idx].arrival_time <= current_time_) {
            ready_queue.push(all_jobs[next_job_idx]);
            next_job_idx++;
        }

        // 현재 작업이 없으면 다음 작업 선택
        if (current_job_.name == 0) {
            if (ready_queue.empty()) {
                if (next_job_idx >= all_jobs.size())
                    return -1; // 모든 작업 완료
                current_time_ += 1;
                return 0; // CPU 유휴
            }

            current_job_ = ready_queue.front();
            ready_queue.pop();

            // 문맥 교환 (첫 작업 제외)
            if (!end_jobs_.empty()) {
                current_time_ += switch_time_;
                // 문맥 교환 시간 동안 도착한 작업 추가
                while (next_job_idx < all_jobs.size() && all_jobs[next_job_idx].arrival_time <= current_time_) {
                    ready_queue.push(all_jobs[next_job_idx]);
                    next_job_idx++;
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

class SPN : public Scheduler
{
private:
    /*
    * 구현 (멤버 변수/함수 추가 및 삭제 가능)
    */

public:
    SPN(std::queue<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        /* 
        * 구현 (멤버 변수/함수 추가 및 삭제 가능)
        */
    }

    int run() override
    {
        /*
        * 구현
        */
        return -1;
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
