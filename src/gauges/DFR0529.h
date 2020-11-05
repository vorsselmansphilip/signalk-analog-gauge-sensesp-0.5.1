#ifndef _dfr0529_H_
#define _dfr0529_H_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <set>

#include <DFRobot_Display.h>

#include <functional>
#include <stdint.h>
#include <system/valueconsumer.h>
#include <system/configurable.h>
#include <system/enable.h>
#include <system/valueproducer.h>
#include <signalk/signalk_delta.h>
#include <signalk/signalk_emitter.h>
#include "sensesp.h"
#include <signalk/signalk_output.h>

//const uint16_t DFR0529_LL = 0;
//const uint16_t DFR0529_L = 1;
//const uint16_t DFR0529_H = 2;
//const uint16_t DFR0529_HH = 3;
//
//const uint16_t DFR0529_alarm = 10;
///const uint16_t DFR0529_event = 11;

//const uint16_t DFR0529_none = 20;
//const uint16_t DFR0529_buzzer = 21;
//const uint16_t DFR0529_delta = 22;
//const uint16_t DFR0529_buzzer_delta = 23;
enum alarm_type {L,LL,H,HH};
enum notification_type {event,alarm};
//enum event_icons{signalk,oil,fuel,temperature,preheat,battery};



/**
 * https://wiki.dfrobot.com/2.2_inches_TFT_LCD_Display_V1.0_(SPI_Interface)_SKU_DFR0529
 * An analog gauge that displays on an 2.2 inch round LCD display.  The gauge supports up to five inputs
 * with each input having a specialized display suffix.
 *   If a BooleanProducer is attached to the gauge, the gauge can scroll thru 
 * five inputs on the digital display.  The analog dial will always display 
 * the incomming value of input 0.
 * 
 */
