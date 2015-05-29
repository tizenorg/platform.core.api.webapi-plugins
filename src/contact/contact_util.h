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

#ifndef CONTACT_CONTACT_UTIL_H_
#define CONTACT_CONTACT_UTIL_H_

#include <ctime>
#include <memory>
#include <string>
#include <contacts.h>
#include "common/picojson.h"
#include "common/platform_exception.h"
#include "common/platform_result.h"

namespace extension {
namespace contact {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;
typedef std::string JsonString;

namespace ContactUtil {

void ContactsDeleter(contacts_record_h *contacts_record);
typedef std::unique_ptr<contacts_record_h, void (*)(contacts_record_h *)>
    ContactsRecordHPtr;

void ContactsListDeleter(contacts_list_h *contacts_list);
typedef std::unique_ptr<contacts_list_h, void (*)(contacts_list_h *)>
    ContactsListHPtr;

void ContactsFilterDeleter(contacts_filter_h contacts_filter);
typedef std::unique_ptr<std::remove_pointer<contacts_filter_h>::type,
                        void (*)(contacts_filter_h)> ContactsFilterPtr;

void ContactsQueryDeleter(contacts_query_h *contacts_query);
typedef std::unique_ptr<contacts_query_h, void (*)(contacts_query_h *)>
    ContactsQueryHPtr;

common::PlatformResult ErrorChecker(int err, const char *message);

common::PlatformResult GetStrFromRecord(contacts_record_h record,
                                        unsigned int property_id, char **value);

common::PlatformResult GetIntFromRecord(contacts_record_h record,
                                        unsigned int property_id, int *value);

common::PlatformResult GetBoolFromRecord(contacts_record_h record,
                                         unsigned int property_id, bool *value);

common::PlatformResult SetStrInRecord(contacts_record_h record,
                                      unsigned int property_id,
                                      const char *value);

common::PlatformResult SetIntInRecord(contacts_record_h record,
                                      unsigned int property_id, int value);

common::PlatformResult SetBoolInRecord(contacts_record_h record,
                                       unsigned int property_id, bool value);

common::PlatformResult ClearAllContactRecord(contacts_record_h contacts_record,
                                             unsigned int property_id);

common::PlatformResult GetNumberOfChildRecord(contacts_record_h contacts_record,
                                              unsigned int property_id,
                                              int *child_count);

common::PlatformResult UpdateAdditionalInformation(
    const ContactsRecordHPtr &contacts_record_ptr, JsonObject *out);

common::PlatformResult ImportContactNameFromContactsRecord(
    contacts_record_h contacts_record, JsonObject *out, bool *is_contact_name);
common::PlatformResult ExportContactNameToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportContactEmailAddressFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject *out);
common::PlatformResult ExportContactEmailAddressToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);

common::PlatformResult ImportContactAddressFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject *out);
common::PlatformResult ExportContactAddressToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportContactPhoneNumberFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject *out);
common::PlatformResult ExportContactPhoneNumberToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportContactOrganizationFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject *out);
common::PlatformResult ExportContactOrganizationToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportContactWebSiteFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject *out);
common::PlatformResult ExportContactWebSiteToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportContactAnniversariesFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject *out,
    bool *ret);
common::PlatformResult ExportContactAnniversariesToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportContactRelationshipFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject *out);
common::PlatformResult ExportContactRelationshipToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportContactInstantMessengerFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonObject *out);
common::PlatformResult ExportContactInstantMessengerToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);

common::PlatformResult ImportContactNotesFromContactsRecord(
    contacts_record_h contacts_record, unsigned int index, JsonValue *val);
common::PlatformResult ExportNotesToContactsRecord(
    contacts_record_h contacts_record, const std::string &value);
common::PlatformResult ImportContactFromContactsRecord(
    contacts_record_h contacts_record, JsonObject *out);
common::PlatformResult ExportPersonToContactsRecord(contacts_record_h record,
                                                    const JsonObject &args);

common::PlatformResult ExportContactToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportContactGroupFromContactsRecord(
    contacts_record_h contacts_record, JsonObject *out);
common::PlatformResult ExportContactGroupToContactsRecord(
    contacts_record_h contacts_record, const JsonObject &in);
common::PlatformResult ImportPersonFromContactsRecord(
    contacts_record_h contacts_record, JsonObject *out);

common::PlatformResult CheckDBConnection();

}  // ContactUtil
}  // contact
}  // extension

#endif  // CONTACT_CONTACT_UTIL_H_
