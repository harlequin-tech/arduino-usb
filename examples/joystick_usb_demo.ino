/* Arduino USB Joystick HID demo */

/* Author: Darran Hunt
 * Released into the public domain.
 */

struct {
    int8_t x;
    int8_t y;
    uint8_t buttons;
    uint8_t rfu; 	/* reserved for future use */
} joyReport;

void setup();
void loop();

void setup() 
{
    Serial.begin(115200);
    delay(200);
}

/* Move the joystick in a clockwise square every 5 seconds,
 * and press button 1 then button 2.
 */
void loop() 
{
    int ind;
    delay(5000);

    joyReport.buttons = 0;
    joyReport.x = 0;
    joyReport.y = 0;
    joyReport.rfu = 0;

    /* Move the joystick in a clockwise direction */
    joyReport.x = 100;
    Serial.write((uint8_t *)&joyReport, 4);
    delay(1000);

    joyReport.x = 0;
    joyReport.y = 100;
    Serial.write((uint8_t *)&joyReport, 4);
    delay(1000);

    joyReport.x = -100;
    joyReport.y = 0;
    Serial.write((uint8_t *)&joyReport, 4);
    delay(1000);

    joyReport.x = 0;
    joyReport.y = -100;
    Serial.write((uint8_t *)&joyReport, 4);
    delay(1000);

    /* Send button 1 then button 2 */
    joyReport.y = 0;
    joyReport.buttons = 1;
    Serial.write((uint8_t *)&joyReport, 4);
    delay(1000);

    joyReport.buttons = 2;
    Serial.write((uint8_t *)&joyReport, 4);
    delay(1000);
    
    joyReport.buttons = 0;
    Serial.write((uint8_t *)&joyReport, 4);
}
