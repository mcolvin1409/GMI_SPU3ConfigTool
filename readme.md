**Overview of Subsystems/Applications
SPU3 Ethernet Auto Configuration Tool will display all of its Graphic User Interface controls in English only. However, this Tool can be run from a non-English OS. 
Also, this Tool can be launched from both versions of Mercury Client Application, i.e. ANSI version and Unicode version. 
The following OS Versions will be supported by this Tool:
•	Windows XP Pro, 32 bit
•	Windows XP Embedded, 32 bit
•	Windows 7, 32 bit and 64 bit
•	Windows 7 Embedded, 32 bit and 64 bit
•	Windows 10 Iot, 64 bit (not yet tested)

The following sections describes the changes incorporated in each subsystem/application to support the implementation of SPU3 Ethernet Auto Configuration Tool.
**Mercury Client Application
•	Added support to launch the SPU3 Ethernet Auto Configuration Tool from Mercury Client Application. For more details refer to the Appendix section of this document.
SPU3 (Servo Power Unit)
•	The SPU3 FW will implement the UDP interface and application protocol messages defined in this document.
Internal Note: SPU3 Simulator Application can be used to verify the UDP communication interface. However, testing with the SPU3 Simulator should be limited to the LAB environment, when real SPU3 Devices are NOT available.
**SPU3 Ethernet Auto Configuration Tool
The following section describes the purpose of each input/output Files:
Mercury System XML file:
o	It is an optional input file. In other words, the SPU3 Ethernet Auto Configuration Tool can be used without an input System XML file.
o	The SPU3 IP Addresses from the System XML file will be used to select & change the IP Address of a SPU3 Device found on the Network. 
o	The SPU3 IP Addresses from the System XML file will be used to compare with each SPU3 Device found on the Network (LAN). This process will determine the SPU3 Devices that are NOT in sync with the System XML configuration.
o	No changes to the System XML file will be allowed form the SPU3 Ethernet Auto Configuration Tool.
**Assumptions
•	Every SPU3 Device would have a valid IP Address assigned, regardless of the communication type (serial / Ethernet)
•	SPU3 Ethernet Auto Configuration Tool would run as a standalone application
•	SPU3 Ethernet Auto Configuration Tool would be a single instance application
•	SPU3 Ethernet Auto Configuration Tool and SPU3 Device would communicate via UDP messages
•	SPU3 Ethernet Auto Configuration Tool would be launched from Mercury Client application
•	SPU3 Ethernet Auto Configuration Tool would allow user to change SPU3 Device settings and Application settings
