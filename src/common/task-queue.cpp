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

#include "task-queue.h"

namespace common {

TaskQueue& TaskQueue::GetInstance() {
    static TaskQueue task_queue;
    return task_queue;
}

template <>
gboolean TaskQueue::AfterWorkCallback<void>(gpointer data) {
    QueueData<void>* d = static_cast<QueueData<void>*>(data);
    if (nullptr != d) {
        d->after_work_callback_();
        delete d;
    }
    return FALSE;
}

template <>
void* TaskQueue::WorkCallback<void>(void* data) {
    QueueData<void>* d = static_cast<QueueData<void>*>(data);
    if (nullptr != d) {
        d->work_callback_();
        if (d->after_work_callback_) {
            g_idle_add(AfterWorkCallback<void>, d);
        }
    }
    return nullptr;
}

void TaskQueue::Queue(const std::function<void()>& work, const std::function<void()>& after_work) {
    QueueData<void>* d = new QueueData<void>();
    d->work_callback_ = work;
    d->after_work_callback_ = after_work;

    if (pthread_create(&d->thread_, nullptr, WorkCallback<void>, d) != 0) {
        LoggerE("Failed to create a background thread.");
        delete d;
    } else {
        pthread_detach(d->thread_);
    }
}

void TaskQueue::Async(const std::function<void()>& work) {
    QueueData<void>* d = new QueueData<void>();
    d->after_work_callback_ = work;
    g_idle_add(AfterWorkCallback<void>, d);
}

//TODO check if it would be needed in future
//void TaskQueue::AsyncResponse(int callback_handle, const std::shared_ptr<picojson::value>& response) {
//    Async<picojson::value>([callback_handle](const std::shared_ptr<picojson::value>& response) {
//        wrt::common::NativeContext::GetInstance()->InvokeCallback(callback_handle,
//                                                                 response->serialize());
//    }, response);
//}
} // namespace common
