// ntp
// IPAddress timeServer(132, 163, 4, 101); // time-a.timefreq.bldrdoc.gov NTP server
// IPAddress timeServer(132, 163, 4, 102); // time-b.timefreq.bldrdoc.gov NTP server
IPAddress timeServer(81, 218, 208, 141); // time-c.timefreq.bldrdoc.gov NTP server

const int NTP_PACKET_SIZE = 48; // NTP time stamp is in the first 48 bytes of the message
byte packetBuffer[ NTP_PACKET_SIZE]; //buffer to hold incoming and outgoing packets 


//// send an NTP request to the time server at the given address 
unsigned long sendNTPpacket(IPAddress& address)
{
  Serial.println("Sending NTP packet...");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE); 
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49; 
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp: 
    	   
  Udp.beginPacket(address, 123); //NTP requests are to port 123	
  Udp.write(packetBuffer,NTP_PACKET_SIZE);
  Udp.endPacket(); 
  Serial.println("NTP packet sent...");
}


void setNTP()
{
    // NTP 
  Serial.println("Getting NTP time... "); 
  Udp.begin(localPort);
  Serial.println("Opened port...");
  unsigned long epoch = 0;
  sendNTPpacket(timeServer); // send an NTP packet to a time server
  // wait to see if a reply is available
  delay(2000);  
  if ( Udp.parsePacket() ) {   
    Serial.println("Got NTP reply...");
    // We've received a packet, read the data from it
    Udp.read(packetBuffer,NTP_PACKET_SIZE);  // read the packet into the buffer
    //the timestamp starts at byte 40 of the received packet and is four bytes,
    // or two words, long. First, esxtract the two words:

    unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
    unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);  
    // combine the four bytes (two words) into a long integer
    // this is NTP time (seconds since Jan 1 1900):
    unsigned long secsSince1900 = highWord << 16 | lowWord;  
    Serial.print("NTP time since1900: ");
    Serial.println(secsSince1900);
    if (secsSince1900 > 0) {
      // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
      const unsigned long seventyYears = 2208988800UL;           
      // subtract seventy years:
      unsigned long epoch = secsSince1900 - seventyYears;  
      // print Unix time:
      Serial.print("NTP time:"); 
      Serial.println(epoch);
      Serial.print("RTC time:");
      Serial.println(RTC.get());
      int delta = RTC.get() - epoch;
      
      if ( delta > 10 or delta < 10) {
        Serial.print("Correcting clock delta of: "); 
        Serial.println(delta);
        RTC.set(epoch);   // set the RTC and the system time to the received value             
      }
    } else {
        Serial.println("0 NTP reply...");
    } 
  } else {
      Serial.println("No NTP data received...");
  }

  Serial.println("Setting time from RTC...");
  setTime(RTC.get());
}
