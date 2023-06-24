/*******************************************************

Arduino UNO R4 minima first project
Roni Bandini bandini.medium.com
June 2023 MIT License

Machine Learning Soccer forecast using FIFA datasets

Fifa Rank https://www.fifa.com/fifa-world-ranking/men?dateId=id13974
DFRobot LCD Keypad https://wiki.dfrobot.com/_SKU_DFR0374__SKU_DFR0936_LCD_Keypad_Shield_V2.0?tracking=61357a929f73e

********************************************************/

#include <LiquidCrystal.h>
#include <Soccer_forecast_with_Arduino_Uno_R4_inferencing.h>

float features[5];

int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, features + offset, length * sizeof(float));
    return 0;
}

void print_inference_result(ei_impulse_result_t result);

int idCountries[]   = {9, 42, 28, 215, 44, 77, 102, 136, 191, 214, 70};
String countries[]  = {"Argentine","Chile","Brazil", "USA","Colombia","Germany","Italy","Mexico","Spain","England","France"};
int fifaRank[]      = {1, 31, 3, 13, 17, 14, 8, 15, 10, 5, 2};
int arrayIndex=0;
int arrayMax=10;
int neutralLocation=0;

int indexCountry1=9999;
int indexCountry2=9999;

LiquidCrystal lcd(8, 9, 4, 5, 6, 7);        

#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

int lcd_key     = 0;
int adc_key_in  = 0;

unsigned long tepTimer ;

int read_LCD_buttons()     
{
 adc_key_in = analogRead(0);           
 if (adc_key_in > 1000) return btnNONE;
 if (adc_key_in < 50)   return btnRIGHT;
 if (adc_key_in < 250)  return btnUP;
 if (adc_key_in < 450)  return btnDOWN;
 if (adc_key_in < 650)  return btnLEFT;
 if (adc_key_in < 850)  return btnSELECT;
 return btnNONE;
}

void setup(){
    lcd.begin(16, 2);    
    lcd.setCursor(0, 0);                   
    lcd.print("Arduino Uno R4");
    lcd.setCursor(0, 1); 
    lcd.print("Soccer forecast");
    delay(2000);
    lcd.clear();

    lcd.setCursor(0, 0);                   
    lcd.print("Roni Bandini");
    lcd.setCursor(0, 1); 
    lcd.print("V1.0 6/2023");
    delay(2000);
    lcd.clear();
    lcd.setCursor(0, 0); 
    lcd.print("Select home team");    
    
}

void clearScreen(){
  lcd.clear();
}

void clearLine2(){  
  lcd.setCursor(0, 1); 
  lcd.print("                ");
}

