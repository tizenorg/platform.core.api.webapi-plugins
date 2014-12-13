// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "callhistory.h"

namespace extension {
namespace callhistory {

CallHistory::CallHistory()
{
}

CallHistory::~CallHistory()
{
}

CallHistory* CallHistory::getInstance(){
    static CallHistory instance;
    return &instance;
}

void CallHistory::find()
{

}

void CallHistory::remove()
{

}

void CallHistory::removeBatch()
{

}

void CallHistory::removeAll()
{

}

long CallHistory::addChangeListener()
{

}

void CallHistory::removeChangeListener()
{

}

} // namespace callhistory
} // namespace extension
