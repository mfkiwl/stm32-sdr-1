## STM32 SDR

This is a software defined radio receiver (SDR) based on
a Nucleo-STM32F401RE microcontroller development board in
combination with a Tayloe quadrature downconverter.

The local oscillator signal is supplied by a SiLabs Si5351
and the baseband data is digitized using a WM8731L audio codec.
The audio codec is also used to drive a pair of headphones
with the demodulated audio.

The software currently supports demodulation of AM and SSB transmissions.


## License 

Copyright (C) 2020  Y. Ritterbusch

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
