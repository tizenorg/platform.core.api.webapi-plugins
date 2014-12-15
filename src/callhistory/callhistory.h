// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CALLHISTORY_CALLHISTORY_H_
#define CALLHISTORY_CALLHISTORY_H_

namespace extension {
namespace callhistory {

class CallHistory
{
public:
    static CallHistory* getInstance();

    void find();
    void remove();
    void removeBatch();
    void removeAll();
    long addChangeListener();
    void removeChangeListener();

private:
    CallHistory();
    virtual ~CallHistory();

};

} // namespace callhistory
} // namespace extension

#endif // CALLHISTORY_CALLHISTORY_H_
