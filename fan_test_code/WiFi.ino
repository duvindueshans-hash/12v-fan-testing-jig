int connectWiFi(const char *ssid, const char *password) {
  Serial.println();
  Serial.println("******************************************************");
  //changePage(24);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password); 
  int count = 0;
  while (WiFi.status() != WL_CONNECTED && count < 20) {
    delay(500);
    Serial.print(".");
    count += 1;
    if (count % 2 == 0) {
      digitalWrite(wifi_led, HIGH);
    } else {
      digitalWrite(wifi_led, LOW);
    }
  }
  if (count < 20) {
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());

  //  nextion.begin(9600, SERIAL_8N1, NEXTION_RX, NEXTION_TX);  // Nextion baud rate
  Serial1.begin(9600, SERIAL_8N1, 13, 14);  // QR Code Scanner on UART2 (RX=GPIO13, TX=GPIO14)
   // Serial.println("UART2 Initialized on Pins 13 and 14");

    return 1;
  } else {
    Serial1.begin(9600, SERIAL_8N1, 13, 14);
    return 0;
  }
}

void saveStringToEEPROM(int addr, const String &str) {
  for (int i = 0; i < str.length(); i++) {
    EEPROM.write(addr + i, str[i]);
  }
  EEPROM.write(addr + str.length(), '\0');  // Null-terminate
  EEPROM.commit();
}

String readStringFromEEPROM(int addr) {
  String str;
  char ch;
  while ((ch = EEPROM.read(addr++)) != '\0') {
    str += ch;
  }
  return str;
}


// WARNING: SysProvEvent is called from a separate FreeRTOS task (thread)!
void SysProvEvent(arduino_event_t *sys_event) {
  switch (sys_event->event_id) {
    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.print("\nConnected IP address : ");
      Serial.println(IPAddress(sys_event->event_info.got_ip.ip_info.ip.addr));
      wifi_connected = 1;
      break;
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("\nDisconnected. Connecting to the AP again... ");
      break;
    case ARDUINO_EVENT_PROV_START:
      Serial.println("\nProvisioning started\nGive Credentials of your access point using smartphone app");
      break;
    case ARDUINO_EVENT_PROV_CRED_RECV:
      {
        wifi_ssid = String((const char *)sys_event->event_info.prov_cred_recv.ssid);
        wifi_password = String((char const *)sys_event->event_info.prov_cred_recv.password);

        Serial.println("\nReceived Wi-Fi credentials");
        Serial.print("\tSSID : ");
        Serial.println(wifi_ssid);
        Serial.print("\tPassword : ");
        Serial.println(wifi_password);
        break;
      }
    case ARDUINO_EVENT_PROV_CRED_FAIL:
      {
        Serial.println("\nProvisioning failed!\nPlease reset to factory and retry provisioning\n");
        if (sys_event->event_info.prov_fail_reason == WIFI_PROV_STA_AUTH_ERROR)
          Serial.println("\nWi-Fi AP password incorrect");
        else
          Serial.println("\nWi-Fi AP not found....Add API \" nvs_flash_erase() \" before beginProvision()");
        break;
      }
    case ARDUINO_EVENT_PROV_CRED_SUCCESS:
      {
        Serial.println("\nProvisioning Successful");
        // Save SSID and password to EEPROM
        saveStringToEEPROM(EEPROM_SSID_ADDR, wifi_ssid);
        saveStringToEEPROM(EEPROM_PASS_ADDR, wifi_password);

        // Print saved credentials
        String saved_ssid = readStringFromEEPROM(EEPROM_SSID_ADDR);
        String saved_password = readStringFromEEPROM(EEPROM_PASS_ADDR);

        Serial.println("\nSaved credentials to EEPROM:");
        Serial.print("SSID_in_eeprom: ");
        Serial.println(saved_ssid);
        Serial.print("Password_in_eeprom: ");
        Serial.println(saved_password);
        break;
      }
    case ARDUINO_EVENT_PROV_END:
      Serial.println("\nProvisioning Ends");
      break;
    default:
      break;
  }
}

// #############################