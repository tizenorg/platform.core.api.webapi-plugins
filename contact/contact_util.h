/*
 * Copyright (c) 2014 Samsung Electronics Co., Ltd All Rights Reserved
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

namespace extension {
namespace contact {

typedef picojson::value JsonValue;
typedef picojson::object JsonObject;
typedef picojson::array JsonArray;

namespace ContactUtil {

extern const char *kContactReadPrivileges;
extern const char *kContactWritePrivileges;

void ContactsDeleter(contacts_record_h *contacts_record);
typedef std::unique_ptr<contacts_record_h, void (*)(contacts_record_h *)> ContactsRecordHPtr;

void ContactsListDeleter(contacts_list_h *contacts_list);
typedef std::unique_ptr<contacts_list_h, void (*)(contacts_list_h *)> ContactsListHPtr;

void ContactsFilterDeleter(contacts_filter_h contacts_filter);
typedef std::unique_ptr<std::remove_pointer<contacts_filter_h>::type, void (*)(contacts_filter_h)>
        ContactsFilterPtr;

void ContactsQueryDeleter(contacts_query_h *contacts_query);
typedef std::unique_ptr<contacts_query_h, void (*)(contacts_query_h *)> ContactsQueryHPtr;

void ErrorChecker(int err, const char *message);

void GetStrFromRecord(contacts_record_h record, unsigned int property_id, char **value);

void GetIntFromRecord(contacts_record_h record, unsigned int property_id, int *value);

void GetBoolFromRecord(contacts_record_h record, unsigned int property_id, bool *value);

void SetStrInRecord(contacts_record_h record, unsigned int property_id, const char *value);

void SetIntInRecord(contacts_record_h record, unsigned int property_id, int value);

void SetBoolInRecord(contacts_record_h record, unsigned int property_id, bool value);

void ClearAllContactRecord(contacts_record_h contacts_record, unsigned int property_id);

unsigned int GetNumberOfChildRecord(contacts_record_h contacts_record, unsigned int property_id);

void UpdateAdditionalInformation(const ContactsRecordHPtr &contacts_record_ptr, JsonObject *out);

JsonValue ImportBirthdayFromContactsRecord(contacts_record_h contacts_record, unsigned int index);
void ExportBirthdayToContactsRecord(contacts_record_h contacts_record, int date);
bool ImportContactNameFromContactsRecord(contacts_record_h contacts_record, JsonObject *out);
void ExportContactNameToContactsRecord(contacts_record_h contacts_record, const JsonObject &in);
void ImportContactEmailAddressFromContactsRecord(contacts_record_h contacts_record,
                                                 unsigned int index, JsonObject *out);
void ExportContactEmailAddressToContactsRecord(contacts_record_h contacts_record,
                                               const JsonObject &in);

void ImportContactAddressFromContactsRecord(contacts_record_h contacts_record, unsigned int index,
                                            JsonObject *out);
void ExportContactAddressToContactsRecord(contacts_record_h contacts_record, const JsonObject &in);
void ImportContactPhoneNumberFromContactsRecord(contacts_record_h contacts_record,
                                                unsigned int index, JsonObject *out);
void ExportContactPhoneNumberToContactsRecord(contacts_record_h contacts_record,
                                              const JsonObject &in);
void ImportContactOrganizationFromContactsRecord(contacts_record_h contacts_record,
                                                 unsigned int index, JsonObject *out);
void ExportContactOrganizationToContactsRecord(contacts_record_h contacts_record,
                                               const JsonObject &in);
void ImportContactWebSiteFromContactsRecord(contacts_record_h contacts_record, unsigned int index,
                                            JsonObject *out);
void ExportContactWebSiteToContactsRecord(contacts_record_h contacts_record, const JsonObject &in);
bool ImportContactAnniversariesFromContactsRecord(contacts_record_h contacts_record,
                                                  unsigned int index, JsonObject *out);
void ExportContactAnniversariesToContactsRecord(contacts_record_h contacts_record,
                                                const JsonObject &in);
void ImportContactRelationshipFromContactsRecord(contacts_record_h contacts_record,
                                                 unsigned int index, JsonObject *out);
void ExportContactRelationshipToContactsRecord(contacts_record_h contacts_record,
                                               const JsonObject &in);
void ImportContactInstantMessengerFromContactsRecord(contacts_record_h contacts_record,
                                                     unsigned int index, JsonObject *out);
void ExportContactInstantMessengerToContactsRecord(contacts_record_h contacts_record,
                                                   const JsonObject &in);

JsonValue ImportContactNotesFromContactsRecord(contacts_record_h contacts_record,
                                               unsigned int index);
JsonValue ImportContactNotesFromContactsRecord(contacts_record_h contacts_record,
                                               unsigned int index);
void ExportNotesToContactsRecord(contacts_record_h contacts_record, const std::string &value);
void ImportContactFromContactsRecord(contacts_record_h contacts_record, JsonObject *out);
void ExportPersonToContactsRecord(contacts_record_h record, const JsonObject &args);

void ExportContactToContactsRecord(contacts_record_h contacts_record, const JsonObject &in);
void ImportContactGroupFromContactsRecord(contacts_record_h contacts_record, JsonObject *out);
void ExportContactGroupToContactsRecord(contacts_record_h contacts_record, const JsonObject &in);
void ImportPersonFromContactsRecord(contacts_record_h contacts_record, JsonObject *out);

}  // ContactUtil
}  // contact
}  // extension

#endif  // CONTACT_CONTACT_UTIL_H_
