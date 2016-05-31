 //
// Library for controlling Electronic Assembly eDIPTFT displays
//
//      Copyright (c) 2013 Stefan Gofferje. All rights reserved.
//
//      This library is free software; you can redistribute it and/or
//      modify it under the terms of the GNU Lesser General Public
//      License as published by the Free Software Foundation; either
//      version 2.1 of the License, or (at your option) any later
//      version.
//
//      This library is distributed in the hope that it will be
//      useful, but WITHOUT ANY WARRANTY; without even the implied
//      warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
//      PURPOSE.  See the GNU Lesser General Public License for more
//      details.
//
//      You should have received a copy of the GNU Lesser General
//      Public License along with this library; if not, write to the
//      Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
//      Boston, MA 02110-1301 USA
//

#include <ediptft.h>
#define DEBUG false


EDIPTFT::EDIPTFT(boolean smallprotocol) {
  _smallprotocol = smallprotocol;
}


void EDIPTFT::begin(long baud) {
    SERIAL_DEV.begin(baud);
}


void EDIPTFT::sendByte(char data) {
  SERIAL_DEV.write(data);
}


char EDIPTFT::readByte() {
  return SERIAL_DEV.read();
}


void EDIPTFT::waitBytesAvailable() {
  while (bytesAvailable() == 0) {
  }
}


char EDIPTFT::waitandreadByte() {
  waitBytesAvailable();
  char result = readByte();
  return(result);
}


unsigned char EDIPTFT::bytesAvailable() {
    return SERIAL_DEV.available();
}


void EDIPTFT::sendData(char* data, char len) {
  if (DEBUG) {
    unsigned char i;
    for (i = 0; i < len; i++) {
      SERIAL_DEV.print(byte(data[i]), HEX);
      SERIAL_DEV.print(" ");
    }
    SERIAL_DEV.println();
  }

  if (_smallprotocol) {
    sendSmall(data, len);
  }
  else {
    unsigned char i;
    for(i=0; i < len; i++) {
      sendByte(data[i]);
    }
  }
}


void EDIPTFT::sendSmall(char* data, char len) {
  unsigned char i, bcc;
  char ok = 0;

  while (ok == 0) {
    sendByte(0x11);
    bcc = 0x11;

    sendByte(len);
    bcc = bcc + len;

    for(i=0; i < len; i++) {
      sendByte(data[i]);
      bcc = bcc + data[i];
    }
    sendByte(bcc);
    waitBytesAvailable();
    if (bytesAvailable() > 0) {
      if (readByte() == ACK) ok = 1;
      else {
        ok = 0;
      }
    }
    else {
      delay(2000);
      ok = 0;
    }
  }
}


void EDIPTFT::sendSmallDC2(char* data, char len) {
  unsigned char i, bcc;
  char ok = 0;

  while (ok == 0) {

    sendByte(0x12);
    bcc = 0x12;

    for(i=0; i < len; i++) {
      sendByte(data[i]);
      bcc = bcc + data[i];
    }
    sendByte(bcc);
    waitBytesAvailable();
    if (bytesAvailable() > 0) {
      if (readByte() == ACK) ok = 1;
      else ok = 0;
    }
    else {
      delay(2000);
      ok = 0;
    }
  }
}


void EDIPTFT::smallProtoSelect(char address) {
  char command [] = {
    0x03, 'A', 'S', address
  };
  sendSmallDC2(command, sizeof(command));
}


void EDIPTFT::smallProtoDeselect(char address) {
  char command [] = {
    0x03, 'A', 'D', address
  };
  sendSmallDC2(command, sizeof(command));
}


unsigned char EDIPTFT::datainBuffer() {
  unsigned char result;
  char command [] = {
    0x01, 'I'
  };
  sendSmallDC2(command, sizeof(command));
  waitandreadByte();
  waitandreadByte();
  result=waitandreadByte();
  waitandreadByte();
  waitandreadByte();
  return result;
}


