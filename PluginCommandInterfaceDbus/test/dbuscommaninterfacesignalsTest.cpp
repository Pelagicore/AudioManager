/*
 * dbuscommaninterfacesignalsTest.cpp
 *
 *  Created on: Dec 16, 2011
 *      Author: christian
 */

#include "dbuscommaninterfacesignalsTest.h"
#include <iostream>
#include <fstream>
#include <Python.h>
#include "../include/DBusCommandSender.h"
#include "../include/DBusMessageHandler.h"
#include "DbusCommandInterfaceBackdoor.h"
#include "MockInterfaces.h"

using namespace am;
using namespace testing;

DLT_IMPORT_CONTEXT(DLT_CONTEXT)

dbuscommaninterfacesignalsTest::dbuscommaninterfacesignalsTest()
{
	DLT_REGISTER_APP("DPtest","RoutingInterfacetest");
	DLT_REGISTER_CONTEXT(DLT_CONTEXT,"Main","Main Context");
	DLT_LOG(DLT_CONTEXT,DLT_LOG_INFO, DLT_STRING("RoutingSendInterface Test started "));

}

dbuscommaninterfacesignalsTest::~dbuscommaninterfacesignalsTest()
{
	DLT_UNREGISTER_CONTEXT(DLT_CONTEXT);
}

void* NumberOfMainConnectionsChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	sender.cbNumberOfMainConnectionsChanged();
}

void* cbSinkAdded(void*)
{
	sleep(1);
	std::vector<am_SinkType_s> list;
	am_SinkType_s mysink;
	mysink.name="MySink";
	mysink.sinkID=23;
	mysink.availability.availability=A_MAX;
	mysink.availability.availabilityReason=AR_MIN;
	mysink.muteState=MS_MIN;
	mysink.sinkClassID=3;
	mysink.volume=234;
	list.push_back(mysink);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	EXPECT_CALL(receiver,getListMainSinks(_)).WillOnce(DoAll(SetArgReferee<0>(list),Return(E_OK)));
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	sender.cbNumberOfSinksChanged();
}

void* cbSourceAdded(void*)
{
	sleep(1);
	std::vector<am_SourceType_s> list;
	am_SourceType_s myource;
	myource.name="MySink";
	myource.sourceID=42;
	myource.availability.availability=A_MAX;
	myource.availability.availabilityReason=AR_MIN;
	myource.sourceClassID=15;
	list.push_back(myource);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	EXPECT_CALL(receiver,getListMainSources(_)).WillOnce(DoAll(SetArgReferee<0>(list),Return(E_OK)));
	sender.cbNumberOfSourcesChanged();
}

void* cbSourceRemoved(void*)
{
	sleep(1);
	std::vector<am_SourceType_s> list;
	am_SourceType_s myource;
	myource.name="MySink";
	myource.sourceID=42;
	myource.availability.availability=A_MAX;
	myource.availability.availabilityReason=AR_MIN;
	myource.sourceClassID=15;
	list.push_back(myource);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	backdoor.setListSources(&sender,list);
	DBusError error;
	dbus_error_init(&error);
	list.clear();
	EXPECT_CALL(receiver,getListMainSources(_)).WillOnce(DoAll(SetArgReferee<0>(list),Return(E_OK)));
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	sender.cbNumberOfSourcesChanged();
}

void* cbSinkRemoved(void*)
{
	sleep(1);
	std::vector<am_SinkType_s> list;
	am_SinkType_s mysink;
	mysink.name="MySink";
	mysink.sinkID=23;
	mysink.availability.availability=A_MAX;
	mysink.availability.availabilityReason=AR_MIN;
	mysink.muteState=MS_MIN;
	mysink.sinkClassID=3;
	mysink.volume=234;
	list.push_back(mysink);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	backdoor.setListSinks(&sender,list);
	DBusError error;
	dbus_error_init(&error);
	list.clear();
	EXPECT_CALL(receiver,getListMainSinks(_)).WillOnce(DoAll(SetArgReferee<0>(list),Return(E_OK)));
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	sender.cbNumberOfSinksChanged();
}

void* NumberOfSinkClassesChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	sender.cbNumberOfSinkClassesChanged();
}

void* NumberOfSourceClassesChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	sender.cbNumberOfSourceClassesChanged();
}


void* MainConnectionStateChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	am_connectionID_t id=4;
	am_ConnectionState_e state=CS_CONNECTING;
	sender.cbMainConnectionStateChanged(id,state);
}

void* MainSinkSoundPropertyChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	am_sinkID_t sinkID=3;
	am_MainSoundProperty_s soundProperty;
	soundProperty.type=MSP_TEST;
	soundProperty.value=23;
	sender.cbMainSinkSoundPropertyChanged(sinkID,soundProperty);
}

void* MainSourceSoundPropertyChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	am_sourceID_t sourceID=35;
	am_MainSoundProperty_s soundProperty;
	soundProperty.type=MSP_TEST;
	soundProperty.value=233;
	sender.cbMainSourceSoundPropertyChanged(sourceID,soundProperty);
}

void* cbSinkAvailabilityChangedLoop(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	am_Availability_s av;
	av.availability=A_AVAILABLE;
	av.availabilityReason=AR_UNKNOWN;
	sender.cbSinkAvailabilityChanged(4,av);
}

void* VolumeChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	am_volume_t volume=344;
	sender.cbVolumeChanged(23,volume);
}

void* cbSourceAvailabilityChangedLoop(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	am_Availability_s av;
	av.availability=A_AVAILABLE;
	av.availabilityReason=AR_UNKNOWN;
	sender.cbSourceAvailabilityChanged(2,av);
}

void* SinkMuteStateChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	sender.cbSinkMuteStateChanged(42,MS_MUTED);
}

void* SystemPropertyChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	am_SystemProperty_s property;
	property.type=SYP_TEST;
	property.value=355;
	sender.cbSystemPropertyChanged(property);
}

void* TimingInformationChanged(void*)
{
	sleep(1);
	DbusCommandSender sender;
	MockCommandReceiveInterface receiver;
	DbusCommandInterfaceBackdoor backdoor;
	backdoor.setReceiveInterface(&sender,&receiver);
	DBusError error;
	dbus_error_init(&error);
	DBusConnection* co=dbus_bus_get(DBUS_BUS_SESSION, &error);
	backdoor.setDbusConnection(&sender,co);
	sender.cbTimingInformationChanged(42,233);
}

