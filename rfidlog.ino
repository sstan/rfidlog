extern "C" {
  #include "rfidlog.h"
}

#define POLL_RECEIVED_TAG_DELAY_MS 100

void wait_ok(rfidlog_cmd cmd)
{
  int cntr = 0;
  int wait_time = 200; /* initial wait time in milliseconds. */
  Serial.print("Waiting.");
  do
  {
    rfidlog_send_command(cmd);
    delay(wait_time);
    wait_time = 500; /* Increase the wait time. */
    Serial.print(".");
    
    if (rcv == 1)
    {
      Serial.println((char *)rfid_tag_str);
      rcv = 0; /* reset the "something received" flag */
    }
  } while(rfid_ok == 0);
  rfid_ok = 0; /* reset the OK received flag */
  Serial.println("[OK]");
}

void setup() {
  // put your setup code here, to run once:
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  rfidlog_uart_init();
  
  Serial.println("Waiting for a response from RFIDLOG.");
  wait_ok(SET_FORMAT_DECIMAL);
  wait_ok(SET_OUTPUT_MODE_1); /* USING THIS MODE AVOIDS THE NEED TO USE READ_LOG_FILE */

  Serial.println("RFIDLOG seems to be ready. Entering loop().");
}

void loop() {
  
  
  rfidlog_send_command(READ_LOG_FILE); /* NOT RECOMMENDED, REMOVE THIS LINE. */
  
  while(rcv == 0)
  {
    delay(POLL_RECEIVED_TAG_DELAY_MS);
  }
  rcv = 0; /* reset flag */
  
  Serial.print("Received: ");
  Serial.println((char*)rfid_tag_str);
  /* If a valid tag has been received */
  if (rfid_tag != 0)
  {
    rfid_tag = 0;
    Serial.println("TAG VALID");
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
  }
  else
  {
    Serial.println("NOT a valid tag.");
  }

  delay(1000); /* debug only */
}
