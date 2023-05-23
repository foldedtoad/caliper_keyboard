# Caliper - Digital Caliper Data Over Bluetooth Via Keyboard Emulation

This poject's goal is to provide a remote interface to the various cheap Digital Calipers: below is a photo of one such product.  This project is being developed on Zephyr Version 3.3.99.

![here](https://github.com/foldedtoad/caliper_keyboard/blob/master/images/caliper_product_package.jpg)
  
 The photo below shows an early prototype, which demostratess the basic connectivity from caliper to Bluetooth-supporting microprocessor to iPhone or Android app.  
 The Bluetooth microprocessor is the nRF52832 and the iPhone or Android app is Nordic's NRF Connect app: a dedicated phone app is under development.  

 ![here](https://github.com/foldedtoad/caliper_keyboard/blob/master/images/caliper_beta_prototype.jpg)
  
## Raison d'Etra

  * The need to record a series of measurements reliably.
  * The need to take a measurement in an awkward position or location, where the on-caliper display can not be easily viewed.
  * Used as the basis for a quasi-DRO in machining equipment.
  
## Theory of Operation

  * The supporting RTOS should be Zephyr.  It has good Bluetooth support and good thread support. 
  * Bluetooth communications should be kept simple as possible.  
    Advertisement data will carry the caliper data as "Manufacturer Data". This allows for a simpler implementation Bluetooth code on the microprocessor.  It also allows multiple iPhones to receive data from a give device.  Further, it allows a iPhone to receive data from multiple calipers. 
  * Since Bluetooth Advertisement packets are redundantly sent on three special channels, the chances of the receiver not getting the caliper data is greatly reduce.
  * Support for "Sample Now" button, which when pressed will transmit caliper data to iPhone.  Currently, the microprocessor receives a stream of caliper data and only when the value changes does it transmit the data upline.  Pressing the Sample Now button may also have a buzzer to provide audiable feedback.
  * Microprossor code base should provide support for both inverting and non-inverting level-shifters.
  * For "Tethered" type configurations, the Caliper and level shifter reside at opposite ends of a cable. The level shifter and microprocessor may reside in a case attached to the user's belt, thereby facilitating a larger battery. In the case of the beta prototype, this cable is actually a USB mini cable. High quality USB cable are available which provide good signal integrity for 6 feet or 2 meter lengths.
  * For "On Caliper" type configurations, the level shifter/microprocessor unit are attached to the back of the caliper.  The unit is then powered by an in-case coin cell battery (CR2032 for example).

## Caliper [BOM and Tool List](../master/CALIPER_BOM_TOOLS.md).
 
