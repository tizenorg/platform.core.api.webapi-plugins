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

#ifndef EXIF_EXIF_JPEG_FILE_H_
#define EXIF_EXIF_JPEG_FILE_H_

#include <libexif/exif-data.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-utils.h>

#include <cstdio>
#include <memory>
#include <map>
#include <string>
#include <vector>

#include "common/platform_result.h"

namespace extension {
namespace exif {

enum JpegMarker{
  JPEG_MARKER_UNKNOWN     = 0x00,
  JPEG_MARKER_LOWEST_ID   = 0xc0,
  JPEG_MARKER_SOI         = 0xd8,  // Start Of Image
  JPEG_MARKER_EOI         = 0xd9,  // End Of Image
  JPEG_MARKER_SOS         = 0xda,  // Start Of Stream
  JPEG_MARKER_APP1        = 0xe1,  // Application Data 1 - for Exif
  JPEG_MARKER_HIGHEST_ID  = 0xfe
};

struct JpegFileSection;
typedef std::shared_ptr<JpegFileSection> JpegFileSectionPtr;

struct JpegFileSection {
  JpegFileSection() :
    type(JPEG_MARKER_UNKNOWN),
    data_ptr(NULL),
    size(0),
    exif_data(NULL) {}

  JpegMarker type;
  unsigned char* data_ptr;
  unsigned short size;

  ExifData* exif_data;
};


class JpegFile;
typedef std::shared_ptr<JpegFile> JpegFilePtr;

class JpegFile {
 public:
  static common::PlatformResult loadFile(const std::string& path,
                                         JpegFilePtr* jpg_ptr);
  ~JpegFile();

  common::PlatformResult setNewExifData(ExifData* new_exif_data);

  /**
   * You are responsible to unreference returned data with: exif_data_unref(...)
   * Example:
   *   ExifData* ed = jpeg.getExifData();
   *   if(ed) {
   *     doSth(ed);
   *     exif_data_unref(ed);
   *   }
   */
  ExifData* getExifData();

  common::PlatformResult saveToFile(const std::string& out_path);

 private:
  JpegFile();

  common::PlatformResult load(const std::string &path);

  common::PlatformResult generateListOfSections();

  std::string getPartOfFile(const std::size_t offset,
                            const std::size_t num_bytes_before = 10,
                            const std::size_t num_bytes_after = 10);

  JpegFileSectionPtr getExifSection();

  common::PlatformResult saveToFilePriv(const std::string &out_path);

  /**
   * Search for first occurrence of specific tag inside buffer.
   *
   * buffer_end is the first byte that should not be checked:
   * [buffer_start ... buffer_end)
   *
   * For example EOI - search for first 'ffd9' in buffer
   */
  static bool searchForTagInBuffer(const unsigned char* buffer_start,
      const unsigned char* buffer_end,
      const JpegMarker marker,
      std::size_t& out_index);

  std::string m_source_file_path;

  unsigned char* m_in_data;
  std::size_t m_in_data_size;

  unsigned char* m_image_data;
  std::size_t m_image_size;

  /**
   * This contains any bytes after EOI.
   * Usually there should be no extra bytes after EOI unfortunately
   * some cameras saves extra bytes (for example Android).
   */
  unsigned char* m_padding_data;
  std::size_t m_padding_data_size;

  FILE* m_in_file;
  FILE* m_out_file;

  typedef std::vector<JpegFileSectionPtr> SectionsVec;
  SectionsVec m_sections;
};

}  // namespace exif
}  // namespace extension

#endif  // EXIF_EXIF_JPEG_FILE_H_
