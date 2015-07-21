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
 
#include "exif/rational.h"

#include <cmath>
#include <sstream>

#include "common/logger.h"

#include "exif/exif_util.h"

namespace extension {
namespace exif {

namespace {
const double DOUBLE_ERROR_REPRESENTATION = static_cast<double>(0x7FFFFFFF);
}  // namespace

Rational::Rational() :
    nominator(0),
    denominator(0) {
}

Rational::Rational(ExifLong nom, ExifLong denom) :
    nominator(nom),
    denominator(denom) {
}

Rational::Rational(const ExifRational& exif_rational) :
    nominator(exif_rational.numerator),
    denominator(exif_rational.denominator) {
}

Rational Rational::createFromDouble(const double value, const long precision) {
  LoggerD("Entered value:%f precision:%d", value, precision);
  if (value < 0.0) {
    LoggerW("Trying to create negative Rational: %f!", value);
    return Rational();
  }

  if (value < 0.000000001) {
    LoggerD("Skipping calculation returning: Rational(0,1)");
    return Rational(0, 1);
  }

  long m[2][2];
  double x, startx;
  long ai;

  startx = x = value;

  // initialize matrix
  m[0][0] = m[1][1] = 1;
  m[0][1] = m[1][0] = 0;

  // loop finding terms until daemon gets too big
  do {
    ai = static_cast<long>(x);
    if (m[1][0] * ai + m[1][1] > precision) {
      break;
    }

    long t = m[0][0] * ai + m[0][1];
    m[0][1] = m[0][0];
    m[0][0] = t;

    t = m[1][0] * ai + m[1][1];
    m[1][1] = m[1][0];
    m[1][0] = t;

    if (x == static_cast<double>(ai)) {
      break;   // AF: division by zero
    }

    x = 1 / (x - static_cast<double>(ai));
    if (x > DOUBLE_ERROR_REPRESENTATION) {
      break;  // AF: representation failure
    }
  } while (1);

  // now remaining x is between 0 and 1/ai
  // approx as either 0 or 1/m where m is max that will fit in precision
  // first try zero
  const double error0 =
      startx - (static_cast<double>(m[0][0]) / static_cast<double>(m[1][0]));
  const long numerator0 = m[0][0];
  const long denominator0 = m[1][0];

  LoggerD("%ld/%ld, error = %e\n", numerator0, denominator0, error0);

  /* now try other possibility */
  ai = static_cast<long>(static_cast<double>(precision - m[1][1]) / static_cast<double>(m[1][0]));
  m[0][0] = m[0][0] * ai + m[0][1];
  m[1][0] = m[1][0] * ai + m[1][1];

  double error1m = startx -
      (static_cast<double>(m[0][0]) / static_cast<double>(m[1][0]));
  LoggerD("%ld/%ld, error = %e\n", m[0][0], m[1][0], error1m);

  long result_numerator = 0;
  long result_denominator = 0;

  if (error0 < error1m) {
    result_numerator = numerator0;
    result_denominator = denominator0;
  } else {
    result_numerator = m[0][0];
    result_denominator = m[1][0];
  }

  if (result_numerator < 0) {
    result_numerator *= -1;
  }
  if (result_denominator < 0) {
    result_denominator *= -1;
  }

  LoggerD("Rational(%d, %d) error0 < error1m:%d",
      result_numerator, result_denominator, error0 < error1m);

  return Rational(numerator0, denominator0);
}

Rational Rational::createInvalid() {
  return Rational(0, 0);
}

bool Rational::isValid() const {
  if (0 == denominator) {
    return false;
  } else {
    return true;
  }
}

double Rational::toDouble() const {
  if (!isValid()) {
    return NAN;
  }

  return static_cast<double>(nominator) / static_cast<double>(denominator);
}

Rational Rational::createFromExposureTimeString(const std::string& exp_time) {
  LoggerD("Entered");
  if (exp_time.length() == 0) {
    return Rational::createInvalid();  // lets assume that empty string means 0,
                                       // however exposure time = 0 is
                                       // not valid value
  }

  std::string integer_part;
  std::string fraction_part;

  int first_space_at = -1;
  int first_slash_at = -1;

  for (size_t i = 0; i < exp_time.size(); ++i) {
    const char& cur = exp_time[i];
    if (first_space_at < 0 && ' ' == cur) {
      first_space_at = i;
    }
    if (first_slash_at < 0 && '/' == cur) {
      first_slash_at = i;
    }
  }

  if (first_slash_at > 0) {
    if (first_space_at > 0) {
      integer_part = exp_time.substr(0, first_space_at);
      fraction_part = exp_time.substr(first_space_at + 1,
          exp_time.size() - (first_space_at + 1));
    } else {
      fraction_part = exp_time;
    }
  } else {
    integer_part = exp_time;
  }

  LoggerD("first_space_at: %d first_slash_at:%d int: [%s] , frac: [%s]",
      first_space_at, first_slash_at,
      integer_part.c_str(), fraction_part.c_str());

  long integer_value = 0;
  long nominator = 0;
  long denominator = 1;

  if (integer_part.length() > 0) {
    integer_value = atol(integer_part.c_str());
  }

  if (fraction_part.length() > 0) {
    if (sscanf(
          fraction_part.c_str(), "%5ld/%5ld", &nominator, &denominator) != 2) {
      LoggerD("Failed to parse nominator/denominator string: [%s]",
          fraction_part.c_str());
      return Rational::createInvalid();
    }
  }

  nominator += denominator * integer_value;
  LoggerD("%d/%d -> %f",
      nominator, denominator, static_cast<float>(nominator) / denominator);

  if (0 == nominator) {
    // Exposure time = 0 is invalid value
    return Rational::createInvalid();
  }

  return Rational(nominator, denominator);
}

std::string Rational::toString() const {
  std::stringstream ss;
  ss << nominator << "/" << denominator;
  return ss.str();
}

std::string Rational::toExposureTimeString() const {
  LoggerD("Entered");
  if (!isValid() || 0 == nominator) {
    return std::string();
  }

  std::string output_str;

  if (nominator < denominator) {
    output_str = toString();
  } else if (nominator % denominator == 0) {
    std::stringstream ss;
    ss << nominator / denominator;
    output_str = ss.str();
  } else {
    ExifLong new_nominator = nominator % denominator;
    ExifLong new_denominator = denominator;
    ExifLong integer_value = nominator / denominator;

    std::stringstream ss;
    ss << integer_value << " ";
    ss << new_nominator << "/" << new_denominator;
    output_str = ss.str();
  }

  return output_str;
}

}  // namespace exif
}  // namespace extension
