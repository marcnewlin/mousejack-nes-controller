# MouseJack NES Controller Build Guide

## Part 1: Initial case modifications

### Working with the NES Controllers

- The plastic is soft, and easily gets white stress marks. Use care when making the modifications to the case.
- The decal on the front of the controller is very sensitive to heat. Too much heat from the hot glue gun, heat gun, or soldering iron will cause the decal to lose its texture (which ends up looking really bad).
- Exacto knives and/or a dremel work well for case modifications.

### 1. Disassemble the controller

- Remove the six screws from the back of the case
- Open the case, and remove the screw holding in the PCB
- Take everything out of the case

![Part 1, Step 1](part-1-step-1.jpg?raw=true)

### 2. Make initial plastic modifications to the front of the case

There are a number of struts and support elements that get in the way of our components, so we need to remove them.

![Part 1, Step 2a](part-1-step-2a.jpg?raw=true)

A) Remove strut adjacent to the 'A' button

B) Remove top-center strut

C) Remove the plastic around the cable hole so that it is flush with the rest of the case

D) Remove the two struts closest to the cable hole

E) Cut off the top off of the top-right d-pad strut so that it is level with the adjacent strut

F) Remove the other top support element

![Part 1, Step 2b](part-1-step-2b.jpg?raw=true)

### 3. Replace plastic screw posts with metal screw posts

The plastic screw posts strip extremely easily, so it is necessary to replace them with fittings for machine screws.

<span style="color: red;">RED - 5.5mm tall</span>

<span style="color: blue;">BLUE - 7mm tall
</span>

- Use superglue to attach the metal screw terminals.
- Metal screw terminals are 3.5mm and 5.5mm tall

![Part 1, Step 3a](part-1-step-3a.jpg?raw=true)

![Part 1, Step 3b](part-1-step-3b.jpg?raw=true)

### 4. Bore out the back plate screw holes to accommodate the M2.5 screws

- Carefully bore out the PCB so that M2.5 screws will fit
- Carefully bore out the back of the case so the M2.5 screws (and heads) will fit

![Part 1, Step 4](part-1-step-4.jpg?raw=true)

### 5. Screw together the case, and let the superglue cure for 5-10 minutes

- 6mm M2.5 screw in the PCB
- 8mm M2.5 screws in the case
- Top center screw in the case back is not used

![Part 1, Step 5a](part-1-step-5a.jpg?raw=true)

![Part 1, Step 5b](part-1-step-5b.jpg?raw=true)

### 6. Prepare the back half of the case interior

Remove the three "upper middle" standoffs, remove the USB cable hole support to mirror the front half of the case, and shave down the two left standoffs so that an nRF24L01+ will fit snugly.

![Part 1, Step 6](part-1-step-6.jpg?raw=true)

## Part 2: Modify and prep the PCB

### 1. Remove all components soldered onto the NES PCB

Using a soldering iron and pliers (or other such magic), remove the components from the front and back of the PCB.

#### Before

![Part 2, Step 1a](part-2-step-1a.jpg?raw=true)

![Part 2, Step 1b](part-2-step-1b.jpg?raw=true)

#### After

![Part 2, Step 1c](part-2-step-1c.jpg?raw=true)

![Part 2, Step 1d](part-2-step-1d.jpg?raw=true)

### 2. Cut off the unused portion of the PCB with a Dremel

![Part 2, Step 2a](part-2-step-2a.jpg?raw=true)

![Part 2, Step 2b](part-2-step-2b.jpg?raw=true)

### 3. Cut all traces going to the USB controller chip

![Part 2, Step 3](part-2-step-3.jpg?raw=true)

### 4. Cut the trace between d-pad up and down

![Part 2, Step 4](part-2-step-4.jpg?raw=true)

### 5. Expose d-pad left trace solder point

![Part 2, Step 5](part-2-step-5.jpg?raw=true)

### 6. Bridge the ground for d-pad up and right

![Part 2, Step 6](part-2-step-6.jpg?raw=true)

### 7. Solder wires on for each button and ground, and glue/tape in place

The wires will get later trimmed, so it's easiest to make them long (6+ inches).

