## Overview
The LTE Shield uses SIMCOM's SIM7000-series LTE modules which are low-power 4G LTE modules that support the new LTE CAT-M technology and also have integrated GNSS (GPS,GLONASS and BeiDou/Compass, Galileo, QZSS standards) for location tracking. NB-IoT is achieved with the different versions, namely the SIM7000E, SIM7000C, and SIM7000E-N modules. To use the LTE Shield, simply plug the shield into an Arduino, attach a LiPo battery to power the cellular module, insert a compatible SIM card, attach the LTE/GNSS antenna, and you're good to go!

All PCB design files and hardware are released under the [Creative Commons Attribution Share Alike 4.0 license](https://choosealicense.com/licenses/cc-by-sa-4.0/).

All other software is released under the [GNU General Public License v3.0](https://choosealicense.com/licenses/gpl-3.0/).

## Introduction

With the emergence of low-power IoT devices with cellular connectivity and the phase-out of 2G (with only T-mobile supporting 2G/GSM until 202), everything is moving toward LTE and this has left many people scrambling to find better solutions. However, this has also left many hobbyists facepalming with legacy 2G technology like the SIM800-series modules from SIMCOM. Although these 2G and 3G modules are a great starting point, it's time to move forward and [SIMCOM recently announced](http://simcomm2m.com/En/media/detail.aspx?id=86) their new SIM7000A LTE CAT-M module at a developer's conference. How exciting!   :)

The amazing part of all of this is that SIMCOM made it extremely easy to migrate from their 2G and 3G modules to this new module! The SIM7000-series use many of the same AT commands which minimizes the software development by miles! Also, Adafruit already has a wonderful [FONA library on Github](https://github.com/adafruit/Adafruit_FONA) that can be used to introduce this new SIM7000 into the party!

## What is LTE CAT-M?

LTE CAT-M1 is considered the second-generation LTE technology and is lower-power and more suitable for IoT devices. NarrowBand IoT (NB-IoT) or "CAT-M2" technology is a Low-Power Wide Area Network (LPWAN) technology specifically designed for low-power IoT devices. It is a relatively new technology that is, unfortunately, not yet available in the US, although there are rumors that certain large companies like AT&T are working on testing and building the infrastructure. For IoT devices using radio technology (RF) there are several things to keep in mind:

- Power consumption
- Bandwidth
- Range
- Packet size (send lots of data
- Cost

Each of these have tradeoffs (and I won't really explain them all); for example, large bandwidth allows devices to send lots of data (like your phone, which can stream YouTube!) but this also means it's very power-hungry. Increasing the range (the "area" of the network) also increase power consumption. In the case of NB-IoT, cutting down the bandwidth means that you won't be able to send much data, but for IoT devices shooting morsels of data to the cloud this is perfect! Hence, "narrow"-band technology, ideal for low-power devices with little amounts of data but still with long range (wide area)!

## The LTE Shield for Arduino

The LTE shield that I've designed uses the SIM7000-series to enable users to have extremely low-power LTE CAT-M technology and GNSS at the tip of their fingers! The shield also supports LiPo battery charging mainly to power the device (as it can take large current spikes). Currently I am using the [SIM7000A](http://simcomm2m.com/En/module/detail.aspx?id=173) (the CAT-M1 module) because in the US we poor souls don't have NB-IoT yet, unlike our European counterparts. However, for those in other places of the world with NB-IoT it's as easy as swapping out the SIM7000A with other versions like the [SIM7000E](http://simcomm2m.com/En/module/detail.aspx?id=168), [SIM7000C](http://m.simcomm2m.com/En/module/detail.aspx?id=167), and [SIM7000E-N](http://simcomm2m.com/En/module/detail.aspx?id=175) modules.

Since I'm a huge fan of open-source (as you can see from my Reflowduino project) I've made this project completely open-source as well, with special thanks to Adafruit for their FONA library! (And if they'd allow it, I'd call this the "FONA LTE" but I'm not making that official due to copyright purposes lol).

## Goals for this Project

Currently I'd like to have this open-source design fully-integrated to the hobbyist market for Arduino users all around the world! I believe this is where the future is for cellular communication. In order to do that I'm planning on launching a crowdfunding campaign on Indiegogo in the near future after getting a fully-functional and tested design! Stay tuned for updates and feel free to comment, give suggestions, and share, because that's what this is all about!

Best,

~ Tim

"Share to learn, learn to share!"

