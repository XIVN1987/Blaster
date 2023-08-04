# Blaster
Altera USB Blaster porting to Artery AT32F425 and WCH CH32V203.

原版 USB Blaster 由 FT245 + CPLD 实现，其中 FT245 实现 USB 通信，CPLD 实现 JTAG 时序。此实现使用一颗 MCU 同时实现 USB 通信和 JTAG 时序,为了能够使用 USB Blaster 的驱动程序，需要 MCU 模拟 FT245 的通信协议。

## Blaster-AT32F425
Blaster based on Artery AT32F425 (support crystal-less USB), supports JTAG only.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| JTAG_TCK | PA.2  |
| JTAG_TMS | PA.1  |
| JTAG_TDI | PA.3  |
| JTAG_TDO | PA.0  |

## Blaster-CH32V203
Blaster based on WCH CH32V203 (support crystal-less USB), supports JTAG only.

### Pin map
|  FUNC    | Pin   |
|  :----   | :---- |
| JTAG_TCK | PA.2  |
| JTAG_TMS | PA.1  |
| JTAG_TDI | PA.3  |
| JTAG_TDO | PA.0  |
