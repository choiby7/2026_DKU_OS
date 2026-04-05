/*
 *	DKU Operating System Lab (2026)
 *	    Lab1 (Scheduler Algorithm Simulator Bonus)
 *	    Student id : 32224626
 *	    Student name : 최범연
 */

#include <string>
#include <stdio.h>
#include <iostream>
#include <queue>
#include <algorithm>
#include <random>
#include <unordered_map>
#include "sched.h"


class Lottery : public Scheduler
{
private:
    int total_tickets_ = 0;
    std::mt19937 gen;

public:
    Lottery(std::list<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "Lottery";
        // 난수 생성기 초기화
        uint seed = 10; // seed 값 수정 금지
        gen = std::mt19937(seed);
        total_tickets_ = 0;
        for (const auto& job : job_list_) {
            total_tickets_ += job.tickets;
        }
    }

    int getRandomNumber(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen);
    }

    /*
     * Lottery 목차 (주요 로직)
     * 1. 모든 작업이 완료되었는지 확인
     * 2. 랜덤 티켓 추첨으로 당첨 작업 선택
     * 3. 문맥 교환
     * 4. 1초 실행
     * 5. 작업 완료 처리
     */
    int run() override
    {
        // 1. 모든 ��업이 완료되었는지 확인
        if (job_list_.empty())
            return -1;

        // 2. 랜덤 티켓 추���으로 당첨 작업 선택
        int winner = getRandomNumber(0, total_tickets_ - 1);
        int counter = 0;
        Job* selected = nullptr;

        for (auto& job : job_list_) {
            counter += job.tickets;
            if (counter > winner) {
                selected = &job;
                break;
            }
        }

        // 현재 작업 설정
        current_job_ = *selected;

        // 3. 문맥 교환 (다른 작업으로 전환할 때만)
        if (last_job_name_ != 0 && last_job_name_ != current_job_.name) {
            current_time_ += switch_time_;
        }

        // 첫 실행인 경우 first_run_time 기록
        if (current_job_.remain_time == current_job_.service_time) {
            current_job_.first_run_time = current_time_;
        }

        // 4. 1초 실행
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        // 5. 작업 완료 처리
        if (current_job_.remain_time == 0) {
            current_job_.completion_time = current_time_;
            end_jobs_.push_back(current_job_);
            // 완료된 작업의 티켓을 총 티켓에서 ��거
            total_tickets_ -= current_job_.tickets;
            // job_list_에서 완료된 작업 제거
            job_list_.remove_if([&](const Job& j) { return j.name == current_job_.name; });
            last_job_name_ = current_job_.name;
            current_job_.name = 0;
        } else {
            // job_list_의 해당 작업 remain_time 갱신
            for (auto& job : job_list_) {
                if (job.name == current_job_.name) {
                    job.remain_time = current_job_.remain_time;
                    job.first_run_time = current_job_.first_run_time;
                    break;
                }
            }
            last_job_name_ = current_job_.name;
        }

        return running;
    }

private:
    int last_job_name_ = 0;
};


class Stride : public Scheduler
{
private:
    std::unordered_map<int, int> pass_map_;
    std::unordered_map<int, int> stride_map_;
    const int BIG_NUMBER = 10000;
    int last_job_name_ = 0;

public:
    Stride(std::list<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "Stride";
        // job_list_에 있는 각 ��업에 대해 stride와 초기 pass 값(0)을 설정
        for (auto& job : job_list_) {
            stride_map_[job.name] = BIG_NUMBER / job.tickets;
            pass_map_[job.name] = 0;
        }
    }

    /*
     * Stride 목차 (주요 로직)
     * 1. 모든 작업이 완료되었는지 확인
     * 2. pass 값이 가장 작은 작업 선택 (동일하면 이름순)
     * 3. 문맥 교환
     * 4. 1초 실행
     * 5. pass 값 갱신 및 작업 완료 처리
     */
    int run() override
    {
        // 1. 모든 작업이 완료되었���지 확인
        if (job_list_.empty())
            return -1;

        // 2. pass 값��� 가장 작은 작업 선택 (동일하면 이름순)
        auto selected = std::min_element(job_list_.begin(), job_list_.end(),
            [&](const Job& a, const Job& b) {
                if (pass_map_[a.name] == pass_map_[b.name])
                    return a.name < b.name;
                return pass_map_[a.name] < pass_map_[b.name];
            });

        current_job_ = *selected;

        // 3. 문맥 교환 (다��� 작업으로 전환할 때만)
        if (last_job_name_ != 0 && last_job_name_ != current_job_.name) {
            current_time_ += switch_time_;
        }

        // 첫 실행인 경우 first_run_time 기록
        if (current_job_.remain_time == current_job_.service_time) {
            current_job_.first_run_time = current_time_;
        }

        // 4. 1초 실행
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        // 5. pass 값 갱신 및 작업 완료 처리
        pass_map_[current_job_.name] += stride_map_[current_job_.name];

        if (current_job_.remain_time == 0) {
            current_job_.completion_time = current_time_;
            end_jobs_.push_back(current_job_);
            // job_list_에서 완료된 작업 제거
            job_list_.erase(selected);
            last_job_name_ = current_job_.name;
            current_job_.name = 0;
        } else {
            // job_list_의 해당 작업 remain_time 갱신
            selected->remain_time = current_job_.remain_time;
            selected->first_run_time = current_job_.first_run_time;
            last_job_name_ = current_job_.name;
        }

        return running;
    }
};
