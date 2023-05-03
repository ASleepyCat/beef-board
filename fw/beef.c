// Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

#include "beef.h"

// buffer to hold the previously generated HID report, for comparison purposes inside the HID class driver.
static uint8_t PrevJoystickHIDReportBuffer[sizeof(USB_JoystickReport_Data_t)];

// LUFA HID Class driver interface configuration and state
// information. This structure is passed to all HID Class driver
// functions, so that multiple instances of the same class within a
// device can be differentiated from one another.
USB_ClassInfo_HID_Device_t Joystick_HID_Interface = {
  .Config = {
    .InterfaceNumber = INTERFACE_ID_Joystick,
    .ReportINEndpoint = {
      .Address = JOYSTICK_IN_EPADDR,
      .Size = JOYSTICK_EPSIZE,
      .Banks = 1,
    },
    .PrevReportINBuffer = PrevJoystickHIDReportBuffer,
    .PrevReportINBufferSize = sizeof(PrevJoystickHIDReportBuffer),
  },
};

int8_t turntablePosition = 0;
// bit-field storing button state. bits 0-10 map to buttons 1-11
uint16_t button = 0;
// flag to represent whether the LEDs are controlled by host or not
// when not controlled by host, LEDs light up while the corresponding
// button is held
bool reactiveLightingMode = true;

