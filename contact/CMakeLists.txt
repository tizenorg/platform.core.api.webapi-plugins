PKG_CHECK_MODULES(platform_pkgs_contact REQUIRED
    contacts-service2
    #libpcrecpp
)

SET(INCLUDES
    ${INCLUDE_COMMON}
    # external dependencies
    ${platform_pkgs_contact_INCLUDE_DIRS}
)

SET(SRCS
    "addressbook.cpp"
    "contact-manager.cpp"
    "contact-plugin.cpp"
    "contact-util.cpp"
    "person.cpp"
)

SET(LIBS
    ${LIBS_COMMON}
    # external dependencies
    ${platform_pkgs_contact_LIBRARIES}
)

add_target_library(contact "${INCLUDES}" "${SRCS}" "${LIBS}")