TEST_F(dbuscommaninterfacesignalsTest,cbSourceAvailabilityChanged)
{

	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_t pythonloop;
	pthread_create(&pythonloop,NULL,NumberOfMainConnectionsChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchsignal(*arg, **kwarg):\n"
			"	print ('Caught NumberOfMainConnectionsChanged') \n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchsignal, signal_name='NumberOfMainConnectionsChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	pthread_create(&pythonloop,NULL,cbSinkAdded,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchSinkAdded(*arg, **karg):\n"
			"	print ('Caught signal (in SinkAdded handler) ') \n"
			"	print (arg[0])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchSinkAdded, signal_name='SinkAdded', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	std::ifstream ifs("/tmp/result.txt");
	std::string line;
	int lineCounter=0, result=0;
	while(std::getline(ifs, line))
	{
		ASSERT_EQ(line.compare("dbus.Struct((dbus.UInt16(23), dbus.String(u'MySink'), dbus.Struct((dbus.Int16(3), dbus.Int16(0)), signature=None), dbus.Int16(234), dbus.Int16(0), dbus.UInt16(3)), signature=None)"),0);
	}
	ifs.close();

	pthread_create(&pythonloop,NULL,cbSinkRemoved,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchSinkRemoved(*arg, **karg):\n"
			"	print ('Caught signal (in SinkRemoved handler) ') \n"
			"	print (arg[0])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchSinkRemoved, signal_name='SinkRemoved', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
    	std::stringstream(line) >> result;
    	ASSERT_EQ(result,23);
	}
	ifs.close();

	pthread_create(&pythonloop,NULL,cbSourceAdded,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchSourceAdded(*arg, **karg):\n"
			"	print ('Caught signal (in SourceAdded handler) ') \n"
			"	print (arg[0])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchSourceAdded, signal_name='SourceAdded', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
		ASSERT_EQ(line.compare("dbus.Struct((dbus.UInt16(42), dbus.String(u'MySink'), dbus.Struct((dbus.Int16(3), dbus.Int16(0)), signature=None), dbus.UInt16(15)), signature=None)"),0);
	}
	ifs.close();


	pthread_create(&pythonloop,NULL,cbSourceRemoved,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchSourceRemoved(*arg, **karg):\n"
			"	print ('Caught signal (in SinkRemoved handler) ') \n"
			"	print (arg[0])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchSourceRemoved, signal_name='SourceRemoved', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
    	std::stringstream(line) >> result;
    	ASSERT_EQ(result,42);
	}
	ifs.close();


	pthread_create(&pythonloop,NULL,NumberOfSinkClassesChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchNumberOfSinkClassesChanged(*arg, **kwarg):\n"
			"	print ('Caught catchNumberOfSinkClassesChanged') \n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchNumberOfSinkClassesChanged, signal_name='NumberOfSinkClassesChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	pthread_create(&pythonloop,NULL,NumberOfSourceClassesChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def CatchNumberOfSourceClassesChanged(*arg, **kwarg):\n"
			"	print ('Caught CatchNumberOfSourceClassesChanged') \n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(CatchNumberOfSourceClassesChanged, signal_name='NumberOfSourceClassesChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);


	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,MainConnectionStateChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchMainConnectionStateChanged(*arg, **karg):\n"
			"	print ('Caught signal (in catchMainConnectionStateChanged handler) ') \n"
			"	print (arg[0])\n"
			"	print (arg[1])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchMainConnectionStateChanged, signal_name='MainConnectionStateChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
	    if(lineCounter==0)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,4);
	    }
	    else if (lineCounter==1)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,CS_CONNECTING);
	    }
	    lineCounter++;
	}
	ifs.close();


	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,MainSinkSoundPropertyChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchMainSinkSoundPropertyChanged(*arg, **karg):\n"
			"	print ('Caught signal (in catchMainSinkSoundPropertyChanged handler) ') \n"
			"	print (arg[0])\n"
			"	print (arg[1])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchMainSinkSoundPropertyChanged, signal_name='MainSinkSoundPropertyChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
	    if(lineCounter==0)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,3);
	    }
	    else if (lineCounter==1)
	    {
	    	ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(1), dbus.Int16(23)), signature=None)"),0);
	    }
	    lineCounter++;
	}
	ifs.close();

	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,MainSourceSoundPropertyChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchMainSourceSoundPropertyChanged(*arg, **karg):\n"
			"	print ('Caught signal (in catchMainSourceSoundPropertyChanged handler) ') \n"
			"	print (arg[0])\n"
			"	print (arg[1])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchMainSourceSoundPropertyChanged, signal_name='MainSourceSoundPropertyChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
	    if(lineCounter==0)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,35);
	    }
	    else if (lineCounter==1)
	    {
	    	ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(1), dbus.Int16(233)), signature=None)"),0);
	    }
	    lineCounter++;
	}
	ifs.close();


	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,cbSinkAvailabilityChangedLoop,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchSinkAvailabilityChanged(*arg, **karg):\n"
			"	print ('Caught signal (in catchSinkAvailabilityChanged handler) ') \n"
			"	print (arg[0])\n"
			"	print (arg[1])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchSinkAvailabilityChanged, signal_name='SinkAvailabilityChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
	    if(lineCounter==0)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,4);
	    }
	    else if (lineCounter==1)
	    {
	    	ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(0), dbus.Int16(3)), signature=None)"),0);
	    }
	    lineCounter++;
	}
	ifs.close();


	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,cbSourceAvailabilityChangedLoop,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchSourceAvailability(*arg, **karg):\n"
			"	print ('Caught signal (in catchSourceAvailability handler) ') \n"
			"	print (arg[0])\n"
			"	print (arg[1])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchSourceAvailability, signal_name='SourceAvailabilityChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
	    if(lineCounter==0)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,2);
	    }
	    else if (lineCounter==1)
	    {
	    	ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(0), dbus.Int16(3)), signature=None)"),0);
	    }
	    lineCounter++;
	}
	ifs.close();


	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,VolumeChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchVolumeChanged(*arg, **karg):\n"
			"	print ('Caught signal (in catchVolumeChanged handler) ') \n"
			"	print (arg[0])\n"
			"	print (arg[1])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchVolumeChanged, signal_name='VolumeChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
	    if(lineCounter==0)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,23);
	    }
	    else if (lineCounter==1)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,344);
	    }
	    lineCounter++;
	}
	ifs.close();


	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,SinkMuteStateChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchSinkMuteStateChanged(*arg, **karg):\n"
			"	print ('Caught signal (in catchSinkMuteStateChanged handler) ') \n"
			"	print (arg[0])\n"
			"	print (arg[1])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchSinkMuteStateChanged, signal_name='SinkMuteStateChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
	    if(lineCounter==0)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,42);
	    }
	    else if (lineCounter==1)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,MS_MUTED);
	    }
	    lineCounter++;
	}
	ifs.close();

	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,SystemPropertyChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchSystemPropertyChanged(*arg, **karg):\n"
			"	print ('Caught signal (in catchSystemPropertyChanged handler) ') \n"
			"	print (arg[0])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchSystemPropertyChanged, signal_name='SystemPropertyChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
		ASSERT_EQ(line.compare("dbus.Struct((dbus.Int16(0), dbus.Int16(355)), signature=None)"),0);
	}
	ifs.close();

	//ok, now we want to test the signals. It did not work out that the python receiver worked in an own thread (as originally intended)
	//so it is running in the main context and the signals are send from threads...
	pthread_create(&pythonloop,NULL,TimingInformationChanged,(void*)NULL);
	PyRun_SimpleStringFlags(
			"import sys\n"
			"import traceback\n"
			"import gobject\n"
			"import dbus\n"
			"import dbus.mainloop.glib\n"
			"loop = gobject.MainLoop()\n"
			"def catchTimingInformationChanged(*arg, **karg):\n"
			"	print ('Caught signal (in catchTimingInformationChanged handler) ') \n"
			"	print (arg[0])\n"
			"	print (arg[1])\n"
			"	f = open('/tmp/result.txt','w')\n"
			"	f.write(str(arg[0]) + '\\n' + str (arg[1]));\n"
			"	f.close()\n"
			"	loop.quit()\n"
			"dbus.mainloop.glib.DBusGMainLoop(set_as_default=True)\n"
			"bus = dbus.SessionBus()\n"
			"bus.add_signal_receiver(catchTimingInformationChanged, signal_name='TimingInformationChanged', dbus_interface = 'org.genivi.audiomanager', message_keyword='dbus_message')\n"
			"loop.run()\n",NULL);
	pthread_join(pythonloop,NULL);

	ifs.open("/tmp/result.txt");
	lineCounter=0;
	result=0;
	while(std::getline(ifs, line))
	{
	    if(lineCounter==0)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,42);
	    }
	    else if (lineCounter==1)
	    {
	    	std::stringstream(line) >> result;
	    	ASSERT_EQ(result,233);
	    }
	    lineCounter++;
	}
	ifs.close();

}



int main(int argc, char **argv)
{
	::testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}

void dbuscommaninterfacesignalsTest::SetUp()
{
	Py_Initialize();
}



void dbuscommaninterfacesignalsTest::TearDown()
{
	Py_Finalize();
}





