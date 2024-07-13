#ifndef ELB_PIPE_STATE_MACHINE_H_
#define ELB_PIPE_STATE_MACHINE_H_

#include <cstdbool>
#include <cstdint>
#include <thread>
#include <functional>
#include <initializer_list>
#include <map>
#include <atomic>


namespace ELB::StateMachine
{

class Pipe
{
public:
    using TaskID = uint32_t;
    using FrameID = uint32_t;
    using AsyncTask = std::function<void(const TaskID task_id, const FrameID frame_id)>;
    using OnFrameComplete = std::function<void(const FrameID frame_id)>;

    struct Task
    {
        AsyncTask async_task;
        TaskID    id;
    };

    Pipe();
    Pipe(const Pipe&) = delete;
    ~Pipe();
    Pipe& operator=(const Pipe&) = delete;
    void Pause();
    void Play();
    bool IsFullLoad();
    void Start(const std::initializer_list<Task> tasks, const OnFrameComplete on_frame_complete);
    void Stop();
    void TaskDone(const TaskID task_id, const FrameID frame_id);
    bool IsRun();
    bool IsPaused();
    uint32_t GetTasksCount();
    uint32_t GetRunningTasksCount();

private:
    struct TaskPair
    {
        Task task;
        volatile bool is_done;
    };
    using Priority = uint32_t;

    std::thread running_thread_;
    std::atomic<bool> is_running_;
    std::atomic<bool> is_paused_;
    std::atomic<bool> is_stop_requested_;
    std::map<Priority, TaskPair> tasks_;
    OnFrameComplete on_frame_complete_;
    FrameID frame_id_;
    uint32_t lower_bound_index_;
    uint32_t upper_bound_index_;

    void Run();
    void StartTasks();
    void WaitForTasks();
    void AddTasks(const std::initializer_list<Task>& tasks, const OnFrameComplete& on_frame_complete);
    void WaitOnPause();
    void FrameComplete();
    void UpdateFrameID();
    bool IsLastTaskStopped();
    void UpdateLowerBoundPriority();
    void UpdateUpperBoundPriority();
};

}

#endif /* ELB_PIPE_STATE_MACHINE_H_ */