int EDIPTFT::readBuffer(char* data) {
    unsigned char len, i;
    char command [] = {
        0x01, 'S'
    };
    sendSmallDC2(command, sizeof(command));
    waitandreadByte();
    len=waitandreadByte();
    char result[len];
    for (i = 0; i < len; i++) {
        result[i] = waitandreadByte();
    }
    memcpy(data, result, len);
    waitandreadByte();
    return len;
}


void EDIPTFT::clear() {
  this->deleteDisplay();
  this->removeTouchArea(0, 1);
}


void EDIPTFT::deleteDisplay() {
    char command [] = {
      27, 'D', 'L'
    };
    sendData(command, sizeof(command));
}


void EDIPTFT::invert() {
  char command [] = {
    27, 'D', 'I'
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setDisplayColor(char fg, char bg) {
  char command [] = {
    27, 'F', 'D', fg, bg
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::fillDisplayColor(char bg) {
  char command [] = {
    27, 'D', 'F', bg
  };
  sendData(command, sizeof(command));
}

void EDIPTFT::displayIllumination(unsigned char on) {
  char command [] = {
    27, 'Y', 'L', on
  };
  sendData(command, sizeof(command));
}

void EDIPTFT::setDisplayIlluminationLevel(unsigned char level) {
  char command [] = {
    27, 'Y', 'H', level
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTouchBuzzer(boolean on) {
  char command [] = {
    27, 'A', 'S', on
  };
  sendData(command, sizeof(command));
}

void EDIPTFT::soundBuzzer(unsigned char duration) {
  char command [] = {
    27, 'Y', 'S', duration
  };
  sendData(command, sizeof(command));
}

void EDIPTFT::setOutputPort(unsigned char port, unsigned char value) {
  char command [] = {
    27, 'Y', 'W', port, value
  };
  sendData(command, sizeof(command));
}

void EDIPTFT::terminalOn(boolean on) {
  if (on) {
    char command [] = {27, 'T', 'E'};
    sendData(command, sizeof(command));
  }
  else {
    char command [] = {27, 'T', 'A'};
    sendData(command, sizeof(command));
  }
}


void EDIPTFT::loadImage(int x1, int y1, int nr) {
    char command [] = {27, 'U', 'I',
    #if COORD_SIZE == 1
        (char)x1, (char)y1,
    #else
        lowByte(x1), highbyte(x1), lowByte(y1), highByte(y1),
    #endif
        nr};
    sendData(command, sizeof(command));
}


void EDIPTFT::cursorOn(boolean on) {
  if (on) {
    char command [] = {27, 'T', 'C', 1};
    sendData(command, sizeof(command));
  }
  else {
    char command [] = {27, 'T', 'C', 0};
    sendData(command, sizeof(command));
  }
}


void EDIPTFT::setCursor(char col, char row) {
  char command [] = {27, 'T', 'P', col, row};
  sendData(command, sizeof(command));
}


void EDIPTFT::defineBargraph(char dir, char no, int x1, int y1, int x2, int y2, byte sv, byte ev, char type, char mst) {
  char command [] = {
    27, 'B', dir, no,
    #if COORD_SIZE == 1
        x1, y1, x2, y2,
    #else
        lowByte(x1), highByte(x1), lowByte(y1), highByte(y1),
        lowByte(x2), highByte(x2), lowByte(y2), highByte(y2),
    #endif
    char(sv),
    char(ev),
    type,
    mst
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::updateBargraph(char no, char val) {
  char command [] = {
    27, 'B', 'A', no, val
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setBargraphColor(char no, char fg, char bg, char fr) {
  char command [] = {
    27, 'F', 'B', no, fg, bg, fr
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::linkBargraphLight(char no) {
  char command [] = {
    27, 'Y', 'B', no
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::makeBargraphTouch(char no) {
  char command [] = {
    27, 'A', 'B', no
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::deleteBargraph(char no,char n1) {
  char command [] = {
    27, 'B', 'D', no, n1
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::defineInstrument(char no, int x1, int y1, char image, char angle, char sv, char ev) {
  char command [] = {
    27, 'I', 'P', no,
    #if COORD_SIZE == 1
        x1, y1,
    #else
        lowByte(x1), highByte(x1), lowByte(y1), highByte(y1),
    #endif
    image, angle, sv, ev
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::updateInstrument(char no, char val) {
  char command [] = {
    27, 'I', 'A', no, val
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::redrawInstrument(char no) {
  char command [] = {
    27, 'I', 'N', no
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::deleteInstrument(char no, char n1, char n2) {
  char command [] = {
    27, 'B', 'D', no, n1, n2
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setLineColor(char fg, char bg) {
  char command [] = {
    27, 'F', 'G', fg, bg
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setLineThick(char x, char y) {
  char command [] = {
    27, 'G', 'Z', x, y
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTextColor(char fg, char bg) {
  char command [] = {
    27, 'F', 'Z', fg, bg
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTextFont(char font) {
  char command [] = {
    27, 'Z', 'F', font
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTextAngle(char angle) {
  // 0 = 0°, 1 = 90°, 2 = 180°, 3 = 270°
  char command [] = {
    27, 'Z', 'W', angle
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::drawText(int x1, int y1, char justification, const char* text) {
  byte len = strlen(text);
  byte i;
  char helper [len + 4 + 2 * COORD_SIZE];
  char command [] = {
    27, 'Z', justification,
    #if COORD_SIZE == 1
        x1, y1,
    #else
        lowByte(x1), highByte(x1), lowByte(y1), highByte(y1),
    #endif
  };
  for (i = 0; i <= 4; i++) helper[i] = command[i];
  for (i = 0; i <= len; i++) {
      helper[i + 5] = text[i];
  }
  sendData(helper, sizeof(helper));
}


void EDIPTFT::drawLine(int x1, int y1, int x2, int y2) {
  char command [] = {
    27,'G','D',
    #if COORD_SIZE == 1
        x1, y1, x2, y2
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2)
    #endif
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::drawRect(int x1, int y1, int x2, int y2) {
  char command [] = {
    27,'G','R',
    #if COORD_SIZE == 1
        x1, y1, x2, y2
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2)
    #endif
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::drawRectf(int x1, int y1, int x2, int y2, char color) {
  char command [] = {
    27,'R','F',
    #if COORD_SIZE == 1
        x1, y1, x2, y2,
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
    #endif
    color
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::clearRect(int x1, int y1, int x2, int y2) {
  char command [] = {
    27,'R','L',
    #if COORD_SIZE == 1
        x1, y1, x2, y2
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2)
    #endif
  };
  sendData(command, sizeof(command));
}

void EDIPTFT::invertRect(int x1, int y1, int x2, int y2) {
  char command [] = {
    27,'R','I',
    #if COORD_SIZE == 1
        x1, y1, x2, y2
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2)
    #endif
  };
  sendData(command, sizeof(command));
}

void EDIPTFT::fillRect(int x1, int y1, int x2, int y2) {
  char command [] = {
    27,'R','S',
    #if COORD_SIZE == 1
        x1, y1, x2, y2
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2)
    #endif
  };
  sendData(command, sizeof(command));
}

void EDIPTFT::fillRectp(int x1, int y1, int x2, int y2, char pattern) {
  char command [] = {
    27,'R','M',
    #if COORD_SIZE == 1
        x1, y1, x2, y2, pattern
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
        pattern
    #endif
  };
  sendData(command, sizeof(command));
}




void EDIPTFT::defineTouchKey(int x1, int y1, int x2, int y2, char down, char up,
                             const char* text) {
  byte len = strlen(text);
  byte i;
  char helper [len + 6 + 4 * COORD_SIZE];
  char command [] = {
    27, 'A', 'T',
    #if COORD_SIZE == 1
        x1, y1, x2, y2,
    #else
        lowByte(x1), highByte(x1), lowByte(y1), highByte(y1),
        lowByte(x2), highByte(x2), lowByte(y2), highByte(y2),
    #endif
    down, up
  };
  for (i = 0; i < (5 + 4 * COORD_SIZE); i++) helper[i] = command[i];
  for (i = 0; i <= len+1; i++) helper[i + 5 + 4 * COORD_SIZE] = text[i];
  sendData(helper, sizeof(helper));
}


void EDIPTFT::defineTouchSwitch(int x1, int y1, int x2, int y2,
                                char down, char up, const char* text) {
  byte len = strlen(text);
  byte i;
  char helper [len + 6 + 4 * COORD_SIZE];
  char command [] = {
    27, 'A', 'K',
    #if COORD_SIZE == 1
        x1, y1, x2, y2,
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
    #endif
    down, up
  };
  for (i = 0; i < 5 + 4 * COORD_SIZE; i++) helper[i] = command[i];
  for (i = 0; i <= len; i++) helper[i + 5 + 4 * COORD_SIZE] = text[i];
  sendData(helper, sizeof(helper));
}


void EDIPTFT::defineTouchSwitch(int x, int y, int img, char downcode,
                                char upcode, const char* text) {
  byte len = strlen(text);
  byte i;
  byte n = 6 + 2 * COORD_SIZE;
  char helper [len + n + 1];
  char command [] = {
    27, 'A', 'J',
    #if COORD_SIZE == 1
        x, y,
    #else
        lowByte(x), highByte(x), lowByte(y), highByte(y),
    #endif
    img, downcode, upcode
  };
  for (i = 0; i < n; i++) helper[i] = command[i];
  for (i = 0; i <= len; i++) helper[i + n] = text[i];
  sendData(helper, sizeof(helper));
}


void EDIPTFT::setTouchSwitch(char code,char value) {
  char command [] = {
    27, 'A', 'P', code, value
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTouchkeyColors(
  char n1, char n2, char n3, char s1, char s2, char s3) {
  char command [] = {
    27, 'F', 'E', n1, n2, n3, s1, s2, s3
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTouchkeyFont(char font) {
  char command [] = {
    27, 'A', 'F', font
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTouchkeyLabelColors(char nf, char sf) {
  char command [] = {
    27, 'F', 'A', nf, sf
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTouchGroup(char group) {
  char command [] = {
    27, 'A', 'R', group
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::removeTouchArea(char code, char n1) {
  char command [] = {
    27, 'A', 'L', code, n1
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::callMacro(uint nr) {
  char command[] = {
    27, 'M', 'N', nr
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::callTouchMacro(uint nr) {
  char command[] = {
    27, 'M', 'T', nr
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::callMenuMacro(uint nr) {
  char command[] = {
    27, 'M', 'M', nr
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::defineTouchMenu(int x1, int y1, int x2, int y2,
    char downcode, char upcode, char mnucode, const char *text) {
  byte len = strlen(text);
  byte n = 6 + 4 * COORD_SIZE;
  char helper [len + n + 1];

  char command [] = {
    27, 'A', 'M',
    #if COORD_SIZE == 1
        x1, y1, x2, y2,
    #else
        lowByte(x1),highByte(x1),lowByte(y1),highByte(y1),
        lowByte(x2),highByte(x2),lowByte(y2),highByte(y2),
    #endif
    downcode, upcode, mnucode
  };
  for (int i = 0; i < n; i++) helper[i] = command[i];
  for (int i = 0; i <= len; i++) helper[i + n] = text[i];
  sendData(helper, sizeof(helper));
}


void EDIPTFT::openTouchMenu() {
  char command [] = {
    27, 'N', 'T', 2
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setMenuFont(char font) {
  char command [] = {
    27, 'N', 'F', font
  };
  sendData(command, sizeof(command));
}


void EDIPTFT::setTouchMenuAutomation(bool val) {
  char n1 = val ? 1 : 0;
  char command [] = {
    27, 'N', 'T', n1
  };
  sendData(command, sizeof(command));
}
