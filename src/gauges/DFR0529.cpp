#include <Arduino.h>
#include "DFR0529.h"
#include "GaugeIcons.h"
#include "images.h"
#include "sensesp_app.h"
#include <ESP8266WiFi.h>
#include <math.h>
#include <cstring>
#include <signalk/signalk_emitter.h>

#include <RemoteDebug.h>

#ifndef PI
#define PI 3.1457
#endif

#define MAX_GAUGE_INPUTS 5

// adding gauge events
DFR0529::GaugeEvent::GaugeEvent() {
}

DFR0529::GaugeEvent::GaugeEvent(float Threshold, uint16_t alarm_Type, uint16_t notification_Type, uint16_t notification_Action, uint16_t Event_color) : Threshold{Threshold}, alarm_Type{alarm_Type}, notification_Type{notification_Type},notification_Action{notification_Action}, event_Color{event_Color} {
}

DFR0529::GaugeEvent::GaugeEvent(JsonObject& obj) : Threshold{obj["Threshold"]}, alarm_Type{obj["alarm_Type"]}, notification_Type{obj["notification_Type"]}, notification_Action{obj["notification_Action"]}, event_Color{obj["event_Color"]} {
}



DFR0529::ValueColor::ValueColor() {
}

DFR0529::ValueColor::ValueColor(float minVal, float maxVal, uint16_t color) : minVal{minVal}, maxVal{maxVal}, color{color} {
}

DFR0529::ValueColor::ValueColor(JsonObject& obj) : minVal{obj["minVal"]}, maxVal{obj["maxVal"]}, color{obj["color"]} {
}


DFR0529::DFR0529(DFRobot_Display* pDisplay,
                         double minVal, double maxVal,
                         String config_path) :
   NumericConsumer(),
   Configurable(config_path),
   display(*pDisplay), minVal{minVal}, maxVal{maxVal} {

   maxDisplayChannel = 0;
   currentDisplayChannel = 0;

   input = new double[MAX_GAUGE_INPUTS];
   valueSuffix = new char[MAX_GAUGE_INPUTS];
   std::memset(valueSuffix, ' ', MAX_GAUGE_INPUTS);
   precision = new uint8[MAX_GAUGE_INPUTS];
   std::memset(precision, 0, MAX_GAUGE_INPUTS);
   

   display.fillScreen(GaugeFaceplateBackGroundColor);
   display.setTextColor(DISPLAY_WHITE);
   display.setTextBackground(GaugeFaceplateBackGroundColor);
   display.setCursor(26, 58);
   display.drawBmp((uint8_t*)gImage_signalk_b_100_100, -50, -80, 100, 100,1,DISPLAY_BLUE);
   display.setTextSize(2);
   display.printf("SensESP");
   display.setCursor(25, 80);
   display.setTextSize(1);
   display.println("Connecting to");
   display.setCursor(20, 90);
   display.println("Signal K server");
   pGaugeIcon = (uint8_t*)image_data_icontemp;
 

}


void DFR0529::addValueRange(const ValueColor& newRange) {
   ValueColor* pRange = new ValueColor(newRange);
   valueColors.insert(*pRange);
}

void DFR0529::addGaugeEvent(const GaugeEvent& newEvent) {
   GaugeEvent* pGaugeEvent = new GaugeEvent(newEvent);
   gaugeEvents.insert(*pGaugeEvent);
  
}

