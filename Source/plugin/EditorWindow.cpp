#include <fp_cplug.h>

#include "../interface/ParamManager.h"
#include "../ui/SliderEx.h"
#include "EditorWindow.h"
#include "Plugin.h"

//==============================================================================
EditorWindow::EditorWindow() {
    brightnessSlider = std::make_unique<SliderEx>(Slider::LinearVertical, Slider::NoTextBox);
    brightnessSlider->setRange(.0, 100., 0);
    addAndMakeVisible(brightnessSlider.get());
}

EditorWindow::~EditorWindow() {
    brightnessSlider = nullptr;
}

void EditorWindow::showEditor(void *hwnd) {
    setVisible(true);
    setOpaque(true);
    setSize(800, 600);
    addToDesktop(ComponentPeer::StyleFlags::windowIsResizable, hwnd);
}

void EditorWindow::hideEditor() {
    removeFromDesktop();
}

void *EditorWindow::getWindowHandle() {
    ComponentPeer *peer = getPeer();
    if (peer) {
        return peer->getNativeHandle();
    } else {
        return NULL;
    }
}

//==============================================================================
void EditorWindow::paint(Graphics &g) {
    g.fillAll(Colour(0xff323e44));
}

void EditorWindow::resized() {
    static int width = 80;
    juce::Rectangle<int> b = getBounds();
    b.setX((b.getWidth() - width) / 2);
    b.setWidth(width);
    brightnessSlider->setBounds(b.removeFromLeft(width));
}

void EditorWindow::linkControllers(ParamManager &paramManager) {
    paramManager.bindControllerComponent(prmBrightness, brightnessSlider.get());
}
