# LTE Shield
The LTE Shield is a low-power 4G LTE CAT-M1 shield that supports the new LTE CAT-M1 technology and integrates GNSS (GPS,GLONASS and BeiDou/Compass, Galileo, QZSS standards) for location tracking. Simply plug the shield into an Arduino, attach a LiPo battery to power the cellular module, insert a compatible SIM card, attach the LTE/GNSS antenna, and you're good to go!

Order your own Reflowduino board here! Also, don’t forget to check out the comprehensive Github wiki! I also posted this Instructable as a quick 'n dirty summary of everything here as well as on Hackster.io!

All PCB design files and hardware are released under the Creative Commons Attribution Share Alike 4.0 license.

All other software is released under the GNU General Public License v3.0.

## What is LTE CAT-M1 and NB-IoT?
LTE CAT-M1 is considered the second-generation LTE technology and is lower-power and more suitable for IoT devices. NarrowBand IoT (NB-IoT) or "CAT-M2" technology is a Low-Power Wide Area Network (LPWAN) technology specifically designed for low-power IoT devices. It is a relatively new technology that is, unfortunately, not yet available in the US, although there are rumors that certain large companies like AT&T are working on testing and building the infrastructure. For IoT devices using radio technology (RF) there are several things to keep in mind:

- Power consumption
- Bandwidth
- Range
- Packet size (send lots of data
- Cost

Each of these have tradeoffs (and I won't really explain them all); for example, large bandwidth allows devices to send lots of data (like your phone, which can stream YouTube!) but this also means it's very power-hungry. Increasing the range (the "area" of the network) also increase power consumption. In the case of NB-IoT, cutting down the bandwidth means that you won't be able to send much data, but for IoT devices shooting morsels of data to the cloud this is perfect! Hence, "narrow"-band technology, ideal for low-power devices with little amounts of data but still with long range (wide area)!

## Shield Versions
There are a few variants of the LTE Shield, so make sure you choose carefully!

### LTE CAT-M1 Shield
The CAT-M1 shield will come with 

### NB-IoT Shield

