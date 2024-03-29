#include <fp_plugclass.h>
#include <cassert>

#include "PluginInterface.h"
#include "PluginInterfaceSEHWrapper.h"
#include "plugin/Plugin.h"
#include "interface/Param.h"
#include "interface/Utils.h"
#include "ui/utils/Console.h"

#define CONSOLE_DEBUGGING

TFruityPlug *_stdcall CreatePlugInstance(TFruityPlugHost *Host, int Tag) {
#ifdef CONSOLE_DEBUGGING
    Console::allocateConsole();
#endif

#ifdef _DEBUG
    return new PluginInterfaceSEHWrapper(Tag, Host);
#else
    return new FruityPluginInterface(Tag, Host);
#endif
}

// the information structure describing this plugin to the host
TFruityPlugInfo PlugInfo = {
        CurrentSDKVersion,
        "Fl Plugin",
        "Fl Plugin Test",
        FPF_Type_Effect,
        0/*NumParams*/,
        0  // infinite
};

FruityPluginInterface::FruityPluginInterface(int Tag, TFruityPlugHost *Host)
        : TCPPFruityPlug(Tag, Host, nullptr), paramManager(dynamic_cast<TCPPFruityPlug *>(this)) {
    juce::initialiseJuce_GUI();
    Process::setCurrentModuleInstanceHandle(HInstance);

    createPlugin();
    //ResetParams();
    Info = &PlugInfo;
    PlugInfo.NumParams = (int) paramManager.getParamList().size();
}

void FruityPluginInterface::createPlugin() {
    plugin = new Plugin;
    plugin->plugin = this;
    plugin->onCreateParams(paramManager);
    pluginGui = plugin->getGUI();
}

void __stdcall FruityPluginInterface::DestroyObject() {
    std::cout << "Plugin destroyed." << std::endl;
    juce::shutdownJuce_GUI();
    delete plugin;
#ifdef CONSOLE_DEBUGGING
    Console::freeConsole();
#endif
}

intptr_t __stdcall FruityPluginInterface::Dispatcher(intptr_t ID, intptr_t Index, intptr_t Value) {
    Utils::traceDispatchLog(ID, Index, Value);

    switch (ID) {
        // show the editor
        case FPD_ShowEditor : {
            if (!pluginGui) {
                break;
            }
            if (Value == 0) {
                pluginGui->hideEditor();
            } else {
                pluginGui->showEditor((void *) Value);
            }

            EditorHandle = (HWND) pluginGui->getWindowHandle();
            break;
        }
        default:
            break;
    }

    return 0;
}

void __stdcall FruityPluginInterface::Eff_Render(PWAV32FS SourceBuffer, PWAV32FS DestBuffer, int Length) {
    memcpy(DestBuffer, SourceBuffer, Length * sizeof(TWAV32FS));
}

void __stdcall FruityPluginInterface::GetName(int Section, int Index, int Value, char *Name) {
    if (Section == FPN_Param) {
        Param &param = paramManager.getParamByPosition(Index);
        strcpy_s(Name, 256, param.name.c_str());
    } else if (Section == FPN_ParamValue) {
        Param &param = paramManager.getParamByPosition(Index);
        std::string &str = param.desc;
        strcpy_s(Name, 256, str.c_str());
    }
}

void FruityPluginInterface::Idle() {
    std::string hint = plugin->getHint();
    strcpy_s(AppHint, 256, hint.c_str());
    PlugHost->OnHint(HostTag, AppHint);
}

int __stdcall FruityPluginInterface::ProcessParam(int index, int value, int recFlags) {
    Utils::traceProcessParamLog(index, value, recFlags);

    Param &param = paramManager.getParamByPosition(index);
    if (recFlags & REC_GetValue) {
        return (int) (100.f * param.value);
    }

    float val;
    // translate from 0..65536 to the parameter's range
    if (recFlags & REC_FromMIDI) {
        val = (float) value / 65536.f;
    } else {
        val = (float) value / 100.f;
    }

    if (recFlags & REC_UpdateValue) {
        paramManager.setValue(param, val, false);

        // if the parameter value has changed,
        // then we notify the host that the controller has changed
        // (!) beware of messages that are sent by (other ?) internal controllers
        // (!) convert the value from its own range to 0..65536
        ///if ((recFlags & REC_InternalCtrl) == 0) {
        ///    DebugBreak();
        ///    PlugHost->OnControllerChanged(HostTag, index, value);
        ///}
    }

    // update the parameter control's value
    if (recFlags & REC_UpdateControl) {
        paramManager.setValue(param, val, false);
    }

    // we show the parameter value as a hint
    if (recFlags & REC_ShowHint) {
        char *hint = nullptr;
        if (param.valueFormatter) {
            hint = param.valueFormatter(val).data();
        } else {
            std::string hint_str = std::to_string((int) (val * 100.)) + '%';
            hint = hint_str.data();
        }
        PlugHost->OnHint(HostTag, hint);
    }

    // make sure we return the value
    return value;
}

void FruityPluginInterface::ResetParams() {
    for (Param *param : paramManager.getParamList()) {
        assert(param != nullptr);
        param->paramSetter(param->defaultValue);
    }
}

void __stdcall FruityPluginInterface::SaveRestoreState(IStream *Stream, BOOL Save) {
    if (Save) {
        paramManager.saveToStream(*Stream);
    } else {
        paramManager.loadFromStream(*Stream);
    }
}