![Part 2, Step 6](part-2-step-7.jpg?raw=true)

## Part 3: Prep the components and OLED viewport

### 1. Remove pin headers from the nRF24L01+ boards

![Part 3, Step 1](part-3-step-1.jpg?raw=true)

### 2. If your Teensy came with pin headers, remove them too

![Part 3, Step 2](part-3-step-2.jpg?raw=true)

### 3. Cut the VUSB trace on the bottom of the Teensy

![Part 3, Step 3](part-3-step-3.jpg?raw=true)

### 4. Modify the OLED to fit nicely in the case

The specifics here depend on the exact OLED you purchased, but in this case, the bottom legs of the OLED needed to be removed.

The goal here is to have the OLED fit nicely in the case, in a position where the display will be vertically and horizontally aligned with the edges of the case.

![Part 3, Step 4](part-3-step-4.jpg?raw=true)

### 5. Cut a viewport for the OLED

This part is a pain in the ass, and if you don't screw it up the first time, you are probably a superhero.

I found it works best to first cut off the label material, and then cut through the case.

Once you have the viewport cut and the OLED lining up nicely, use a black sharpie to fill in the gray edges of the viewport walls.

![Part 3, Step 5a](part-3-step-5a.jpg?raw=true)

![Part 3, Step 5b](part-3-step-5b.jpg?raw=true)

![Part 3, Step 5c](part-3-step-5c.jpg?raw=true)

![Part 3, Step 5d](part-3-step-5d.jpg?raw=true)

### 6. Solder some wires onto the OLED for SPI

![Part 3, Step 6](part-3-step-6.jpg?raw=true)

### 7. Get the OLED lined up, and hot glue it into place

![Part 3, Step 7](part-3-step-7.jpg?raw=true)

## Part 4: Component test fit and remaining case modifications

Now you can place all of the components in the case, make sure everything fits, and make the remaining case modifications.

You will need to cut holes for the following, in the position you will glue everything into place:

- On/off switch
- Teensy USB port
- microSD card
- WS2812B LED (where the original USB cable exited the case)

The exact placement is left as an exercise for the reader, as each controller ends up being a little different.

![Part 4, Step 1](part-4-step-1.jpg?raw=true)

## Part 5: Wire everything up

For the most part, the specific I/O pin mapping is flexible, as long as you remember where you connected everything. Once it's all wired up, you will specify the pin mapping in config.h.

### 1. Radios, OLED, and microSD

The radios, OLED, and microSD reader all communicate over SPI. This means that you will have unique CS and CE wires for each component, and then shared VDD/GND/MOSI/MISO/SCK.

Each radio and the microSD reader needs to have the following pins wired up:

- VDD
- GND
- MOSI
- MISO
- CS
- CE
- SCK

The OLED display needs the following pins wired up:

- VIN
- GND
- MOSI
- SA0
- RST
- DATA
- SCK

The most space efficient method seems to be daisy-chaining VDD/GND/MOSI/MISO/SCK.

### 2. Battery, switch, and charge controller

| Connect This | To This |
| ------------ | ------- |
| Teensy 3.3V | Charge controller IN+ |
| Teensy GND | Charge controller IN- |
| Battery+ | Charge controler B+ |
| Battery- | Charge controller B- **WIRE THE SWITCH IN HERE** |
| Teensy VIN | Charge controller OUT+ |
| Teensy GND | Charge controller OUT- |

### 3. NES buttons

Connect the NES PCB ground to Teensy ground, and each of the button leads to a digital pin on the Teensy.

### 4. Voltage divider

If you want to use the voltage readout and charge status on the controller, you will need to wire up a simple voltage divider between the charge controller in/out and Teensy.

The specifics here will depend on the LiPo charge controller you are using, but you can see how this is consumed in battery.cpp.

### 5. WS2812B LED

The LED needs 3.3V, GND connected to the Teensy, along with a data pin of your choosing.

## Part 6: Final installation

### 1. Glue everything in place

Glue all of the components into place, being careful to avoid warping the front decal with too much heat.

When it's all said and done, you should have something that looks approximately like this:

![Part 6, Step 1](part-6-step-1.jpg?raw=true)