/*
*  send an event when asked
*/
void DFR0529::SendGaugeEvent(String gauge_event){
   //debugI("Class %s SendGaugeEvent", className);
   if (gauge_event != last_notification){
      debugI("Gauge event is now %s",gauge_event.c_str());
      last_notification = gauge_event;

         String debuginfo="";

   DynamicJsonBuffer jsonBuffer;
   char json [200];

   JsonObject& root = jsonBuffer.createObject();
   JsonArray& values = root.createNestedArray("values");
  
   JsonObject& sk_path = jsonBuffer.createObject();
      sk_path["path"] = "notifications.electrical.gauge." + gauge_event;
         JsonObject& sk_value = jsonBuffer.createObject();
         sk_value["message"]= gauge_event + " reached ";
         //"state": "[normal|alert|warn|alarm|emergency]"
         sk_value["state"]="warn";
               JsonArray& method = jsonBuffer.createArray();
               method.add("visual");
               method.add("sound");
         sk_value["method"] = method;
      sk_path["value"] = sk_value;
  
   values.add(sk_path);
   

  


   root.printTo(json);
   debugI("json: %s",json);
   String teaseme(json);
   
   } 

   //String mydata =
   /*
         "values": [
        {
          "path": "notifications.mob",
          "value": {
            "message": "MOB",
            "state": "emergency",
            "method": ["visual", "sound"]
          }
        }
      ]
   */
   
   //sk_delta->append(mydata);

   //sk_delta->append(teaseme);
   //debugI(json);
   
   
   
   //String sk_path = sk_emitter->get_sk_path();
   
/*
   StaticJsonBuffer<200> jsonBuffer;
// create an object
   JsonObject& root = jsonBuffer.createObject();
  //sk_delta->append() 
   
   String debuginfo ="";
   root["values"]["path"]="notifications.mob";
   root["values"]["value"]["message"] = "LL on Gauge";
   root["values"]["value"]["start"] = "warn";
   //root["values"]["value"]["method"] = '["visual", "sound"]'; 

   root.printTo(debuginfo);
   debugI("json: %s",debuginfo.c_str());

   */
   //debugI("json %s SendGaugeEvent", sk_path.c_str());

   // message "any text"
   //"state": "[normal|alert|warn|alarm|emergency]"
   //JsonObject& object1 = jsonBuffer.createObject();

   /*
   *     "values": [
        {
          "path": "notifications.mob",
          "value": {
            "message": "MOB",
            "state": "emergency",
            "method": ["visual", "sound"]
          }
   */
  //this->notify();
   //pNotification->printTo(debuginfo);
   //debugI("json: %s",debuginfo.c_str());
  



}

/*
* decide on the current value
* if we are in range, LL,L,inRange,H,HH
* if we need to colour the needle

*/
uint16_t DFR0529::processGaugeEvents(float currentValue) {
   float old_limit = 0.0;
   
   std::set<GaugeEvent>::iterator it = gaugeEvents.begin();
   while (it != gaugeEvents.end()) {
      auto& event = *it;

      Current_Color = IR_color;
      if(event.alarm_Type == 3){
         HH_Threshold = event.Threshold;
        Current_Color = HH_color;
      }
      if(event.alarm_Type == 2){
         H_Threshold = event.Threshold;
         Current_Color = H_color;
      }
      if(event.alarm_Type == 1){
         L_Threshold = event.Threshold;
        // L_color = event.event_Color;
         Current_Color = L_color;
      }
      if(event.alarm_Type == 0){
         LL_Threshold = event.Threshold;
         Current_Color = LL_color;
      }

      String old_message;




      if (currentValue >= HH_Threshold && event.alarm_Type == 3) {
         // Here is our match

         if(displayEventsOn){
            display.setCursor(20,70);         
            display.print("Event:HH ");
            display.print(event.notification_Type + " ");
            display.print(event.Threshold);
            display.print(event.notification_Type);
         }
         this->SendGaugeEvent("HH");

        return Current_Color;
     }

      if (currentValue >= H_Threshold && currentValue <= HH_Threshold && event.alarm_Type == 2) {
         old_limit = event.Threshold;
         // Here is our match

         if(displayEventsOn){
            display.setCursor(20,70);         
            display.print("Event:H ");
            display.print(event.notification_Type + " ");
            display.print(event.Threshold);
         }
         //display.print(event.notification_Type);
         this->SendGaugeEvent("H");

         return Current_Color;
     }

      if (currentValue <= L_Threshold && currentValue >= LL_Threshold && event.alarm_Type == 1) {
         old_limit = event.Threshold;
         // Here is our match
         if(displayEventsOn){
            display.setCursor(20,70);
            display.print("Event:L ");
            display.print(event.notification_Type + " ");
            display.print(event.Threshold);
         }
         this->SendGaugeEvent("L");

        return Current_Color;
     }

      if (currentValue <= LL_Threshold && event.alarm_Type == 0) {
         old_limit = event.Threshold;
         // Here is our match
         if(displayEventsOn){
            display.setCursor(20,70);
            display.print("Event:LL ");
            display.print(event.notification_Type + " ");
            display.print(event.Threshold);
         }

         this->SendGaugeEvent("LL");

         return Current_Color;
     }

     




     it++;
  } // while
   if(displayEventsOn){
      display.setCursor(20,70);
      display.print("In Range:");
      //display.print(event.notification_Type + " ");
      //display.print(event.Threshold);
   }

   this->SendGaugeEvent("IR");

   return IR_color;
   //display.setCursor(30,30);
   //display.print("color");
   //display.print(needleColor);
}


