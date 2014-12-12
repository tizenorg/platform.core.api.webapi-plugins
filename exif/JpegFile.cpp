//
// Tizen Web Device API
// Copyright (c) 2014 Samsung Electronics Co., Ltd.
//
// Licensed under the Apache License, Version 2.0 (the License);
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//

//
// For details of JPEG file format see:
// http://www.media.mit.edu/pia/Research/deepview/exif.html

#include "JpegFile.h"

#include <iomanip>
#include <limits>
#include <stdio.h>
#include <sstream>

#include <Logger.h>
#include <PlatformException.h>

namespace DeviceAPI {

using namespace DeviceAPI::Common;

namespace Exif {

/**
 * Size of maximal JPEG's section data length
 * (since it is stored as unsigned short limit is 2^16 -1)
 */
const unsigned int MAX_JPEG_SECTION_DATA_SIZE = 65535;

/**
 * JPEG's section data length includes 2 bytes for length therefore we need to
 * substract 2 from MAX_JPEG_SECTION_DATA_SIZE
 */
const unsigned int MAX_AVAILABLE_JPEG_SECTION_DATA_SIZE = MAX_JPEG_SECTION_DATA_SIZE - 2;

bool isJpegMarker(const int value)
{
    return value >= JPEG_MARKER_LOWEST_ID && value <= JPEG_MARKER_HIGHEST_ID;
}

JpegMarker castToJpegMarker(const unsigned char byte)
{
    if (byte < JPEG_MARKER_LOWEST_ID || byte > JPEG_MARKER_HIGHEST_ID) {
        return JPEG_MARKER_UNKNOWN;
    }

    return static_cast<JpegMarker>(byte);
}

long readUShortBE(unsigned char* src)
{
    return ((static_cast<long>(src[0]) << 8) | static_cast<long>(src[1]));
}

void writeUShortBE(unsigned short value, unsigned char* buffer)
{
    buffer[0] = static_cast<unsigned char>(value >> 8);
    buffer[1] = static_cast<unsigned char>(value);
}

struct CArrayDeleter {
  void operator()(void* buffer) { free(buffer); }
};

JpegFile::JpegFile() :
    m_in_data(NULL),
    m_in_data_size(0),
    m_image_data(NULL),
    m_image_size(0),
    m_padding_data(NULL),
    m_padding_data_size(0),
    m_in_file(NULL),
    m_out_file(NULL)
{

}

JpegFile::~JpegFile()
{
    delete [] m_in_data;
    m_in_data = NULL;
    m_in_data_size = 0;

    m_padding_data = NULL;
    m_padding_data_size = 0;

    for(SectionsVec::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
        JpegFileSectionPtr cur = *it;

        if (cur->exif_data) {
            exif_data_unref(cur->exif_data);
            cur->exif_data = NULL;
        }

        cur->data_ptr = NULL;
        cur->size = 0;
        cur->type = JPEG_MARKER_UNKNOWN;
    }

    m_image_data = NULL;
    m_image_size = 0;

    if (m_in_file) {
        fclose(m_in_file);
        m_in_file = NULL;
    }

    if (m_out_file) {
        fclose(m_out_file);
        m_out_file = NULL;
    }
}

JpegFilePtr JpegFile::loadFile(const std::string& path)
{
    JpegFile* new_jpg = new (std::nothrow) JpegFile();
    if (!new_jpg) {
        LOGE("Couldn't allocate Jpegfile!");
        throw UnknownException("Memory allocation failed");
    }

    JpegFilePtr jpg_ptr(new_jpg);
    jpg_ptr->load(path);
    return jpg_ptr;
}

void JpegFile::load(const std::string& path)
{
    LOGD("Entered file:%s", path.c_str());

    m_source_file_path = path;

    m_in_file = fopen(path.c_str(), "rb");
    if (!m_in_file) {
        LOGE("Couldn't open Jpeg file: [%s]", path.c_str());
        throw NotFoundException("Could not open JPG file");
    }

    fseek(m_in_file, 0, SEEK_END);
    const size_t in_file_size = static_cast<size_t>(ftell(m_in_file));
    fseek(m_in_file, 0, SEEK_SET);
    LOGD("JPEG file: [%s] size:%d", path.c_str(), in_file_size);
    if (0 == in_file_size) {
        LOGE("Input file [%s] is empty!", path.c_str());
        throw UnknownException("JPEG file is invalid");
    }

    m_in_data = new (std::nothrow) unsigned char[in_file_size];
    if (!m_in_data) {
        LOGE("Couldn't allocate buffer with size: %d", in_file_size);
        throw UnknownException("Memory allocation failed");
    }

    m_in_data_size = in_file_size;

    const size_t read_bytes = fread(m_in_data, 1, m_in_data_size, m_in_file);
    if (read_bytes != m_in_data_size) {
        LOGE("Couldn't read all: %d bytes. Read only: %d bytes!", m_in_data_size,
                read_bytes);
        throw UnknownException("Could not read JPEG file");
    }

    if (fclose(m_in_file) == EOF) {
        LOGE("Couldn't close input file: %s!", path.c_str());
    }
    m_in_file = NULL;

    generateListOfSections();
}

std::string JpegFile::getPartOfFile(const size_t offset,
        const size_t num_bytes_before,
        const size_t num_bytes_after)
{
    long long int start = static_cast<long long int>(offset) - num_bytes_before;
    if (start < 0) {
        start = 0;
    }

    long long int end = static_cast<long long int>(offset) + num_bytes_after;
    if (end >= m_in_data_size) {
        end = m_in_data_size - 1;
    }

    std::stringstream ss;
    ss << std::setfill('0') << std::setw(2) << std::hex;
    for(long long int i = start; i <= end; ++i) {
        ss << static_cast<int>(m_in_data[i]);
    }
    return ss.str();
}


void JpegFile::generateListOfSections()
{
    LOGD("Entered");

    //JPEG starts with:
    //FFD8 (2 bytes) - SOI Marker
    //
    //then:
    //N sections - format of section:
    //0xFF(1 byte) + Marker Number(1 byte) + Data size(2 bytes) + Data
    //
    //then:
    //SOS 0xFF(1 byte) + Marker Number(1 byte) + Data size(2 bytes) + Data
    //
    //Image data
    //
    //FFD9 (2 bytes) - EOI Marker
    //
    //Warning: some images taken on Android contains some extra data at the end
    //we will keep it in m_padding_data

    m_padding_data = NULL;
    m_padding_data_size = 0;

    for(size_t offset = 0, iterration = 0; offset < m_in_data_size;++iterration) {

        LOGD("offset:%d | Starting iteration: %d", offset, iterration);
        const size_t search_len = 10;
        size_t search_offset = 0;
        for(search_offset = 0; search_offset < search_len; ++search_offset) {
            //Skip bytes until first no 0xff
            unsigned char& tmp_marker = m_in_data[offset + search_offset];
            if (tmp_marker != 0xff) {
                break;
            }
        }

        if (search_len == search_offset) {
            LOGE("offset:%d | Couldn't find marker! RAW DATA:{%s}", offset,
                    getPartOfFile(offset, 0, 10).c_str());
            throw UnknownException("JPEG file is invalid");
        }

        const size_t section_offset = offset + search_offset - 1;
        unsigned char* section_begin = m_in_data + section_offset;

        offset = section_offset;    //Move to section begin
        LOGD("offset:%d | Moved to section begin", offset);

        if (!isJpegMarker(section_begin[1])) {
            LOGE("offset:%d | Is not valid marker: 0x%x RAW DATA:{%s}", offset,
                    section_begin[1], getPartOfFile(section_offset,0,4).c_str());
            throw UnknownException("JPEG file is invalid");
        }

        const JpegMarker cur_marker = castToJpegMarker(section_begin[1]);
        LOGD("offset:%d | Found valid marker: 0x%x RAW DATA:{%s}", offset,
                cur_marker,
                getPartOfFile(section_offset,0,4).c_str());

        offset += 2;    //Read 0xffxx marker tag - 2 bytes

        JpegFileSectionPtr section;
        {
            JpegFileSection* sec = new (std::nothrow) JpegFileSection();
            if (!sec) {
                LOGE("Couldn't allocate JpegFileSection");
                throw UnknownException("Memory allocation failed");
            }

            section = JpegFileSectionPtr(sec);
        }

        section->type = cur_marker;
        m_sections.push_back(section);
        if (cur_marker == JPEG_MARKER_SOI ||
            cur_marker == JPEG_MARKER_EOI) {
            LOGD("offset:%d | Found: %s marker, moving to next marker at:%d",
                    section_offset, ((cur_marker == JPEG_MARKER_SOI) ? "SOI" : "EOI"),
                    offset);

            if(cur_marker == JPEG_MARKER_EOI && m_padding_data != NULL) {
                LOGW("Padding data have been found - do not try to parse end of file");
                break;
            }
        }
        else {
            //From JPEG/EXIF info:
            // Please notice that "Data" contains Data size descriptor, if there is
            // a Marker like this;
            //
            // FF C1 00 0C
            // It means this Marker(0xFFC1) has 0x000C(equal 12)bytes of data. But the
            // data size '12' includes "Data size" descriptor, it follows only 10 bytes of
            // data after 0x000C.
            //

            const long total_section_len = readUShortBE(section_begin + 2); //Include data
                                                                            //size 2 bytes

            const long section_data_len = total_section_len - 2;            //Exclude data
                                                                            //size 2 bytes

            LOGD("offset:%d tag:0x%x | Read total_section_len:%d (data len:%d)",
                    section_offset, cur_marker, total_section_len, section_data_len);

            offset += 2;    //Read data size - 2 bytes

            if (total_section_len < 0) {
                LOGE("offset:%d tag:0x%x | Error: total_section_len is: %d < 0", offset,
                        cur_marker, total_section_len);
                throw UnknownException("JPEG file is invalid");
            }

            if (section_offset + 2 + total_section_len > m_in_data_size) {
                LOGE("offset:%d tag:0x%x | Error: current section offset:%d"
                        " + 2 + total_section_len:%d = %d is greater then file size:%d",
                        offset, cur_marker,
                        section_offset, total_section_len,
                        section_offset + total_section_len, m_in_data_size);
                throw UnknownException("JPEG file is invalid");
            }

            if (JPEG_MARKER_APP1 == cur_marker) {
                //TODO: verify this
                //-4 --> 0xFF(1 byte)+Marker Number(1 byte)+Data size(2 bytes))
                //const unsigned int exif_data_size = section_length - 4;

                const unsigned int exif_data_size = total_section_len + 2;
                section->exif_data = exif_data_new_from_data (section_begin,
                        exif_data_size);

                LOGD("offset:%d tag:0x%x | Loading exif from offset:%d"
                        " len:%d exif_data_new_from_data returned: %p",
                        offset, cur_marker, section_offset, exif_data_size,
                        section->exif_data);

                if (!section->exif_data) {
                    LOGW("offset:%d tag:0x%x | Couldn't load Exif!", offset, cur_marker);
                }
            }

            //This just saves pointer not copying data
            section->data_ptr = section_begin + 2 + 2; //2 bytes marker + 2 bytes data size
            section->size = section_data_len;  //Exclude data size

            if (JPEG_MARKER_SOS == cur_marker) {
                //Calculate offset of first image data which is just after this SOS section
                const size_t image_data_offset = section_offset + 2 + total_section_len;

                //Calculate size of image data from start to expected EOI at end of file.
                //
                //-2 (exclude ending EOI marker (2 bytes)
                size_t image_size = m_in_data_size - image_data_offset - 2;
                LOGW("offset:%d tag:0x%x | Image data offset:%d Estimated image size:%d",
                        offset, cur_marker, image_data_offset, image_size);

                m_image_data = m_in_data + image_data_offset;

                size_t eoi_tag_index = 0;
                bool found_eoi_tag = searchForTagInBuffer(m_in_data + image_data_offset,
                        m_in_data + m_in_data_size, JPEG_MARKER_EOI, eoi_tag_index);
                if(!found_eoi_tag) {
                    LOGE("Could not find EOI tag! Assume that there is no EOI and rest of "
                            "JPEG file contains image data stream: image_size+= 2");
                    image_size += 2; //Skip expected EOI tag which is not present
                } else {
                    LOGD("EOI tag found at offset: %d from SOS data", eoi_tag_index);

                    if(eoi_tag_index != image_size) {
                        LOGW("Estimated image size:%d doesn't match EOI tag index:%d"
                                " delta:%d", image_size, eoi_tag_index,
                                image_size - eoi_tag_index);

                        LOGW("Setting image_size to EOI tag: %d", eoi_tag_index);
                        image_size = eoi_tag_index;

                        m_padding_data = m_image_data + image_size + 2; //(skip EOI tag)
                        m_padding_data_size = (m_in_data + m_in_data_size) - m_padding_data;
                        LOGW("Saving padding data from offset:%d with size:%d",
                            m_padding_data - m_in_data, m_padding_data_size);
                    }
                }

                m_image_size = image_size;

                offset = image_data_offset + image_size;
                LOGD("offset:%d tag:0x%x | SOS Offset moved to next marker", offset,
                        cur_marker);
            }
            else {
                offset += section_data_len;
                LOGD("offset:%d tag:0x%x | Offset moved to next marker", offset, cur_marker);
            }
        }
    }
}

bool JpegFile::searchForTagInBuffer(const unsigned char* buffer_start,
        const unsigned char* buffer_end,
        const JpegMarker marker,
        size_t& out_index)
{
    LOGD("Entered start:%p end:%p marker:0x%x", buffer_start, buffer_end, marker);

    if(!buffer_start) {
        LOGE("buffer_start is NULL");
        return false;
    }

    if(!buffer_end) {
        LOGE("buffer_end is NULL");
        return false;
    }

    if(buffer_end <= buffer_start) {
        LOGE("buffer_end: %p <= buffer_start: %p", buffer_end, buffer_start);
        return false;
    }

    LOGD("Bytes to scan: %d", static_cast<size_t>(buffer_end - buffer_start));
    const unsigned char marker_uchar = static_cast<unsigned char>(marker);

    for(const unsigned char* ptr = buffer_start; ptr < buffer_end; ++ptr) {

        if((0xff == *ptr) && (ptr+1 < buffer_end)) {
            if(marker_uchar == *(ptr+1)) {
                out_index = static_cast<size_t>(ptr - buffer_start);
                return true;
            }
        }
    }

    out_index = 0;
    return false;
}

void JpegFile::setNewExifData(ExifData* new_exif_data)
{
    if (!new_exif_data) {
        LOGE("Trying to set NULL exif_data!");
        throw UnknownException("Could not save Exif in JPEG file");
    }

    JpegFileSectionPtr exif = getExifSection();
    if (!exif) {
        LOGW("Could't find Exif section - creating new one");
        {
            JpegFileSection* new_sec = new (std::nothrow) JpegFileSection();
            if (!new_sec) {
                LOGE("Couldn't allocate JpegFileSection");
                throw UnknownException("Memory allocation failed");
            }
            new_sec->type = JPEG_MARKER_APP1;

            exif = JpegFileSectionPtr(new_sec);
        }

        SectionsVec::iterator insert_it = m_sections.begin();
        bool soi_is_present = false;

        if (insert_it != m_sections.end()) {
            if ((*insert_it)->type != JPEG_MARKER_SOI) {
                LOGW("First section is not SOI - Start Of Image!");
            }
            else {
                soi_is_present = true;
            }
        }

        if (!soi_is_present) {
            LOGW("SOI section is missing");
            throw UnknownException("JPEG file is invalid");
        }

        //Insert new Exif sections just after SOI
        ++insert_it;
        if (insert_it != m_sections.begin()) {
            m_sections.insert(insert_it, exif);
        }
        else {
            //This shouldn't happen since we at lest need SOS and EOI sections
            m_sections.push_back(exif);
        }
    }

    //We don't want to save old data
    exif->data_ptr = NULL;
    exif->size = 0;

    exif_data_unref(exif->exif_data);
    exif_data_ref (new_exif_data);
    exif->exif_data = new_exif_data;
}

ExifData* JpegFile::getExifData()
{
    JpegFileSectionPtr exif = getExifSection();
    if (!exif) {
        return NULL;
    }

    exif_data_ref(exif->exif_data);
    return exif->exif_data;
}

void JpegFile::saveToFile(const std::string& out_path)
{
    LOGD("Entered out_path:%s", out_path.c_str());
    try {
        saveToFilePriv(out_path);
    }
    catch (...) {
        LOGE("Exception occured during saveToFilePriv "
                "original file: [%] new: [%s]",
                m_source_file_path.c_str(),
                out_path.c_str());

        if (out_path == m_source_file_path) {

            LOGD("Trying to recover broken JPEG file: [%s]", out_path.c_str());
            //We were writing to source file and since something went wrong let's
            //restore old file - we have it in m_in_data

            FILE* outf = fopen(out_path.c_str(), "wb");
            if (!outf) {
                LOGE("Couldn't open output file: [%s] - JPEG file will not be restored!");
            }
            else {
                size_t bytes_wrote = fwrite(m_in_data, 1, m_in_data_size, outf);
                if (bytes_wrote != m_in_data_size) {
                    LOGE("Couldn't restore whole JPEG! "
                            "Only %d of %d bytes have been wrote!",
                            bytes_wrote, m_in_data_size);
                }
                if (EOF == fclose(outf)) {
                    LOGE("Couldn't close restore output file: [%s]", out_path.c_str());
                }
            }
        }

        throw;
    }
}

void JpegFile::saveToFilePriv(const std::string& out_path)
{
    LOGD("Entered out_path:%s", out_path.c_str());

    m_out_file = fopen(out_path.c_str(), "wb");
    if (!m_out_file) {
        LOGE("Couldn't open output file: %s", out_path.c_str());
        throw UnknownException("Could not write JPEG file");
    }

    unsigned char tmp_buf[128];
    size_t offset = 0;

    int section_index = 0;
    for(SectionsVec::iterator it = m_sections.begin();
            it != m_sections.end();
            ++it, ++ section_index) {

        JpegFileSectionPtr cur = *it;
        const JpegMarker cur_marker = cur->type;

        LOGD("offset:%d | Section: %d marker 0x%x", offset, section_index, cur_marker);

        size_t bytes_to_write = 0;
        size_t bytes_wrote = 0;

        tmp_buf[0] = 0xff;
        tmp_buf[1] = cur_marker;
        bytes_to_write += 2;

        bool write_section_data = false;

        bool write_exif_data = false;

        std::unique_ptr<unsigned char, CArrayDeleter> exif_output_data;
        unsigned int exif_output_size = 0;

        if (cur_marker != JPEG_MARKER_SOI &&
                cur_marker != JPEG_MARKER_EOI) {

            unsigned short section_size = 2;
            if (JPEG_MARKER_APP1 && cur->exif_data) {

                unsigned char* tmp = NULL;
                exif_data_save_data (cur->exif_data, &tmp, &exif_output_size);
                if (!tmp || 0 == exif_output_size) {
                    LOGE("Couldn't generate RAW Exif data!");
                    throw UnknownException("Could not save Exif in JPEG file");
                }

                LOGD("offset:%d | Generated Exif RAW Data length:%d", offset,
                        exif_output_size);

                exif_output_data.reset(tmp);

                if (exif_output_size > MAX_AVAILABLE_JPEG_SECTION_DATA_SIZE) {
                    LOGE("exif_output_size:%d is greater then maximum JPEG section"
                            "data block size: %d", exif_output_size,
                            MAX_AVAILABLE_JPEG_SECTION_DATA_SIZE);
                    throw UnknownException("Exif data is to big to be saved in JPEG file");
                }
                section_size += exif_output_size;
                write_exif_data = true;
            }
            else {
                section_size += cur->size;
                write_section_data = true;
            }

            writeUShortBE(section_size, tmp_buf + bytes_to_write);
            bytes_to_write += 2;
        }

        LOGD("offset:%d | Writing section: marker:0x%x size:%d", offset, cur_marker,
                cur->size);

        bytes_wrote = fwrite(tmp_buf, 1, bytes_to_write, m_out_file);
        offset += bytes_wrote;

        if (bytes_wrote != bytes_to_write) {
            LOGE("Couldn't wrote %d bytes! Only %d bytes wrote", bytes_to_write,
                    bytes_wrote);
            throw UnknownException("Could not write JPEG file");
        }

        if (write_section_data && cur->size > 0) {
            LOGD("offset:%d | Writing data with length:%d", offset, cur->size);

            bytes_to_write = cur->size;
            bytes_wrote = fwrite(cur->data_ptr, 1, bytes_to_write, m_out_file);
            offset += bytes_wrote;

            if (bytes_wrote != bytes_to_write) {
                LOGE("Couldn't wrote %d bytes! Only %d bytes wrote", bytes_to_write,
                        bytes_wrote);
                throw UnknownException("Could not write JPEG file");
            }
        }

        if (write_exif_data && exif_output_data && exif_output_size > 0) {
            LOGD("offset:%d | Writing new exif data with length:%d", offset,
                    exif_output_size);

            bytes_to_write = exif_output_size;
            bytes_wrote = fwrite(exif_output_data.get(), 1, bytes_to_write, m_out_file);
            offset += bytes_wrote;

            if (bytes_wrote != bytes_to_write) {
                LOGE("Couldn't wrote %d bytes! Only %d bytes wrote", bytes_to_write,
                        bytes_wrote);
                throw UnknownException("Could not write JPEG file");
            }
        }

        if (JPEG_MARKER_SOS == cur_marker) {
            LOGD("offset:%d | Writing image data stream with lenght:%d", offset,
                    m_image_size);

            bytes_to_write = m_image_size;
            bytes_wrote = fwrite(m_image_data, 1, bytes_to_write, m_out_file);
            offset += bytes_wrote;

            if (bytes_wrote != bytes_to_write) {
                LOGE("Couldn't wrote %d bytes! Only %d bytes wrote", bytes_to_write,
                        bytes_wrote);
                throw UnknownException("Could not write JPEG file");
            }
        }
    }

    if(m_padding_data && m_padding_data_size > 0) {
        LOGD("Padding data exists and contains:%d bytes saving to JPEG file");
        const size_t bytes_wrote = fwrite(m_image_data, 1, m_padding_data_size,
                m_out_file);

        if (bytes_wrote != m_padding_data_size) {
            LOGE("Couldn't wrote %d bytes! Only %d bytes wrote", m_padding_data_size,
                    bytes_wrote);
            throw UnknownException("Could not write JPEG file");
        }
    }

    if (fclose(m_out_file) == EOF) {
        LOGE("Couldn't close output file: %s", out_path.c_str());
        m_out_file = NULL;
    }  else {
        m_out_file = NULL;
        LOGD("Closed output file: %s wrote:%d bytes: %d", out_path.c_str(), offset);
    }
}

JpegFileSectionPtr JpegFile::getExifSection()
{
    size_t num_exif_sections = 0;
    JpegFileSectionPtr first_exif_section;

    for(SectionsVec::iterator it = m_sections.begin(); it != m_sections.end(); ++it) {
        JpegFileSectionPtr cur = *it;

        if (JPEG_MARKER_APP1 == cur->type) {
            if (!cur->exif_data) {
                LOGW("Warning: found APP1 section but exif_data is NULL (Not Exif?)");
                continue;
            }

            ++num_exif_sections;
            if (!first_exif_section) {
                first_exif_section = cur;
            }
            else {
                LOGW("Warning: found %d APP1/Exif sections - only first is currently supported!");
            }
        }
    }

    return first_exif_section;
}


} // namespace Exif
} // namespace DeviceAPI
