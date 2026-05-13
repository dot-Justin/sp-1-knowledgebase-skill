To prepare your application for the bootloader on the SP-1, there are a few things to keep in mind:

- The bootloader will write the application starting at `0x20000` in the nRF52840 flash and the application's max size is `0xDEFFF`. 
- The bootloader starts the watchdog. If your app doesn't keep the watchdog happy, the device will reset after 5 seconds.
- `LFCLK` and `HFCLK` and some peripherals like `PWM2`, `PWM3`, `SAADC` are started by the bootloader. If you use the nRF5 SDK to setup your peripherals and clocks, some calls might fail because the SDK assumes they haven't been inited before. 
- The SP-1 doesn't have a hard reset. You need to provide a way for the device to go into `SYSTEM_OFF` to be able to get back to the bootloader.
- `RESETREAS` needs to be cleared before going back to the bootloader.

You can upload new firmware to your SP-1 with the [update utility](https://solderless.engineering) by Solderless.

>**⚠️ WARNING** 
> 
>This is a warning for beginners (and reckless experts 😎)
> 
> There is no hard reset or reset pin on the SP-1 and no easy way to power cycle the device.  
> Please fully test new firmware on a device with debugger access before loading it onto a stock SP-1! 
>
> Always make sure you can properly reset to bootloader from your firmware! 