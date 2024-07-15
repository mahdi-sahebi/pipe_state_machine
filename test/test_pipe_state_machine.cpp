/* (x,y): Task(FrameID, TaskID)
 * T\F  : Time\FrameID       
 * L    : Full Load            
 *                             
 * T\F  0 1 2 3 4 5 6 7 8 9 a 
 *  0  |0|1|2|3| | | | | | | |
 *  1  | |0|1|2|3| | | | | | |
 *  2  | | |0|1|2|3| | | | | |
 *  3  | | | |0|1|2|3| | | | |
 *  4  | | | | |0|1|2|3| | | |
 *  5  | | | | | |0|1|2|3| | |
 *  6  | | | | | | |0|1|2|3| |
 *  7  | | | | | | | |0|1|2|3|
 *  8  | | | | | | | | | | | |
 *            L L L L L
 *                                                                                    
 * Frame\Cost                                                                            
 * 0___________________________
 * 101 |--->
 * 
 * 1___________________________
 * 101 |--->
 *   57 |--------->
 * 
 * 2___________________________
 * 101 |--->
 *   57 |--------->
 *    94 |------>
 * 
 * 3___________________________
 * 101 |--->
 *   57 |--------->
 *    94 |------>
 *    193 |---->                                                                              
 * 4___________________________
 * 101 |--->
 *   57 |--------->
 *    94 |------>
 *    193 |---->                                                                              
 * 
 * 5___________________________
 *  ...                                                                                  
 *                                                                                    
 */

#include <cassert>
#include <thread>
#include <chrono>
#include <algorithm>
#include "gtest/gtest.h"
#include "pipe_state_machine/pipe_state_machine.hpp"


using namespace std;
using namespace std::chrono;
using namespace std::this_thread;
using namespace ELB::StateMachine;


