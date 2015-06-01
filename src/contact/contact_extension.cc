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

#include "contact/contact_extension.h"
#include "contact/contact_instance.h"

extern const char kSource_contact_api[];

common::Extension* CreateExtension() { return new ContactExtension; }

ContactExtension::ContactExtension() {
  SetExtensionName("tizen.contact");
  SetJavaScriptAPI(kSource_contact_api);

  const char* entry_points[] = {
      "tizen.ContactRef",              "tizen.ContactName",
      "tizen.ContactOrganization",     "tizen.ContactWebSite",
      "tizen.ContactAnniversary",      "tizen.ContactAddress",
      "tizen.ContactPhoneNumber",      "tizen.ContactEmailAddress",
      "tizen.ContactGroup",            "tizen.ContactRelationship",
      "tizen.ContactInstantMessenger", "tizen.Contact",
      "tizen.AddressBook",             NULL};
  SetExtraJSEntryPoints(entry_points);
}

ContactExtension::~ContactExtension() {}

common::Instance* ContactExtension::CreateInstance() {
  return new extension::contact::ContactInstance();
}
