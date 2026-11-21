void saveDataCloud() {
  Serial.println("Sending data to cloud...");

  String jsonData = "{";
  jsonData += "\"hashKey\":\"" + hashKey + "\",";
  jsonData += "\"deviceMac\":\"" + deviceMac + "\",";
  jsonData += "\"qrCode\":\"" + qrCode + "\",";
  jsonData += "\"visualInspection\":" + String(visualInspectionState ? "true" : "false") + ",";
  jsonData += "\"drawCurrent\":" + String(finalCurrent, 2) + ",";
  jsonData += "\"drawCurrentState\":" + String(drawCurrentState ? "true" : "false") + ",";
  jsonData += "\"fanSpeed\":" + String(finalRPM, 1) + ",";
  jsonData += "\"fanSpeedState\":" + String(fanSpeedState ? "true" : "false") + ",";
  jsonData += "\"overallFanState\":" + String(overallFanState ? "true" : "false");
  jsonData += "}";

  Serial.println("Payload:");
  Serial.println(jsonData);

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://bt.wavetec-qc.com/biotab_e0652_api/v1/test/add/fan-test");
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsonData);

    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    Serial.println("Response:");
    Serial.println(http.getString());

    if (httpResponseCode == 200) {
      Serial.println("Data uploaded successfully.");
    } else {
      Serial.println("Error uploading data.");
    }

    http.end();
  } else {
    Serial.println("WiFi not connected. Cannot send data.");
  }
}