uint16_t DFR0529::getValueColor(float value) {

  std::set<ValueColor>::iterator it = valueColors.begin();

  while (it != valueColors.end()) {
     auto& range = *it;

     if (value >= range.minVal && value <= range.maxVal) {
        // Here is our match
        return range.color;
     }

     it++;
  } // while

  return defaultValueColor;

}



void DFR0529::setValueSuffix(char suffix, int inputChannel) {
   if (inputChannel >= 0 && inputChannel < MAX_GAUGE_INPUTS) {

      if (inputChannel > maxDisplayChannel) {
         maxDisplayChannel = inputChannel;
      }

      valueSuffix[inputChannel] = suffix;
   }
   else {
      debugW("WARNING! calling DFR0529::setValueSuffix with an input channel out of range - ignoring.");
   }
}


void DFR0529::setPrecision(uint8 precision, int inputChannel) {
   if (inputChannel >= 0 && inputChannel < MAX_GAUGE_INPUTS) {

      if (inputChannel > maxDisplayChannel) {
         maxDisplayChannel = inputChannel;
      }

      this->precision[inputChannel] = precision;
   }
   else {
      debugW("WARNING! calling DFR0529::setPrecision with an input channel out of range - ignoring.");
   }
}

void DFR0529::setGaugeFaceplate(double gaugeStart, double gaugeRange){
   gauge_start = gaugeStart;
   gauge_range = gaugeRange;
   if (gauge_start <=134.999){
      gauge_start = 135;
      debugW("WARNING! calling DFR0529::setGaugeFaceplate with a value out of range - ignoring.");
   }
}

void DFR0529::setGaugeMinMaxLimits(double lowerLimit, double upperLimit){
   lower_limit = lowerLimit;
   upper_limit = upperLimit;

}

void DFR0529::setGaugeFaceplateBackgroundColor(uint16_t gaugeFaceplateBackGroundColor){
   GaugeFaceplateBackGroundColor = gaugeFaceplateBackGroundColor;
}

void DFR0529::AlarmColorToNeedle(bool alarm_color_on){
      alarmColorToNeedle = alarm_color_on;
   if (alarm_color_on == false){needleColor = defaultValueColor;}
}
void DFR0529::AlarmColorToNeedleCenter(bool alarm_color_on){
    alarmColorToNeedleCenter = alarm_color_on;
   if (alarm_color_on == false){needleCenterColor = defaultValueColor;}  
}
void DFR0529::AlarmColorToGaugeValue(bool alarm_color_on){
   alarmColorToGaugeValue = alarm_color_on;
   if (alarm_color_on == false){currentValueColor = defaultValueColor;}
}
void DFR0529::AlarmColorToGaugeEngUnits(bool alarm_color_on){
   alarmColorToGaugeEngUnits = alarm_color_on;
   if (alarm_color_on == false){engUnitsColor = DISPLAY_CYAN;}
   
}

void DFR0529::enable_hourcounter(bool enable){
   enableHourCounter = enable;   
}

void DFR0529::enable_events(bool enable){
   enableEvents = enable;   
}

void DFR0529::display_events(bool display_events_on){
   displayEventsOn = display_events_on;   
}