void loop(){

 lcd_key = read_LCD_buttons();  
  
 switch (lcd_key)               
 {
   case btnRIGHT:
     {
     break;
     }
   case btnLEFT:
     {   
     break;
     }
     
   case btnUP:
     {
     
     arrayIndex++;
     
     if (arrayIndex>arrayMax){
      arrayIndex=0;
      }     
         
     clearLine2();
     lcd.setCursor(0, 1);       
     
     lcd.print(countries[arrayIndex] + " (" + String(fifaRank[arrayIndex])+")" );
     
     break;
     }
   case btnDOWN:
     {
      arrayIndex--;
     
     if (arrayIndex<0){
      arrayIndex=arrayMax;
      }
     
     clearLine2();
     lcd.setCursor(0, 1);              
     lcd.print(countries[arrayIndex] + " (" +String(fifaRank[arrayIndex])+")" );
     break;
     }
     
   case btnSELECT:
     {      
      
     if (indexCountry1==9999){
      
      // assign to team 1
      
      indexCountry1=arrayIndex;  
      clearScreen(); 

      lcd.setCursor(0, 0); 
      lcd.print("Selected team 1");          
      lcd.setCursor(0, 1); 
      lcd.print(countries[indexCountry1]);          
      delay(1500);
      
      clearScreen(); 
      lcd.setCursor(0, 0);       
      lcd.print("Select away team");          
     }
     
     else{      
     
      if (indexCountry1!=arrayIndex){

        // assign to team 2 and forecast
        
        indexCountry2=arrayIndex;  
        clearScreen();  

        // read neutral location
        
        lcd.setCursor(0, 0);  
        lcd.print("Neutral location?");
        lcd.setCursor(0, 1);  
        lcd.print("L: yes R: no");
        

        lcd_key = read_LCD_buttons();  
        while (lcd_key!=btnRIGHT and lcd_key!=btnLEFT){        
         delay(50);
         lcd_key = read_LCD_buttons();  
        }
        clearLine2();       
        lcd.setCursor(0, 1); 
        
        if (lcd_key==btnRIGHT){
          neutralLocation=0;                
          lcd.print("No");
          
          }
          else
        {
          neutralLocation=1;
          lcd.print("Yes");
         }
           
        delay(2000);
        
        clearScreen();  
        lcd.setCursor(0, 0);  
        lcd.print("Forecasting with");
        lcd.setCursor(0, 1);  
        lcd.print("Machine Learning");
        delay(2000);
        
        // call inference        
        features[0]=idCountries[indexCountry1]; // home team
        features[1]=idCountries[indexCountry2]; // away team
        features[2]=fifaRank[indexCountry1]; // home rank
        features[3]=fifaRank[indexCountry2]; // away rank
        features[4]=neutralLocation; // neutral location


           if (sizeof(features) / sizeof(float) != EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE) {
            ei_printf("The size of your 'features' array is not correct. Expected %lu items, but had %lu\n",
                EI_CLASSIFIER_DSP_INPUT_FRAME_SIZE, sizeof(features) / sizeof(float));
            delay(1000);
            return;
        }
    
        ei_impulse_result_t result = { 0 };
    
        // the features are stored into flash, and we don't want to load everything into RAM
        signal_t features_signal;
        features_signal.total_length = sizeof(features) / sizeof(features[0]);
        features_signal.get_data = &raw_feature_get_data;
    
        // invoke the impulse
        EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
        ei_printf("run_classifier returned: %d\n", res);
    
        if (res != 0) return;
    
        // print predictions
        ei_printf("Predictions ");
        ei_printf("(DSP: %d ms., Classification: %d ms., Anomaly: %d ms.)",
            result.timing.dsp, result.timing.classification, result.timing.anomaly);
        ei_printf(": \n");
        ei_printf("[");
        for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
            ei_printf("%.5f", result.classification[ix].value);
          #if EI_CLASSIFIER_HAS_ANOMALY == 1
                  ei_printf(", ");
          #else
                  if (ix != EI_CLASSIFIER_LABEL_COUNT - 1) {
                      ei_printf(", ");
                  }
          #endif
              }
          #if EI_CLASSIFIER_HAS_ANOMALY == 1
              ei_printf("%.3f", result.anomaly);
          #endif
              ei_printf("]\n");
    
          // human-readable predictions
          for (size_t ix = 0; ix < EI_CLASSIFIER_LABEL_COUNT; ix++) {
              ei_printf("    %s: %.5f\n", result.classification[ix].label, result.classification[ix].value);     
               
              
              if (result.classification[ix].label=="win"){
                  clearScreen(); 
                  lcd.setCursor(0, 0);  
                  lcd.print(countries[indexCountry1]+"-"+countries[indexCountry2]);                                     
                  lcd.setCursor(0, 1); 
                  lcd.print("Home win "+String(result.classification[ix].value*100)+"%");   
                  delay(5000);
                  
                  // reset selections
                  indexCountry1=9999;
                  indexCountry2=9999;
                  neutralLocation=0;
                  clearScreen();  
                  lcd.setCursor(0, 0); 
                  lcd.print("Select home team");  
                     
                }
            
          }
          
      }
      else
      // same team twice
      {
       clearLine2(); 
       lcd.setCursor(0, 1); 
       lcd.print("Error " +String(indexCountry1) +"-"+String(arrayIndex)); 
       indexCountry2=0;
      }
      
      }// second team               
    
     delay(1000);
         
  
     break;
     }
     case btnNONE:
     {

     break;
     }
 }       
 delay(100);
}

void print_inference_result(ei_impulse_result_t result) {

    // Print how long it took to perform inference
    ei_printf("Timing: DSP %d ms, inference %d ms, anomaly %d ms\r\n",
            result.timing.dsp,
            result.timing.classification,
            result.timing.anomaly);

    // Print the prediction results (object detection)
#if EI_CLASSIFIER_OBJECT_DETECTION == 1
    ei_printf("Object detection bounding boxes:\r\n");
    for (uint32_t i = 0; i < result.bounding_boxes_count; i++) {
        ei_impulse_result_bounding_box_t bb = result.bounding_boxes[i];
        if (bb.value == 0) {
            continue;
        }
        ei_printf("  %s (%f) [ x: %u, y: %u, width: %u, height: %u ]\r\n",
                bb.label,
                bb.value,
                bb.x,
                bb.y,
                bb.width,
                bb.height);
    }

    // Print the prediction results (classification)
#else
    ei_printf("Predictions:\r\n");
    for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
        ei_printf("  %s: ", ei_classifier_inferencing_categories[i]);
        ei_printf("%.5f\r\n", result.classification[i].value);
    }
#endif

    // Print anomaly result (if it exists)
#if EI_CLASSIFIER_HAS_ANOMALY == 1
    ei_printf("Anomaly prediction: %.3f\r\n", result.anomaly);
#endif

}
