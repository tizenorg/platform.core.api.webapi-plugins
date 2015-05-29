/*
 * Copyright (c) 2015 Samsung Electronics Co., Ltd All Rights Reserved
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */
 
#ifndef WEBAPI_PLUGINS_COMMON_THREAD_SCHEDULER_H_
#define WEBAPI_PLUGINS_COMMON_THREAD_SCHEDULER_H_

#include <glib.h>
#include <functional>
#include <memory>
#include <pthread.h>

#include "logger.h"

namespace common {

class TaskQueue {
public:
    TaskQueue(const TaskQueue&) = delete;
    TaskQueue& operator=(const TaskQueue&) = delete;

    static TaskQueue& GetInstance();

    /**
     * @brief Schedules work to be executed in a separate thread, after_work is going
     *        to be called in main glib loop.
     *
     * @param[in] work - callback is going to be called in a separate thread
     * @param[in] after_work - callback is going to be called in main glib loop
     * @param[in] data - data passed to both callbacks
     */
    template <class T>
    void Queue(const std::function<void(const std::shared_ptr<T>&)>& work,
               const std::function<void(const std::shared_ptr<T>&)>& after_work,
               const std::shared_ptr<T>& data);

    /**
     * @brief Schedules work to be executed in a separate thread, after_work is going
     *        to be called in main glib loop.
     *
     * @param[in] work - callback is going to be called in a separate thread
     * @param[in] after_work - callback is going to be called in main glib loop
     */
    void Queue(const std::function<void()>& work,
               const std::function<void()>& after_work = std::function<void()>());

    /**
     * @brief Schedules work to be executed in main glib loop.
     *
     * @param[in] work - callback is going to be called in main glib loop
     * @param[in] data - data passed to callback
     */
    template <class T>
    void Async(const std::function<void(const std::shared_ptr<T>&)>& work,
               const std::shared_ptr<T>& data);

    /**
     * @brief Schedules work to be executed in main glib loop.
     *
     * @param[in] work - callback is going to be called in main glib loop
     */
    void Async(const std::function<void()>& work);

    //TODO not needed now, but maybe in future
//    void AsyncResponse(int callback_handle, const std::shared_ptr<picojson::value>& response);

private:
    TaskQueue() {}

    template <class T>
    struct QueueData {
        pthread_t thread_;
        std::function<void(const std::shared_ptr<T>&)> work_callback_;
        std::function<void(const std::shared_ptr<T>&)> after_work_callback_;
        std::shared_ptr<T> data_;
    };

    template <class T>
    static void* WorkCallback(void* data);

    template <class T>
    static gboolean AfterWorkCallback(gpointer data);
};

template <>
struct TaskQueue::QueueData<void> {
    pthread_t thread_;
    std::function<void()> work_callback_;
    std::function<void()> after_work_callback_;
};

template <class T>
gboolean TaskQueue::AfterWorkCallback(gpointer data) {
    QueueData<T>* d = static_cast<QueueData<T>*>(data);
    if (nullptr != d) {
        d->after_work_callback_(d->data_);
        delete d;
    }
    return FALSE;
}

template <class T>
void* TaskQueue::WorkCallback(void* data) {
    QueueData<T>* d = static_cast<QueueData<T>*>(data);
    if (nullptr != d) {
        d->work_callback_(d->data_);
        g_idle_add(AfterWorkCallback<T>, d);
    }
    return nullptr;
}

template <class T>
void TaskQueue::Queue(const std::function<void(const std::shared_ptr<T>&)>& work,
                      const std::function<void(const std::shared_ptr<T>&)>& after_work,
                      const std::shared_ptr<T>& data) {
    QueueData<T>* d = new QueueData<T>();
    d->work_callback_ = work;
    d->after_work_callback_ = after_work;
    d->data_ = data;

    if (pthread_create(&d->thread_, nullptr, WorkCallback<T>, d) != 0) {
        LoggerE("Failed to create a background thread.");
        delete d;
    } else {
        pthread_detach(d->thread_);
    }
}

template <class T>
void TaskQueue::Async(const std::function<void(const std::shared_ptr<T>&)>& work,
                      const std::shared_ptr<T>& data) {
    QueueData<T>* d = new QueueData<T>();
    d->after_work_callback_ = work;
    d->data_ = data;
    g_idle_add(AfterWorkCallback<T>, d);
}

} // namespace common

#endif // WEBAPI_PLUGINS_COMMON_THREAD_SCHEDULER_H_