void DFR0529::setGaugeEngUnits(String engUnits){
   eng_units = engUnits;
}

void DFR0529::calcPoint(double pct, uint16_t radius, int16_t& x, int16_t& y) {
    double angle = PI * (1.0 - pct);
    x = radius * cos(angle);
    y = -(radius * sin(angle));
}

/*
* TODO: Obsolete functioin will be removed
*/
void DFR0529::drawHash(double pct, uint16_t startRadius, uint16_t endRadius, uint16_t color) {

  int16_t x0, y0, x1, y1;
  calcPoint(pct, startRadius, x0, y0);
  calcPoint(pct, endRadius, x1, y1);
  display.drawLine(x0, y0, x1, y1, color);
}

/*
* TODO: Obsolete functioin will be removed
*/
void DFR0529::drawGaugeRange(double minPct, double maxPct, double inc, uint16_t color) {
  for (double pct = minPct; pct <= maxPct; pct += inc) {
     drawHash(pct, 58, 70, color);
  }
}

/*
* TODO: Obsolete functioin will be removed
*/
void DFR0529::drawGaugeTick(double pct, uint16_t startRadius, uint16_t endRadius, uint16_t color) {
  double minVal = pct - 0.01;
  double maxVal = pct + 0.01;
  for (double val = minVal; val <= maxVal; val += 0.0033) {
     drawHash(val, startRadius, endRadius, color);
  }
}


/*
* draw needle adapted to new version
*/
void DFR0529::drawNeedle(double value, uint16_t color) {
   int16_t x0, y0, x1, y1;
   /*
   * old version
   * double pct = (value - minVal) / valueRange;
   * drawHash(pct, 5, 40, color);
   */
   double range = gauge_range/(upper_limit - (lower_limit));
   double current_value = gauge_start + (range * (value - lower_limit));

   x0 = round(5 * cos(radians(current_value)));
   y0 = round(5 * sin(radians(current_value)));

   x1 = round(40 * cos(radians(current_value)));
   y1 = round(40 * sin(radians(current_value)));

   display.drawLine(x0, y0, x1, y1, color);


}


