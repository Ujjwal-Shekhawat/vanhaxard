#include <hidboot.h>
#include <usbhub.h>
#include <Mouse.h>
#include <Wire.h>
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
USB Usb;
USBHub Hub(&Usb);
HIDBoot< USB_HID_PROTOCOL_KEYBOARD | USB_HID_PROTOCOL_MOUSE > HidComposite(&Usb);
HIDBoot<USB_HID_PROTOCOL_MOUSE> HidMouse(&Usb);

int delta[2];
int tX, tY = 0;
int negMax = -127;
int posMax = 127;
float mouseSmooth = 0.7;

int lmb = 0;
int rmb = 0;
int mmb = 0;
bool toggle = true;

class MouseRptParser : public MouseReportParser {
protected:
  void OnMouseMove(MOUSEINFO *mi);
  void OnLeftButtonUp(MOUSEINFO *mi);
  void OnLeftButtonDown(MOUSEINFO *mi);
  void OnRightButtonUp(MOUSEINFO *mi);
  void OnRightButtonDown(MOUSEINFO *mi);
  void OnMiddleButtonUp(MOUSEINFO *mi);
  void OnMiddleButtonDown(MOUSEINFO *mi);
};

void MouseRptParser::OnMouseMove(MOUSEINFO *mi) {
  delta[0] = mi->dX;
  delta[1] = mi->dY;
};
void MouseRptParser::OnLeftButtonUp(MOUSEINFO *mi) {
  lmb = 0;
};
void MouseRptParser::OnLeftButtonDown(MOUSEINFO *mi) {
  lmb = 1;
};
void MouseRptParser::OnRightButtonUp(MOUSEINFO *mi) {
  rmb = 0;
};
void MouseRptParser::OnRightButtonDown(MOUSEINFO *mi) {
  rmb = 1;
};
void MouseRptParser::OnMiddleButtonUp(MOUSEINFO *mi) {
  mmb = 0;
};
void MouseRptParser::OnMiddleButtonDown(MOUSEINFO *mi) {
  mmb = 1;
};
MouseRptParser MousePrs;

void setup() {
  Mouse.begin();
  Serial.begin(115200);
  Usb.Init();
  HidComposite.SetReportParser(1, &MousePrs);
  HidMouse.SetReportParser(0, &MousePrs);
  pinMode(13, OUTPUT);
}

void handleEvents() {
  // Left Mouse
  if (lmb == 0) {
    Mouse.release(MOUSE_LEFT);
  } else if (lmb == 1) {
    Mouse.press(MOUSE_LEFT);
  }
  // Right Mouse
  if (rmb == 0) {
    Mouse.release(MOUSE_RIGHT);
  } else if (rmb == 1) {
    Mouse.press(MOUSE_RIGHT);
  }
  // Middle Mouse
  if (mmb == 0) {
    Mouse.release(MOUSE_MIDDLE);
  } else if (mmb == 1) {
    Mouse.press(MOUSE_MIDDLE);
  }
}

void toggleTracking() {
  if (Mouse.isPressed(MOUSE_MIDDLE)) {
    toggle = !toggle;
    delay(100);
  }

  if (toggle == false) {
    digitalWrite(13, LOW);
  } else {
    digitalWrite(13, HIGH);
  }
}

void loop() {
  delta[0] = 0;
  delta[1] = 0;
  Usb.Task();

  handleEvents();

  toggleTracking();

  if (Serial.available() > 0 && toggle) {
    String data = Serial.readStringUntil('x');

    int dataIndex = data.indexOf(':');

    tX = data.substring(0, dataIndex).toInt();
    tY = data.substring(dataIndex + 1).toInt();

    delta[0] += (tX - delta[0]) * mouseSmooth;
    delta[1] += (tY - delta[1]) * mouseSmooth;

    // char x = Serial.read();
    // if (x == 'g' || Mouse.isPressed()) {
    //   // Mouse.click();
    //   // Mouse.move(0, -35);
    // }

    Mouse.move(delta[0], delta[1]);
  } else {
    Mouse.move(delta[0], delta[1]);
  }
}
