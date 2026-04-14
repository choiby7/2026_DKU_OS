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
    // 단일 연결 리스트 노드
    struct Node {
        Job job;
        Node* next;
        Node(const Job& j) : job(j), next(nullptr) {}
    };

    Node* head_ = nullptr; // 연결 리스트 헤드
    int counter = 0; // 티켓 누적 카운터 (추첨 시 사용)
    int total_tickets = 0; // 전체 티켓 수
    int winner = 0; // 당첨 티켓 번호
    int last_job_name_ = 0; // 마지막으로 실행한 작업 이름 (문맥 교환 판단용)
    std::mt19937 gen;

public:
    Lottery(std::list<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead) {
        name = "Lottery";
        // 난수 생성기 초기화
        uint seed = 10; // seed 값 수정 금지
        gen = std::mt19937(seed);
        total_tickets = 0;
        // job_list_ 내용을 단일 연결 리스트로 복사하면서 총 티켓 수 계산
        Node* tail = nullptr;
        for (const auto& job : job_list_) {
            Node* node = new Node(job);
            if (head_ == nullptr) {
                head_ = node;
            } else {
                tail->next = node;
            }
            tail = node;
            total_tickets += job.tickets;
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
        // 1. 모든 작업이 완료되었는지 확인
        if (head_ == nullptr)
            return -1; // 모든 작업이 끝났다고 판단

        // 2. 랜덤 티켓 추첨으로 당첨 작업 선택
        winner = getRandomNumber(0, total_tickets - 1); // 당첨 티켓 번호 결정
        counter = 0; // 티켓 누적 카운터 초기화

        //  head부터 current를 전진시키며 누적 티켓이 winner를 넘는 순간 멈춤
        Node* prev = nullptr;   // 삭제 시 사용할 이전 노드
        Node* current = head_;
        // loop : 티켓의 합의 값이winner 보다 커질 때 까지
        while (current) {
            counter = counter + current->job.tickets;
            // counter > winner 이면 이전루프의 current를 사용(마지막 검사한 노드)
            if (counter > winner) {
                break; // 'current' 이 winner. break.
            }
            prev = current; // prev을 현재노드로
            current = current->next; // 다음 노드로 이동
        }

        // 현재 작업 설정
        Node* selected = current;
        current_job_ = selected->job;

        // 3. 문맥 교환 (다른 작업으로 전환할 때만)
        if (last_job_name_ != 0 && last_job_name_ != current_job_.name) {
            current_time_ += switch_time_;
        }

        // 첫 실행인 경우 first_run_time 기록
        if (current_job_.remain_time == current_job_.service_time) {
            current_job_.first_run_time = current_time_; // 시작 시간 기록
        }

        // 4. 1초 실행 - 최소 시간단위 수행 (클럭 추상화)
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        // 5. 작업 완료 처리
        if (current_job_.remain_time == 0) { // 지정한 시간이 다 지나면 수행
            current_job_.completion_time = current_time_; // completion_time 기록
            last_job_name_ = current_job_.name; // 마지막 실행 작업 기록
            end_jobs_.push_back(current_job_); // end_jobs 큐에 완료된 작업 추가
            // 완료된 작업의 티켓을 총 티켓에서 제거
            total_tickets -= current_job_.tickets;
            // 연결 리스트에서 당첨 노드 제거
            if (prev == nullptr) {
                head_ = selected->next; // 헤드 노드 제거
            } else {
                prev->next = selected->next; // 중간/끝 노드 제거
            }
            delete selected;
            current_job_.name = 0; // 현재 작업 없음으로 설정
        } else {
            // 연결 리스트의 해당 노드 remain_time 갱신
            selected->job.remain_time = current_job_.remain_time;
            selected->job.first_run_time = current_job_.first_run_time;
            last_job_name_ = current_job_.name; // 마지막 실행 작업 기록
        }

        return running; // 현재 실행 중인 작업 이름 반환
    }
};


class Stride : public Scheduler
{
private:
    // 단일 연결 리스트 노드
    struct Node {
        Job job;
        Node* next;
        Node(const Job& j) : job(j), next(nullptr) {}
    };