void DFR0529::drawDisplay() {
   
   double ax,ay,bx,by,cx,cy;
   double currentangle = gauge_start;
   double gauge_w = display.getWidth();
   double gauge_end = gauge_start - (360 - gauge_range);
 
   display.fillScreen(GaugeFaceplateBackGroundColor);

   //TODO: Remove these lines
   //display.drawBmp((uint8_t*)gImage_fuel_56_56, -50, -50, 56, 56);
   //display.drawBmp((uint8_t*)gImage_preheat_28, -60, -30, 28, 28,1,DISPLAY_ORANGE);
   //display.drawBmp((uint8_t*)gImage_battery_28, -30, -30, 28, 28,1,DISPLAY_ORANGE);
   //display.drawBmp((uint8_t*)gImage_oil_28, 0, -30, 28, 28,1,DISPLAY_RED);
   //display.drawBmp((uint8_t*)gImage_fuel_28, 30, -30, 28, 28,1,DISPLAY_ORANGE);
   total_div = LL_div + L_div + IR_div + H_div + HH_div;
   div_angle = (gauge_range  / total_div); 
   gauge_angle = (gauge_range - (total_div-2 * gauge_div))/total_div;

   //set de backgound of the faceplate
   display.fillCircle(0,0,gauge_w/2,GaugeFaceplateBackGroundColor);
   //set the backgound of the text
   display.setTextBackground(GaugeFaceplateBackGroundColor);
   
   display.setCursor(64,10);
   //display.print(total_div);
   display.setCursor(64,20);
   //display.print(div_angle);
   display.setCursor(64,30);
   //display.print(gauge_angle);
   display.setCursor(64,40);
   //display.print(currentangle);
   display.setCursor(64,50);
   //display.print(gauge_end);
   display.setCursor(64,60);
   //display.print(gauge_w);


   //calulate side b
   ax = 0;
   ay = 0;
   //calculate coordinates
   // b coordinates
   bx = gauge_w * cos(radians(gauge_start));
   by = gauge_w * sin(radians(gauge_start));
   //c coordinates
      //div_angle = 2.0;
   cx = gauge_w * cos(radians(gauge_end));
   cy = gauge_w * sin(radians(gauge_end));
   

   
   if(LL_div > 0){
      // Calculate the LL
      bx = gauge_w * cos(radians(gauge_start));
      by = gauge_w * sin(radians(gauge_start));
      // recalculate c point
      currentangle = currentangle + (LL_div * gauge_angle);
      cx = gauge_w  * cos(radians(currentangle));
      cy = gauge_w  * sin(radians(currentangle));
      display.fillTriangle(ax,ay,bx,by,cx,cy,LL_color);
   }

   if(L_div > 0){
      // Calculate the L
      bx = cx;
      by = cy;
      // recalculate c point
      currentangle = currentangle + (L_div * gauge_angle);
      cx = gauge_w  * cos(radians(currentangle));
      cy = gauge_w  * sin(radians(currentangle));
      display.fillTriangle(ax,ay,bx,by,cx,cy,L_color);
   }


      if(IR_div > 0){
      // Calculate the InRange point
      bx = cx;
      by = cy;
      // recalculate c point
      currentangle = currentangle + (IR_div * gauge_angle);
      cx = gauge_w  * cos(radians(currentangle));
      cy = gauge_w  * sin(radians(currentangle));
      display.fillTriangle(ax,ay,bx,by,cx,cy,IR_color);
      }

   if(H_div > 0){
      // Calculate the H
      bx = cx;
      by = cy;
      // recalculate c point
      currentangle = currentangle + (H_div * gauge_angle);
      cx = gauge_w  * cos(radians(currentangle));
      cy = gauge_w  * sin(radians(currentangle));
      display.fillTriangle(ax,ay,bx,by,cx,cy,H_color);
   }

   if(HH_div > 0){
      // Calculate the HH
      bx = cx;
      by = cy;
      // recalculate c point
      currentangle = currentangle + (HH_div * gauge_angle);
      cx = gauge_w  * cos(radians(currentangle));
      cy = gauge_w  * sin(radians(currentangle));
      display.fillTriangle(ax,ay,bx,by,cx,cy,HH_color);
   }

   gauge_blindspot = 360 - gauge_range;
   
   if(gauge_blindspot >= 180){

      //calulate side b
      ax = 0;
      ay = 0;
      //calculate coordinates
      // b coordinates
      bx = gauge_w * cos(radians(90));
      by = gauge_w * sin(radians(90));
      //c coordinates
         //div_angle = 2.0;
      //cx = gauge_w * cos(radians(gauge_end));
      //cy = gauge_w * sin(radians(gauge_end));
      display.fillTriangle(ax,ay,bx,by,cx,cy,GaugeFaceplateBackGroundColor);

      cx = bx;
      cy = by;

      gauge_blindspot = gauge_blindspot - (90 - currentangle);

   }

   if(gauge_blindspot >= 90){

      //calulate side b
      ax = 0;
      ay = 0;
      //calculate coordinates
      // b coordinates
      bx = gauge_w * cos(radians(gauge_start));
      by = gauge_w * sin(radians(gauge_start));
      //c coordinates
      //div_angle = 2.0;
      //cx = gauge_w * cos(radians(gauge_end));
      //cy = gauge_w * sin(radians(gauge_end));
      display.fillTriangle(ax,ay,bx,by,cx,cy,GaugeFaceplateBackGroundColor);

   }





   //draw gauge lines starting from gauge_start
   display.setLineWidth(2);
   currentangle = gauge_start;
   bx = gauge_w * cos(radians(currentangle));
   by = gauge_w * sin(radians(currentangle));

   display.drawLine(ax, ay ,bx,by ,div_color);

   //display.drawLine(cx*7/8,cy*7/8,cx,cy,div_color);

   for(int x= 0; x < total_div; x++){
      currentangle = currentangle + gauge_angle;
      bx = gauge_w * cos(radians(currentangle));
      by = gauge_w * sin(radians(currentangle));
      display.drawLine(ax, ay ,bx ,by ,div_color);
   }
   // add a gauge plate
   display.fillCircle(0,0,50,GaugeFaceplateBackGroundColor);





  // Do some pre-calc
  valueRange = maxVal - minVal;

  // Draw gauge ranges
  //drawGaugeTick(0.01, 45, 57, DISPLAY_WHITE);
  //drawGaugeTick(0.25, 50, 57, DISPLAY_WHITE);
  //drawGaugeTick(0.5, 45, 57, DISPLAY_WHITE);
  //drawGaugeTick(0.75, 50, 57, DISPLAY_WHITE);
  //drawGaugeTick(1.0, 45, 57, DISPLAY_WHITE);


  // Draw color ranges (if any)...
  std::set<ValueColor>::iterator it = valueColors.begin();
  while (it != valueColors.end()) {
     auto& range = *it;
     double minPct = (range.minVal - minVal) / valueRange;
     double maxPct = (range.maxVal - minVal) / valueRange;
     //drawGaugeRange(minPct, maxPct, 0.002, range.color);
     it++;
  } // while


  display.fillCircle(0, 0, 3, needleCenterColor);
  display.setLineWidth(1);
  //draw alignment crosshair
  //display.drawLine(0,-64,0,64,DISPLAY_WHITE);
  //display.drawLine(-64,0,64,0,DISPLAY_WHITE);

   //display.drawBmp((uint8_t*)gImage_fuel_56_56, -50, -50, 56, 56);
   //display.drawBmp((uint8_t*)gImage_preheat_28, -42, -28, 28, 28,1,DISPLAY_ORANGE);
   //display.drawBmp((uint8_t*)gImage_battery_28, -13, -28, 28, 28,1,DISPLAY_ORANGE);
   //display.drawBmp((uint8_t*)gImage_oil_28, 17, -28, 28, 28,1,DISPLAY_RED);
   //display.drawBmp((uint8_t*)gImage_fuel_28, 30, -30, 28, 28,1,DISPLAY_ORANGE);


  

}



