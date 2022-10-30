# NTP_clock_on_TM1637_display
library: https://github.com/tehniq3/TM1637-display or // https://github.com/avishorp/TM1637

article no.1: https://nicuflorica.blogspot.com/2022/10/ceas-ntp-pe-afisaj-cu-tm1637.html

article no.2: https://nicuflorica.blogspot.com/2022/10/ceas-ntp-pe-afisaj-cu-tm1637-format-de.html

article no.3: https://nicuflorica.blogspot.com/2022/10/ceas-ntp-pe-afisaj-led-cu-tm1637-reglaj.html

article no.4:: https://nicuflorica.blogspot.com/2022/10/termometruhigrometru-cu-dht2211-si-ceas.html

basic sketch (24-hour format): IoTClockCode1.ino using schematic from http://www.esp8266learning.com/tm1637-7-segment-display-example.php
![basic schematic](https://github.com/tehniq3/NTP_clock_on_TM1637_display/blob/main/wemos-and-TM1637_bb.jpg)

sketch for 12-hour format: https://github.com/tehniq3/NTP_clock_on_TM1637_display/blob/main/IoTClockCode1b.ino

![albastru](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEg20-1u6pfqt0P6uqmjdVp9uc7U1P2HaSVSflQ2bcJ5porYFKUsoCcFRy9_0DWW-bKnJleebmGTInYdik-y_WKtgQV58OA20uTqsIXN5oNXncPM7mFcqVFvQsRR5Ca0z0Se0FV4kRYIcvHjXNRlqIoSoBDEjHnhQOCTCd9WoRCCbSPQKB74Fxp2Cb_3Dw/w200-h150/TM1637_05.jpg)
![galben](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEg2Q1b2gyRXUEmCZjEICY-T3CLC9izl3qOoP3_JE8r4vYLiL4uEKStPJjSdbwDP_TiPZzq0hGgYp2JOqkd02850qpkINuccoCzsyzvRtLh1405VRSNWCJPR5dhOZ1KKsQD11n0Rj7yirzi8bV6oDLh8A6kpQyrcccnymzXK3o9OuOxOPgwbB6JFT7m_hw/w200-h150/TM1637_06.jpg)
![alb](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEgRmfJ_mJEeY6H_QVtXESX-vegdeywY3gV0ZOrlnMfVncW5ziMbjHSAKacWzfV0WyYCK2eEAuL7aqCsQVQs1oXMN4VhGGJs95_uxthkVAS5voqPnBOef-ClFZocaXD4AsY16fC25V2dD8bUUrUA18MAklgS5fcB9_C196wV9sD4CVcpAfIWbfVvfdMqHQ/w200-h150/TM1637_07.jpg)

![webcontrol](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEicMxwoEOVYKlz0EYNeGAlGqOOQVkLwDae1waAGi6SKVjXXTJjkYNwfDSMF32eMwvblrFkk-o7JQ71Vbg-PYpJ13j8xtwG8Dzhus75Za-ZfNx1DJqdpBQZWWvk0O8uP17K5Rb-tP8AMnLRIeV0FfiJJmQ17NN0IDl9SthRASY6JegL0rEv3O9jnkJ3msQ/w150-h200/ora_auto_0.jpg)

![ntpCLOCK_dht](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEgc2jy-Cd5Vv3vkg0bVez9L7NJ7ob4vxBuRToA8dd24hkBHgIb_8vihazQqZ2eEGkQYaXHUZ69Hd-3PcwSIcFM_OPo2ICy1sGX1ZY_l26xuvTs-lQ1jKtM0-04_qy6z-q-RM4dFlgtIxyDyTaIt-mnARH1TijXGYg6HeDL0qrvQQens-0XWP90lIAKJUQ/s1139/TM1637_DHT_WemosD1_sch1.png)

![TEMPERATURE](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEi0Fv9qsf9ej4QL4KzLn6JQow6yUfIB88YizHvvbYRrvJS0YDVsZjXA9cdbejvbnszmQxoqCywcnBDwcVFOVdRXtlHL5mtlY8s_NXuTHb3lDlwMNmx-8LeRYNdLWbCDNo6kSn183flyjjj-deHW6klvNQbXuUYw0MXo8g6rhuocb1mgfFuEZqpaG8z3Jw/w200-h150/proiect4.jpg)
![HUMIDITY](https://blogger.googleusercontent.com/img/b/R29vZ2xl/AVvXsEgoAWxZ6996U_oO_kAyKkVLBHQVqC_XptE0l4SaT47MTESQuGDOKNowe7wSVvyIeWxytDZH5_M4MIuKOa8faGI9ByNAaMCl-FhzrBguHLhKbgKO0ZJ_r0Kq-bu8Um_0asNg_GJfgMkvK-AgG0JpDtt1SVxjLdAkheZkfNWiOj0Y2RgdJ2WBzrP6yQ0e5w/w200-h150/proiect3.jpg)

Sketch for DHT22: https://github.com/tehniq3/NTP_clock_on_TM1637_display/blob/main/ntp_clock_webcontrol_TM1637_2.ino

Updated sketch for control extra info: https://github.com/tehniq3/NTP_clock_on_TM1637_display/blob/main/ntp_clock_webcontrol_TM1637_2a.ino
