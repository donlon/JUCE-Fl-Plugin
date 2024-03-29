#pragma once

#include <JuceHeader.h>

#include <iostream>
#include <vector>
#include <map>
#include <ObjIdlbase.h>

class Param;

class TCPPFruityPlug;

struct IStream;

class ParamManager {
    static const int version = 0;

    std::vector<Param *> paramList;
    std::map<Component *, Param *> paramComponentMap;
    std::map<int, Param *> paramIdMap;
    TCPPFruityPlug *fruityPlug;

    bool isPreviousVersionOutOfDate = false;

public:
    ParamManager(TCPPFruityPlug *fruityPlug);

    ~ParamManager();

    Param &addParam(int id, const char *name, float defaultValue);

    void bindControllerComponent(int id, Component *component);

    void bindControllerComponent(int id, Slider *slider);

    Param &getParamByPosition(int pos);

    Param *getParamById(int id);

    Param *getParam(Component *index);

    float getValue(int index);

    void setValue(Param &param, float value, int isActive = true);

    void setValue(int index, float value, int isActive = true);

    void notifyHostValueChanged(int index, float value);

    std::vector<Param *> &getParamList();

    void saveToStream(IStream &stream);

    void loadFromStream(IStream &stream);
};
