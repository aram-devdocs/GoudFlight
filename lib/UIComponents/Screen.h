#ifndef SCREEN_H
#define SCREEN_H

#include <Arduino.h>
#include <DisplayManager.h>
#include <ButtonManager.h>

class Screen {
public:
    Screen(DisplayManager* displayManager, ButtonManager* buttonManager = nullptr)
        : m_displayManager(displayManager),
          m_buttonManager(buttonManager),
          m_needsRedraw(true),
          m_isActive(false) {
    }
    
    virtual ~Screen() = default;
    
    virtual void onEnter() {
        m_isActive = true;
        m_needsRedraw = true;
    }
    
    virtual void onExit() {
        m_isActive = false;
    }
    
    virtual void update() {
        if (m_needsRedraw) {
            draw();
            m_needsRedraw = false;
        }
        handleInput();
    }
    
    virtual void draw() = 0;
    
    virtual void handleInput() {
        if (!m_buttonManager) return;
        m_buttonManager->update();
    }
    
    void requestRedraw() { m_needsRedraw = true; }
    bool needsRedraw() const { return m_needsRedraw; }
    bool isActive() const { return m_isActive; }
    
    DisplayManager* getDisplayManager() { return m_displayManager; }
    const DisplayManager* getDisplayManager() const { return m_displayManager; }
    
    ButtonManager* getButtonManager() { return m_buttonManager; }
    const ButtonManager* getButtonManager() const { return m_buttonManager; }
    
protected:
    DisplayManager* m_displayManager;
    ButtonManager* m_buttonManager;
    bool m_needsRedraw;
    bool m_isActive;
};

#endif