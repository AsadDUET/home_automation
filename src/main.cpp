#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Fingerprint.h>
#include <Keypad.h>
#include <Servo.h>
#define COLUMS 16
#define ROWS 2
#define PAGE ((COLUMS) * (ROWS))
#define DHTPIN 8
#define DHTTYPE DHT22
#define fpSerial Serial3

const int ROW_NUM = 4;	  //four rows
const int COLUMN_NUM = 4; //four columns
char keys[ROW_NUM][COLUMN_NUM] = {
	{'1', '2', '3', 'A'},
	{'4', '5', '6', 'B'},
	{'7', '8', '9', 'C'},
	{'*', '0', '#', 'D'}};
uint8_t id;
byte pin_rows[ROW_NUM] = {53, 51, 49, 47};		//connect to the row pinouts of the keypad
byte pin_column[COLUMN_NUM] = {45, 43, 41, 39}; //connect to the column pinouts of the keypad
char key;
boolean door_open = false;
unsigned long door_open_time = 0;
DHT_Unified dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&fpSerial);
Keypad keypad = Keypad(makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM);
Servo myservo;
int pos = 0;
class Adht
{
public:
	Adht()
	{
		delayMS = 3000;
		_time = millis();
		dht.begin();
	}
	void measure()
	{
		//Serial.println(_time);
		if ((millis() - _time) >= 3000)
		{
			_time = millis();
			// Get temperature event and print its value.
			sensors_event_t event;
			dht.temperature().getEvent(&event);
			if (isnan(event.temperature))
			{
				Serial.println(F("Error reading temperature!"));
			}
			else
			{
				lcd.setCursor(0, 0);
				lcd.print("T:");
				lcd.print(event.temperature);
				lcd.print(F("C"));
				if (event.temperature > 30)
				{
					digitalWrite(50, HIGH);
				}
				else
				{
					digitalWrite(50, LOW);
				}
			}
			// Get humidity event and print its value.
			dht.humidity().getEvent(&event);
			if (isnan(event.relative_humidity))
			{
				Serial.println(F("Error reading humidity!"));
			}
			else
			{

				lcd.print(" H:");
				lcd.print(event.relative_humidity);
				lcd.print(F("%"));
			}
		}
	}

private:
	unsigned long _time;
	uint32_t delayMS;
};
Adht sensor;
uint8_t getFingerprintEnroll()
{
	int p = -1;
	lcd.clear();
	lcd.print("place finter");
	Serial.println(id);
	while (p != FINGERPRINT_OK)
	{
		p = finger.getImage();
		switch (p)
		{
		case FINGERPRINT_OK:
			Serial.println("Image taken");
			break;
		case FINGERPRINT_NOFINGER:
			Serial.println(".");
			break;
		case FINGERPRINT_PACKETRECIEVEERR:
			Serial.println("Communication error");
			break;
		case FINGERPRINT_IMAGEFAIL:
			Serial.println("Imaging error");
			break;
		default:
			Serial.println("Unknown error");
			break;
		}
	}

	// OK success!

	p = finger.image2Tz(1);
	switch (p)
	{
	case FINGERPRINT_OK:
		Serial.println("Image converted");
		break;
	case FINGERPRINT_IMAGEMESS:
		Serial.println("Image too messy");
		return p;
	case FINGERPRINT_PACKETRECIEVEERR:
		Serial.println("Communication error");
		return p;
	case FINGERPRINT_FEATUREFAIL:
		Serial.println("Could not find fingerprint features");
		return p;
	case FINGERPRINT_INVALIDIMAGE:
		Serial.println("Could not find fingerprint features");
		return p;
	default:
		Serial.println("Unknown error");
		return p;
	}

	lcd.clear();
	lcd.print("Remove finger");
	delay(2000);
	p = 0;
	while (p != FINGERPRINT_NOFINGER)
	{
		p = finger.getImage();
	}
	Serial.print("ID ");
	Serial.println(id);
	p = -1;
	lcd.clear();
	lcd.print("Place same finger");
	while (p != FINGERPRINT_OK)
	{
		p = finger.getImage();
		switch (p)
		{
		case FINGERPRINT_OK:
			Serial.println("Image taken");
			break;
		case FINGERPRINT_NOFINGER:
			Serial.print(".");
			break;
		case FINGERPRINT_PACKETRECIEVEERR:
			Serial.println("Communication error");
			break;
		case FINGERPRINT_IMAGEFAIL:
			Serial.println("Imaging error");
			break;
		default:
			Serial.println("Unknown error");
			break;
		}
	}

	// OK success!

	p = finger.image2Tz(2);
	switch (p)
	{
	case FINGERPRINT_OK:
		Serial.println("Image converted");
		break;
	case FINGERPRINT_IMAGEMESS:
		Serial.println("Image too messy");
		return p;
	case FINGERPRINT_PACKETRECIEVEERR:
		Serial.println("Communication error");
		return p;
	case FINGERPRINT_FEATUREFAIL:
		Serial.println("Could not find fingerprint features");
		return p;
	case FINGERPRINT_INVALIDIMAGE:
		Serial.println("Could not find fingerprint features");
		return p;
	default:
		Serial.println("Unknown error");
		return p;
	}

	// OK converted!
	Serial.print("Creating model for #");
	Serial.println(id);

	p = finger.createModel();
	if (p == FINGERPRINT_OK)
	{
		lcd.clear();
		lcd.print("Prints matched!");
	}
	/*else if (p == FINGERPRINT_PACKETRECIEVEERR)
	{
		Serial.println("Communication error");
		return p;
	}
	else if (p == FINGERPRINT_ENROLLMISMATCH)
	{
		Serial.println("Fingerprints did not match");
		return p;
	}*/
	else
	{
		lcd.clear();
		lcd.print("Unknown error");
		return p;
	}

	Serial.print("ID ");
	Serial.println(id);
	p = finger.storeModel(id);
	if (p == FINGERPRINT_OK)
	{
		lcd.clear();
		lcd.print("Stored!");
		delay(1000);
	}
	/*else if (p == FINGERPRINT_PACKETRECIEVEERR)
	{
		Serial.println("Communication error");
		return p;
	}
	else if (p == FINGERPRINT_BADLOCATION)
	{
		Serial.println("Could not store in that location");
		return p;
	}
	else if (p == FINGERPRINT_FLASHERR)
	{
		Serial.println("Error writing to flash");
		return p;
	}*/
	else
	{
		lcd.clear();
		lcd.print("saving error");
		return p;
	}

	return true;
}
int getFingerprintIDez()
{
	uint8_t p = finger.getImage();
	if (p != FINGERPRINT_OK)
		return -1;

	p = finger.image2Tz();
	if (p != FINGERPRINT_OK)
		return -1;

	p = finger.fingerFastSearch();
	if (p != FINGERPRINT_OK)
		return -1;

	// found a match!
	Serial.print("Found ID #");
	Serial.print(finger.fingerID);
	Serial.print(" with confidence of ");
	Serial.println(finger.confidence);
	return finger.fingerID;
}
void enrole()
{
	String input_id;
	while (1)
	{
		lcd.clear();
		lcd.print("Enter ID # 1-127");
		while (1)
		{
			key = keypad.getKey();
			if (key)
			{
				if (key == '#')
				{

					Serial.println("Break");
					break;
				}
				else if (key == '1' || key == '2' || key == '3' || key == '4' || key == '5' || key == '6' || key == '7' || key == '8' || key == '9' || key == '0')
				{
					input_id += key;
					lcd.clear();
					lcd.print("Enter ID # 1-127");
					lcd.setCursor(0, 1);
					lcd.print(input_id);
				}
				else if (key == 'C')
				{

					break;
				}
			}
		}
		if (key == 'C')
		{

			lcd.clear();
			lcd.print("Cancel");
			break;
		}
		id = input_id.toInt();
		if (id == 0)
		{
			lcd.clear();
			lcd.print("0 not Allowed");
		}
		else
		{
			lcd.clear();
			lcd.print("Enrolling ID # ");
			lcd.print(id);
			while (!getFingerprintEnroll())
				;
		}
	}
}
int limit = 800;
int data[5][2] = {{1, 1},
				  {1, 1},
				  {1, 1},
				  {1, 1},
				  {1, 1}};