class DFR0529 : public NumericConsumer, public BooleanConsumer,
                    public Configurable, public Enable {

    public:
        class ValueColor {
            public:
               float minVal;
               float maxVal;
               uint16_t color;

               ValueColor();
               ValueColor(float minVal, float maxVal, uint16_t color);
               ValueColor(JsonObject& obj);

               friend bool operator<(const ValueColor& lhs, const ValueColor& rhs) { return lhs.minVal < rhs.minVal; }               
        };

        class GaugeEvent {
            public:
                float Threshold;
                uint16_t alarm_Type;
                uint16_t event_Color;
                uint16_t notification_Type;
                uint16_t notification_Action;
                

                GaugeEvent();
                /*
                * float: Threshold
                * AlarmType: LL = 0,L = 1,H =2,HH=3
                * notification_type: event = 0, alarm = 1                
                * notification_action: none = 0,buzzer = 1, notification = 2
                * EventColor: DISPLAYRED
                * */
                GaugeEvent(float Threshold, uint16_t alarm_Type, uint16_t notification_Type, uint16_t notification_Action, uint16_t event_Color);
                GaugeEvent(float Threshold, uint16_t alarm_Type, uint16_t notification_Type, uint16_t notification_Action, uint16_t event_Color, String event_icon);  
                //GaugeEvent(float Threshold, alarm_type Alarm_type, uint16_t Event_color, notification_type Notification_type);              
                //GaugeEvent(float Threshold, uint16_t Alarm_type,uint16_t Event_color, uint16_t Notification_type ,uint16_t Notification_action);
                GaugeEvent(JsonObject& obj);

                friend bool operator<(const GaugeEvent& lhs, const GaugeEvent& rhs) { return lhs.Threshold < rhs.Threshold; }

                

        };


        DFR0529(DFRobot_Display* pDisplay,
                    double minVal = 0.0, double maxVal = 100.0,
                    String config_path = "");

        virtual void enable() override;

        /**
         * Handles input of new values to display in digital portion.
         */
        virtual void set_input(float newValue, uint8_t inputChannel = 0) override;

        /**
         * Handles button presses
         */
        virtual void set_input(bool newValue, uint8_t inputChannel = 0) override;

        /*
        * Activates a simulation 
        */

        
        virtual void set_simulation(bool SimulationOn = false);
       

        void setGaugeIcon(uint8_t* pNewIcon) { pGaugeIcon = pNewIcon; } 

        void setValueSuffix(char suffix, int inputChannel = 0);

        void setPrecision(uint8 precision, int inputChannel = 0);

        /*
        * set GaugeFaceplate the angele to start (clockwise) and the gauge_range starting from gauge_start
        * in degrees 0 -> 360 starting right clockwise
        */
        void setGaugeFaceplate(double gauge_start, double gauge_range);

        

        /*
        * set Gauge upper and lower limits 
        * in double according to the signal specifications
        */
        void setGaugeMinMaxLimits(double lower_limit, double upper_limit);

        /*
        * set Gauge engineering units
        * example bar, psi, K
        * warning: ° is not allowed
        */
        void setGaugeEngUnits(String engUnits);

        /*
        * set Gauge Faceplate BackGround Color
        * example: DISPLAY_BLACK or  0xF81F
        * TODO: recheck if a color other then black is used
        */

        void setGaugeFaceplateBackgroundColor( uint16_t gaugeFaceplateBackGroundColor);

        /*
        * set Gauge Faceplate BackGround Color
        * example: DISPLAY_BLACK or  0xF81F
        * TODO: recheck if a color other then black is used
        */
        virtual void AlarmColorToGaugeValue( bool alarm_color_on = false);
        virtual void AlarmColorToNeedle( bool alarm_color_on = false);
        virtual void AlarmColorToNeedleCenter( bool alarm_color_on = false);
        virtual void AlarmColorToGaugeEngUnits( bool alarm_color_on = false);


        void setDefaultDisplayIndex(int inputChannel) { currentDisplayChannel = inputChannel; }


        /**
         * Value ranges allow for setting gauge colors like "normal operating range"
         * and "red line" values.
         */
        void addValueRange(const ValueColor& newRange);

        /**
         * GaugeEvent allows for setting gauge colors like "normal operating range"
         * and "red line" values.
         */
        void addGaugeEvent(const GaugeEvent& newEvent);

        /**
         * Returns the color associated with the specified value. If no specific color
         * range can be found, the default color is returned.
         * @see addValueRange()
         * @see setDefaultValueColor
         */
        uint16_t getValueColor(float value);
        uint16_t processGaugeEvents(float value);

        void enable_hourcounter(bool enable);
        void enable_events(bool enable);
        /**
         * Displays the events and thresholds on screen when they trigger
         * is by default off
         */
        void display_events(bool display_events_on);


        /**
         * Sets the default color for the display when a value range is requested an no specified
         * value range color can be found.
         */
        void setDefaultValueColor(uint16_t newDefaultColor) { defaultValueColor = newDefaultColor; }


        uint16_t getDefaultValueColor() { return defaultValueColor; }


        // For reading and writing the configuration
        virtual void get_configuration(JsonObject& doc) override;
        virtual bool set_configuration(const JsonObject& config) override;
        virtual String get_config_schema() override;
        
    private:
        // values needed for the calculation
        // TODO: move precision to here uint8* precision;
        SKDelta* sk_delta;
        SKEmitter* sk_emitter;

        // values to needed to calculate gauge ring
        double gauge_range = 180.0;
        double gauge_start = 150.0;
        // the limits and engineering unit of the signal
        double lower_limit = 327.594;
        double upper_limit = 377.594;
        String eng_units = "K";

        //Gauge Faceplate background color
        uint16_t GaugeFaceplateBackGroundColor = DISPLAY_BLACK;

        uint16_t inRange_color = DISPLAY_GREEN;
        uint16_t div_color = DISPLAY_WHITE;

        double LL_Threshold = 1000.0;
        double L_Threshold  = 2000.0;
        double H_Threshold  = 6000.0;
        double HH_Threshold = 7000.0;
        //colors for the limits
        uint16_t defaultValueColor = DISPLAY_WHITE;
        uint16_t LL_color = DISPLAY_BLACK;
        uint16_t L_color = DISPLAY_PINK;
        uint16_t IR_color = DISPLAY_GREEN;
        uint16_t H_color = DISPLAY_YELLOW;
        uint16_t HH_color = DISPLAY_RED;

        uint16_t Current_Color = defaultValueColor;
        bool alarmColorToNeedle = false;
        bool alarmColorToNeedleCenter = false;
        bool alarmColorToGaugeValue = false;
        bool alarmColorToGaugeEngUnits = false;
        uint16_t needleColor = defaultValueColor;
        uint16_t needleCenterColor = defaultValueColor;
        uint16_t currentValueColor = defaultValueColor;
        uint16_t engUnitsColor = defaultValueColor;

        // placeholders for the divisions
        int LL_div = 3;
        int L_div = 0;
        int IR_div = 1;
        int H_div = 1;
        int HH_div = 1;

        // don't touch my privates

        double gauge_div = 4;
        double gauge_needle_last_x,gauge_needle_last_y;
        int total_div; 
        double div_angle;
        double gauge_angle;
        double gauge_blindspot;

        JsonObject* pNotification;
        String last_notification="";
        

        bool SimulationActivated;
        float SimulationSinVal;                                                   
        int Simulation_x = 0;

        bool enableHourCounter = false;
        bool enableEvents = true;
        bool displayEventsOn = false;



        double minVal;
        double maxVal;
        double valueRange;

        double lastDialValue = -99.9;
        double lastDisplayValue = -99.9;
        double* input;
        char* valueSuffix;
        uint8* precision;
        int currentDisplayChannel;
        int maxDisplayChannel;
        bool ipDisplayed = false;

       

        int blinkCount = 0;
        int displayIcon = -1;
        uint8_t* pGaugeIcon;

        DFRobot_Display& display;

        void drawDisplay();
        void updateGauge();
        void updateWifiStatus();
        void SimulationActive();
        void SendGaugeEvent(String gauge_event);

        void calcPoint(double pct, uint16_t radius, int16_t& x, int16_t& y);
        void drawHash(double pct, uint16_t startRadius, uint16_t endRadius, uint16_t color);
        void drawGaugeRange(double minPct, double maxPct, double inc, uint16_t color);
        void drawGaugeTick(double pct, uint16_t startRadius, uint16_t endRadius, uint16_t color);
        void drawNeedle(double value, uint16_t color);

        std::set<ValueColor> valueColors;
        std::set<GaugeEvent> gaugeEvents;
};

#endif