void DFR0529::updateWifiStatus() {

   bool wifiConnected = sensesp_app->isWifiConnected();
   bool sigkConnected = sensesp_app->isSignalKConnected();

   int blinkRate = 2;

   bool blinkDisplay =  (blinkCount % blinkRate == 0);

   int newIcon;

   if (wifiConnected && sigkConnected) {
      newIcon = 2;
   }
   else if (blinkDisplay) {
     newIcon = (displayIcon+1) % 4;
     if (!wifiConnected) {
        if (newIcon == 2) {
           newIcon = 0;
        }
     } 
   }
   else {
     newIcon = displayIcon;
   }

   switch (newIcon) {
      case 0:
         if (displayIcon != 0) {
            //display.drawBmp((uint8_t*)image_data_iconwifi, -12, 35, 23, 20); 
            displayIcon = 0;
         }
         break;

      case 1:
      case 3:
         if (displayIcon != 1 && displayIcon != 3) {
            //display.fillRect(-14, 35, 28, 26, DISPLAY_BLACK);
            displayIcon = newIcon;
         }
         break;

      case 2:
         if (displayIcon != 2) {
            //temporary disabeld
            //display.drawBmp(pGaugeIcon, -14, 35, 28, 26);   
            displayIcon = 2;
         }
         break;

   } // switch

   blinkCount = (blinkCount + 1) % blinkRate;
}

void DFR0529::set_simulation(bool SimulationOn){
   SimulationActivated = SimulationOn;
}

