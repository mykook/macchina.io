//
// TriggerRemoteObject.cpp
//
// Library: IoT/Devices
// Package: Generated
// Module:  TriggerRemoteObject
//
// This file has been generated.
// Warning: All changes to this will be lost when the file is re-generated.
//
// Copyright (c) 2014-2015, Applied Informatics Software Engineering GmbH.
// All rights reserved.
// 
// SPDX-License-Identifier: Apache-2.0
//


#include "IoT/Devices/TriggerRemoteObject.h"
#include "IoT/Devices/TriggerEventDispatcher.h"
#include "Poco/Delegate.h"
#include "Poco/RemotingNG/ORB.h"


namespace IoT {
namespace Devices {


TriggerRemoteObject::TriggerRemoteObject(const Poco::RemotingNG::Identifiable::ObjectId& oid, Poco::SharedPtr<IoT::Devices::Trigger> pServiceObject):
	IoT::Devices::ITrigger(),
	Poco::RemotingNG::RemoteObject(oid),
	_pServiceObject(pServiceObject)
{
	_pServiceObject->stateChanged += Poco::delegate(this, &TriggerRemoteObject::event__stateChanged);
}


TriggerRemoteObject::~TriggerRemoteObject()
{
	try
	{
		_pServiceObject->stateChanged -= Poco::delegate(this, &TriggerRemoteObject::event__stateChanged);
	}
	catch (...)
	{
		poco_unexpected();
	}
}


void TriggerRemoteObject::remoting__enableEvents(Poco::RemotingNG::Listener::Ptr pListener, bool enable)
{
}


void TriggerRemoteObject::remoting__enableRemoteEvents(const std::string& protocol)
{
	Poco::RemotingNG::EventDispatcher::Ptr pEventDispatcher = new TriggerEventDispatcher(this, protocol);
	Poco::RemotingNG::ORB::instance().registerEventDispatcher(remoting__getURI().toString(), pEventDispatcher);
}


bool TriggerRemoteObject::remoting__hasEvents() const
{
	return true;
}


void TriggerRemoteObject::event__stateChanged(const bool& data)
{
	stateChanged(this, data);
}


} // namespace Devices
} // namespace IoT

