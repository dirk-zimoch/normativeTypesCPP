/* ntscalarMultiChannel.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvDataCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#include <algorithm>

#define epicsExportSharedSymbols
#include <pv/ntscalarMultiChannel.h>
#include <pv/ntutils.h>

using namespace std;
using namespace epics::pvData;

namespace epics { namespace nt { 


static FieldCreatePtr fieldCreate = getFieldCreate();
static PVDataCreatePtr pvDataCreate = getPVDataCreate();
static NTFieldPtr ntField = NTField::get();

namespace detail {

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::value(ScalarType scalarType)
{
    valueType = scalarType;
    return shared_from_this();
}


NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addDescriptor()
{
    descriptor = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addAlarm()
{
    alarm = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addTimeStamp()
{
    timeStamp = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addSeverity()
{
    severity = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addStatus()
{
    status = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addMessage()
{
    message = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addSecondsPastEpoch()
{
    secondsPastEpoch = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addNanoseconds()
{
    nanoseconds = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addUserTag()
{
    userTag = true;
    return shared_from_this();
}

NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::addIsConnected()
{
    isConnected = true;
    return shared_from_this();
}

StructureConstPtr NTScalarMultiChannelBuilder::createStructure()
{
    StandardFieldPtr standardField = getStandardField();
    size_t nfields = 2;
    size_t extraCount = extraFieldNames.size();
    nfields += extraCount;
    if(descriptor) ++nfields;
    if(alarm) ++nfields;
    if(timeStamp) ++nfields;
    if(severity) ++nfields;
    if(status) ++nfields;
    if(message) ++nfields;
    if(secondsPastEpoch) ++nfields;
    if(nanoseconds) ++nfields;
    if(userTag) ++nfields;
    if(isConnected) ++nfields;
    FieldConstPtrArray fields(nfields);
    StringArray names(nfields);
    size_t ind = 0;
    names[ind] = "value";
    fields[ind++] =  fieldCreate->createScalarArray(valueType);
    names[ind] = "channelName";
    fields[ind++] =  fieldCreate->createScalarArray(pvString);
    if(descriptor) {
        names[ind] = "descriptor";
        fields[ind++] = fieldCreate->createScalar(pvString);
    }
    if(alarm) {
        names[ind] = "alarm";
        fields[ind++] = standardField->alarm();
    }
    if(timeStamp) {
        names[ind] = "timeStamp";
        fields[ind++] = standardField->timeStamp();
    }
    if(severity) {
        names[ind] = "severity";
        fields[ind++] = fieldCreate->createScalarArray(pvInt);
    }
    if(status) {
        names[ind] = "status";
        fields[ind++] = fieldCreate->createScalarArray(pvInt);
    }
    if(message) {
        names[ind] = "message";
        fields[ind++] = fieldCreate->createScalarArray(pvString);
    }
    if(secondsPastEpoch) {
        names[ind] = "secondsPastEpoch";
        fields[ind++] = fieldCreate->createScalarArray(pvLong);
    }
    if(nanoseconds) {
        names[ind] = "nanoseconds";
        fields[ind++] = fieldCreate->createScalarArray(pvInt);
    }
    if(userTag) {
        names[ind] = "userTag";
        fields[ind++] = fieldCreate->createScalarArray(pvInt);
    }
    if(isConnected) {
        names[ind] = "isConnected";
        fields[ind++] =  fieldCreate->createScalarArray(pvBoolean);
    }
    for (size_t i = 0; i< extraCount; i++) {
        names[ind] = extraFieldNames[i];
        fields[ind++] = extraFields[i];
    }

    StructureConstPtr st = fieldCreate->createStructure(NTScalarMultiChannel::URI,names,fields);
    reset();
    return st;
}

PVStructurePtr NTScalarMultiChannelBuilder::createPVStructure()
{
    return pvDataCreate->createPVStructure(createStructure());
}

NTScalarMultiChannelPtr NTScalarMultiChannelBuilder::create()
{
    return NTScalarMultiChannelPtr(new NTScalarMultiChannel(createPVStructure()));
}

NTScalarMultiChannelBuilder::NTScalarMultiChannelBuilder()
: valueType(pvDouble)
{
    reset();
}

void NTScalarMultiChannelBuilder::reset()
{
    extraFieldNames.clear();
    extraFields.clear();
    valueType = pvDouble;
    descriptor = false;
    alarm = false;
    timeStamp = false;
    severity = false;
    status = false;
    message = false;
    secondsPastEpoch = false;
    nanoseconds = false;
    userTag = false;
    // TODO When client code updated, don't include isConnected by default
    // i.e. change line below to isConnected = false;
    isConnected = true;
}


NTScalarMultiChannelBuilder::shared_pointer NTScalarMultiChannelBuilder::add(string const & name, FieldConstPtr const & field)
{
    extraFields.push_back(field); extraFieldNames.push_back(name);
    return shared_from_this();
}

}

const std::string NTScalarMultiChannel::URI("epics:nt/NTScalarMultiChannel:1.0");

NTScalarMultiChannel::shared_pointer NTScalarMultiChannel::wrap(PVStructurePtr const & structure)
{
    if(!isCompatible(structure)) return shared_pointer();
    return wrapUnsafe(structure);
}

NTScalarMultiChannel::shared_pointer NTScalarMultiChannel::wrapUnsafe(PVStructurePtr const & structure)
{
    return shared_pointer(new NTScalarMultiChannel(structure));
}

bool NTScalarMultiChannel::is_a(StructureConstPtr const &structure)
{
    return NTUtils::is_a(structure->getID(), URI);
}


bool NTScalarMultiChannel::isCompatible(StructureConstPtr const & structure)
{
    if (!structure.get()) return false;

    ScalarArrayConstPtr valueField = structure->getField<ScalarArray>("value");
    if (!valueField.get()) return false;

    ScalarArrayConstPtr channelNameField = structure->getField<ScalarArray>(
        "channelName");
    if (!channelNameField.get()) return false;
    if (channelNameField->getElementType() != pvString) return false;

    FieldConstPtr field = structure->getField("severity");
    if (field.get())
    {
        ScalarArrayConstPtr severityField = structure->getField<ScalarArray>("severity");
        if (!severityField.get() || severityField->getElementType() != pvInt)
            return false;
    }

    field = structure->getField("status");
    if (field.get())
    {
        ScalarArrayConstPtr statusField = structure->getField<ScalarArray>("status");
        if (!statusField.get() || statusField->getElementType() != pvInt)
            return false;
    }

    field = structure->getField("message");
    if (field.get())
    {
        ScalarArrayConstPtr messageField = structure->getField<ScalarArray>("message");
        if (!messageField.get() || messageField->getElementType() != pvString)
           return false;
    }

    field = structure->getField("secondsPastEpoch");
    if (field.get())
    {
        ScalarArrayConstPtr secondsPastEpochField = structure->getField<ScalarArray>("secondsPastEpoch");
        if (!secondsPastEpochField.get() || secondsPastEpochField->getElementType() != pvLong)
            return false;
    }

    field = structure->getField("nanoseconds");
    if (field.get())
    {
        ScalarArrayConstPtr nanosecondsField = structure->getField<ScalarArray>("nanoseconds");
        if (!nanosecondsField.get() || nanosecondsField->getElementType() != pvInt)
            return false;
    }

    field = structure->getField("userTag");
    if (field.get())
    {
        ScalarArrayConstPtr userTagField = structure->getField<ScalarArray>("userTag");
        if (!userTagField.get() || userTagField->getElementType() != pvInt)
            return false;
    }

    field = structure->getField("descriptor");
    if (field.get())
    {
        ScalarConstPtr descriptorField = structure->getField<Scalar>("descriptor");
        if (!descriptorField.get() || descriptorField->getScalarType() != pvString)
            return false;
    }

    NTFieldPtr ntField = NTField::get();

    field = structure->getField("alarm");
    if (field.get() && !ntField->isAlarm(field))
        return false;

    field = structure->getField("timeStamp");
    if (field.get() && !ntField->isTimeStamp(field))
        return false;

    return true;
}


bool NTScalarMultiChannel::isCompatible(PVStructurePtr const &pvStructure)
{
    if(!pvStructure.get()) return false;

    return isCompatible(pvStructure->getStructure());
}

NTScalarMultiChannelBuilderPtr NTScalarMultiChannel::createBuilder()
{
    return NTScalarMultiChannelBuilderPtr(new detail::NTScalarMultiChannelBuilder());
}


NTScalarMultiChannel::NTScalarMultiChannel(PVStructurePtr const & pvStructure)
: pvNTScalarMultiChannel(pvStructure),
  pvTimeStamp(pvStructure->getSubField<PVStructure>("timeStamp")),
  pvAlarm(pvStructure->getSubField<PVStructure>("alarm")),
  pvValue(pvStructure->getSubField<PVScalarArray>("value")),
  pvChannelName(pvStructure->getSubField<PVStringArray>("channelName")),
  pvIsConnected(pvStructure->getSubField<PVBooleanArray>("isConnected")),
  pvSeverity(pvStructure->getSubField<PVIntArray>("severity")),
  pvStatus(pvStructure->getSubField<PVIntArray>("status")),
  pvMessage(pvStructure->getSubField<PVStringArray>("message")),
  pvSecondsPastEpoch(pvStructure->getSubField<PVLongArray>("secondsPastEpoch")),
  pvNanoseconds(pvStructure->getSubField<PVIntArray>("nanoseconds")),
  pvUserTag(pvStructure->getSubField<PVIntArray>("userTag")),
  pvDescriptor(pvStructure->getSubField<PVString>("descriptor"))
{
}


bool  NTScalarMultiChannel::attachTimeStamp(PVTimeStamp &pv) const
{
    if (pvTimeStamp)
        return pv.attach(pvTimeStamp);
    else
        return false;
}

bool  NTScalarMultiChannel::attachAlarm(PVAlarm &pv) const
{
    if (pvAlarm)
        return pv.attach(pvAlarm);
    else
        return false;
}

}}
