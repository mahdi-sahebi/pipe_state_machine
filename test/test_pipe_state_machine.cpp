#include "gtest/gtest.h"


TEST(creation, valid)
{
  EXPECT_NO_THROW({
  Pipe state();
  });
}

TEST(start, zero_tasks)
{
  Pipe state;

  EXPECT_THROW({
  state.Start({}, nullptr);
  , invalid_argument);
}

TEST(execution, valid)
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

TEST(execution, restart)
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

TEST(execution, restop)
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

TEST(execution, blocking)
{
  FAIL();
}

TEST(execution, async)
{
  FAIL();
}

int main()
{
   testing::InitGoogleTest();
    return RUN_ALL_TESTS();
}
