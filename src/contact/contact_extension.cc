// Copyright 2014 Samsung Electronics Co, Ltd. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "contact/contact_extension.h"
#include "contact/contact_instance.h"

extern const char kSource_contact_api[];

common::Extension* CreateExtension() {
  return new ContactExtension;
}

ContactExtension::ContactExtension() {
  SetExtensionName("tizen.contact");
  SetJavaScriptAPI(kSource_contact_api);

  const char* entry_points[] = {
      "tizen.ContactRef",
      "tizen.ContactName",
      "tizen.ContactOrganization",
      "tizen.ContactWebSite",
      "tizen.ContactAnniversary",
      "tizen.ContactAddress",
      "tizen.ContactPhoneNumber",
      "tizen.ContactEmailAddress",
      "tizen.ContactGroup",
      "tizen.ContactRelationship",
      "tizen.ContactInstantMessenger",
      "tizen.Contact",
      "tizen.AddressBook",
      "tizen.Person",
      NULL
    };
    SetExtraJSEntryPoints(entry_points);
}

ContactExtension::~ContactExtension() {}

common::Instance* ContactExtension::CreateInstance() {
  return new extension::contact::ContactInstance;
}
