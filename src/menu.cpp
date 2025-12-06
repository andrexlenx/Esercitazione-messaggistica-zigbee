#include <Arduino.h>
#include <iot_board.h>
#include "menu.h"

MenuClass * MenuClass::_instance = nullptr;

std::vector<String> splitTextToLines(String text, int width) {
    std::vector<String> lines;
    int len = text.length();
    if (len == 0) {
        lines.push_back("");
        return lines;
    }
    int start = 0;
    while (start < len) {
        int end = start + width;
        if (end > len) end = len;
        lines.push_back(text.substring(start, end));
        start = end;
    }
    return lines;
}

std::vector<String> getItemRenderLines(String name, bool selected, int width) {
    std::vector<String> renderLines;
    String prefix = selected ? "> " : "- ";
    
    int start = 0;
    int len = name.length();
    bool firstLine = true;
    
    if (len == 0) {
        renderLines.push_back(prefix);
        return renderLines;
    }

    while (start <= len) {
        int newlinePos = name.indexOf('\n', start);
        if (newlinePos == -1) newlinePos = len;
        
        String segment = name.substring(start, newlinePos);
        
        String fullLine;
        if (firstLine) {
            fullLine = prefix + segment;
            firstLine = false;
        } else {
            fullLine = segment;
        }
        
        std::vector<String> wrapped = splitTextToLines(fullLine, width);
        renderLines.insert(renderLines.end(), wrapped.begin(), wrapped.end());
        
        if (newlinePos == len) break;
        start = newlinePos + 1;
    }
    return renderLines;
}

void MenuClass::nextitem(){
    if (CurrentMenu->children.size() == 0) return;
    selectedItem = (selectedItem + 1) % (CurrentMenu->children.size() + 1);
    if (selectedItem == CurrentMenu->children.size() && CurrentMenu == Root) {
        selectedItem = 0;
    }
    Menu::render();
}

void MenuClass::selectItem(){    
    if (CurrentMenu->children.size() < selectedItem) return;
    if (CurrentMenu->children.size() == selectedItem) {
        if (CurrentMenu->parent != nullptr) {
            CurrentMenu = CurrentMenu->parent;
        }else {
            CurrentMenu = Root;
        }
        selectedItem = 0;
        Menu::render();
        return;
    }
    MenuItem* selectedMenuItem = CurrentMenu->children[selectedItem];
    selectedMenuItem->triggerAction();
    if (selectedMenuItem->children.size() > 0) {
        CurrentMenu = selectedMenuItem;
        selectedItem = 0;
    }
    Menu::render();
}

void btn_nextitem(uint8_t pinBtn){
    MenuClass::get_instance()->nextitem();
}
void btn_selectitem(uint8_t pinBtn){
    MenuClass::get_instance()->selectItem();
}

void MenuClass::init_menu(MenuItem* root) {
    IoTBoard::init_buttons();
    buttons->onBtn1Release(btn_selectitem);
    buttons->onBtn2Release(btn_nextitem);
    setRoot(root);
    setCurrentMenu(root);
    setSelectedItem(0);
}

void MenuClass::render() {
    display->clearDisplay();
    display->setCursor(menuxoffset, menuyoffset);
    
    String title = CurrentMenu->name;
    int newlinePos = title.indexOf('\n');
    if (newlinePos != -1) title = title.substring(0, newlinePos);
    if (title.length() > textablewidth) title = title.substring(0, textablewidth);
    display->println(title);

    int menulinecount = floor((textableheight * textheight - menuyoffset)/textableheight) - 1;

    // Generate all render lines
    std::vector<String> allLines;
    int selectedLineStart = -1;
    int selectedLineEnd = -1;
    int nextLineEnd = -1;
    
    size_t totalItems = CurrentMenu->children.size();
    bool hasBack = (CurrentMenu != Root);
    size_t loopCount = hasBack ? totalItems + 1 : totalItems;
    
    for (size_t i = 0; i < loopCount; i++) {
        String name;
        bool isSelected = (i == selectedItem);
        
        if (i < totalItems) {
            name = CurrentMenu->children[i]->name;
        } else {
            name = "Back";
        }
        
        std::vector<String> lines = getItemRenderLines(name, isSelected, textablewidth);
        
        if (isSelected) selectedLineStart = allLines.size();
        
        allLines.insert(allLines.end(), lines.begin(), lines.end());
        
        if (isSelected) selectedLineEnd = allLines.size();
        if (i == selectedItem + 1) nextLineEnd = allLines.size();
    }
    
    // Calculate window
    int windowStart = 0;
    int targetEnd = -1;
    
    if (selectedItem < loopCount - 1) {
        targetEnd = nextLineEnd;
    } else {
        targetEnd = selectedLineEnd;
    }
    
    if (targetEnd == -1) targetEnd = selectedLineEnd;
    
    int idealStart = targetEnd - menulinecount;
    windowStart = idealStart;
    
    if (windowStart > selectedLineStart) {
        windowStart = selectedLineStart;
    }
    
    if (windowStart < 0) windowStart = 0;
    
    // Render visible lines
    for (int i = 0; i < menulinecount; i++) {
        int idx = windowStart + i;
        if (idx < (int)allLines.size()) {
            display->println(allLines[idx]);
        }
    }
    
    display->display();
}
