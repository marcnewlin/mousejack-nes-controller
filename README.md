# MouseJack NES Controller

The MouseJack NES controller allows you to hijack and control nearby wireless mice for that nostalgic trolling goodness.

This project is the love child of Burning Man and my wireless mouse/keyboard research, and I am open sourcing it so that you too can troll people with an NES controller.

![Controller Front](guide/controller.jpg?raw=true)

## History

In preparation for Burning Man in 2015, I built a big LED covered top hat so I could efficient blind my campmates. I needed a way to provide input to the hat, and the obvious thing to do was make a wireless NES controller (duh).

Around the same time, I was starting to investigate Logitech wireless mice, which use the same type of radio transceiver as my NES controller. So just before DEF CON 23, I hacked together some code to hijack other peoples' Logitech wireless mice, and spent the week trolling people at BSidesLV (sorry CTFers) and DEF CON. This proved to be hilarious, and the IoT Village was even using a Logitech mouse for their presentation clicker.

The initial controller was pretty quaint, with only a single radio and no display, so I decided to iterate on it after DEF CON and Burning Man had wrapped up. I tend to get really excited and overdo these sort of projects, and I ended up filling the controller to the brim.

The result was a controller containing a Teensy, 500mAh LiPo battery, 5x nRF24L01+ radios, 32GB of microSD storage, and a 128x64 OLED display.

The ToorCon crew was kind enough to let me talk about the beefed up NES controller at ToorCon 17 (which you should do sometime if you like epic Nerf battles).

ToorCon got me excited about trying my hand at vulnerability research, and I ended up using the NES controller to find a keystroke injection vulnerability in Logitech wireless keyboards. This evolved into 16 different mouse and keyboard vulnerabilities and a lot of unhappy vendors.

![Hat Rainbow](guide/hat-rainbow-controlled.gif?raw=true)

## Hardware

- [Tools, Parts and Supplies](guide/tools-parts-supplies.md)
- [Build Guide](guide/build-guide.md)

## Software

- [Software Configuration](guide/software-config.md)

The firmware is configured as a PlatformIO project, and can be installed as follows:

```
cd firmware
platofrmio run --target upload
```

The firmware currently supports Logitech mice, but if there is sufficient interest, I can add support for other vendors. I will also gladly accept pull requests.

Usage is pretty straight forward:

1. Turn on the controller
2. When a Logitech mouse is discovered, it will show up in the device list
3. Use the d-pad to scroll up/down and select a target mouse
4. Press 'select' to enter hijack mode
5. In hijack mode, the d-pad and a/b buttons are used to move the cursor and click the mouse buttons
6. Press select again to exit hijack mode and return to the list

![Controller Device List](guide/controller-device-list.jpg?raw=true)
![Controller Hijack Mode](guide/controller-hijack.jpg?raw=true)

## Related Materials

- [Blog - Burning Man LED Top Hat](http://www.marcnewlin.me/2015/09/lightwad-top-hat.html)
- [Slides - Hacking Wireless Mice with an NES Controller @ ToorCon 17](marc-newlin-toorcon-17-slides.pdf)
- [Slides - MouseJack: Inecting Keystrokes into Wireless Mice @ DEF CON 24](DEFCON-24-Marc-Newlin-MouseJack-Injecting-Keystrokes-Into-Wireless-Mice.slides.pdf)
- [Whitepaper - MouseJack: Inecting Keystrokes into Wireless Mice @ DEF CON 24](DEFCON-24-Marc-Newlin-MouseJack-Injecting-Keystrokes-Into-Wireless-Mice.whitepaper.pdf)
- [Video - MouseJack: Inecting Keystrokes into Wireless Mice @ DEF CON 24](https://www.youtube.com/watch?v=00A36VABIA4)
- [Video - Hak5 Interview @ DEF CON 24](https://www.hak5.org/episodes/season-20/hak5-2026-def-con-24-warwalking-at-def-con-semaphor-mousejack-and-keysniffer)
- [Info - MouseJack/Bastille](http://www.bastille.net/research/vulnerabilities/mousejack/)
- [Code - nRF24LU1+ Research Firmware](https://github.com/BastilleResearch/nrf-research-firmware)
- [Code - gr-nordic (nRF24L GNU Radio Module)](https://github.com/BastilleResearch/gr-nordic)