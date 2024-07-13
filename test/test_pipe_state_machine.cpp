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


class PipeTesting : public testing::Test
{
public:
    PipeTesting();
    void SetUp() override;
    void TearDown() override;

protected:
    PipeStateMachine* state_;
    std::vector<uint32_t> delays_;
    std::chrono::time_point begin_time_;
    std::chrono::time_point end_time_;
    void ValidateFullLoadDuration();
};

PipeTesting::PipeTesting()
{
    delays.clear();
    delays.push_back(PROCESS_TIME_MS_101);
    delays.push_back(PROCESS_TIME_MS_57);
    delays.push_back(PROCESS_TIME_MS_94);
    delays.push_back(PROCESS_TIME_MS_193);
}

void PipeTesting::SetUp()
{
    pipe_ = new PipeStateMachine();
}

void PipeTesting::TearDown()
{
    delete pipe_;
}

void PipeTesting::ValidateFullLoadDuration()
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

TEST_F(PipeTesting, creation_valid)
{
    EXPECT_NO_THROW({
        Pipe state();
    });
}

TEST_F(PipeTesting, start_zero_tasks)
{
    Pipe state;

    EXPECT_THROW({
        state.Start({}, nullptr);
    , invalid_argument);
}

TEST_F(PipeTesting, valid_execution)
{
    Pipe state;

    const auto task_1 = [&state](const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
    {
        state_->TaskDone(task_id, frame_id);
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

TEST_F(PipeTesting, restart)
{
    Pipe state;

    const auto task_1 = [&state](const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
    {
        state_->TaskDone(task_id, frame_id);
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

TEST_F(PipeTesting, restop)
{
    Pipe state;

    const auto task_1 = [&state](const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
    {
        state_->TaskDone(task_id, frame_id);
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

TEST_F(PipeTesting, blocking)
{
  FAIL();
}

TEST_F(PipeTesting, async)
{
Pipe* state_ = nullptr;



static void PrintStart(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    cout << "[Start " << task_id << "] Task:" << task_id << " - Frame: " << frame_id << std::endl;
}

static void PrintDone(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
{
    cout << "[Done " << task_id << "] Task:" << task_id << " - Frame: " << frame_id << std::endl;
}

static void DummyProcess(const uint32_t wait_ms)
{
    sleep_for(milliseconds(wait_ms));
}

static void AsyncTask_101(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
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

static void AsyncTask_57(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
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

static void AsyncTask_94(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
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

static void AsyncTask_10193(const Pipe::TaskID task_id, const Pipe::FrameID frame_id)
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

static void OnFrameComplete(const Pipe::FrameID frame_id)
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