int plus_one[5][2] = {{1, 1},
					  {0, 1},
					  {0, 0},
					  {1, 0},
					  {1, 1}};
int minus_one[5][2] = {{1, 1},
					   {1, 0},
					   {0, 0},
					   {0, 1},
					   {1, 1}};
int i;
int j;
bool equal = true;
int person = 0;

void check(int a8, int a9)
{

	if (a8 != data[4][0] || a9 != data[4][1])
	{
		/*Serial.print(a8);
		Serial.print(",");
		Serial.println(a9);*/
		for (i = 0; i < 4; i++)
		{
			data[i][0] = data[i + 1][0];
			data[i][1] = data[i + 1][1];
		}
		data[4][0] = a8;
		data[4][1] = a9;
		/*for (i = 0; i < 5; i++)
		{
			Serial.print(data[i][0]);
			Serial.print(",");
			Serial.println(data[i][1]);
			
		}
		Serial.println("#############");*/

		if ((data[4][0] == 1) && (data[4][1] == 1))
		{
			equal = true;
			for (i = 0; i < 5; i++)
			{
				for (j = 0; j < 2; j++)
				{
					if (data[i][j] != plus_one[i][j])
					{
						equal = false;
						break;
					}
				}
				if (!equal)
				{
					break;
				}
			}
			if (equal)
			{
				person += 1;
				lcd.clear();
				lcd.print(person);
			}
			else
			{
				equal = true;
				for (i = 0; i < 5; i++)
				{
					for (j = 0; j < 2; j++)
					{
						if (data[i][j] != minus_one[i][j])
						{
							equal = false;
							break;
						}
					}
					if (!equal)
					{
						break;
					}
				}
				if (equal)
				{
					person -= 1;
					if (person < 0)
					{
						person = 0;
					}
					lcd.clear();
					lcd.print(person);
				}
			}

			Serial.println(person);
		}
	}
}
void setup()
{
	pinMode(37, OUTPUT);
	pinMode(50, OUTPUT);
	myservo.attach(12);
	myservo.write(0);
	Serial.begin(115200);
	Serial.println("Home Automation");

	// set the data rate for the sensor serial port
	finger.begin(57600);

	if (finger.verifyPassword())
	{
		Serial.println("Found fingerprint sensor!");
	}
	else
	{
		Serial.println("Did not find fingerprint sensor :(");
		//while (1)
		;
	}

	lcd.init();
	lcd.backlight();
	delay(1000);
	lcd.setCursor(0, 1);
	lcd.print("Hello ");
}

