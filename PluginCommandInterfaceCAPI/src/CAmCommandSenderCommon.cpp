/**
 *  Copyright (c) 2012 BMW
 *
 *  \author Aleksandar Donchev, aleksander.donchev@partner.bmw.de BMW 2013
 *
 *  \copyright
 *  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
 *  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 *  subject to the following conditions:
 *  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 *  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
 *  THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 *  For further information see http://www.genivi.org/.
 */


#include "CAmCommandSenderCommon.h"

/**
 * Utility functions
 */
void CAmConvertAvailablility(const am_Availability_s & amAvailability, org::genivi::am::am_Availability_s & result)
{
	result.availability = CAmConvert2CAPIType(amAvailability.availability);
	result.availabilityReason = amAvailability.availabilityReason;
}

void CAmConvertMainSoundProperty(const am_MainSoundProperty_s & amMainSoundProperty, org::genivi::am::am_MainSoundProperty_s & result)
{
	result.type = amMainSoundProperty.type;
	result.value = amMainSoundProperty.value;
}

void CAmConvertSystemProperty(const am_SystemProperty_s & amSystemProperty, org::genivi::am::am_SystemProperty_s & result)
{
	result.type = amSystemProperty.type;
	result.value = amSystemProperty.value;
}

org::genivi::am::am_Availability_e CAmConvert2CAPIType(const am_Availability_e & availability)
{
	return (A_MAX==availability)?org::genivi::am::am_Availability_e::A_MAX
								 :
								 static_cast<org::genivi::am::am_Availability_e>(availability);
}

org::genivi::am::am_MuteState_e CAmConvert2CAPIType(const am_MuteState_e & muteState)
{
	return MS_MAX==muteState?
							org::genivi::am::am_MuteState_e::MS_MAX
							:
							static_cast<org::genivi::am::am_MuteState_e>(muteState);
}

am_MuteState_e CAmConvertFromCAPIType(const org::genivi::am::am_MuteState_e & muteState)
{
	return org::genivi::am::am_MuteState_e::MS_MAX==muteState?
							MS_MAX:static_cast<am_MuteState_e>(muteState);
}

org::genivi::am::am_ConnectionState_e CAmConvert2CAPIType(const am_ConnectionState_e & connectionState)
{
	return CS_MAX==connectionState?
							org::genivi::am::am_ConnectionState_e::CS_MAX
							:
							static_cast<org::genivi::am::am_ConnectionState_e>(connectionState);
}
am_ConnectionState_e CAmConvertFromCAPIType(const org::genivi::am::am_ConnectionState_e & connectionState)
{
	return org::genivi::am::am_ConnectionState_e::CS_MAX==connectionState?
			CS_MAX:static_cast<am_ConnectionState_e>(connectionState);
}

org::genivi::am::am_NotificationStatus_e CAmConvert2CAPIType(const am_NotificationStatus_e & notificationStatus)
{
	return NS_MAX==notificationStatus?
							org::genivi::am::am_NotificationStatus_e::NS_MAX
							:
							static_cast<org::genivi::am::am_NotificationStatus_e>(notificationStatus);
}
am_NotificationStatus_e CAmConvertFromCAPIType(const org::genivi::am::am_NotificationStatus_e & notificationStatus)
{
	return org::genivi::am::am_NotificationStatus_e::NS_MAX==notificationStatus?
			NS_MAX:static_cast<am_NotificationStatus_e>(notificationStatus);
}

org::genivi::am::am_Error_e CAmConvert2CAPIType(const am_Error_e & error)
{
	return E_MAX==error?
							org::genivi::am::am_Error_e::E_MAX
							:
							static_cast<org::genivi::am::am_Error_e>(error);
}
am_Error_e CAmConvertFromCAPIType(const org::genivi::am::am_Error_e & error)
{
	return org::genivi::am::am_Error_e::E_MAX==error?
			E_MAX:static_cast<am_Error_e>(error);
}