class PipeStateMachine : public testing::Test
{
public:
    PipeStateMachine();
    void SetUp() override;
    void TearDown() override;
    void AsyncTask_101(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void AsyncTask_57(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void AsyncTask_94(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void AsyncTask_193(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void OnFrameComplete(const Pipe::FrameID frame_id);

protected:
    Pipe* state_;
    std::vector<uint32_t> delays_;
    time_point<high_resolution_clock> begin_time_;
    time_point<high_resolution_clock> end_time_;
    uint32_t pause_time_ms_;

    uint32_t ValidateFullLoadDuration(const Pipe::FrameID frame_id);
    void PrintStart(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void PrintDone(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void DummyProcess(const uint32_t wait_ms);
};

PipeStateMachine::PipeStateMachine() :
    pause_time_ms_{250}
{
    delays_.clear();
    delays_.push_back(13);
    delays_.push_back(61);
    delays_.push_back(30);
    delays_.push_back(28);
}

void PipeStateMachine::SetUp()
{
    state_ = new Pipe();
}

void PipeStateMachine::TearDown()
{
    delete state_;
}

uint32_t PipeStateMachine::ValidateFullLoadDuration(const Pipe::FrameID frame_id)
{
    end_time_ = high_resolution_clock::now();
    auto all_tasks_time = duration_cast<milliseconds>(end_time_ - begin_time_).count();
    if (3 == frame_id)
        all_tasks_time -= pause_time_ms_;
    
    const auto max_delay = *max_element(cbegin(delays_), cend(delays_));
    
    uint32_t sum_delays = 0;
    for (const auto& delay : delays_)
        sum_delays += delay;
    
    
    EXPECT_LT(max_delay, sum_delays);
    EXPECT_LT(all_tasks_time, sum_delays);
    EXPECT_LT(all_tasks_time, uint32_t(max_delay * 1.2));/* Because of overhead */

    begin_time_ = high_resolution_clock::now();

    return all_tasks_time;
}

TEST_F(PipeStateMachine, creation_valid)
{
    EXPECT_NO_THROW({
        Pipe state{};
    });
}

TEST_F(PipeStateMachine, start_zero_tasks)
{
    Pipe state;

    EXPECT_THROW({
        state.Start({}, nullptr);
    }, invalid_argument);
}

TEST_F(PipeStateMachine, valid_execution)
{
    Pipe state;

    const auto task_1 = [&state](const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
    {
        state.TaskDone(task_id, frame_id);
    };

    EXPECT_NO_THROW({
        state.Start(
        {
          Pipe::Task{task_1, 100}
        }, nullptr);
    });

    state.Stop();

    while (state.IsRun())
        yield();
}

TEST_F(PipeStateMachine, restart)
{
    Pipe state;

    const auto task_1 = [&state](const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
    {
        state.TaskDone(task_id, frame_id);
    };

    EXPECT_NO_THROW({
    state.Start(
        {
            Pipe::Task{task_1, 100}
        }, nullptr);
    });

    EXPECT_THROW({
    state.Start(
        {
            Pipe::Task{task_1, 100}
        }, nullptr);
    }, runtime_error);

    state.Stop();

    while (state.IsRun())
        yield();
}

TEST_F(PipeStateMachine, restop)
{
    Pipe state;

    const auto task_1 = [&state](const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
    {
        state.TaskDone(task_id, frame_id);
    };

    EXPECT_NO_THROW({
    state.Start(
        {
            Pipe::Task{task_1, 100}
        }, nullptr);
    });

    state.Stop();

    while (state.IsRun())
        yield();

    EXPECT_THROW({
        state.Stop();
    }, runtime_error);
}

void PipeStateMachine::PrintStart(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    cout << "[Start " << task_id << "] Task:" << task_id << " - Frame: " << frame_id << std::endl;
}

void PipeStateMachine::PrintDone(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    cout << "[Done " << task_id << "] Task:" << task_id << " - Frame: " << frame_id << std::endl;
}

void PipeStateMachine::DummyProcess(const uint32_t wait_ms)
{
    sleep_for(milliseconds(wait_ms));
}

void PipeStateMachine::AsyncTask_101(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    assert(101 == task_id);
    PrintStart(task_id, frame_id);

    std::thread process([this](const auto task_id, const auto frame_id)
    {
        DummyProcess(delays_[0]);
        PrintDone(task_id, frame_id);
        assert(101 == task_id);
        state_->TaskDone(task_id, frame_id);
    }, task_id, frame_id);

    process.detach();
}

void PipeStateMachine::AsyncTask_57(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    assert(57 == task_id);
    PrintStart(task_id, frame_id);

    std::thread process([this](const auto task_id, const auto frame_id)
    {
        DummyProcess(delays_[1]);
        PrintDone(task_id, frame_id);
        assert(57 == task_id);
        state_->TaskDone(task_id, frame_id);
    }, task_id, frame_id);

    process.detach();
}

void PipeStateMachine::AsyncTask_94(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    assert(94 == task_id);
    PrintStart(task_id, frame_id);

    std::thread process([this](const auto task_id, const auto frame_id)
    {
        DummyProcess(delays_[2]);
        PrintDone(task_id, frame_id);
        assert(94 == task_id);
        state_->TaskDone(task_id, frame_id);
    }, task_id, frame_id);

    process.detach();
}

void PipeStateMachine::AsyncTask_193(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    assert(193 == task_id);
    PrintStart(task_id, frame_id);

    std::thread process([this](const auto task_id, const auto frame_id)
    {
        DummyProcess(delays_[3]);
        PrintDone(task_id, frame_id);
        assert(193 == task_id);
        state_->TaskDone(task_id, frame_id);
    }, task_id, frame_id);

    process.detach();
}

void PipeStateMachine::OnFrameComplete(const Pipe::FrameID frame_id)
{
    if (3 == frame_id) {
        state_->Pause();
        while (!state_->IsPaused());
        sleep_for(milliseconds(pause_time_ms_));
        state_->Play();
    } if (7 == frame_id) {
        state_->Stop();
    }

    const auto frame_duration_ms = ValidateFullLoadDuration(frame_id);
    cout << "[Frame Complete " << frame_id << "] " << 
        frame_duration_ms << "ms - Full load: " << state_->IsFullLoad() << " - " <<
        state_->GetRunningTasksCount() << "/" << state_->GetTasksCount() << std::endl << std::endl;
}

TEST_F(PipeStateMachine, flow)
{
    begin_time_ = high_resolution_clock::now();

    auto task_101       = bind(&PipeStateMachine::AsyncTask_101,   this, placeholders::_1, placeholders::_2);
    auto task_57        = bind(&PipeStateMachine::AsyncTask_57,    this, placeholders::_1, placeholders::_2);
    auto task_94        = bind(&PipeStateMachine::AsyncTask_94,    this, placeholders::_1, placeholders::_2);
    auto task_193       = bind(&PipeStateMachine::AsyncTask_193,   this, placeholders::_1, placeholders::_2);
    auto frame_complete = bind(&PipeStateMachine::OnFrameComplete, this, placeholders::_1);

    state_->Start(
    {
         Pipe::Task{task_101, 101},
         Pipe::Task{task_57,  57},
         Pipe::Task{task_94,  94},
         Pipe::Task{task_193, 193}
    }, frame_complete);

    while (state_->IsRun())
        yield();
}

int main()
{
   testing::InitGoogleTest();
   return RUN_ALL_TESTS();
}