void DFR0529::updateGauge() {

   float newDialValue = input[0];

   if (newDialValue > maxVal) {
      newDialValue = maxVal;
   }
   else if (newDialValue < minVal) {
      newDialValue = minVal;
   }

  
   uint16_t gaugeColor = processGaugeEvents(newDialValue);
   

   // Update the dial...
   if (newDialValue != lastDialValue) {
      drawNeedle(lastDialValue, GaugeFaceplateBackGroundColor);
      lastDialValue = newDialValue;

      if(gaugeColor == GaugeFaceplateBackGroundColor){
         
         gaugeColor = DISPLAY_WHITE - gaugeColor;
      }

      if(alarmColorToNeedle){
         drawNeedle(newDialValue, gaugeColor);
      }else{
         drawNeedle(newDialValue, DISPLAY_CYAN);
      }

      if(alarmColorToNeedleCenter){
        display.fillCircle(0, 0, 3, gaugeColor);
      }else{
         display.fillCircle(0, 0, 3, DISPLAY_CYAN);
      }

      

      

   }


   bool wifiConnected = sensesp_app->isWifiConnected();
   if (!wifiConnected || currentDisplayChannel > maxDisplayChannel) {
      if (!ipDisplayed) {
         // Display the device's IP address...
         display.fillRect(-64, 6, 128, 25, DISPLAY_BLACK);
         display.setTextColor(gaugeColor);
         display.setTextBackground(DISPLAY_BLACK);
         display.setCursor(40, 80);
         display.setTextSize(1);
         display.printf("%s", WiFi.localIP().toString().c_str());
         ipDisplayed = true;
      }
   }
   else {
      if (ipDisplayed) {
         // Clear out the IP address
         display.fillRect(-64, 6, 128, 25, DISPLAY_BLACK);
         ipDisplayed = false;
      }

      // Update the digital display...
      float newDisplayValue = input[currentDisplayChannel];
      if (newDisplayValue != lastDisplayValue) {

         display.setTextColor(gaugeColor);
         display.setTextBackground(DISPLAY_BLACK);
         display.setCursor(40, 78);
         display.setTextSize(2);
         char fmtStr[10];
         uint8 prec = precision[currentDisplayChannel];
         if (prec > 0) {
            int wholeSize = 6 - prec;
            sprintf(fmtStr, "%%%d.%df%%c" , wholeSize, (int)prec);
         }
         else {
            strcpy(fmtStr, "%5.0f%c ");
         }
         //temporary disabeld
         //display.printf(fmtStr, newDisplayValue, valueSuffix[currentDisplayChannel]);

      }
   }     // add dummy text for alignment
   //display.setTextColor(inRange_color);
  // display.setCursor(42,80);
  // display.setTextSize(2);
   //display.print("9999");
   // add the engineering units
  // display.setCursor(49,100);
   //display.setTextSize(1);
  // display.print(" " + eng_units);
   // add placeholder hour counter
   //display.setCursor(40,114);
   //display.setTextSize(1);
   //display.print("00:00:00");

}


void DFR0529::enable() {
    load_configuration();
    drawDisplay();
    //app.onRepeat(500, [this]() { this->updateWifiStatus(); });
   if(!SimulationActivated){
      app.onRepeat(750, [this]() { this->updateGauge(); });
   } else {
      app.onRepeat(50, [this]() { 
         Simulation_x ++;
         //== generatingh the next sine value ==//
         SimulationSinVal = (sin(radians(Simulation_x)));
         if (Simulation_x == 180){
            Simulation_x = 0;}
         display.setTextBackground(GaugeFaceplateBackGroundColor);
         display.setTextSize(1);
         display.setTextColor(DISPLAY_WHITE);
        // display.setCursor(15,70);
        // display.print("Sin Value: ");
         //display.println(SimulationSinVal);
         //display.print(SimulationSinVal,4);
         //display.printf("%s", WiFi.localIP().toString().c_str());
         this->SimulationActive(); 
         });
   }
}

