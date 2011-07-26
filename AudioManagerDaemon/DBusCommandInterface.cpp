/**
 * Copyright (C) 2011, BMW AG
 *
 * AudioManangerDeamon
 *
 * \file DBusCommandInterface.cpp
 *
 * \date 20.05.2011
 * \author Christian Müller (christian.ei.mueller@bmw.de)
 *
 * \section License
 * GNU Lesser General Public License, version 2.1, with special exception (GENIVI clause)
 * Copyright (C) 2011, BMW AG – Christian Müller  Christian.ei.mueller@bmw.de
 *
 * This program is free software; you can redistribute it and/or modify it under the terms of the GNU Lesser General Public License, version 2.1, as published by the Free Software Foundation.
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License, version 2.1, for more details.
 * You should have received a copy of the GNU Lesser General Public License, version 2.1, along with this program; if not, see <http://www.gnu.org/licenses/lgpl-2.1.html>.
 * Note that the copyright holders assume that the GNU Lesser General Public License, version 2.1, may also be applicable to programs even in cases in which the program is not a library in the technical sense.
 * Linking AudioManager statically or dynamically with other modules is making a combined work based on AudioManager. You may license such other modules under the GNU Lesser General Public License, version 2.1. If you do not want to license your linked modules under the GNU Lesser General Public License, version 2.1, you may use the program under the following exception.
 * As a special exception, the copyright holders of AudioManager give you permission to combine AudioManager with software programs or libraries that are released under any license unless such a combination is not permitted by the license of such a software program or library. You may copy and distribute such a system following the terms of the GNU Lesser General Public License, version 2.1, including this special exception, for AudioManager and the licenses of the other code concerned.
 * Note that people who make modified versions of AudioManager are not obligated to grant this special exception for their modified versions; it is their choice whether to do so. The GNU Lesser General Public License, version 2.1, gives permission to release a modified version without this exception; this exception also makes it possible to release a modified version which carries forward this exception.
 *
 *
 */

#include "DBusCommandInterface.h"

CommandInterface::CommandInterface() {
}

void CommandInterface::startupInterface() {
	DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Startup of DBUS Command interface"));

//	new DBusCommand(this);
//	QDBusConnection connection = QDBusConnection::sessionBus();
//	connection.registerService(QString(SERVICEINTERFACE));
//	if (connection.isConnected()) {
//		if (connection.registerObject(
//				"/Control",
//				this,
//				(QDBusConnection::ExportAdaptors
//						| QDBusConnection::ExportAllSignals))) {
//			DLT_LOG(AudioManager,DLT_LOG_INFO, DLT_STRING("Registered DBUS Command interface succsessfully"));
//		} else {
//			DLT_LOG(AudioManager,DLT_LOG_ERROR, DLT_STRING("Failed to register DBUS Command interface succsessfully"));
//		}
//	}
//
}

int CommandInterface::connect(int Source_ID, int Sink_ID) {

}

int CommandInterface::disconnect(int Source_ID, int Sink_ID) {

}

int CommandInterface::interruptRequest(const std::string &SourceName,

}

int CommandInterface::interruptResume(int InterruptID) {
//	emit signal_interruptResume(InterruptID);
	return 1;
}

void CommandInterface::registerAudioManagerCore(AudioManagerCore* core) {
	m_core = core;
//	QObject::connect((const QObject*) m_core,
//			SIGNAL(signal_connectionChanged()), (const QObject*) this,
//			SLOT(slot_connectionChanged()));
//	QObject::connect((const QObject*) m_core,
//			SIGNAL(signal_numberOfSinksChanged()), (const QObject*) this,
//			SLOT(slot_numberOfSinksChanged()));
//	QObject::connect((const QObject*) m_core,
//			SIGNAL(signal_numberOfSourcesChanged()), (const QObject*) this,
//			SLOT(slot_numberOfSourcesChanged()));
}

void CommandInterface::slot_connectionChanged() {
//	emit signal_connectionChanged();
}
void CommandInterface::slot_numberOfSinksChanged() {
//	emit signal_numberOfSinksChanged();
}
void CommandInterface::slot_numberOfSourcesChanged() {
//	emit signal_numberOfSourcesChanged();
}

int CommandInterface::setVolume(int SinkID, int Volume) {

}

std::list<SourceType> CommandInterface::getListSources() {

}

std::list<SinkType> CommandInterface::getListSinks() {
	return m_core->getListSinks();
}

std::list<ConnectionType> CommandInterface::getListConnections() {
	return m_core->getListConnections();
}
