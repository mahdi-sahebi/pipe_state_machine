#include "gtest/gtest.h"


TEST(creation, valid)
{
  EXPECT_NO_THROW({
  Pipe pipe();
  });
}

TEST(start, zero_tasks)
{
  FAIL();
}

TEST(execution, valid)
{
  FAIL();
}

TEST(execution, restart)
{
  FAIL();
}

TEST(execution, restop)
{
  FAIL();
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