    Node* head_ = nullptr; // 연결 리스트 헤드
    // 각 작업의 현재 pass 값과 stride 값을 관리하는 맵
    std::unordered_map<int, int> pass_map_;
    std::unordered_map<int, int> stride_map_;
    const int BIG_NUMBER = 10000;
    int last_job_name_ = 0; // 마지막으로 실행한 작업 이름 (문맥 교환 판단용)

public:
    Stride(std::list<Job> jobs, double switch_overhead) : Scheduler(jobs, switch_overhead)
    {
        name = "Stride";
        // job_list_ 내용을 단일 연결 리스트로 복사하며 stride/pass 초기값 설정
        Node* tail = nullptr;
        for (const auto& job : job_list_) {
            Node* node = new Node(job);
            if (head_ == nullptr) {
                head_ = node;
            } else {
                tail->next = node;
            }
            tail = node;
            // stride = BIG_NUMBER / tickets (tickets는 0이 아님을 가정)
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
        // 1. 모든 작업이 완료되었는지 확인
        if (head_ == nullptr)
            return -1; // 모든 작업이 끝났다고 판단

        // 2. pass 값이 가장 작은 작업 선택 (동일하면 이름순)
        // head부터 current를 전진시키며 최소 pass 값을 가진 노드를 찾음
        Node* selected = head_;         // 현재까지 찾은 최소 pass 노드
        Node* selected_prev = nullptr;  // selected의 이전 노드 (head면 nullptr)
        Node* prev = head_;             // current의 이전 노드 (current=head_->next로 시작하므로 head_)
        Node* current = head_->next;

        while (current) {
            int cur_pass = pass_map_[current->job.name]; // 현재 job의 pass
            int sel_pass = pass_map_[selected->job.name];// 스케줄링 기준 누적 pass
            bool better = (cur_pass < sel_pass) || // 누적 pass 보다 현재 job의 pass가 작으면 스케줄
                          (cur_pass == sel_pass && current->job.name < selected->job.name); // 동률 시 결정론적 순서 보장, 조건: 동률 시 작업 이름이 더 작은 것 선택
            if (better) { 
                selected = current; // 선택
                selected_prev = prev; // 삭제를 위해 이전 노드도 저장
            }
            prev = current; // prev 업데이트
            current = current->next; // 다음 노드로 이동
        }

        current_job_ = selected->job; // 가장 낮은 pass 값의 작업을 현재 작업으로 선택

        // 3. 문맥 교환 (다른 작업으로 전환할 때만)
        if (last_job_name_ != 0 && last_job_name_ != current_job_.name) {
            current_time_ += switch_time_;
        }

        // 첫 실행인 경우 first_run_time 기록
        if (current_job_.remain_time == current_job_.service_time) {
            current_job_.first_run_time = current_time_; // 시작 시간 기록
        }

        // 4. 1초 실행 - 최소 시간단위 수행 (클럭 추상화)
        int running = current_job_.name;
        current_job_.remain_time--;
        current_time_ += 1;

        // 5. pass 값 갱신 및 작업 완료 처리
        pass_map_[current_job_.name] += stride_map_[current_job_.name]; // pass 값 갱신

        if (current_job_.remain_time == 0) { // 지정한 시간이 다 지나면 수행
            current_job_.completion_time = current_time_; // completion_time 기록
            last_job_name_ = current_job_.name; // 마지막 실행 작업 기록
            end_jobs_.push_back(current_job_); // end_jobs 큐에 완료된 작업 추가
            // 연결 리스트에서 선택 노드 제거
            if (selected_prev == nullptr) {
                head_ = selected->next; // 헤드 노드 제거
            } else {
                selected_prev->next = selected->next; // 중간/끝 노드 제거
            }
            delete selected;
            current_job_.name = 0; // 현재 작업 없음으로 설정
        } else {
            // 연결 리스트의 해당 노드 remain_time 갱신
            selected->job.remain_time = current_job_.remain_time;
            selected->job.first_run_time = current_job_.first_run_time;
            last_job_name_ = current_job_.name; // 마지막 실행 작업 기록
        }

        return running; // 현재 실행 중인 작업 이름 반환
    }
};
