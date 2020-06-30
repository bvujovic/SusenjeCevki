// Aparat koji s vremena na vreme ukljucuje ventilator koji gura vazduh kroz vlazna gumena creva
// koriscena pri menjanju vode u akvarijumima.

// #define UNO    // UNO ili Tiny
#define DEBUG

#include <Arduino.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <EEPROM.h>
const int eprPos = 25; // Pozicija na EEPROMu na kojoj se cuva broj minuta bleje x (itvFanOff = x * 1000 * 60)

#ifdef UNO
const int pinFan = 8;
const int pinPot = A0;
#else
const int pinFan = 0;
const int pinPot = 2; // Analogni input pin 2 je na ATtiny85 pin 4.
#endif
const int pinBtn = 3; // Ako se promeni ovaj pin, promeniti i kôd u setup-u za enable interrupt-a.

const int itvFanOn = 1000 * 5; // Koliko msec ventilator radi.
int idxFanOff;
int cntFanOff = 4; //! izmeniti ovo sa sizeof()
int fanOffMins[] = {1, 3, 10, 30};

void vent(bool on)
{
  int valPot = analogRead(pinPot);
  int valPwm = on ? map(valPot, 0, 1023, 0, 255) : 0;

  analogWrite(pinFan, valPwm);
  //B digitalWrite(pinFan, on);
#ifdef UNO
  analogWrite(LED_BUILTIN, valPwm);
  //B digitalWrite(LED_BUILTIN, on);
#endif
}

void LoadItvFanOff()
{
  int idx = EEPROM.read(eprPos);
  idxFanOff = (idx >= 0 && idx < cntFanOff) ? idx : 0;
}

// Promena broja minuta bleje ventilatora. Ide ovako kroz niz fanOffMins: 1, 3, 10, 30, 1, 3 ...
void ChangeItvFanOff()
{
  idxFanOff = (idxFanOff + 1) % cntFanOff;
  EEPROM.write(eprPos, idxFanOff);
}

void setup()
{
  pinMode(pinFan, OUTPUT);
#ifdef UNO
  pinMode(LED_BUILTIN, OUTPUT);
#endif
  pinMode(pinPot, INPUT);
  pinMode(pinBtn, INPUT_PULLUP);

  LoadItvFanOff();

  GIMSK |= _BV(PCIE);   // Enable Pin Change Interrupts
  PCMSK |= _BV(PCINT3); // Use PBX as interrupt pin
  sei();                // Enable interrupts
}

ISR(PCINT0_vect)
{
  //B vent(!digitalRead(pinBtn));
  if (digitalRead(pinBtn))
    ChangeItvFanOff();
}

void loop()
{
#ifndef DEBUG
  // priprema - upozorenje korisniku da ce propeler da se zavrti
  vent(true);
  delay(120);
  vent(false);
  delay(1000);

  // susenje cevi i ondak dugacka bleja
  vent(true);
  delay(itvFanOn);
  vent(false);
  delay(fanOffMins[idxFanOff] * 1000L * 60);
#else
  vent(true);
  delay(1000);
  vent(false);
  delay(fanOffMins[idxFanOff] * 1000L); // pri testiranju bleja traje x sekundi umesto x minuta
#endif
}