void DFR0529::SimulationActive(){
   // do a bit the same as the original code
   //display.setCursor(15,74);
   //display.print("Simulation Active");
   //display.setCursor(7,94);
   //display.print("-: ");
   //display.print(minVal);
   //display.print(" +: ");
   //display.print(maxVal);
   //display.setCursor(15,104);
   //display.print("Current : ");
   //display.print(minVal+(SimulationSinVal * (maxVal - minVal)));
   float actual_value = minVal+(SimulationSinVal * (maxVal - minVal));
   input[0] = minVal+(SimulationSinVal * (maxVal - minVal));

   //uint16_t gaugeColor = getValueColor(actual_value);
   uint16_t gaugeColor = processGaugeEvents(actual_value);
   
   display.setTextBackground(GaugeFaceplateBackGroundColor);
   display.setTextColor(defaultValueColor);
   display.setCursor(42,80);
   display.setTextSize(2);

   if(gaugeColor == GaugeFaceplateBackGroundColor){
         
         gaugeColor = DISPLAY_WHITE - gaugeColor;
      }

   if(alarmColorToGaugeValue){
     display.setTextColor(gaugeColor);
     display.print(actual_value,1);;
   } else {
     display.setTextColor(DISPLAY_CYAN); 
     display.print(actual_value,1);
   }
   display.setTextColor(defaultValueColor);

   
   display.setCursor(49,100);
   display.setTextSize(1);
   //TODO: fix copying ° and center eng_units
   if(alarmColorToGaugeEngUnits){
      display.setTextColor(gaugeColor);
      display.print("" + eng_units);
   } else {
      display.setTextColor(DISPLAY_CYAN);
      display.print("" + eng_units);
   }
   display.setTextColor(defaultValueColor);

  
   // add placeholder hour counter
   //display.setTextColor(gaugeColor);

   if(enableHourCounter){
      display.setCursor(40,114);
      display.setTextSize(1);
      display.print("00:00:00");
   }
   updateGauge();
   


}


void DFR0529::set_input(float newValue, uint8_t idx) {

   if (idx >= 0 && idx < MAX_GAUGE_INPUTS) {

      input[idx] = newValue;
   }
}


void DFR0529::set_input(bool buttonPressed, uint8_t idx) {

   if (buttonPressed) {
       if (currentDisplayChannel > maxDisplayChannel) {
         currentDisplayChannel = 0;
       }
       else {
         currentDisplayChannel++;
       }
      updateGauge();
   }
}


JsonObject& DFR0529::get_configuration(JsonObject& root) {
  root["default_display"] = currentDisplayChannel;
  root["minVal"] = minVal;
  root["maxVal"] = maxVal;

  JsonArray& jRanges = root.createNestedArray("ranges");
  for (auto& range : valueColors) {
    JsonObject& entry = buf.createObject();
    entry["minVal"] = range.minVal;
    entry["maxVal"] = range.maxVal;
    entry["color"] = range.color;
    jRanges.add(entry);
  }

  return root;
}


bool DFR0529::set_configuration(const JsonObject& config) {

  String expected[] = { "default_display", "minVal", "maxVal" };
  for (auto str : expected) {
    if (!config.containsKey(str)) {
      debugE("Can not set DFR0529 configuration: missing json field %s\n", str.c_str());
      return false;
    }
  }

  currentDisplayChannel = config["default_display"];
  minVal = config["minVal"];
  maxVal = config["maxVal"];

  JsonArray& arr = config["ranges"];
  if (arr.size() > 0) {
    valueColors.clear();
    for (auto& jentry : arr) {
        ValueColor range(jentry.as<JsonObject>());
        valueColors.insert(range);
    }

  }

  return true;

}


static const char SCHEMA[] PROGMEM = R"({
   "type": "object",
   "properties": {
      "minVal": { "title": "Minimum analog value", "type": "number"},
      "maxVal": { "title": "Maximum analog value", "type": "number"},
      "default_display": { "title": "Default input to display", "type": "number", "minimum": 0, "maximum": 5 },
      "ranges": { "title": "Ranges and colors",
                     "type": "array",
                     "items": { "title": "Range",
                                 "type": "object",
                                 "properties": {
                                       "minVal": { "title": "Min value for color", "type": "number" },
                                       "maxVal": { "title": "Max value for color", "type": "number" },
                                       "color": { "title": "Display color RGB", "type": "number" }
                                 }}}

      }
   })";


String DFR0529::get_config_schema() {
   return FPSTR(SCHEMA);
}