void loop()
{
	if (person > 0)
	{
		digitalWrite(37, HIGH);
	}
	else
	{
		digitalWrite(37, LOW);
	}

	if (door_open)
	{
		if (millis() - door_open_time > 5000)
		{
			for (pos = 90; pos >= 0; pos -= 1)
			{
				myservo.write(pos);
				delay(15);
			}
			door_open = false;
		}
	}
	if (door_open)
	{
		check(analogRead(A8) > limit, analogRead(A9) > limit);

		//Serial.print(analogRead(A8));
		//Serial.print(",");
		//Serial.println(analogRead(A9));
	}
	else
	{
		sensor.measure();
		key = keypad.getKey();
		if (key == 'A')
		{
			lcd.clear();
			lcd.print(key);
			enrole();
			lcd.clear();
			lcd.print("End Enrole");
		}
		if (key == 'D')
		{
			for (pos = 0; pos <= 90; pos += 1)
			{
				myservo.write(pos);
				delay(15);
			}
			door_open = true;
			door_open_time = millis();
		}
		int known = getFingerprintIDez();
		if (known != -1)
		{
			lcd.clear();
			lcd.print("Access ID: ");
			lcd.print(known);
			for (pos = 0; pos <= 90; pos += 1)
			{
				myservo.write(pos);
				delay(15);
			}
			door_open = true;
			door_open_time = millis();
		}
		lcd.setCursor(0, 1);
		lcd.print(person);
		lcd.print(" Person inside");
	}
}