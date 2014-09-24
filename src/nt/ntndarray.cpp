/* ntndarray.cpp */
/**
 * Copyright - See the COPYRIGHT that is included with this distribution.
 * EPICS pvDataCPP is distributed subject to a Software License Agreement found
 * in file LICENSE that is included with this distribution.
 */

#include <algorithm>

#include <pv/ntndarray.h>

using namespace std;
using namespace epics::pvData;

namespace epics { namespace nt {

static NTFieldPtr ntField = NTField::get();

namespace detail {

const std::string ntAttrStr("uri:ev4:nt/2014/pwd:NTAttribute");

static FieldCreatePtr fieldCreate = getFieldCreate();
static PVDataCreatePtr pvDataCreate = getPVDataCreate();

StructureConstPtr NTNDArrayBuilder::createStructure()
{
    StandardFieldPtr standardField = getStandardField();
    FieldBuilderPtr fb = fieldCreate->createFieldBuilder();

    for (int i = pvBoolean; i < pvString; ++i)
    {
        ScalarType st = static_cast<ScalarType>(i);
        fb->addArray(std::string(ScalarTypeFunc::name(st)) + "Value", st);
    }
    UnionConstPtr valueType = fb->createUnion();                

    StructureConstPtr codecStruc = fb->setId("codec_t")->
        add("name", pvString)->
        add("parameters", fieldCreate->createVariantUnion())->
        createStructure();

    StructureConstPtr dimensionStruc = fb->setId("dimension_t")->
        add("size", pvInt)->
        add("offset", pvInt)->
        add("fullSize", pvInt)->
        add("binning", pvInt)->
        add("reverse", pvBoolean)->
        createStructure();

    StructureConstPtr attributeStruc = fb->setId(ntAttrStr)->
        add("name", pvString)->
        add("value", fieldCreate->createVariantUnion())->
        add("descriptor", pvString)->
        add("sourceType", pvInt)->
        add("source", pvString)->
       createStructure();

    fb->setId(NTNDArray::URI)->
        add("value", valueType)->
        add("codec", codecStruc)->
        add("compressedSize", pvLong)->
        add("uncompressedSize", pvLong)->
        addArray("dimension", dimensionStruc)->
        add("uniqueId", pvInt)->
        add("dataTimeStamp", standardField->timeStamp())->
        addArray("attribute", attributeStruc);

    if (descriptor)
        fb->add("descriptor", pvString);

    if (timeStamp)
        fb->add("timeStamp", standardField->timeStamp());

    if (alarm)
        fb->add("alarm", standardField->alarm());

    if (display)
        fb->add("display", standardField->display());

    size_t extraCount = extraFieldNames.size();
    for (size_t i = 0; i< extraCount; i++)
        fb->add(extraFieldNames[i], extraFields[i]);

    return fb->createStructure();
}

NTNDArrayBuilder::shared_pointer NTNDArrayBuilder::addDescriptor()
{
    descriptor = true;
    return shared_from_this();
}

NTNDArrayBuilder::shared_pointer NTNDArrayBuilder::addAlarm()
{
    alarm = true;
    return shared_from_this();
}

NTNDArrayBuilder::shared_pointer NTNDArrayBuilder::addTimeStamp()
{
    timeStamp = true;
    return shared_from_this();
}

NTNDArrayBuilder::shared_pointer NTNDArrayBuilder::addDisplay()
{
    display = true;
    return shared_from_this();
}

PVStructurePtr NTNDArrayBuilder::createPVStructure()
{
    return getPVDataCreate()->createPVStructure(createStructure());
}

NTNDArrayPtr NTNDArrayBuilder::create()
{
    return NTNDArrayPtr(new NTNDArray(createPVStructure()));
}

NTNDArrayBuilder::NTNDArrayBuilder()
{
    reset();
}

void NTNDArrayBuilder::reset()
{
    descriptor = false;
    timeStamp = false;
    alarm = false;
    display = false;
    extraFieldNames.clear();
    extraFields.clear();
}

NTNDArrayBuilder::shared_pointer NTNDArrayBuilder::add(string const & name, FieldConstPtr const & field)
{
    extraFields.push_back(field); extraFieldNames.push_back(name);
    return shared_from_this();
}


}

const std::string NTNDArray::URI("uri:ev4:nt/2014/pwd:NTNDArray");
const std::string ntAttrStr("uri:ev4:nt/2014/pwd:NTAttribute");

static FieldCreatePtr fieldCreate = getFieldCreate();
static PVDataCreatePtr pvDataCreate = getPVDataCreate();

NTNDArray::shared_pointer NTNDArray::narrow(PVStructurePtr const & structure)
{
    if (!structure || !is_a(structure->getStructure()))
        return shared_pointer();

    return narrow_unsafe(structure);
}

NTNDArray::shared_pointer NTNDArray::narrow_unsafe(PVStructurePtr const & structure)
{
    return shared_pointer(new NTNDArray(structure));
}

bool NTNDArray::is_a(StructureConstPtr const & structure)
{
    return structure->getID() == URI;
}

bool NTNDArray::is_compatible(PVStructurePtr const & pvStructure)
{
    PVUnionPtr pvValue = pvStructure->getSubField<PVUnion>("value");
    if(!pvValue) return false;
    PVFieldPtr pvField = pvStructure->getSubField("descriptor");
    if(pvField && !pvStructure->getSubField<PVString>("descriptor")) return false;
    pvField = pvStructure->getSubField("alarm");
    if(pvField && !ntField->isAlarm(pvField->getField())) return false;
    pvField = pvStructure->getSubField("timeStamp");
    if(pvField && !ntField->isTimeStamp(pvField->getField())) return false;
    pvField = pvStructure->getSubField("display");
    if(pvField && !ntField->isDisplay(pvField->getField())) return false;
    if(!pvStructure->getSubField<PVLong>("compressedSize")) return false;
    if(!pvStructure->getSubField<PVLong>("uncompressedSize")) return false;
    PVStructurePtr pvCodec = pvStructure->getSubField<PVStructure>("codec");
    if(!pvCodec) return false;
    if(!pvCodec->getSubField<PVString>("name")) return false;
    if(!pvCodec->getSubField<PVUnion>("parameters")) return false;
    PVStructureArrayPtr pvDimension = pvStructure->getSubField<PVStructureArray>("dimension");
    if(pvDimension->getStructureArray()->getStructure()->getID().compare("dimension_t")!=0) return false;
    if(!pvStructure->getSubField<PVInt>("uniqueId")) return false;
    pvField = pvStructure->getSubField("dataTimeStamp");
    if(pvField && !ntField->isTimeStamp(pvField->getField())) return false;
    PVStructureArrayPtr pvAttribute = pvStructure->getSubField<PVStructureArray>("attribute");
    if(!pvAttribute->getStructureArray()->getStructure()->getID().compare("ntAttrStr")!=0) return false;
    return true;
}


NTNDArrayBuilderPtr NTNDArray::createBuilder()
{
    return NTNDArrayBuilderPtr(new detail::NTNDArrayBuilder());
}


bool NTNDArray::attachTimeStamp(PVTimeStamp &pvTimeStamp) const
{
    PVStructurePtr ts = getTimeStamp();
    if (ts)
        return pvTimeStamp.attach(ts);
    else
        return false;
}

bool NTNDArray::attachDataTimeStamp(PVTimeStamp &pvTimeStamp) const
{
    PVStructurePtr ts = getDataTimeStamp();
    if (ts)
        return pvTimeStamp.attach(ts);
    else
        return false;
}

bool NTNDArray::attachAlarm(PVAlarm &pvAlarm) const
{
    PVStructurePtr al = getAlarm();
    if (al)
        return pvAlarm.attach(al);
    else
        return false;
}

PVStructurePtr NTNDArray::getPVStructure() const
{
    return pvNTNDArray;
}

PVUnionPtr NTNDArray::getValue() const
{
    return pvNTNDArray->getSubField<PVUnion>("value");
}

PVStructurePtr NTNDArray::getCodec() const
{
    return pvNTNDArray->getSubField<PVStructure>("codec");
}

PVLongPtr NTNDArray::getCompressedDataSize() const
{
    return pvNTNDArray->getSubField<PVLong>("compressedSize");
}

PVLongPtr NTNDArray::getUncompressedDataSize() const
{
    return pvNTNDArray->getSubField<PVLong>("uncompressedSize");
}

PVStructureArrayPtr NTNDArray::getAttribute() const
{
    return pvNTNDArray->getSubField<PVStructureArray>("attribute");
}

PVStructureArrayPtr NTNDArray::getDimension() const
{
    return pvNTNDArray->getSubField<PVStructureArray>("dimension");
}

PVStructurePtr NTNDArray::getDataTimeStamp() const
{
    return pvNTNDArray->getSubField<PVStructure>("dataTimeStamp");
}

PVStringPtr NTNDArray::getDescriptor() const
{
    return pvNTNDArray->getSubField<PVString>("descriptor");
}

PVStructurePtr NTNDArray::getTimeStamp() const
{
    return pvNTNDArray->getSubField<PVStructure>("timeStamp");
}

PVStructurePtr NTNDArray::getAlarm() const
{
    return pvNTNDArray->getSubField<PVStructure>("alarm");
}

PVStructurePtr NTNDArray::getDisplay() const
{
    return pvNTNDArray->getSubField<PVStructure>("display");
}


NTNDArray::NTNDArray(PVStructurePtr const & pvStructure) :
    pvNTNDArray(pvStructure)
{}


}}
