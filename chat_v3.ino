#include <SPI.h>
#include <NRFLite.h>

/*
nRF24L01+ to Arduino Uno Wiring
Top view of 2x4 header. Pin 1 (GND) has a square outline

                    ╔══════╦──────┐
Arduino GND         ║  GND ║  VCC │      Arduino 3.3V
                    ╠══════╬──────┤
Arduino 9           │   CE │  CSN │      Arduino 10 (SS)
                    ├──────┼──────┤
Arduino 13 (SCK)    │  SCK │ MOSI │      Arduino 11 (MOSI)
                    ├──────┼──────┤
Arduino 12 (MISO)   │ MISO │  IRQ │      (Not Connected)
                    └──────┴──────┘
*/

// #define RADIO_ID 0  // Alaina
// #define RADIO_ID 1  // Alex
// #define RADIO_ID 2  // Alice
// #define RADIO_ID 3  // Anna
// #define RADIO_ID 4  // Cleo
// #define RADIO_ID 5  // Egidijus
// #define RADIO_ID 6  // Inti
// #define RADIO_ID 7  // Jenna
// #define RADIO_ID 8  // Ji Hyun
// #define RADIO_ID 9  // Laura
// #define RADIO_ID 10 // Lia A
// #define RADIO_ID 11 // Lia C
// #define RADIO_ID 12 // Louisa
// #define RADIO_ID 13 // Makiko
// #define RADIO_ID 14 // Meera
// #define RADIO_ID 15 // Robin
// #define RADIO_ID 16 // Sahl
// #define RADIO_ID 17 // Sandra
// #define RADIO_ID 18 // Sannidhi
// #define RADIO_ID 19 // Sekina
// #define RADIO_ID 20 // Tom
// #define RADIO_ID 21 // Veronica

// --- Radio Pin Configuration ---
const static uint8_t PIN_RADIO_CE = 9;
const static uint8_t PIN_RADIO_CSN = 10;

String studentNames[] = 
{
    "Alaina", "Alex", "Alice", "Anna", "Cleo", "Egidijus", "Inti", "Jenna",
    "Ji Hyun", "Laura", "Lia A", "Lia C", "Louisa", "Makiko", "Meera", "Robin",
    "Sahl", "Sandra", "Sannidhi", "Sekina", "Tom", "Veronica"
};

const static uint8_t studentCount = sizeof(studentNames) / sizeof(studentNames[0]);

// This struct organizes our data. We send the sender's ID (1 byte)
// and then the message text. This is much more efficient than sending the full name.
// The total size is 1 + 31 = 32 bytes, which is the max payload size.
struct RadioPacket
{
    uint8_t senderId;
    char message[31];
};

// --- Global variables ---
NRFLite _radio;
RadioPacket packet; // A single packet instance to be reused for sending and receiving

void setup() {
    Serial.begin(115200);
    while (!Serial) {} // Wait for serial connection

    if (!_radio.init(RADIO_ID, PIN_RADIO_CE, PIN_RADIO_CSN))
    {
        Serial.println("Cannot communicate with the radio. Please check wiring.");
        while (1); // loop forever
    }

    Serial.print("Chatting as: ");
    Serial.println(studentNames[RADIO_ID]);
}

void loop()
{
    // --- SENDING LOGIC ---
    if (Serial.available() > 0)
    {
        String input = Serial.readStringUntil('\n');
        input.trim(); // Remove any leading/trailing whitespace

        if (input.length() > 0)
        {
            // Prepare the packet
            packet.senderId = RADIO_ID;
            input.toCharArray(packet.message, sizeof(packet.message));

            // Display your own message immediately
            Serial.print(studentNames[RADIO_ID]); 
            Serial.print(": ");
            Serial.println(packet.message);

            // Broadcast the packet to all other students
            for (int i = 0; i < studentCount; i++)
            { 
                if (i != RADIO_ID) // Don't send the message to yourself
                { 
                    _radio.send(i, &packet, sizeof(packet));
                }
            }
        }
    }

    // --- RECEIVING LOGIC ---
    if (_radio.hasData())
    {
        // Read the incoming packet data into our 'packet' struct
        _radio.readData(&packet);

        // Check if the senderId is valid (between 0 and one less than the student count)
        if (packet.senderId >= 0 && packet.senderId < studentCount)
        { 
            Serial.print(studentNames[packet.senderId]);
        }
        else 
        {
            Serial.print("Anonymous");
        }

        Serial.print(": ");
        Serial.println(packet.message);
    }
}