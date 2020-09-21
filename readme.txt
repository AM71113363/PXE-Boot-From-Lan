build.bat will work only with Compiler:  Dev-C++ 4.9.9.*


run BOOT_LAN.exe (32Bit Windows) 

1.Set Lan IP 192.168.1.1
2.Connect two PC with Crossover Ethernet Cable
3.Drag-Drop BootLoader( "grldr" from Hiren's Boot CD ) to start DHCP & TFTP server.
4.Turn on The other PC >>PRESS F12<< set >>Boot From LAN<<.
5.From the MENU choose [ Hiren15.2 ] then [ Windows XP ]
DONE.

IMPORTANT:
You must put in the same Folder "BOOT_LAN.exe" + "grldr" + "menu.lst" + "Hiren15.2.iso"
You can edit "menu.lst" and change the name of BOOTABLE ISO file.
Current ISO filenames in "menu.lst" are [ "Hiren15.2.iso" ] [ Hiren9.iso ]
You can add more than 1 ISO file or just leave 1.

The size of BOOTABLE ISO FILE must be less than PC RAM SIZE

PS: You Can Edit Hiren's ISO file and remove the Apps you don't need.
    My "Hiren15.2.iso" size is 47MB.


////////////////////////////// menu.lst /////////////////////////////////
color blue/green yellow/red white/magenta white/magenta
timeout 400
default /default


title Hiren15.2 FIRST ISO FILE
map --mem (pd)/Hiren15.2.iso (0xFF)
map --hook
root (0xFF)
chainloader (0xFF)

title Hiren9 SECOND ISO FILE
map --mem (pd)/Hiren9.iso (0xFF)
map --hook
root (0xFF)
chainloader (0xFF)

title reboot
reboot


title PowerOff
halt

////////////////////////////////////////////////////////////////////////////////