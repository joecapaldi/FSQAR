
// ======================================================================= 
// Flight Simulator Quick Access Recorder and Analyzer (FSQAR)
// -----------------------------------------------------------------------
// 
// Created by AlexShag, 2016
//
// This program is free software: you can redistribute it and/or modify 
// it under the terms of the GNU GPL (see License.txt).
//
// WARNING:
// This is a very bad code! This is not C++ code but "C with objects" one. 
// It breaks many basic principles of Objected-Oriented Programming.  
// It is badly designed and ugly written. I don’t advise to use it directly 
// without any modifications. This code may be used only as a starting point 
// for developing more advanced application.
//
// See more details in Readme.txt
// 
// =======================================================================

#ifndef __SIMVARIDS_H__
#define __SIMVARIDS_H__

#define SIMVAR_BASE_ID				5000

#define IDS_SIMVAR_ALPHA			(SIMVAR_BASE_ID+1)
#define IDS_SIMVAR_BETA				(SIMVAR_BASE_ID+2)
#define IDS_SIMVAR_BETADOT			(SIMVAR_BASE_ID+3)
#define IDS_SIMVAR_TAS				(SIMVAR_BASE_ID+4)
#define IDS_SIMVAR_MACH				(SIMVAR_BASE_ID+5)
#define IDS_SIMVAR_VSPEED			(SIMVAR_BASE_ID+6)
#define IDS_SIMVAR_GRORCE			(SIMVAR_BASE_ID+7)
#define IDS_SIMVAR_WEIGHT			(SIMVAR_BASE_ID+8)
#define IDS_SIMVAR_DYNPRESSURE		(SIMVAR_BASE_ID+9)
#define IDS_SIMVAR_IAS				(SIMVAR_BASE_ID+10)
#define IDS_SIMVAR_LAT				(SIMVAR_BASE_ID+11)
#define IDS_SIMVAR_LON				(SIMVAR_BASE_ID+12)
#define IDS_SIMVAR_ALT				(SIMVAR_BASE_ID+13)
#define IDS_SIMVAR_PITCH			(SIMVAR_BASE_ID+14)
#define IDS_SIMVAR_BANK				(SIMVAR_BASE_ID+15)
#define IDS_SIMVAR_HTRUE			(SIMVAR_BASE_ID+16)
#define IDS_SIMVAR_HMAG				(SIMVAR_BASE_ID+17)
#define IDS_SIMVAR_MAGVAR			(SIMVAR_BASE_ID+18)
#define IDS_SIMVAR_HEIGHT			(SIMVAR_BASE_ID+19)
#define IDS_SIMVAR_GSPEED			(SIMVAR_BASE_ID+20)
#define IDS_SIMVAR_CGLON			(SIMVAR_BASE_ID+21)
#define IDS_SIMVAR_CGLAT			(SIMVAR_BASE_ID+22)
#define IDS_SIMVAR_VBX				(SIMVAR_BASE_ID+23)
#define IDS_SIMVAR_VBY				(SIMVAR_BASE_ID+24)
#define IDS_SIMVAR_VBZ				(SIMVAR_BASE_ID+25)
#define IDS_SIMVAR_VWX				(SIMVAR_BASE_ID+26)
#define IDS_SIMVAR_VWY				(SIMVAR_BASE_ID+27)
#define IDS_SIMVAR_VWZ				(SIMVAR_BASE_ID+28)
#define IDS_SIMVAR_AWX				(SIMVAR_BASE_ID+29)
#define IDS_SIMVAR_AWY				(SIMVAR_BASE_ID+30)
#define IDS_SIMVAR_AWZ				(SIMVAR_BASE_ID+31)
#define IDS_SIMVAR_ABX				(SIMVAR_BASE_ID+32)
#define IDS_SIMVAR_ABY				(SIMVAR_BASE_ID+33)
#define IDS_SIMVAR_ABZ				(SIMVAR_BASE_ID+34)
#define IDS_SIMVAR_RBX				(SIMVAR_BASE_ID+35)
#define IDS_SIMVAR_RBY				(SIMVAR_BASE_ID+36)
#define IDS_SIMVAR_RBZ				(SIMVAR_BASE_ID+37)
#define IDS_SIMVAR_WINDX			(SIMVAR_BASE_ID+38)
#define IDS_SIMVAR_WINDY			(SIMVAR_BASE_ID+39)
#define IDS_SIMVAR_WINDZ			(SIMVAR_BASE_ID+40)
#define IDS_SIMVAR_AMBDENS			(SIMVAR_BASE_ID+41)
#define IDS_SIMVAR_AMBTEMP			(SIMVAR_BASE_ID+42)
#define IDS_SIMVAR_STDTEMP			(SIMVAR_BASE_ID+43)
#define IDS_SIMVAR_TAT				(SIMVAR_BASE_ID+44)
#define IDS_SIMVAR_AWVEL			(SIMVAR_BASE_ID+45)
#define IDS_SIMVAR_AWDIR			(SIMVAR_BASE_ID+46)
#define IDS_SIMVAR_AWINDX			(SIMVAR_BASE_ID+47)
#define IDS_SIMVAR_AWINDY			(SIMVAR_BASE_ID+48)
#define IDS_SIMVAR_AWINDZ			(SIMVAR_BASE_ID+49)
#define IDS_SIMVAR_ACWINDX			(SIMVAR_BASE_ID+50)
#define IDS_SIMVAR_ACWINDY			(SIMVAR_BASE_ID+51)
#define IDS_SIMVAR_ACWINDZ			(SIMVAR_BASE_ID+52)
#define IDS_SIMVAR_AMBPRESS			(SIMVAR_BASE_ID+53)
#define IDS_SIMVAR_BAROPRESS		(SIMVAR_BASE_ID+54)
#define IDS_SIMVAR_SLPRESS			(SIMVAR_BASE_ID+55)
#define IDS_SIMVAR_AMBVIS			(SIMVAR_BASE_ID+56)
#define IDS_SIMVAR_RRM				(SIMVAR_BASE_ID+57)
#define IDS_SIMVAR_LEVER			(SIMVAR_BASE_ID+58)
#define IDS_SIMVAR_EGT				(SIMVAR_BASE_ID+59)
#define IDS_SIMVAR_OILPRESS			(SIMVAR_BASE_ID+60)
#define IDS_SIMVAR_OILTEMP			(SIMVAR_BASE_ID+61)
#define IDS_SIMVAR_FUEL				(SIMVAR_BASE_ID+62)
#define IDS_SIMVAR_TIME				(SIMVAR_BASE_ID+63)
#define IDS_SIMVAR_N1				(SIMVAR_BASE_ID+64)
#define IDS_SIMVAR_N2				(SIMVAR_BASE_ID+65)
#define IDS_SIMVAR_CORRN1			(SIMVAR_BASE_ID+66)
#define IDS_SIMVAR_CORRN2			(SIMVAR_BASE_ID+67)
#define IDS_SIMVAR_CORRFF			(SIMVAR_BASE_ID+68)
#define IDS_SIMVAR_TORQUE			(SIMVAR_BASE_ID+69)
#define IDS_SIMVAR_EPR				(SIMVAR_BASE_ID+70)
#define IDS_SIMVAR_ITT				(SIMVAR_BASE_ID+71)
#define IDS_SIMVAR_THRUST			(SIMVAR_BASE_ID+72)
#define IDS_SIMVAR_BLEED			(SIMVAR_BASE_ID+73)
#define IDS_SIMVAR_FF				(SIMVAR_BASE_ID+74)
#define IDS_SIMVAR_VIB				(SIMVAR_BASE_ID+75)
#define IDS_SIMVAR_REVERS			(SIMVAR_BASE_ID+76)
#define IDS_SIMVAR_EDEF				(SIMVAR_BASE_ID+77)
#define IDS_SIMVAR_EPCT				(SIMVAR_BASE_ID+78)
#define IDS_SIMVAR_ADEF				(SIMVAR_BASE_ID+79)
#define IDS_SIMVAR_APCT				(SIMVAR_BASE_ID+80)
#define IDS_SIMVAR_RDEF				(SIMVAR_BASE_ID+81)
#define IDS_SIMVAR_RPCT				(SIMVAR_BASE_ID+82)
#define IDS_SIMVAR_ETRIM			(SIMVAR_BASE_ID+83)
#define IDS_SIMVAR_ATRIM			(SIMVAR_BASE_ID+84)
#define IDS_SIMVAR_RTRIM			(SIMVAR_BASE_ID+85)
#define IDS_SIMVAR_PROPRPM			(SIMVAR_BASE_ID+86)
#define IDS_SIMVAR_PPRPMPCT			(SIMVAR_BASE_ID+87)
#define IDS_SIMVAR_PROPBETA			(SIMVAR_BASE_ID+88)
#define IDS_SIMVAR_PROPTHRUST		(SIMVAR_BASE_ID+89)

#endif // __SIMVARIDS_H__			
									
