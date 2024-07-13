#include <cassert>
#include <thread>
#include <chrono>
#include "gtest/gtest.h"


// TOOD(MN): use ms
constexpr uint32_t PROCESS_TIME_MS_101 = 13;
constexpr uint32_t PROCESS_TIME_MS_57 = 61;
constexpr uint32_t PROCESS_TIME_MS_94 = 30;
constexpr uint32_t PROCESS_TIME_MS_193 = 28;


class PipeTesting : public testing::Test
{
public:
    void SetUp() override;
    void TearDown() override;

private:
};

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
        State->TaskDone(task_id, frame_id);
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
        State->TaskDone(task_id, frame_id);
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
        State->TaskDone(task_id, frame_id);
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
Pipe* State = nullptr;



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
        State->TaskDone(task_id, frame_id);
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
        State->TaskDone(task_id, frame_id);
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
        State->TaskDone(task_id, frame_id);
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
        State->TaskDone(task_id, frame_id);
    }, task_id, frame_id);

    process.detach();
}

static void OnFrameComplete(const Pipe::FrameID frame_id)
{
    cout << "[Frame Complete " << frame_id << "] Full load: " << State->IsFullLoad() << " - " <<
        State->GetRunningTasksCount() << "/" << State->GetTasksCount() << std::endl << std::endl;

    if (3 == frame_id) {
        State->Pause();
        while (!State->IsPaused());
        sleep_for(milliseconds(250));
        State->Play();
    } if (7 == frame_id) {
        State->Stop();
    }
}

TEST(state_machine, flow)
{
    State = new Pipe();
    State->Start(
        {
         Pipe::Task{AsyncTask_101, 101},
         Pipe::Task{AsyncTask_57,  57},
         Pipe::Task(AsyncTask_94,  94),
         Pipe::Task(AsyncTask_193, 193),
         }, OnFrameComplete);

    while (State->IsRun())
        yield();

    delete State;
}

int main()
{
   testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
