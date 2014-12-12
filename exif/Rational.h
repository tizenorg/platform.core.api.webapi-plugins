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


#ifndef __TIZEN_EXIF_RATIONAL_H_
#define __TIZEN_EXIF_RATIONAL_H_

#include <string>
#include <vector>
#include <memory>

#include <libexif/exif-utils.h>
#include <libexif/exif-entry.h>
#include <libexif/exif-data.h>

#include "ExifUtil.h"

namespace DeviceAPI {
namespace Exif {

class Rational;
typedef std::vector<Rational> Rationals;
typedef std::shared_ptr<Rationals> RationalsPtr;

/**
 * This class represents fraction as nominator/denominator - two ExifLong values
 * Rational type is present in Exif specification - used for example in GPS coordinates
 */
class Rational
{
public:
    /**
     * Default constructor sets to 0/0 - invalud rational number
     */
    Rational();

    Rational(ExifLong nom, ExifLong denom);
    Rational(const ExifRational& exif_rational);

    static Rational createFromDouble(const double value, const long precision = 1000);
    static Rational createInvalid();

    /**
     * Returns true if denominator is valid (!= 0) and therefore whole Rational is valid
     */
    bool isValid() const;

    double toDouble() const;

    /**
     * Returns string in format: nominator/denominator,
     * for example: "1/4", "1/1", "5/3".
     *
     */
    std::string toString() const;

    /**
     * Create rational number from exposure string
     * Accepted format "(integer) (nominator/denominator)"
     * for example:
     * "1/5", "4/5" - just fraction part
     * "1", "5" - just integer part
     * "1 1/5", "4 1/4" - integer + fraction part
     */
    static Rational createFromExposureTimeString(const std::string& exp_time);

    /**
     * Return time exposure string in following format:
     *
     * nominator < denominator                               : "1/5", "4/5"
     * nominator == x*denominator                            : "1", "5"
     * nominator > denominator && nominator != x*denominator : "1 1/5", "4 1/4"
     */
    std::string toExposureTimeString() const;

    ExifLong nominator;
    ExifLong denominator;
};

} // Exif
} // DeviceApi

#endif // __TIZEN_EXIF_RATIONAL_H_