int main(void) {
	SetupHardware();
	USB_Init();

	// TT wired to F0/F1
	DDRF  &= 0b11111100;
	PORTF |= 0b00000011;

	// buttons mapping:
	// <name>   : [input pin] : [LED pin]
	// BUTTON 1 : B0 : B1
	// BUTTON 2 : B2 : B3
	// BUTTON 3 : B4 : B5
	// BUTTON 4 : B6 : B7
	// BUTTON 5 : D0 : D1
	// BUTTON 6 : D2 : D3
	// BUTTON 7 : D4 : D5
	// START    : D6 : D7
	// VEFX     : C0 : C1
	// EFFECT   : C2 : C3
	// AUX      : C4 : C5
	DDRB  = 0b10101010;
	DDRD  = 0b10101010;
	DDRC &= 0b11101010;
	DDRC |= 0b00101010;

	PORTB  = 0b01010101;
	PORTD  = 0b01010101;
	PORTC |= 0b00010101;
	PORTC &= 0b11010101;

	int8_t prev = -1;
	int8_t curr = -1;

	GlobalInterruptEnable();

	while (1) {
	  HID_Device_USBTask(&Joystick_HID_Interface);
	  HID_Task();
	  USB_USBTask();

	  // TT logic
	  // curr is binary number ab
	  // where a is the signal of F0
	  // and b is the signal of F1
	  // e.g. when F0 == 1 and F1 == 0, then curr == 0b10
	  int8_t a = PINF & (1 << 0) ? 1 : 0;
	  int8_t b = PINF & (1 << 1) ? 1 : 0;
	  curr = (a << 1) | b;

	  if (prev == 3 && curr == 1 ||
	      prev == 1 && curr == 0 ||
	      prev == 0 && curr == 2 ||
	      prev == 2 && curr == 3) {
	    turntablePosition++;
	  } else if (prev == 1 && curr == 3 ||
		     prev == 0 && curr == 1 ||
		     prev == 2 && curr == 0 ||
		     prev == 3 && curr == 2) {
	    turntablePosition--;
	  }
	  prev = curr;

	  // BUTTON 1 : B0 : B1
	  if (~PINB & (1 << 0)) {
	    button |= (1 << 0);
	    if (reactiveLightingMode) PORTB |= (1 << 1);
	  } else {
	    button &= ~(1 << 0);
	    if (reactiveLightingMode) PORTB &= ~(1 << 1);
	  }

	  // BUTTON 2 : B2 : B3
	  if (~PINB & (1 << 2)) {
	    button |= (1 << 1);
	    if (reactiveLightingMode) PORTB |= (1 << 3);
	  } else {
	    button &= ~(1 << 1);
	    if (reactiveLightingMode) PORTB &= ~(1 << 3);
	  }

	  // BUTTON 3 : B4 : B5
	  if (~PINB & (1 << 4)) {
	    button |= (1 << 2);
	    if (reactiveLightingMode) PORTB |= (1 << 5);
	  } else {
	    button &= ~(1 << 2);
	    if (reactiveLightingMode) PORTB &= ~(1 << 5);
	  }

	  // BUTTON 4 : B6 : B7
	  if (~PINB & (1 << 6)) {
	    button |= (1 << 3);
	    if (reactiveLightingMode) PORTB |= (1 << 7);
	  } else {
	    button &= ~(1 << 3);
	    if (reactiveLightingMode) PORTB &= ~(1 << 7);
	  }

	  // BUTTON 5 : D0 : D1
	  if (~PIND & (1 << 0)) {
	    button |= (1 << 4);
	    if (reactiveLightingMode) PORTD |= (1 << 1);
	  } else {
	    button &= ~(1 << 4);
	    if (reactiveLightingMode) PORTD &= ~(1 << 1);
	  }

	  // BUTTON 6 : D2 : D3
	  if (~PIND & (1 << 2)) {
	    button |= (1 << 5);
	    if (reactiveLightingMode) PORTD |= (1 << 3);
	  } else {
	    button &= ~(1 << 5);
	    if (reactiveLightingMode) PORTD &= ~(1 << 3);
	  }

	  // BUTTON 7 : D4 : D5
	  if (~PIND & (1 << 4)) {
	    button |= (1 << 6);
	    if (reactiveLightingMode) PORTD |= (1 << 5);
	  } else {
	    button &= ~(1 << 6);
	    if (reactiveLightingMode) PORTD &= ~(1 << 5);
	  }

	  // START : D6 : D7
	  if (~PIND & (1 << 6)) {
	    button |= (1 << 7);
	    if (reactiveLightingMode) PORTD |= (1 << 7);
	  } else {
	    button &= ~(1 << 7);
	    if (reactiveLightingMode) PORTD &= ~(1 << 7);
	  }

	  // VEFX : C0 : C1
	  if (~PINC & (1 << 0)) {
	    button |= (1 << 8);
	    if (reactiveLightingMode) PORTC |= (1 << 1);
	  } else {
	    button &= ~(1 << 8);
	    if (reactiveLightingMode) PORTC &= ~(1 << 1);
	  }

	  // EFFECT : C2 : C3
	  if (~PINC & (1 << 2)) {
	    button |= (1 << 9);
	    if (reactiveLightingMode) PORTC |= (1 << 3);
	  } else {
	    button &= ~(1 << 9);
	    if (reactiveLightingMode) PORTC &= ~(1 << 3);
	  }

	  // AUX : C4 : C5
	  if (~PINC & (1 << 4)) {
	    button |= (1 << 10);
	    if (reactiveLightingMode) PORTC |= (1 << 5);
	  } else {
	    button &= ~(1 << 10);
	    if (reactiveLightingMode) PORTC &= ~(1 << 5);
	  }
	}
}

// configure board hardware and chip peripherals
void SetupHardware(void) {
  // disable watchdog if enabled by bootloader/fuses
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  // disable clock division
  clock_prescale_set(clock_div_1);

  // hardware init
  USB_Init();
  reactiveLightingMode = true;
}

// event handler for USB connection event
void EVENT_USB_Device_Connect(void){}

// event handler for USB disconnection event
void EVENT_USB_Device_Disconnect(void){}

// event handler for USB config change event
void EVENT_USB_Device_ConfigurationChanged(void) {
  bool ConfigSuccess = true;

  // setup HID report endpoints
  ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_IN_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
  ConfigSuccess &= Endpoint_ConfigureEndpoint(JOYSTICK_OUT_EPADDR, EP_TYPE_INTERRUPT, JOYSTICK_EPSIZE, 1);
}

