/**
* Copyright (C) 2011, BMW AG
*
* GeniviAudioMananger
*
* \file  
*
* \date 20-Oct-2011 3:42:04 PM
* \author Christian Mueller (christian.ei.mueller@bmw.de)
*
* \section License
* GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
* Copyright (C) 2011, BMW AG Christian M?ller  Christian.ei.mueller@bmw.de
*
* This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
* This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
* You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
* Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
* Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
* As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
* Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
*
* THIS CODE HAS BEEN GENERATED BY ENTERPRISE ARCHITECT GENIVI MODEL. PLEASE CHANGE ONLY IN ENTERPRISE ARCHITECT AND GENERATE AGAIN
*/
#if !defined(EA_DE721E5B_F385_42e5_9347_0A683B877C1D__INCLUDED_)
#define EA_DE721E5B_F385_42e5_9347_0A683B877C1D__INCLUDED_

namespace am {
	/**
	 * This enum classifies the format in which data is exchanged within a connection. The enum itself is project specific although there are some Genivi standard formats defined.
	 * @author christian
	 * @version 1.0
	 * @created 08-Dec-2011 1:05:12 PM
	 */
	enum am_ConnectionFormat_e
	{
		CF_STEREO = 0,
		CF_MONO,
		CF_ANALOG,
		CF_MAX,
		CF_MIN = CF_STEREO
	};

	/**
	 * This enum gives the information about reason for reason for Source/Sink change
	 * @author christian
	 * @version 1.0
	 * @created 08-Dec-2011 1:05:12 PM
	 */
	enum am_AvailabilityReason_e
	{
		AR_NEWMEDIA = 0,
		AR_SAMEMEDIA,
		AR_NOMEDIA,
		AR_UNKNOWN,
		AR_TEMPERATURE,
		AR_VOLTAGE,
		AR_MAX,
		AR_MIN = AR_NEWMEDIA
	};

	/**
	 * product specific identifier of property
	 * @author christian
	 * @version 1.0
	 * @created 08-Dec-2011 1:05:12 PM
	 */
	enum am_ClassProperty_e
	{
		/**
		 * defines the source type of a source. Project specific, could be for example differentiation between interrupt source and main source.
		 */
		CP_SOURCE_TYPE = 0,
		/**
		 * defines the SINK_TYPE. Project specific
		 */
		CP_SINK_TYPE,
		CP_MAX,
		CP_MIN = CP_SOURCE_TYPE
	};

	/**
	 * The given ramp types here are just a possiblity. for products, different ramp types can be defined here.
	 * It is in the responsibility of the product to make sure that the routing plugins are aware of the ramp types used.
	 * @author christian
	 * @version 1.0
	 * @created 08-Dec-2011 1:05:12 PM
	 */
	enum am_RampType_e
	{
		/**
		 * this ramp type triggers a direct setting of the value without a ramp
		 */
		RAMP_DIRECT = 0,
		/**
		 * This ramp type will set the volume as fast as possible.  
		 */
		RAMP_NO_BLOB,
		RAMP_LOG,
		RAMP_STRAIGHT,
		RAMP_MAX,
		RAMP_MIN = RAMP_DIRECT
	};

	/**
	 * sound properties. Within genivi only the standard properties are defined, for products these need to be extended.
	 * @author christian
	 * @version 1.0
	 * @created 08-Dec-2011 1:05:12 PM
	 */
	enum am_SoundPropertyType_e
	{
		SP_TREBLE = 0,
		SP_MID,
		SP_BASS,
		SP_MAX,
		SP_MIN = SP_TREBLE
	};

	/**
	 * Here are all SoundProperties that can be set via the CommandInterface. Product specific
	 * @author christian
	 * @version 1.0
	 * @created 08-Dec-2011 1:05:12 PM
	 */
	enum am_MainSoundPropertyType_e
	{
		/**
		 * gives the navigation offset in percent
		 */
		MSP_NAVIGATION_OFFSET = 0,
		MSP_TEST,
		MSP_MAX,
		MSP_MIN = MSP_NAVIGATION_OFFSET
	};

	/**
	 * describes the different system properties. Project specific
	 * @author christian
	 * @version 1.0
	 * @created 08-Dec-2011 1:05:12 PM
	 */
	enum am_SystemPropertyType_e
	{
		SYP_TEST = 0,
		SYP_MAX,
		SYP_MIN = SYP_TEST
	};
}
#endif // !defined(EA_DE721E5B_F385_42e5_9347_0A683B877C1D__INCLUDED_)