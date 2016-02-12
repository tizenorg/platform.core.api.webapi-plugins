/*
 * Copyright (c) 2016 Samsung Electronics Co., Ltd All Rights Reserved
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

#include "common/tizen_result.h"

#include "common/tools.h"

namespace common {

void TizenResult::ToJson(picojson::object* msg) const {
  for (auto& d : data_) {
    msg->insert(d);
  }
}

TizenSuccess::TizenSuccess()
    : TizenResult(ErrorCode::NO_ERROR) {
  tools::ReportSuccess(data_);
}

TizenSuccess::TizenSuccess(const picojson::value& r)
    : TizenResult(ErrorCode::NO_ERROR) {
  tools::ReportSuccess(r, data_);
}

TizenError::TizenError(const ErrorCode& error_code, const std::string& msg)
    : TizenResult(error_code, msg) {
  tools::ReportError(*this, &data_);
}

TizenError::TizenError(const ErrorCode& error_code, int platform_error)
    : TizenResult(error_code, std::string("Platform error: ") + get_error_message(platform_error)) {
  tools::ReportError(*this, &data_);
}

#define DefineErrorClass(name) \
const ErrorCode name::code_; \
name::name(const std::string& msg) : TizenError(code_, msg) {} \
name::name(int error_code) : TizenError(code_, error_code) {}

DefineErrorClass(IndexSizeError);
DefineErrorClass(DomStringSizeError);
DefineErrorClass(HierarchyRequestError);
DefineErrorClass(WrongDocumentError);
DefineErrorClass(InvalidCharacterError);
DefineErrorClass(NoDataAllowedError);
DefineErrorClass(NoModificationAllowedError);
DefineErrorClass(NotFoundError);
DefineErrorClass(NotSupportedError);
DefineErrorClass(InUseAttributeError);
DefineErrorClass(InvalidStateError);
DefineErrorClass(SyntaxError);
DefineErrorClass(InvalidModificationError);
DefineErrorClass(NamespacesError);
DefineErrorClass(InvalidAccessError);
DefineErrorClass(ValidationError);
DefineErrorClass(TypeMismatchError);
DefineErrorClass(SecurityError);
DefineErrorClass(NetworkError);
DefineErrorClass(AbortError);
DefineErrorClass(UrlMismatchError);
DefineErrorClass(QuotaExceededError);
DefineErrorClass(TimeoutError);
DefineErrorClass(InvalidNodeTypeError);
DefineErrorClass(DataCloneError);
DefineErrorClass(InvalidValuesError);
DefineErrorClass(IoError);
DefineErrorClass(PermissionDeniedError);
DefineErrorClass(ServiceNotAvailableError);
DefineErrorClass(DatabaseError);
DefineErrorClass(VerificationError);
DefineErrorClass(OperationCanceledError);
DefineErrorClass(UnknownError);
#undef DefineErrorClass

}  // namespace common
