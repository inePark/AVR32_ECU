@echo off
setlocal enableextensions enabledelayedexpansion

: This command script programs the released ISP (flash array), the ISP
: configuration word (User page) and the general-purpose fuse bits.

: Copyright (C) 2006-2008, Atmel Corporation All rights reserved.
:
: Redistribution and use in source and binary forms, with or without
: modification, are permitted provided that the following conditions are met:
:
: 1. Redistributions of source code must retain the above copyright notice, this
: list of conditions and the following disclaimer.
:
: 2. Redistributions in binary form must reproduce the above copyright notice,
: this list of conditions and the following disclaimer in the documentation and/
: or other materials provided with the distribution.
:
: 3. The name of ATMEL may not be used to endorse or promote products derived
: from this software without specific prior written permission.
:
: THIS SOFTWARE IS PROVIDED BY ATMEL ``AS IS'' AND ANY EXPRESS OR IMPLIED
: WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
: MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE EXPRESSLY AND
: SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT,
: INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
: BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
: DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
: OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
: NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
: EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


echo.
echo Writing release version of api_demo_plus_3v1 for Eval110 board to AVR32.
batchisp -device at32uc3c0512c -hardware usb -operation erase f memory flash blankcheck loadbuffer KETI_AMG_Player.hex program verify start reset 0