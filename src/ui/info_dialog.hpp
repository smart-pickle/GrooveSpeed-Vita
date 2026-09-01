#ifndef INFO_DIALOG_HPP
#define INFO_DIALOG_HPP

#include "imgui.h"

class InfoDialog {
public:
    InfoDialog();

    void open() { m_isOpen = true; m_shouldOpenPopup = true; }
    void close() { m_isOpen = false; }
    bool isOpen() const { return m_isOpen; }

    void render();

private:
    bool m_isOpen = false;
    bool m_shouldOpenPopup = false;
};

#endif // INFO_DIALOG_HPP
