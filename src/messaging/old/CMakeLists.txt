SET(TARGET_NAME ${messaging_target})
SET(DESTINATION_NAME ${messaging_dest})
SET(TARGET_IMPL_NAME ${messaging_impl})

PKG_CHECK_MODULES(platform_pkgs_messaging REQUIRED msg-service email-service
                  dbus-1 capi-system-info dbus-glib-1 tapi)

ADD_DEFINITIONS("-fvisibility=hidden")

SET(INCLUDE_DIRS
    ${filesystem_include}
    ${messaging_include}
    ${tizen_include}
    ${platform_pkgs_messaging_INCLUDE_DIRS}
)

INCLUDE_DIRECTORIES(${INCLUDE_COMMON} ${INCLUDE_DIRS})
MESSAGE(STATUS "${TARGET_NAME} module includes: INCLUDE_COMMON ${INCLUDE_DIRS}")

SET(CMAKE_INSTALL_RPATH
    ${CMAKE_INSTALL_RPATH}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${filesystem_dest}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${tizen_dest}
    ${CMAKE_INSTALL_PREFIX}/${DESTINATION_LIB_PREFIX}/${DESTINATION_NAME}
)

SET(SRCS_IMPL
    ChangeListenerContainer.cpp
    JSMessagingManager.cpp
    MessagingManager.cpp
    JSMessage.cpp
    JSMessageAttachment.cpp
    JSMessageBody.cpp
    JSMessageConversation.cpp
    JSMessageFolder.cpp
    JSMessageService.cpp
    MessageService.cpp
    MessageServiceShortMsg.cpp
    MessageServiceEmail.cpp
    ShortMsgManager.cpp
    EmailManager.cpp
    MessageCallbackUserData.cpp
    MessagesCallbackUserData.cpp
    ConversationCallbackData.cpp
    FoldersCallbackData.cpp
    JSMessageStorage.cpp
    MessageStorage.cpp
    MessageStorageEmail.cpp
    MessageStorageShortMsg.cpp
    MessagingUtil.cpp
    JSMessageAttachment.cpp
    MessageAttachment.cpp
    Message.cpp
    MessageEmail.cpp
    MessageMMS.cpp
    MessageSMS.cpp
    MessageConversation.cpp
    MessageBody.cpp
    MessageFolder.cpp
    JSMessageFolder.cpp
    MessagesChangeCallback.cpp
    ConversationsChangeCallback.cpp
    FoldersChangeCallback.cpp
    FindMsgCallbackUserData.cpp
    DBus/Connection.cpp
    DBus/Proxy.cpp
    DBus/SyncProxy.cpp
    DBus/LoadBodyProxy.cpp
    DBus/LoadAttachmentProxy.cpp
    DBus/EmailSignalProxy.cpp
    DBus/MessageProxy.cpp
    DBus/SendProxy.cpp
    MessagingDatabaseManager.cpp
)

ADD_LIBRARY(${TARGET_IMPL_NAME} SHARED ${SRCS_IMPL})

SET(LINK_LIBS
    ${LIBS_WIDGETDB}
    ${filesystem_config}
    ${filesystem_impl}
    ${tizen_impl}
    ${platform_pkgs_messaging_LIBRARIES}
)

TARGET_LINK_LIBRARIES(${TARGET_IMPL_NAME} ${LIBS_COMMON} ${LINK_LIBS})
MESSAGE(STATUS "${TARGET_NAME} module links: LIBS_COMMON ${LINK_LIBS}")

SET(SRCS
    plugin_config.cpp
    plugin_initializer.cpp
)

ADD_LIBRARY(${TARGET_NAME} SHARED ${SRCS})

TARGET_LINK_LIBRARIES(${TARGET_NAME}
    ${TARGET_IMPL_NAME}
    "-Wl,--no-as-needed" ${filesystem_config}
)

INSTALL(TARGETS ${TARGET_NAME} ${TARGET_IMPL_NAME} LIBRARY DESTINATION ${DESTINATION_LIB_PREFIX}/${DESTINATION_NAME})
INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/config.xml DESTINATION ${DESTINATION_LIB_PREFIX}/${DESTINATION_NAME})
INSTALL(
    DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${DESTINATION_HEADER_PREFIX}/messaging
    FILES_MATCHING PATTERN "*.h" PATTERN "CMakeFiles" EXCLUDE
)
