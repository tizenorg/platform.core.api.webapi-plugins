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

#ifndef CALLHISTORY_CALLHISTORY_TYPES_H_
#define CALLHISTORY_CALLHISTORY_TYPES_H_

namespace extension {
namespace callhistory {

#define STR_CALLTYPE_TEL "TEL"
#define STR_CALLTYPE_XMPP "XMPP"
#define STR_CALLTYPE_SIP "SIP"

#define STR_CALL "CALL"
#define STR_CALL_VOICE "VOICECALL"
#define STR_CALL_VIDEO "VIDEOCALL"
#define STR_CALL_EMERGENCY "EMERGENCYCALL"

#define STR_DIALED "DIALED"
#define STR_RECEIVED "RECEIVED"
#define STR_MISSED_NEW "MISSEDNEW"
#define STR_MISSED "MISSED"
#define STR_REJECTED "REJECTED"
#define STR_BLOCKED "BLOCKED"

#define STR_DATA "data"
#define STR_ACTION "action"
#define STR_ENTRY_ID "uid"
#define STR_CALL_TYPE "type"
#define STR_TAGS "features"
#define STR_REMOTE_PARTIES "remoteParties"
#define STR_START_TIME "startTime"
#define STR_DURATION "duration"
#define STR_DIRECTION "direction"
#define STR_CALLING_PARTY "callingParty"

#define STR_REMOTE_PARTY "remoteParty"
#define STR_PERSON_ID "personId"

#define STR_RP_REMOTEPARTY "remoteParties.remoteParty"
#define STR_RP_PERSONID "remoteParties.personId"

#define STR_ORDER_ASC "ASC"

#define STR_FILTER_EXACTLY "EXACTLY"
#define STR_FILTER_FULLSTRING "FULLSTRING"
#define STR_FILTER_CONTAINS "CONTAINS"
#define STR_FILTER_STARTSWITH "STARTSWITH"
#define STR_FILTER_ENDSWITH "ENDSWITH"
#define STR_FILTER_EXISTS "EXISTS"

} // namespace callhistory
} // namespace extension

#endif // CALLHISTORY_CALLHISTORY_TYPES_H_
