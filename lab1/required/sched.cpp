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
#include <map>
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
    int current_level_ = NUM_QUEUES - 1; // 현재 우선순위 레벨 (안전 초기값)
    int last_job_name_ = 0;
    std::queue<Job> ready_queues_[NUM_QUEUES]; // Q3=최고, Q0=최저 우선순위
    std::map<int, int> job_left_slice_; // 작업별 현재 레벨에 남은 allotment (key = job(process) name)

    // 해당 queue 레벨의 time slice(allotment) 크기를 반환하는 함수
    int get_time_slice(int level) {
        if (is_2i_) {
            return (1 << (NUM_QUEUES - 1 - level)); // NUM_QUEUES=4 기준, Q3=1, Q2=2, Q1=4, Q0=8
        }
        return 1; // FeedBack_1: 항상 1
    }

    // 비어있지 않은 가장 우선순위가 높은 큐의 번호를 반환((NUM_QUEUES - 1) ~ 0)
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
    }

    /*
     * FeedBack 목차 (주요 로직)
     * 1. 현재 시간까지 도착한 작업을 최상위 Priority 큐에 추가
     * 2. 현재 스케줄된 작업이 없으면 다음 작업 선택
     * 2-1. 우선순위가 가장 높은 비어있지 않은 큐의 front를 선택
     * 2-2. 이전 작업과 다르면 문맥 교환
     * 3. 1초 실행
     * 4. 후처리 (a, b, c의 경우)
     *   4-a. 완료
     *   4-b. allotment 소진 (job_left_slice <= 0) : 
     *     도착 작업 pull 후, 다른 작업이 하나라도 있으면 한 단계 강등 + 새 레벨용 allotment 재설정 /
     *     시스템에 작업이 현재 작업 하나뿐이면 같은 레벨 유지.
     *   4-c. allotment 남음 :
     *     cur 유지 -> 다음 tick 도 현재 작업과 같은 프로세스 수행 (allotment 소진시 까지)
     */
    int run() override {
        // 1. 현재 시간까지 도착한 작업을 최상위 큐(레벨 3)에 추가
        while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) { // job_queue 맨 앞의 요소가 도착했다면
            Job j = job_queue_.front(); // 도착한 작업(프로세스) j
            job_queue_.pop();
            job_left_slice_[j.name] = get_time_slice(NUM_QUEUES - 1); // 최상위 레벨 큐 allotment을 해당 작업의 job_left_slice_에 초기화
            ready_queues_[NUM_QUEUES - 1].push(j); // 도착한 작업을 ready큐에 삽입
        }

        // 2. 현재 스케줄된 작업이 없으면 다음 작업 선택
        if (current_job_.name == 0) {
            int qi = find_highest_queue(); // 작업을 가지고 있는 가장 상위의 큐를 선택
            if (qi == -1) { // 모든 레디 큐가 비어있다면
                if (job_queue_.empty()) // job_queue도 비어있다면, 수행이 다 완료된 상태.
                    return -1; // 모든 작업이 완료
                // job_queue가 비어있지 않다면, CPU 유휴 처리 
                // 그러나 test 상 idle한 경우가 존재하지 않기때문에 들어갈일 없음.
                current_time_ += 1;
                return 0;
            }

            // 2-1. 우선순위가 가장 높은 비어있지 않은 큐의 front를 선택
            current_job_ = ready_queues_[qi].front(); // 레디큐의 front 작업을 current_job_에 할당.
            ready_queues_[qi].pop(); // 선택된 작업을 ready_queues에서 삭제
            current_level_ = qi; // 이번 수행에서 해당 작업의 우선순위 레벨을 저장
            // 잔여 allotment 는 작업별 map 에서 관리되므로 여기서 초기화하지 않음

            // 2-2. 이전 작업과 다르면 문맥 교환
            if (last_job_name_ != 0 && last_job_name_ != current_job_.name) { // 직전에 turn을 마친 작업(4-a, 4-b)과 새로 선택한 작업이 다르다면
                current_time_ += switch_time_; // 문맥 교환 발생
                // 시간이 변경되었기에 최상위 우선순위 큐에 들어온 작업 추가 (로직은 기존과 동일)
                while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) { 
                    Job j = job_queue_.front();
                    job_queue_.pop();
                    job_left_slice_[j.name] = get_time_slice(NUM_QUEUES - 1);
                    ready_queues_[NUM_QUEUES - 1].push(j);
                }
            }

            // 작업의 첫 실행인 경우 first_run_time 기록
            if (current_job_.remain_time == current_job_.service_time) {
                current_job_.first_run_time = current_time_;
            }
        }

        // 3. 1초 실행
        int running = current_job_.name;
        current_job_.remain_time--; // 현재 작업의 remain time 갱신
        current_time_ += 1; 
        job_left_slice_[current_job_.name]--; // 남은 allotment 1 감소

        // 4. 후처리 (a, b, c의 경우)
        // 4-a. 완료
        if (current_job_.remain_time == 0) {
            current_job_.completion_time = current_time_;
            last_job_name_ = current_job_.name; // 다음 tick에 전달할 last_job_name_ 정보 저장
            end_jobs_.push_back(current_job_); // 완료되었으므로 현재 작업을 end_jobs에 추가
            job_left_slice_.erase(current_job_.name); // 다썼으므로 삭제
            current_job_.name = 0; // 다음 tick을 위해 0 할당
        }
        // 4-b. allotment 소진 (job_left_slice <= 0):
        //      도착 작업 pull 후, 다른 작업이 하나라도 있으면 한 단계 강등 + 새 레벨용 allotment 재설정 /
        //      시스템에 작업이 현재 작업 하나뿐이면 같은 레벨 유지.
        else if (job_left_slice_[current_job_.name] <= 0) {
            // 1초 실행했기 때문에 최신 시간에 도착한 작업 레디큐에 추가 (기존과 로직 동일)
            while (!job_queue_.empty() && job_queue_.front().arrival_time <= current_time_) {
                Job j = job_queue_.front();
                job_queue_.pop();
                job_left_slice_[j.name] = get_time_slice(NUM_QUEUES - 1);
                ready_queues_[NUM_QUEUES - 1].push(j);
            }
            last_job_name_ = current_job_.name; // 다음 tick에 전달할 last_job_name_ 정보 저장.
            bool alone = true; // 레디큐 전체에서 현재 작업이 유일한 작업이라면 true
            for (int i = 0; i < NUM_QUEUES; i++) {
                if (!ready_queues_[i].empty()) { alone = false; break; }
            }
            int target_level = alone ? current_level_ : std::max(current_level_ - 1, 0); // 혼자라면 현재 우선순위 레벨 유지, 아니면 강등
            job_left_slice_[current_job_.name] = get_time_slice(target_level); // 다시 새로운 allotment 할당
            ready_queues_[target_level].push(current_job_); // 설정한 우선순위 레벨의 레디큐에 삽입
            current_job_.name = 0; // 다음 tick을 위해 0으로 할당
        }
        // 4-c. allotment 남음:
        //   cur 유지 -> 다음 tick 도 현재 작업과 같은 프로세스 수행 (allotment 소진시 까지)

        return running;
    }
};
