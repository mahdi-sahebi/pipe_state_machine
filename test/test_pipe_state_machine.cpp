#include <cassert>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"


using namespace std;
using namespace std::chrono;
using namespace std::this_thread;
constexpr uint32_t PROCESS_TIME_MS_101 = 13;
constexpr uint32_t PROCESS_TIME_MS_57 = 61;
constexpr uint32_t PROCESS_TIME_MS_94 = 30;
constexpr uint32_t PROCESS_TIME_MS_193 = 28;


class PipeStateMachine : public testing::Test
{
public:
    PipeStateMachine();
    void SetUp() override;
    void TearDown() override;

protected:
    PipeStateMachine* state_;
    std::vector<uint32_t> delays_;
    std::chrono::time_point begin_time_;
    std::chrono::time_point end_time_;

    void ValidateFullLoadDuration();
    void PrintStart(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void PrintDone(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void DummyProcess(const uint32_t wait_ms);
    void AsyncTask_101(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void AsyncTask_57(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void AsyncTask_94(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void AsyncTask_10193(const Pipe::TaskID task_id, const Pipe::FrameID frame_id);
    void OnFrameComplete(const Pipe::FrameID frame_id);
};

PipeStateMachine::PipeStateMachine()
{
    delays.clear();
    delays.push_back(PROCESS_TIME_MS_101);
    delays.push_back(PROCESS_TIME_MS_57);
    delays.push_back(PROCESS_TIME_MS_94);
    delays.push_back(PROCESS_TIME_MS_193);
}

void PipeStateMachine::SetUp()
{
    state_ = new PipeStateMachine();
}

void PipeStateMachine::TearDown()
{
    delete state_;
}

void PipeStateMachine::ValidateFullLoadDuration()
{
    end_time_ = high_resolution_clock::now();
    const auto all_tasks_time = duration_cast<milliseconds>(end_time_ - begin_time_).count();

    const auto max_delay = std::max_element(cbegin(delays_), cend(delays_));
    
    uint32_t sum_delays = 0;
    for (const auto&) delay : delays_)
        sum_delays += delay;
    
    EXPECT_LT(max_delay, sum_delays);
    EXPECT_LT(all_tasks_time, sum_delays);
    EXPECT_LT(all_tasks_time, uint32_t(max_delay * 1.2));

    begin_time_ = high_resolution_clock::now();
}

TEST_F(PipeStateMachine, creation_valid)
{
    EXPECT_NO_THROW({
        Pipe state();
    });
}

TEST_F(PipeStateMachine, start_zero_tasks)
{
    Pipe state;

    EXPECT_THROW({
        state.Start({}, nullptr);
    , invalid_argument);
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
    }

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

    std::thread process([](const auto task_id, const auto frame_id)
    {
        DummyProcess(PROCESS_TIME_MS_101);
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

    std::thread process([](const auto task_id, const auto frame_id)
    {
        DummyProcess(PROCESS_TIME_MS_57);
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

    std::thread process([](const auto task_id, const auto frame_id)
    {
        DummyProcess(PROCESS_TIME_MS_94);
        PrintDone(task_id, frame_id);
        assert(94 == task_id);
        state_->TaskDone(task_id, frame_id);
    }, task_id, frame_id);

    process.detach();
}

void PipeStateMachine::AsyncTask_10193(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    assert(193 == task_id);
    PrintStart(task_id, frame_id);

    std::thread process([](const auto task_id, const auto frame_id)
    {
        DummyProcess(PROCESS_TIME_MS_193);
        PrintDone(task_id, frame_id);
        assert(193 == task_id);
        state_->TaskDone(task_id, frame_id);
    }, task_id, frame_id);

    process.detach();
}

void PipeStateMachine::OnFrameComplete(const Pipe::FrameID frame_id)
{
    cout << "[Frame Complete " << frame_id << "] Full load: " << state_->IsFullLoad() << " - " <<
        state_->GetRunningTasksCount() << "/" << state_->GetTasksCount() << std::endl << std::endl;

    if (3 == frame_id) {
        state_->Pause();
        while (!state_->IsPaused());
        sleep_for(milliseconds(250));
        state_->Play();
    } if (7 == frame_id) {
        state_->Stop();
    }

    ValidateFullLoadDuration();
}

TEST_F(state_machine, flow)
{
    begin_time_ = high_resolution_clock::now();

    state_->Start(
    {
         Pipe::Task{AsyncTask_101, 101},
         Pipe::Task{AsyncTask_57,  57},
         Pipe::Task(AsyncTask_94,  94),
         Pipe::Task(AsyncTask_193, 193),
    }, OnFrameComplete);

    while (state_->IsRun())
        yield();
}

int main()
{
   testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
