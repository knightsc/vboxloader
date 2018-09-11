# vboxloader

Sample code from [_xpn_](https://twitter.com/_xpn_) blog post found here:

[https://www.mdsec.co.uk/2018/08/disabling-macos-sip-via-a-virtualbox-kext-vulnerability/](https://www.mdsec.co.uk/2018/08/disabling-macos-sip-via-a-virtualbox-kext-vulnerability/)  

The VirtualBox VBoxDrv.kext allows loading of arbitrary code into the kernel. The original sample code from the blog post just loads code to change SIP settings but the actual VirtualBox client code dynamically loads Mach-O object files containing strings and symbols.
