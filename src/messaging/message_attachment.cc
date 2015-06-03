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
 
#include <ctype.h>
#include <unordered_map>

#include "message_attachment.h"

#include "common/logger.h"
#include "common/virtual_fs.h"

namespace extension {
namespace messaging {

std::map<std::string, unsigned int>& MessageAttachment::MIMETypeStringToEnumMap = initializeMIMETypeStringToEnumMap();
std::map<unsigned int, std::string>& MessageAttachment::MIMETypeEnumToStringMap = initializeMIMETypeEnumToStringMap();

MessageAttachment::MessageAttachment()
{
    LoggerD("MessageAttachment constructor (%p)", this);
    m_id = -1;
    m_isIdSet = false;
    m_messageId = -1;
    m_isMessageIdSet = false;
    m_mimeType = "";
    m_isMimeTypeSet = false;
    m_filePath = "";
    m_isFilePathSet = false;
    m_isSaved = true;
}

MessageAttachment::~MessageAttachment()
{
    LoggerD("MessageAttachment destructor (%p)", this);
}

// id

int MessageAttachment::getId()
{
    return m_id;
}

void MessageAttachment::setId(int value)
{
    m_id = value;
    m_isIdSet = true;
}

bool MessageAttachment::isIdSet()
{
    return m_isIdSet;
}

void MessageAttachment::unsetId()
{
    m_isIdSet = false;
}

// messageId

int MessageAttachment::getMessageId()
{
    return m_messageId;
}

void MessageAttachment::setMessageId(int value)
{
    m_messageId = value;
    m_isMessageIdSet = true;
}

bool MessageAttachment::isMessageIdSet()
{
    return m_isMessageIdSet;
}

void MessageAttachment::unsetMessageId()
{
    m_isMessageIdSet = false;
}

// mimeType

std::string MessageAttachment::getMimeType()
{
    return m_mimeType;
}

void MessageAttachment::setMimeType(const std::string &value)
{
    m_mimeType = value;
    m_isMimeTypeSet = true;
}

bool MessageAttachment::isMimeTypeSet()
{
    return m_isMimeTypeSet;
}

void MessageAttachment::unsetMimeType()
{
    m_isMimeTypeSet = false;
}

// filePath

std::string MessageAttachment::getFilePath()
{
    return m_filePath;
}

std::string MessageAttachment::getShortFileName() const
{
    LoggerD("Entered");
    if (!m_isFilePathSet) {
        return "";
    }
    size_t pos;
    // find position of last occurence of / sign (get only file name from all path
    pos = m_filePath.find_last_of("/");

    if ((pos + 1) >= m_filePath.size() || pos == std::string::npos) {
        return m_filePath;
    }
    return m_filePath.substr(pos + 1);
}

void MessageAttachment::setFilePath(const std::string &value)
{
    LoggerD("Entered");

    m_filePath = common::VirtualFs::GetInstance().GetRealPath(value);
    m_isFilePathSet = true;
}

bool MessageAttachment::isFilePathSet()
{
    return m_isFilePathSet;
}

void MessageAttachment::unsetFilePath()
{
    m_isFilePathSet = false;
}

void MessageAttachment::setIsSaved(bool isSaved)
{
    m_isSaved = isSaved;
}

bool MessageAttachment::isSaved() const
{
    return m_isSaved;
}

std::map<unsigned int, std::string>& MessageAttachment::initializeMIMETypeEnumToStringMap()
{
    LoggerD("Entered");
    static std::map<unsigned int, std::string> enumToString;
    //0
    enumToString[MIME_ASTERISK] = "*/*";
    //1
    enumToString[MIME_APPLICATION_XML] = "application/xml";
    enumToString[MIME_APPLICATION_WML_XML] = "application/wml+xml";
    enumToString[MIME_APPLICATION_XHTML_XML] = "application/xhtml+xml";
    enumToString[MIME_APPLICATION_JAVA_VM] = "application/java-vm";
    enumToString[MIME_APPLICATION_SMIL] = "application/smil";
    enumToString[MIME_APPLICATION_JAVA_ARCHIVE] = "application/java-archive";
    enumToString[MIME_APPLICATION_JAVA] = "application/java";
    enumToString[MIME_APPLICATION_OCTET_STREAM] = "application/octet-stream";
    enumToString[MIME_APPLICATION_STUDIOM] = "application/studiom";
    enumToString[MIME_APPLICATION_FUNMEDIA] = "application/funMedia";
    enumToString[MIME_APPLICATION_MSWORD] = "application/msword";
    enumToString[MIME_APPLICATION_PDF] = "application/pdf";
    enumToString[MIME_APPLICATION_SDP] = "application/sdp";
    enumToString[MIME_APPLICATION_RAM] = "application/ram";
    enumToString[MIME_APPLICATION_ASTERIC] = "application/*";
    //16
    enumToString[MIME_APPLICATION_VND_WAP_XHTMLXML] = "application/vnd.wap.xhtml+xml";
    enumToString[MIME_APPLICATION_VND_WAP_WMLC] = "application/vnd.wap.wmlc";
    enumToString[MIME_APPLICATION_VND_WAP_WMLSCRIPTC] = "application/vnd.wap.wmlscriptc";
    enumToString[MIME_APPLICATION_VND_WAP_WTA_EVENTC] = "application/vnd.wap.wta-eventc";
    enumToString[MIME_APPLICATION_VND_WAP_UAPROF] = "application/vnd.wap.uaprof";
    enumToString[MIME_APPLICATION_VND_WAP_SIC] = "application/vnd.wap.sic";
    enumToString[MIME_APPLICATION_VND_WAP_SLC] = "application/vnd.wap.slc";
    enumToString[MIME_APPLICATION_VND_WAP_COC] = "application/vnd.wap.coc";
    enumToString[MIME_APPLICATION_VND_WAP_SIA] = "application/vnd.wap.sia";
    enumToString[MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML] = "application/vnd.wap.connectivity-wbxml";
    enumToString[MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA] = "application/vnd.wap.multipart.form-data";
    enumToString[MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES] = "application/vnd.wap.multipart.byteranges";
    enumToString[MIME_APPLICATION_VND_WAP_MULTIPART_MIXED] = "application/vnd.wap.multipart.mixed";
    enumToString[MIME_APPLICATION_VND_WAP_MULTIPART_RELATED] = "application/vnd.wap.multipart.related";
    enumToString[MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE] = "application/vnd.wap.multipart.alternative";
    enumToString[MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC] = "application/vnd.wap.multipart.*";
    enumToString[MIME_APPLICATION_VND_WAP_WBXML] = "application/vnd.wap.wbxml";
    enumToString[MIME_APPLICATION_VND_OMA_DD_XML] = "application/vnd.oma.dd+xml";
    enumToString[MIME_APPLICATION_VND_OMA_DRM_MESSAGE] = "application/vnd.oma.drm.message";
    enumToString[MIME_APPLICATION_VND_OMA_DRM_CONTENT] = "application/vnd.oma.drm.content";
    enumToString[MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML] = "application/vnd.oma.drm.rights+xml";
    enumToString[MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML] = "application/vnd.oma.drm.rights+wbxml";
    enumToString[MIME_APPLICATION_VND_OMA_DRM_RO_XML] = "application/vnd.oma.drm.ro+xml";
    enumToString[MIME_APPLICATION_VND_OMA_DRM_DCF] = "application/vnd.oma.drm.dcf";
    enumToString[MIME_APPLICATION_VND_OMA_ROAPPDU_XML] = "application/vnd.oma.drm.roap-pdu+xml";
    enumToString[MIME_APPLICATION_VND_OMA_ROAPTRIGGER_XML] = "application/vnd.oma.drm.roap-trigger+xml";
    enumToString[MIME_APPLICATION_VND_SMAF] = "application/vnd.smaf";
    enumToString[MIME_APPLICATION_VND_RN_REALMEDIA] = "application/vnd.rn-realmedia";
    enumToString[MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE] = "application/vnd.sun.j2me.java-archive";
    enumToString[MIME_APPLICATION_VND_SAMSUNG_THEME] = "application/vnd.samsung.theme";
    enumToString[MIME_APPLICATION_VND_EXCEL] = "application/vnd.ms-excel";
    enumToString[MIME_APPLICATION_VND_POWERPOINT] = "application/vnd.ms-powerpoint";
    enumToString[MIME_APPLICATION_VND_MSWORD] = "applcation/vnd.ms-word";
    //49
    enumToString[MIME_APPLICATION_X_HDMLC] = "application/x-hdmlc";
    enumToString[MIME_APPLICATION_X_X968_USERCERT] = "application/x-x968-user-cert";
    enumToString[MIME_APPLICATION_X_WWW_FORM_URLENCODED] = "application/x-www-form-urlencoded";
    enumToString[MIME_APPLICATION_X_SMAF] = "application/x-smaf";
    enumToString[MIME_APPLICATION_X_FLASH] = "application/x-shockwave-flash";
    enumToString[MIME_APPLICATION_X_EXCEL] = "application/x-msexcel";
    enumToString[MIME_APPLICATION_X_POWERPOINT] = "application/x-mspowerpoint";
    //56
    enumToString[MIME_AUDIO_BASIC] = "audio/basic";
    enumToString[MIME_AUDIO_MPEG] = "audio/mpeg";
    enumToString[MIME_AUDIO_MP3] = "audio/mp3";
    enumToString[MIME_AUDIO_MPG3] = "audio/mpg3";
    enumToString[MIME_AUDIO_MPEG3] = "audio/mpeg3";
    enumToString[MIME_AUDIO_MPG] = "audio/mpg";
    enumToString[MIME_AUDIO_AAC] = "audio/aac";
    enumToString[MIME_AUDIO_G72] = "audio/g72";
    enumToString[MIME_AUDIO_AMR] = "audio/amr";
    enumToString[MIME_AUDIO_AMR_WB] = "audio/amr-wb";
    enumToString[MIME_AUDIO_MMF] = "audio/mmf";
    enumToString[MIME_AUDIO_SMAF] = "audio/smaf";
    enumToString[MIME_AUDIO_IMELODY] = "audio/iMelody";
    enumToString[MIME_AUDIO_IMELODY2] = "audio/imelody";
    enumToString[MIME_AUDIO_MELODY] = "audio/melody";
    enumToString[MIME_AUDIO_MID] = "audio/mid";
    enumToString[MIME_AUDIO_MIDI] = "audio/midi";
    enumToString[MIME_AUDIO_SP_MIDI] = "audio/sp-midi";
    enumToString[MIME_AUDIO_WAVE] = "audio/wave";
    enumToString[MIME_AUDIO_WAV] = "audio/wav";
    enumToString[MIME_AUDIO_3GPP] = "audio/3gpp";
    enumToString[MIME_AUDIO_MP4] = "audio/mp4";
    enumToString[MIME_AUDIO_MP4A_LATM] = "audio/MP4A-LATM";
    enumToString[MIME_AUDIO_M4A] = "audio/m4a";
    enumToString[MIME_AUDIO_MPEG4] = "audio/mpeg4";
    enumToString[MIME_AUDIO_WMA] = "audio/wma";
    enumToString[MIME_AUDIO_XMF] = "audio/xmf";
    enumToString[MIME_AUDIO_IMY] = "audio/imy";
    enumToString[MIME_AUDIO_MOBILE_XMF] = "audio/mobile-xmf";
    //85
    enumToString[MIME_AUDIO_VND_RN_REALAUDIO] = "audio/vnd.rn-realaudio";
    //86
    enumToString[MIME_AUDIO_X_MPEG] = "audio/x-mpeg";
    enumToString[MIME_AUDIO_X_MP3] = "audio/x-mp3";
    enumToString[MIME_AUDIO_X_MPEG3] = "audio/x-mpeg3";
    enumToString[MIME_AUDIO_X_MPG] = "audio/x-mpg";
    enumToString[MIME_AUDIO_X_AMR] = "audio/x-amr";
    enumToString[MIME_AUDIO_X_MMF] = "audio/x-mmf";
    enumToString[MIME_AUDIO_X_SMAF] = "audio/x-smaf";
    enumToString[MIME_AUDIO_X_IMELODY] = "audio/x-iMelody";
    enumToString[MIME_AUDIO_X_MIDI] = "audio/x-midi";
    enumToString[MIME_AUDIO_X_MPEGAUDIO] = "audio/x-mpegaudio";
    enumToString[MIME_AUDIO_X_PN_REALAUDIO] = "audio/x-pn-realaudio";
    enumToString[MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO] = "audio/x-pn-multirate-realaudio";
    enumToString[MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE] = "audio/x-pn-multirate-realaudio-live";
    enumToString[MIME_AUDIO_X_WAVE] = "audio/x-wave";
    enumToString[MIME_AUDIO_X_WAV] = "audio/x-wav";
    enumToString[MIME_AUDIO_X_MS_WMA] = "audio/x-ms-wma";
    enumToString[MIME_AUDIO_X_MID] = "audio/x-mid";
    enumToString[MIME_AUDIO_X_MS_ASF] = "audio/x-ms-asf";
    enumToString[MIME_AUDIO_X_XMF] = "audio/x-xmf";
    //105
    enumToString[MIME_IMAGE_GIF] = "image/gif";
    enumToString[MIME_IMAGE_JPEG] = "image/jpeg";
    enumToString[MIME_IMAGE_JPG] = "image/jpg";
    enumToString[MIME_IMAGE_TIFF] = "image/tiff";
    enumToString[MIME_IMAGE_TIF] = "image/tif";
    enumToString[MIME_IMAGE_PNG] = "image/png";
    enumToString[MIME_IMAGE_WBMP] = "image/wbmp";
    enumToString[MIME_IMAGE_PJPEG] = "image/pjpeg";
    enumToString[MIME_IMAGE_BMP] = "image/bmp";
    enumToString[MIME_IMAGE_SVG] = "image/svg+xml";
    enumToString[MIME_IMAGE_SVG1] = "image/svg-xml";
    //116
    enumToString[MIME_IMAGE_VND_WAP_WBMP] = "image/vnd.wap.wbmp";
    enumToString[MIME_IMAGE_VND_TMO_GIF] = "image/vnd.tmo.my5-gif";
    enumToString[MIME_IMAGE_VND_TMO_JPG] = "image/vnd.tmo.my5-jpg";
    //119
    enumToString[MIME_IMAGE_X_BMP] = "image/x-bmp";
    //120
    enumToString[MIME_MESSAGE_RFC822] = "message/rfc822";
    //121
    enumToString[MIME_MULTIPART_MIXED] = "multipart/mixed";
    enumToString[MIME_MULTIPART_RELATED] = "multipart/related";
    enumToString[MIME_MULTIPART_ALTERNATIVE] = "multipart/alternative";
    enumToString[MIME_MULTIPART_FORM_DATA] = "multipart/form-data";
    enumToString[MIME_MULTIPART_BYTERANGE] = "multipart/byterange";
    enumToString[MIME_MULTIPART_REPORT] = "multipart/report";
    enumToString[MIME_MULTIPART_VOICE_MESSAGE] = "multipart/voice-message";
    //128
    enumToString[MIME_TEXT_TXT] = "text/txt";
    enumToString[MIME_TEXT_HTML] = "text/html";
    enumToString[MIME_TEXT_PLAIN] = "text/plain";
    enumToString[MIME_TEXT_CSS] = "text/css";
    enumToString[MIME_TEXT_XML] = "text/xml";
    enumToString[MIME_TEXT_IMELODY] = "text/iMelody";
    //134
    enumToString[MIME_TEXT_VND_WAP_WMLSCRIPT] = "text/vnd.wap.wmlscript";
    enumToString[MIME_TEXT_VND_WAP_WML] = "text/vnd.wap.wml";
    enumToString[MIME_TEXT_VND_WAP_WTA_EVENT] = "text/vnd.wap.wta-event";
    enumToString[MIME_TEXT_VND_WAP_CONNECTIVITY_XML] = "text/vnd.wap.connectivity-xml";
    enumToString[MIME_TEXT_VND_WAP_SI] = "text/vnd.wap.si";
    enumToString[MIME_TEXT_VND_WAP_SL] = "text/vnd.wap.sl";
    enumToString[MIME_TEXT_VND_WAP_CO] = "text/vnd.wap.co";
    enumToString[MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR] = "text/vnd.sun.j2me.app-descriptor";
    //142
    enumToString[MIME_TEXT_X_HDML] = "text/x-hdml";
    enumToString[MIME_TEXT_X_VCALENDAR] = "text/x-vCalendar";
    enumToString[MIME_TEXT_X_VCARD] = "text/x-vCard";
    enumToString[MIME_TEXT_X_IMELODY] = "text/x-iMelody";
    enumToString[MIME_TEXT_X_IMELODY2] = "text/x-imelody";
    enumToString[MIME_TEXT_X_VNOTE] = "text/x-vnote";
    //148
    enumToString[MIME_VIDEO_MPEG4] = "video/mpeg4";
    enumToString[MIME_VIDEO_MP4] = "video/mp4";
    enumToString[MIME_VIDEO_H263] = "video/h263";
    enumToString[MIME_VIDEO_3GPP] = "video/3gpp";
    enumToString[MIME_VIDEO_3GP] = "video/3gp";
    enumToString[MIME_VIDEO_AVI] = "video/avi";
    enumToString[MIME_VIDEO_SDP] = "video/sdp";
    enumToString[MIME_VIDEO_MP4_ES] = "video/mp4v-es";
    enumToString[MIME_VIDEO_MPEG] = "video/mpeg";
    //157
    enumToString[MIME_VIDEO_VND_RN_REALVIDEO] = "video/vnd.rn-realvideo";
    enumToString[MIME_VIDEO_VND_RN_REALMEDIA] = "video/vnd.rn-realmedia";
    //159
    enumToString[MIME_VIDEO_X_MP4] = "video/x-mp4";
    enumToString[MIME_VIDEO_X_PV_MP4] = "video/x-pv-mp4";
    enumToString[MIME_VIDEO_X_PN_REALVIDEO] = "video/x-pn-realvideo";
    enumToString[MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO] = "video/x-pn-multirate-realvideo";
    enumToString[MIME_VIDEO_X_MS_WMV] = "video/x-ms-wmv";
    enumToString[MIME_VIDEO_X_MS_ASF] = "video/x-ms-asf";
    enumToString[MIME_VIDEO_X_PV_PVX] = "video/x-pv-pvx";

    return enumToString;
}

std::map<std::string, unsigned int>& MessageAttachment::initializeMIMETypeStringToEnumMap()
{
    LoggerD("Entered");
    static std::map<std::string, unsigned int> stringToEnum;
    //0
    stringToEnum["*/*"] = MIME_ASTERISK;
    //1
    stringToEnum["application/xml"] = MIME_APPLICATION_XML;
    stringToEnum["application/wml+xml"] = MIME_APPLICATION_WML_XML;
    stringToEnum["application/xhtml+xml"] = MIME_APPLICATION_XHTML_XML;
    stringToEnum["application/java-vm"] = MIME_APPLICATION_JAVA_VM;
    stringToEnum["application/smil"] = MIME_APPLICATION_SMIL;
    stringToEnum["application/java-archive"] = MIME_APPLICATION_JAVA_ARCHIVE;
    stringToEnum["application"] = MIME_APPLICATION_JAVA;
    stringToEnum["application/octet-stream"] = MIME_APPLICATION_OCTET_STREAM;
    stringToEnum["application/studiom"] = MIME_APPLICATION_STUDIOM;
    stringToEnum["application/funMedia"] = MIME_APPLICATION_FUNMEDIA;
    stringToEnum["application/msword"] = MIME_APPLICATION_MSWORD;
    stringToEnum["application/pdf"] = MIME_APPLICATION_PDF;
    stringToEnum["application/sdp"] = MIME_APPLICATION_SDP;
    stringToEnum["application/ram"] = MIME_APPLICATION_RAM;
    stringToEnum["application/*"] = MIME_APPLICATION_ASTERIC;
    //16
    stringToEnum["application/vnd.wap.xhtml+xml"] = MIME_APPLICATION_VND_WAP_XHTMLXML;
    stringToEnum["application/vnd.wap.wmlc"] = MIME_APPLICATION_VND_WAP_WMLC;
    stringToEnum["application/vnd.wap.wmlscriptc"] = MIME_APPLICATION_VND_WAP_WMLSCRIPTC;
    stringToEnum["application/vnd.wap.wta-eventc"] = MIME_APPLICATION_VND_WAP_WTA_EVENTC;
    stringToEnum["application/vnd.wap.uaprof"] = MIME_APPLICATION_VND_WAP_UAPROF;
    stringToEnum["application/vnd.wap.sic"] = MIME_APPLICATION_VND_WAP_SIC;
    stringToEnum["application/vnd.wap.slc"] = MIME_APPLICATION_VND_WAP_SLC;
    stringToEnum["application/vnd.wap.coc"] = MIME_APPLICATION_VND_WAP_COC;
    stringToEnum["application/vnd.wap.sia"] = MIME_APPLICATION_VND_WAP_SIA;
    stringToEnum["application/vnd.wap.connectivity-wbxml"] = MIME_APPLICATION_VND_WAP_CONNECTIVITY_WBXML;
    stringToEnum["application/vnd.wap.multipart.form-data"] = MIME_APPLICATION_VND_WAP_MULTIPART_FORM_DATA;
    stringToEnum["application/vnd.wap.multipart.byteranges"] = MIME_APPLICATION_VND_WAP_MULTIPART_BYTERANGES;
    stringToEnum["application/vnd.wap.multipart.mixed"] = MIME_APPLICATION_VND_WAP_MULTIPART_MIXED;
    stringToEnum["application/vnd.wap.multipart.related"] = MIME_APPLICATION_VND_WAP_MULTIPART_RELATED;
    stringToEnum["application/vnd.wap.multipart.alternative"] = MIME_APPLICATION_VND_WAP_MULTIPART_ALTERNATIVE;
    stringToEnum["application/vnd.wap.multipart.*"] = MIME_APPLICATION_VND_WAP_MULTIPART_ASTERIC;
    stringToEnum["application/vnd.wap.wbxml"] = MIME_APPLICATION_VND_WAP_WBXML;
    stringToEnum["application/vnd.oma.dd+xml"] = MIME_APPLICATION_VND_OMA_DD_XML;
    stringToEnum["application/vnd.oma.drm.message"] = MIME_APPLICATION_VND_OMA_DRM_MESSAGE;
    stringToEnum["application/vnd.oma.drm.content"] = MIME_APPLICATION_VND_OMA_DRM_CONTENT;
    stringToEnum["application/vnd.oma.drm.rights+xml"] = MIME_APPLICATION_VND_OMA_DRM_RIGHTS_XML;
    stringToEnum["application/vnd.oma.drm.rights+wbxml"] = MIME_APPLICATION_VND_OMA_DRM_RIGHTS_WBXML;
    stringToEnum["application/vnd.oma.drm.ro+xml"] = MIME_APPLICATION_VND_OMA_DRM_RO_XML;
    stringToEnum["application/vnd.oma.drm.dcf"] = MIME_APPLICATION_VND_OMA_DRM_DCF;
    stringToEnum["application/vnd.oma.drm.roap-pdu+xml"] = MIME_APPLICATION_VND_OMA_ROAPPDU_XML;
    stringToEnum["application/vnd.oma.drm.roap-trigger+xml"] = MIME_APPLICATION_VND_OMA_ROAPTRIGGER_XML;
    stringToEnum["application/vnd.smaf"] = MIME_APPLICATION_VND_SMAF;
    stringToEnum["application/vnd.rn-realmedia"] = MIME_APPLICATION_VND_RN_REALMEDIA;
    stringToEnum["application/vnd.sun.j2me.java-archive"] = MIME_APPLICATION_VND_SUN_J2ME_JAVA_ARCHIVE;
    stringToEnum["application/vnd.samsung.theme"] = MIME_APPLICATION_VND_SAMSUNG_THEME;
    stringToEnum["application/vnd.ms-excel"] = MIME_APPLICATION_VND_EXCEL;
    stringToEnum["application/vnd.ms-powerpoint"] = MIME_APPLICATION_VND_POWERPOINT;
    stringToEnum["applcation/vnd.ms-word"] = MIME_APPLICATION_VND_MSWORD;
    //49
    stringToEnum["application/x-hdmlc"] = MIME_APPLICATION_X_HDMLC;
    stringToEnum["application/x-x968-user-cert"] = MIME_APPLICATION_X_X968_USERCERT;
    stringToEnum["application/x-www-form-urlencoded"] = MIME_APPLICATION_X_WWW_FORM_URLENCODED;
    stringToEnum["application/x-smaf"] = MIME_APPLICATION_X_SMAF;
    stringToEnum["application/x-shockwave-flash"] = MIME_APPLICATION_X_FLASH;
    stringToEnum["application/x-msexcel"] = MIME_APPLICATION_X_EXCEL;
    stringToEnum["application/x-mspowerpoint"] = MIME_APPLICATION_X_POWERPOINT;
    //56
    stringToEnum["audio/basic"] = MIME_AUDIO_BASIC;
    stringToEnum["audio/mpeg"] = MIME_AUDIO_MPEG;
    stringToEnum["audio/mp3"] = MIME_AUDIO_MP3;
    stringToEnum["audio/mpg3"] = MIME_AUDIO_MPG3;
    stringToEnum["audio/mpeg"] = MIME_AUDIO_MPEG3;
    stringToEnum["audio/mpg"] = MIME_AUDIO_MPG;
    stringToEnum["audio/aac"] = MIME_AUDIO_AAC;
    stringToEnum["audio/g72"] = MIME_AUDIO_G72;
    stringToEnum["audio/amr"] = MIME_AUDIO_AMR;
    stringToEnum["audio/amr-wb"] = MIME_AUDIO_AMR_WB;
    stringToEnum["audio/mmf"] = MIME_AUDIO_MMF;
    stringToEnum["audio/smaf"] = MIME_AUDIO_SMAF;
    stringToEnum["audio/iMelody"] = MIME_AUDIO_IMELODY;
    stringToEnum["audio/imelody"] = MIME_AUDIO_IMELODY2;
    stringToEnum["audio/melody"] = MIME_AUDIO_MELODY;
    stringToEnum["audio/mid"] = MIME_AUDIO_MID;
    stringToEnum["audio/midi"] = MIME_AUDIO_MIDI;
    stringToEnum["audio/sp-midi"] = MIME_AUDIO_SP_MIDI;
    stringToEnum["audio/wave"] = MIME_AUDIO_WAVE;
    stringToEnum["audio/wav"] = MIME_AUDIO_WAV;
    stringToEnum["audio/3gpp"] = MIME_AUDIO_3GPP;
    stringToEnum["audio/mp4"] = MIME_AUDIO_MP4;
    stringToEnum["audio/MP4A-LATM"] = MIME_AUDIO_MP4A_LATM;
    stringToEnum["audio/m4a"] = MIME_AUDIO_M4A;
    stringToEnum["audio/mpeg4"] = MIME_AUDIO_MPEG4;
    stringToEnum["audio/wma"] = MIME_AUDIO_WMA;
    stringToEnum["audio/xmf"] = MIME_AUDIO_XMF;
    stringToEnum["audio/imy"] = MIME_AUDIO_IMY;
    stringToEnum["audio/mobile-xmf"] = MIME_AUDIO_MOBILE_XMF;
    //85
    stringToEnum["audio/vnd.rn-realaudio"] = MIME_AUDIO_VND_RN_REALAUDIO;
    //86
    stringToEnum["audio/x-mpeg"] = MIME_AUDIO_X_MPEG;
    stringToEnum["audio/x-mp3"] = MIME_AUDIO_X_MP3;
    stringToEnum["audio/x-mpeg3"] = MIME_AUDIO_X_MPEG3;
    stringToEnum["audio/x-mpg"] = MIME_AUDIO_X_MPG;
    stringToEnum["audio/x-amr"] = MIME_AUDIO_X_AMR;
    stringToEnum["audio/x-mmf"] = MIME_AUDIO_X_MMF;
    stringToEnum["audio/x-smaf"] = MIME_AUDIO_X_SMAF;
    stringToEnum["audio/x-iMelody"] = MIME_AUDIO_X_IMELODY;
    stringToEnum["audio/x-midi"] = MIME_AUDIO_X_MIDI;
    stringToEnum["audio/x-mpegaudio"] = MIME_AUDIO_X_MPEGAUDIO;
    stringToEnum["audio/x-pn-realaudio"] = MIME_AUDIO_X_PN_REALAUDIO;
    stringToEnum["audio/x-pn-multirate-realaudio"] = MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO;
    stringToEnum["audio/x-pn-multirate-realaudio-live"] = MIME_AUDIO_X_PN_MULTIRATE_REALAUDIO_LIVE;
    stringToEnum["audio/x-wave"] = MIME_AUDIO_X_WAVE;
    stringToEnum["audio/x-wav"] = MIME_AUDIO_X_WAV;
    stringToEnum["audio/x-ms-wma"] = MIME_AUDIO_X_MS_WMA;
    stringToEnum["audio/x-mid"] = MIME_AUDIO_X_MID;
    stringToEnum["audio/x-ms-asf"] = MIME_AUDIO_X_MS_ASF;
    stringToEnum["audio/x-xmf"] = MIME_AUDIO_X_XMF;
    //105
    stringToEnum["image/gif"] = MIME_IMAGE_GIF;
    stringToEnum["image/jpeg"] = MIME_IMAGE_JPEG;
    stringToEnum["image/jpga"] = MIME_IMAGE_JPG;
    stringToEnum["image/tiff"] = MIME_IMAGE_TIFF;
    stringToEnum["image/tif"] = MIME_IMAGE_TIF;
    stringToEnum["image/png"] = MIME_IMAGE_PNG;
    stringToEnum["image/wbmp"] = MIME_IMAGE_WBMP;
    stringToEnum["image/pjpeg"] = MIME_IMAGE_PJPEG;
    stringToEnum["image/bmp"] = MIME_IMAGE_BMP;
    stringToEnum["image/svg+xml"] = MIME_IMAGE_SVG;
    stringToEnum["image/svg-xml"] = MIME_IMAGE_SVG1;
    //116
    stringToEnum["image/vnd.wap.wbmp"] = MIME_IMAGE_VND_WAP_WBMP;
    stringToEnum["image/vnd.tmo.my5-gif"] = MIME_IMAGE_VND_TMO_GIF;
    stringToEnum["image/vnd.tmo.my5-jpg"] = MIME_IMAGE_VND_TMO_JPG;
    //119
    stringToEnum["image/x-bmp"] = MIME_IMAGE_X_BMP;
    //120
    stringToEnum["message/rfc822"] = MIME_MESSAGE_RFC822;
    //121
    stringToEnum["multipart/mixed"] = MIME_MULTIPART_MIXED;
    stringToEnum["multipart/related"] = MIME_MULTIPART_RELATED;
    stringToEnum["multipart/alternative"] = MIME_MULTIPART_ALTERNATIVE;
    stringToEnum["multipart/form-data"] = MIME_MULTIPART_FORM_DATA;
    stringToEnum["multipart/byterange"] = MIME_MULTIPART_BYTERANGE;
    stringToEnum["multipart/report"] = MIME_MULTIPART_REPORT;
    stringToEnum["multipart/voice-message"] = MIME_MULTIPART_VOICE_MESSAGE;
    //128
    stringToEnum["text/txt"] = MIME_TEXT_TXT;
    stringToEnum["text/html"] = MIME_TEXT_HTML;
    stringToEnum["text/plain"] = MIME_TEXT_PLAIN;
    stringToEnum["text/css"] = MIME_TEXT_CSS;
    stringToEnum["text/xml"] = MIME_TEXT_XML;
    stringToEnum["text/iMelody"] = MIME_TEXT_IMELODY;
    //134
    stringToEnum["text/vnd.wap.wmlscript"] = MIME_TEXT_VND_WAP_WMLSCRIPT;
    stringToEnum["text/vnd.wap.wml"] = MIME_TEXT_VND_WAP_WML;
    stringToEnum["text/vnd.wap.wta-event"] = MIME_TEXT_VND_WAP_WTA_EVENT;
    stringToEnum["text/vnd.wap.connectivity-xml"] = MIME_TEXT_VND_WAP_CONNECTIVITY_XML;
    stringToEnum["text/vnd.wap.si"] = MIME_TEXT_VND_WAP_SI;
    stringToEnum["text/vnd.wap.sl"] = MIME_TEXT_VND_WAP_SL;
    stringToEnum["text/vnd.wap.co"] = MIME_TEXT_VND_WAP_CO;
    stringToEnum["text/vnd.sun.j2me.app-descriptor"] = MIME_TEXT_VND_SUN_J2ME_APP_DESCRIPTOR;
    //142
    stringToEnum["text/x-hdml"] = MIME_TEXT_X_HDML;
    stringToEnum["text/x-vCalendar"] = MIME_TEXT_X_VCALENDAR;
    stringToEnum["text/x-vCard"] = MIME_TEXT_X_VCARD;
    stringToEnum["text/x-iMelody"] = MIME_TEXT_X_IMELODY;
    stringToEnum["text/x-imelody"] = MIME_TEXT_X_IMELODY2;
    stringToEnum["text/x-vnote"] = MIME_TEXT_X_VNOTE;
    //148
    stringToEnum["video/mpeg4"] = MIME_VIDEO_MPEG4;
    stringToEnum["video/mp4"] = MIME_VIDEO_MP4;
    stringToEnum["video/h263"] = MIME_VIDEO_H263;
    stringToEnum["video/3gpp"] = MIME_VIDEO_3GPP;
    stringToEnum["video/3gp"] = MIME_VIDEO_3GP;
    stringToEnum["video/avi"] = MIME_VIDEO_AVI;
    stringToEnum["video/sdp"] = MIME_VIDEO_SDP;
    stringToEnum["video/mp4v-es"] = MIME_VIDEO_MP4_ES;
    stringToEnum["video/mpeg"] = MIME_VIDEO_MPEG;
    //157
    stringToEnum["video/vnd.rn-realvideo"] = MIME_VIDEO_VND_RN_REALVIDEO;
    stringToEnum["video/vnd.rn-realmedia"] = MIME_VIDEO_VND_RN_REALMEDIA;
    //159
    stringToEnum["video/x-mp4"] = MIME_VIDEO_X_MP4;
    stringToEnum["video/x-pv-mp4"] = MIME_VIDEO_X_PV_MP4;
    stringToEnum["video/x-pn-realvideo"] = MIME_VIDEO_X_PN_REALVIDEO;
    stringToEnum["video/x-pn-multirate-realvideo"] = MIME_VIDEO_X_PN_MULTIRATE_REALVIDEO;
    stringToEnum["video/x-ms-wmv"] = MIME_VIDEO_X_MS_WMV;
    stringToEnum["video/x-ms-asf"] = MIME_VIDEO_X_MS_ASF;
    stringToEnum["video/x-pv-pvx"] = MIME_VIDEO_X_PV_PVX;
    stringToEnum[""] = MIME_UNKNOWN;

    return stringToEnum;
}

unsigned int MessageAttachment::MIMETypeStringToEnum(std::string str)
{
    LoggerD("Entered");
    std::map<std::string, unsigned int>::iterator it = MIMETypeStringToEnumMap.find(str);
    if (it != MIMETypeStringToEnumMap.end()) {
        return it->second;
    }
    return MIME_UNKNOWN;
}

std::string MessageAttachment::MIMETypeEnumToString(unsigned int num)
{
    LoggerD("Entered");
    std::map<unsigned int, std::string>::iterator it = MIMETypeEnumToStringMap.find(num);
    if (it != MIMETypeEnumToStringMap.end()) {
        return it->second;
    }
    return std::string();
}

void MessageAttachment::updateWithAttachmentData(const email_attachment_data_t& attachment_data)
{
    LoggerD("Entered");
    setId(attachment_data.attachment_id);
    setMessageId(attachment_data.mail_id);
    if (attachment_data.attachment_mime_type) {
        setMimeType(attachment_data.attachment_mime_type);
    }

    bool isSaved = false;
    if (attachment_data.attachment_path) {
        setFilePath(attachment_data.attachment_path);

        LoggerD("save status: %d", attachment_data.save_status);
        LoggerD("attachment_size : %d", attachment_data.attachment_size);
        isSaved = attachment_data.save_status;
    }

    setIsSaved(isSaved);
}

} // messaging
} // extension