void EVENT_USB_Device_ControlRequest(void) {
  // handle HID Class specific requests
  switch (USB_ControlRequest.bRequest) {
    case HID_REQ_SetReport:
      if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE)) {
	  Output_t LightsData;

	  Endpoint_ClearSETUP();

	  // read report data from the control endpoint
	  Endpoint_Read_Control_Stream_LE(&LightsData, sizeof(LightsData));
	  Endpoint_ClearIN();

	  ProcessGenericHIDReport(&LightsData);
	}

      break;
    }
}

// process last received report from the host.
void ProcessGenericHIDReport(Output_t* ReportData) {
  reactiveLightingMode = false;

  //forward the lighting data
  Lights_SetState(ReportData->Lights);
}

void HID_Task(void) {
  Endpoint_SelectEndpoint(JOYSTICK_OUT_EPADDR);

  // check if a packet has been sent from the host
  if (Endpoint_IsOUTReceived()) {
    // check if packet contains data
    if (Endpoint_IsReadWriteAllowed()) {
      //temp buffer to hold the read in report from the host
      Output_t LightsData;

      // read generic report data
      Endpoint_Read_Stream_LE(&LightsData, sizeof(LightsData), NULL);

      // process generic report data
      ProcessGenericHIDReport(&LightsData);
    }

    // finalize the stream transfer to send the last packet
    Endpoint_ClearOUT();
  }
}


bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                         uint8_t* const ReportID,
                                         const uint8_t ReportType,
                                         void* ReportData,
                                         uint16_t* const ReportSize) {
  USB_JoystickReport_Data_t* JoystickReport = (USB_JoystickReport_Data_t*)ReportData;

  JoystickReport->X = turntablePosition;
  JoystickReport->Button = button;

  *ReportSize = sizeof(USB_JoystickReport_Data_t);
  return false;
}

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
                                          const uint8_t ReportID,
                                          const uint8_t ReportType,
                                          const void* ReportData,
                                          const uint16_t ReportSize)
{
  // Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}

void Lights_SetState(uint16_t OutputData) {
  // buttons to pins:
  // <name>   : <input pin> : <LED pin>
  // BUTTON 1 : B0 : B1
  if (OutputData & (1 << 0)) {
    PORTB |= (1 << 1);
  } else {
    PORTB &= ~(1 << 1);
  }
  // BUTTON 2 : B2 : B3
  if (OutputData & (1 << 1)) {
    PORTB |= (1 << 3);
  } else {
    PORTB &= ~(1 << 3);
  }
  // BUTTON 3 : B4 : B5
  if (OutputData & (1 << 2)) {
    PORTB |= (1 << 5);
  } else {
    PORTB &= ~(1 << 5);
  }
  // BUTTON 4 : B6 : B7
  if (OutputData & (1 << 3)) {
    PORTB |= (1 << 7);
  } else {
    PORTB &= ~(1 << 7);
  }
  // BUTTON 5 : D0 : D1
  if (OutputData & (1 << 4)) {
    PORTD |= (1 << 1);
  } else {
    PORTD &= ~(1 << 1);
  }
  // BUTTON 6 : D2 : D3
  if (OutputData & (1 << 5)) {
    PORTD |= (1 << 3);
  } else {
    PORTD &= ~(1 << 3);
  }
  // BUTTON 7 : D4 : D5
  if (OutputData & (1 << 6)) {
    PORTD |= (1 << 5);
  } else {
    PORTD &= ~(1 << 5);
  }
  // START    : D6 : D7
  if (OutputData & (1 << 7)) {
    PORTD |= (1 << 7);
  } else {
    PORTD &= ~(1 << 7);
  }
  // VEFX     : C0 : C1
  if (OutputData & (1 << 8)) {
    PORTC |= (1 << 1);
  } else {
    PORTC &= ~(1 << 1);
  }
  // EFFECT   : C2 : C3
  if (OutputData & (1 << 9)) {
    PORTC |= (1 << 3);
  } else {
    PORTC &= ~(1 << 3);
  }
  // AUX      : C4 : C5
  if (OutputData & (1 << 10)) {
    PORTC |= (1 << 5);
  } else {
    PORTC &= ~(1 << 5);
  }
}