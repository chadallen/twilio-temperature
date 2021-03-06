/*

Send an SMS from Arduino Yun via temboo and twilio that contains the temperature reading from an analog sensor

Chad Allen 
Jan 12 2015
More info at: https://github.com/chadallen/twilio-temperature/blob/master/README.md

*/

#include <Bridge.h>
#include <Temboo.h>
#include "TembooAccount.h" // contains Temboo account information
#include "TwilioAccount.h" // contains Twilio account information


void setup() {
	Serial.begin(9600);

  	// for debugging, wait until a serial console is connected
  	delay(4000);
  	while (!Serial);

  	Bridge.begin();

  	// set up the switch
  	pinMode(2, INPUT);
}

void loop()
{

	// a flag to indicate whether we've sent the SMS yet or not
	boolean susccess = false; 

	// store the switch state
	int switchState = 0;

	// set up the temp sensor
	const int sensorPin = A0;

	// store the temperature sensor value
	int sensorVal = 0;

	// store a converted voltage value from the temperature sensor
	float voltage = 0;

	// store a converted temperature value based on voltage
	float temperature = 0;

	// add a manual calibration to temeperature in Farenheit degrees
	int FCalibrate = 22;

	// store the status of the switch
	switchState = digitalRead(2);

	// read the value on AnalogIn pin 0 and store it in a variable
	sensorVal = analogRead(sensorPin);

	// convert the ADC reading to voltage
	voltage = (sensorVal / 1024.0) * 5.0;

	// store a delay value for the loop
	int loopDelay = 1000;

	// convert the voltage to temperature in degrees C
	// the sensor changes 10 mV per degree
	// the datasheet says there's a 500 mV offset
	// ((volatge - 500mV) times 100)
	// convert to F using formula: °C  x  9/5 + 32 = °F
	  
	temperature = ((voltage - .5) * 100) * (1.8) + FCalibrate;  

	// when the switch is closed, send an SMS

  	if (switchState == HIGH) {

	  	Serial.println("Running SendAnSMS...");

	    // we need a Process object to send a Choreo request to Temboo
	    TembooChoreo SendSMSChoreo;

	    // invoke the Temboo client
	    // NOTE that the client must be reinvoked and repopulated with
	    // appropriate arguments each time its run() method is called.
	    SendSMSChoreo.begin();

	    // set Temboo account credentials
	    SendSMSChoreo.setAccountName(TEMBOO_ACCOUNT);
	    SendSMSChoreo.setAppKeyName(TEMBOO_APP_KEY_NAME);
	    SendSMSChoreo.setAppKey(TEMBOO_APP_KEY);

	    // identify the Temboo Library choreo to run (Twilio > SMSMessages > SendSMS)
	    SendSMSChoreo.setChoreo("/Library/Twilio/SMSMessages/SendSMS");

	    // set the required choreo inputs
	    // see https://www.temboo.com/library/Library/Twilio/SMSMessages/SendSMS/
	    // for complete details about the inputs for this Choreo

	    // the first input is a your AccountSID
	    SendSMSChoreo.addInput("AccountSID", TWILIO_ACCOUNT_SID);

	    // next is your Auth Token
	    SendSMSChoreo.addInput("AuthToken", TWILIO_AUTH_TOKEN);

	    // next is your Twilio phone number
	    SendSMSChoreo.addInput("From", TWILIO_NUMBER);

	    // next, what number to send the SMS to
	    SendSMSChoreo.addInput("To", RECIPIENT_NUMBER);

	    static char temperatureString[15];
	    dtostrf(temperature,4,2,temperatureString);

	    //MESSAGE
	    // finally, the text of the message to send
	    String textBody = "Air temperature at Arduino location is: ";
	    textBody += temperatureString;
	    textBody += " F.";
	    Serial.println(textBody);
	    SendSMSChoreo.addInput("Body", textBody);

	    // tell the Process to run and wait for the results. The
	    // return code (returnCode) will tell us whether the Temboo client
	    // was able to send our request to the Temboo servers
	    unsigned int returnCode = SendSMSChoreo.run();

		// a return code of zero (0) means everything worked
		if (returnCode == 0) {
		  	Serial.println("Success! SMS sent!");
		    } else {
		      	// a non-zero return code means there was an error
		      	// read and print the error message
		      	while (SendSMSChoreo.available()) {
		      	char c = SendSMSChoreo.read();
		      	Serial.print(c);
		    }
		}
	SendSMSChoreo.close();

	// do nothing for the next n seconds
	Serial.println("Waiting...");
	delay(loopDelay);

	}
}
