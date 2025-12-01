#include <Arduino.h>
#include <iot_board.h>
#include "menu.h"

MenuClass * MenuClass::_instance = nullptr;

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
    display->println(CurrentMenu->name);
    for (size_t i = 0; i < CurrentMenu->children.size(); i++) {
        if (i == selectedItem) {
            display->print("> ");
        } else {
            display->print("- ");
        }
        display->println(CurrentMenu->children[i]->name);
    }
    if (CurrentMenu != Root) {
        if (selectedItem == CurrentMenu->children.size()) {
            display->print("> ");
        } else {
            display->print("- ");
        }
        display->println("Back");
    }
    display->display();
}
