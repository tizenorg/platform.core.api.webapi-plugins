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

#ifndef COMMON_TIZEN_RESULT_H_
#define COMMON_TIZEN_RESULT_H_

#include "common/picojson.h"
#include "common/platform_result.h"

namespace common {

class TizenResult : public PlatformResult {
 public:
  void ToJson(picojson::object* msg) const;

 protected:
  using PlatformResult::PlatformResult;

  picojson::object data_;
};

class TizenSuccess : public TizenResult {
 public:
  TizenSuccess();
  explicit TizenSuccess(const picojson::value& r);
};

class TizenError : public TizenResult {
 protected:
  TizenError(const ErrorCode& error_code, const std::string& msg = "");
  TizenError(const ErrorCode& error_code, int platform_error);
};

#define DeclareErrorClass(name, code) \
class name : public TizenError { \
 public: \
  explicit name(const std::string& msg = ""); \
  explicit name(int error_code); \
 private: \
  static const ErrorCode code_ = code; \
}

DeclareErrorClass(IndexSizeError, ErrorCode::INDEX_SIZE_ERR);
DeclareErrorClass(DomStringSizeError, ErrorCode::DOMSTRING_SIZE_ERR);
DeclareErrorClass(HierarchyRequestError, ErrorCode::HIERARCHY_REQUEST_ERR);
DeclareErrorClass(WrongDocumentError, ErrorCode::WRONG_DOCUMENT_ERR);
DeclareErrorClass(InvalidCharacterError, ErrorCode::INVALID_CHARACTER_ERR);
DeclareErrorClass(NoDataAllowedError, ErrorCode::NO_DATA_ALLOWED_ERR);
DeclareErrorClass(NoModificationAllowedError, ErrorCode::NO_MODIFICATION_ALLOWED_ERR);
DeclareErrorClass(NotFoundError, ErrorCode::NOT_FOUND_ERR);
DeclareErrorClass(NotSupportedError, ErrorCode::NOT_SUPPORTED_ERR);
DeclareErrorClass(InUseAttributeError, ErrorCode::INUSE_ATTRIBUTE_ERR);
DeclareErrorClass(InvalidStateError, ErrorCode::INVALID_STATE_ERR);
DeclareErrorClass(SyntaxError, ErrorCode::SYNTAX_ERR);
DeclareErrorClass(InvalidModificationError, ErrorCode::INVALID_MODIFICATION_ERR);
DeclareErrorClass(NamespacesError, ErrorCode::NAMESPACE_ERR);
DeclareErrorClass(InvalidAccessError, ErrorCode::INVALID_ACCESS_ERR);
DeclareErrorClass(ValidationError, ErrorCode::VALIDATION_ERR);
DeclareErrorClass(TypeMismatchError, ErrorCode::TYPE_MISMATCH_ERR);
DeclareErrorClass(SecurityError, ErrorCode::SECURITY_ERR);
DeclareErrorClass(NetworkError, ErrorCode::NETWORK_ERR);
DeclareErrorClass(AbortError, ErrorCode::ABORT_ERR);
DeclareErrorClass(UrlMismatchError, ErrorCode::URL_MISMATCH_ERR);
DeclareErrorClass(QuotaExceededError, ErrorCode::QUOTA_EXCEEDED_ERR);
DeclareErrorClass(TimeoutError, ErrorCode::TIMEOUT_ERR);
DeclareErrorClass(InvalidNodeTypeError, ErrorCode::INVALID_NODE_TYPE_ERR);
DeclareErrorClass(DataCloneError, ErrorCode::DATA_CLONE_ERR);
DeclareErrorClass(InvalidValuesError, ErrorCode::INVALID_VALUES_ERR);
DeclareErrorClass(IoError, ErrorCode::IO_ERR);
DeclareErrorClass(PermissionDeniedError, ErrorCode::PERMISSION_DENIED_ERR);
DeclareErrorClass(ServiceNotAvailableError, ErrorCode::SERVICE_NOT_AVAILABLE_ERR);
DeclareErrorClass(DatabaseError, ErrorCode::DATABASE_ERR);
DeclareErrorClass(VerificationError, ErrorCode::VERIFICATION_ERR);
DeclareErrorClass(OperationCanceledError, ErrorCode::OPERATION_CANCELED_ERR);
DeclareErrorClass(UnknownError, ErrorCode::UNKNOWN_ERR);
#undef DeclareErrorClass

}  // namespace common

#endif  // COMMON_TIZEN_RESULT_H_
