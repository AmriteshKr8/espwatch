# Note:
 - Use 24-30AWG wire for display power lines
 - Only use signal wires for display signals
 - Install all libraries from arduino library manager
 - Move the "User_setup.h" to "arduino sketch folder"/libraries/TFT_eSPI/User_setup.h
 - Edit lines:
     - 185: const char* ap_ssid = "ESPWATCH"; // add your watch hotspot ssid
     - 186: const char* ap_password = "<espwatch-pass>"; // add your hotspot password
     - 249: ESP32Time rtc(19800); // set utc offset in seconds
 - First Setup:
     - Hold the up button
     - Navigate to wifi > host_cp
     - Connect to the hotspot ssid you entered earlier using the password you created
     - Click "Create new file" and create the following files with the given contents, mind the case sensitivity:
          - wpa_creds.txt
            
                <SSID1>:<PASSWORD>
                <SSID2>:<PASSWORD>
  
            Note: You can add a max of 5 different ssids
       
          - battery_stats:
     
                2000
       
            Note: you can enter any number between 1000-2000, it'll be overwritten by the system soon
            
          - weather_cache.txt:
            Note: leave this file blank
            
     - Press Sel on the watch to shut down server
     - Select exit twice to shut the watch down
     - Hold the Sel on the watch, select a network and sync ( it'll take a few tries )
     - Wait for the watch to go to sleep
     - Battery calibration:
       - Charge the battery fully
       - Hold the Sel and down button to boot the watch
       - The temperature reading is replaced by the current battery reading
       - The reading is added to the "battery_status" file automatically
       - Its recommended to calibrate the battery once every few days or optimally after each charge.
- Note:
  - the battery life on this watch is not very good.
  - its very hard to assemble.
  - the watch needs to be synced every time it loses power.
# 3D Files:
- body: https://www.printables.com/model/1381299-espwatch
- Strap: https://www.printables.com/model/1030042-24mm-universal-linked-watch-band

## You Need:

- ### Electronics:
  - Display: https://robu.in/product/goldenmorning-full-color-st7789v-12p-240x280-spi-1-69-inch-tft-lcd-display/
  - SeeedStudio Xiao c3: https://robu.in/product/seeed-studio-xiao-esp32c3-tiny-mcu-board-with-wi-fi-and-ble-battery-charge-supported-power-efficiency-and-rich-interface/
  - Backlight Mosfet: https://robu.in/product/bcp56-hxy-mosfet-80v-1-5w-250150ma2v-1a-npn-sot-223-bipolar-bjt-rohs/
  - Battery: https://robu.in/product/wly402020-120mah-3-7v-single-cell-rechargeable-lipo-battery/
  - Buttons: https://robu.in/product/tact-switch-tvbp06-6-0x3-5mm/
  - Signal wires: https://robu.in/product/0-1mm-copper-soldering-solder-ppa-enamelled-repair-reel-wire/
  - resistors:
      - R1 - 2.2k resistor 1/4w
      - R2/R3 - 10k 1/4w

- ### Strap:
  - Watch Band End Link.stl - 1
  - Watch Band End Link (reversed).stl - 1
  - Watch Band Regular Link.stl - varies depending on wrist size
  - Watch Band Pin Part A.stl - as needed
  - Watch Band Pin Part B.stl - as needed
    
- ### Body:
  - case - 1
  - displaycase - 1
  - xiao-mount - 1
  - backplate - 1
  - button-unmarked - 3
  - cage - 1

# Schematic:
<img width="396" height="349" alt="image" src="https://github.com/user-attachments/assets/655c699a-2bfa-44a9-8700-86a684ac3654" />

https://easyeda.com/editor#project_id=a65a7b23be7d4b2380da87abd1f7d38c
