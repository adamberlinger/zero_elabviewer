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
#include "Pinout.h"

const char* Pinout::pkg_names[] = {
  "None",
  "SO8",
  "TSSOP20",
  "LQFP32"
};

const int Pinout::pkg_pin_counts[] = {
  0,
  8,
  20,
  32,
};

const char* Pinout::func_names[] = {
  "None",
  "OSC",
  "PWM",
  "Volt.",
  "PWM_IN",
  "GEN",
};

Pinout* Pinout::fromEncoded(const uint8_t* data, int size){
  if((size < 4) || (((size - 1) % 3) != 0)){
    /* Wrong size */
    return NULL;
  }

  if(data[0] >= Pinout::PKG_LAST){
    /* Unknown package */
    return NULL;
  }
  int pinCount = (size - 1) / 3;

  Pinout* result = new Pinout((Pinout::Package)data[0], pinCount);

  for(int i = 0; i < pinCount;++i){
    result->pins[i].pkg_number = data[1 + i*3];
    result->pins[i].port = 'A' + ((data[2 + i*3] & 0xF0) >> 4);
    result->pins[i].port_number = data[2 + i*3] & 0xF;
    result->pins[i].function = (Pinout::Function)((data[3 + i*3] & 0xF0) >> 4);
    result->pins[i].channel = data[3 + i*3] & 0xF;
    if(!result->isPinValid(i)){
      delete result;
      return NULL;
    }
  }

  return result;
}

Pinout::Pinout(Package package, int pinCount){
  this->package = package;
  this->pinCount = pinCount;
  this->pins = new Pinout::Pin[pinCount];
}

bool Pinout::isPinValid(int i){
  if((int)this->pins[i].function >= Pinout::FUNC_LAST){
    return false;
  }
  if(this->pins[i].port < 'A' || this->pins[i].port > 'Z'){
    return false;
  }
  if(this->pins[i].pkg_number > this->getPackageCount()){
    return false;
  }
  return true;
}

const char* Pinout::getPackageName() const {
  return Pinout::pkg_names[(int)this->package];
}

int Pinout::getPackageCount() const {
  return Pinout::pkg_pin_counts[(int)this->package];
}

const char* Pinout::getPinFunction(int i) const {
  if(i >= this->pinCount) return "";
  return Pinout::func_names[this->pins[i].function];
}

QString Pinout::getPinLabel(const char* format, int index) const {
    return QString(format)
      .arg(this->getPinFunction(index), 6)
      .arg(this->pins[index].port).arg(this->pins[index].port_number,-2)
      .arg(this->pins[index].channel,2);
}

Pinout::~Pinout(){
  if(this->pins){
    delete[] this->pins;
  }
}

QDebug operator<<(QDebug debug, const Pinout& p){
  QDebugStateSaver saver(debug);
  debug.nospace() << "Package: " << p.getPackageName() << "\r\n";

  int pkgCount = p.getPackageCount();
  int li = 0;
  int ri = p.pinCount - 1;

  for(int i = 0;i < pkgCount / 2;++i){
    bool lv = (p.pins[li].pkg_number == (i+1));
    bool rv = (p.pins[ri].pkg_number == (pkgCount-i));
#if 1
    QString ls = (lv)?p.getPinLabel("%1(%4) / P%2%3",li)
      :QString("                 ");
    QString rs = (rv)?p.getPinLabel("P%2%3 / %1(%4)",ri)
      :QString("                 ");
#else
    QString ls = (lv)?QString("%1(%4) / P%2%3")
      .arg(p.getPinFunction(li), 6)
      .arg(p.pins[li].port).arg(p.pins[li].port_number,-2)
      .arg(p.pins[li].channel,2)
      :QString("                 ");
    QString rs = (rv)?QString("P%2%3 / %1(%4)")
      .arg(p.getPinFunction(ri), 6)
      .arg(p.pins[ri].port).arg(p.pins[ri].port_number,-2)
      .arg(p.pins[ri].channel,2)
      :QString("");
#endif
    QString mid = QString(" |%1 %2| ").arg(i+1,2).arg(pkgCount-i,2);
    debug.noquote() << ls << mid << rs << "\r\n";

    if(lv) li++;
    if(rv) ri--;
  }

  return debug;
}
