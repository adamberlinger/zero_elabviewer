/*
 * Copyright 2016 Adam Berlinger
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>
 */
#ifndef _PINOUT_H_
#define _PINOUT_H_

#include <cstdint>
#include <cstddef>
#include <QtGui>

class Pinout {
public:
  enum Package {
    NONE = 0,
    SO8 = 1,
    TSSOP20 = 2,
    LQFP32 = 3,

    PKG_LAST
  } ;

  enum Function {
    OSC = 1,
    PWM = 2,
    VOLT = 3,
    PWM_IN = 4,
    GEN = 5,

    FUNC_LAST
  };
  static const char* pkg_names[];
  static const int pkg_pin_counts[];
  static const char* func_names[];
protected:
  struct Pin {
      int pkg_number;
      char port;
      int port_number;
      int function;
      int channel;
  };

  Package package;
  Pin *pins;
  int pinCount;
  Pinout(Package package, int pinCount);
  bool isPinValid(int i);
public:
  static Pinout* fromEncoded(const uint8_t* data, int size);
  const char* getPackageName() const;
  const char* getPinFunction(int i) const;
  int getPinCount() const {return pinCount;}
  int getPinPackageNum(int i) const {return pins[i].pkg_number;}
  QString getPinLabel(const char* format, int index) const;
  int getPackageCount() const;
  ~Pinout();

  friend QDebug operator<<(QDebug debug, const Pinout& p);
};

QDebug operator<<(QDebug debug, const Pinout& p);

#endif
