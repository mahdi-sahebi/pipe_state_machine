#include <cassert>
#include "pipe_state_machine/pipe_state_machine.hpp"


using namespace ELB::StateMachine;
using namespace std;
using namespace std::chrono;
using namespace std::this_thread;


Pipe::Pipe() :
    is_running_{false}, is_paused_{false}, is_stop_requested_{false}, frame_id_{0}
{
}

Pipe::~Pipe()
{
    if (IsRun())
        Stop();

    if(running_thread_.joinable())
        running_thread_.join();
}

void Pipe::StartTasks()
{
    uint32_t task_index = tasks_.size();

    while (task_index--) {
        if ((task_index < lower_bound_index_) || (task_index > frame_id_))
            continue;

        assert(true == tasks_[task_index].is_done);
        tasks_[task_index].is_done = false;
        tasks_[task_index].task.async_task(tasks_[task_index].task.id, frame_id_ - task_index);
    }
}

void Pipe::WaitForTasks()
{
    uint32_t task_index = tasks_.size();

    while (task_index--) {
        if ((task_index < lower_bound_index_) || (task_index > frame_id_))
            continue;

        while (false == tasks_[task_index].is_done)
            sleep_for(microseconds(1));
    }
}

bool Pipe::IsFullLoad()
{
    return (tasks_.size() == GetRunningTasksCount());
}

void Pipe::WaitOnPause()
{
    while (is_paused_)
        sleep_for(microseconds(1));
}

void Pipe::Pause()
{
    is_paused_ = true;
}

void Pipe::Play()
{
    is_paused_ = false;
}

bool Pipe::IsPaused()
{
    return is_paused_;
};

void Pipe::UpdateFrameID()
{
    frame_id_++;
}

bool Pipe::IsLastTaskStopped()
{
    return ((lower_bound_index_ + 1) >= tasks_.size());
}

void Pipe::UpdateLowerBoundPriority()
{
    if (is_stop_requested_)
        lower_bound_index_++;
}

void Pipe::UpdateUpperBoundPriority()
{
    if ((upper_bound_index_ < tasks_.size()) && (frame_id_ < tasks_.size()))
        upper_bound_index_++;
}

void Pipe::FrameComplete()
{
    if (nullptr != on_frame_complete_)
    on_frame_complete_(frame_id_);
}

void Pipe::Run()
{
    while (is_running_ && !IsLastTaskStopped()) {
        WaitOnPause();
        UpdateLowerBoundPriority();
        StartTasks();
        WaitForTasks();
        FrameComplete();
        UpdateUpperBoundPriority();
        UpdateFrameID();
    }

    is_running_ = false;
}

void Pipe::AddTasks(const initializer_list<Task>& tasks, const OnFrameComplete& on_frame_complete)
{
    tasks_.clear();
    on_frame_complete_ = on_frame_complete;

    Priority priority{0};

    for (auto task = cbegin(tasks); task != cend(tasks); task++) {
        const auto itr = find_if(cbegin(tasks_), cend(tasks_),
            [&task](const pair<const Priority, TaskPair>& data) -> bool
            {
              return data.second.task.id == task->id;
            });

        if (itr != cend(tasks_))
            throw invalid_argument("[Pipe] Repetitive task id");

        tasks_[priority] = TaskPair{*task, true};
        priority++;
    }
}

void Pipe::Start(const initializer_list<Task> tasks, const OnFrameComplete on_frame_complete)
{
    if (0 == tasks.size())
        throw invalid_argument("[Pipe] Tasks list is empty");
    if (is_running_ || running_thread_.joinable())
        throw runtime_error("[Pipe] Is already running");

    AddTasks(tasks, on_frame_complete);

    is_running_ = true;
    is_paused_ = false;
    is_stop_requested_ = false;
    frame_id_ = 0;

    lower_bound_index_ = 0;
    upper_bound_index_ = 1;

    running_thread_ = thread(&Pipe::Run, this);
}

void Pipe::Stop()
{
    if (!is_running_)
        throw runtime_error("[Pipe] Is already closed");
    if (is_stop_requested_)
        throw runtime_error("[Pipe] Is already closing");

    is_stop_requested_ = true;
}

void Pipe::TaskDone(const TaskID task_id, const FrameID frame_id)
{
    auto task = find_if(begin(tasks_), end(tasks_),
        [&task_id](const pair<Priority, TaskPair> element) -> bool
        {
            return (element.second.task.id == task_id);
        });

    if (cend(tasks_) == task)
        throw invalid_argument("[Pipe] Invalid task id");

    task->second.is_done = true;
}

bool Pipe::IsRun()
{
    return is_running_;
}

uint32_t Pipe::GetTasksCount()
{
    return tasks_.size();
}

uint32_t Pipe::GetRunningTasksCount()
{
    return (upper_bound_index_ - lower_bound_index_);
}
