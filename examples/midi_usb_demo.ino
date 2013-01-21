/* Arduino USB MIDI demo */

/* Author: Darran Hunt
 * Released into the public domain.
 */

#define MIDI_COMMAND_NOTE_OFF       0x80
#define MIDI_COMMAND_NOTE_ON        0x90

/* The format of the message to send via serial */
typedef union {
    struct {
	uint8_t command;
	uint8_t channel;
	uint8_t data2;
	uint8_t data3;
    } msg;
    uint8_t raw[4];
} t_midiMsg;

void setup();
void loop();

void setup() 
{
    Serial.begin(115200);
    delay(200);
}

/* List of notes to play, zero terminated */
uint8_t note[] = { 0x3b, 0x3c, 0x3d, 0x3e, 0x3f, 0 };

void loop() 
{
    t_midiMsg midiMsg;
    int ind;

    delay(10000);

    /* Send the notes */
    for (ind=0; note[ind] != 0; ind++) {
	midiMsg.msg.command = MIDI_COMMAND_NOTE_ON;
	midiMsg.msg.channel = 1;
	midiMsg.msg.data2   = note[ind];
	midiMsg.msg.data3   = 64;	/* Velocity */

	/* Send note on */
	Serial.write(midiMsg.raw, sizeof(midiMsg));
	delay(500);

	/* Send note off */
	midiMsg.msg.command = MIDI_COMMAND_NOTE_OFF;
	Serial.write(midiMsg.raw, sizeof(midiMsg));
    }

    /* Check for MIDI commands from the host */
    if (Serial.available() >= sizeof(midiMsg)) {
        for (ind=0; ind<sizeof(midiMsg); ind++) {
            midiMsg.raw[ind] = Serial.read();
        }
    }
}
