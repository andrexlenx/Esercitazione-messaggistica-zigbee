#pragma once

#include <Arduino.h>
#include <iot_board.h>
#include <vector>


#define menuheight 64
#define menuwidth 128
#define textableheight 8
#define textheight 8
#define textablewidth 21
#define textwidth 6

class MenuItem {
    public:
    MenuItem* parent = nullptr;
    std::vector<MenuItem*> children;
    String name;
    void* actionargs = nullptr;
    void (*action)(MenuItem* self, void* args) = nullptr;

    MenuItem(String item_name, MenuItem* parent_item = nullptr) {
        name = item_name;
        parent = parent_item;
        if (parent != nullptr) {
            parent->children.push_back(this);
        }
    }

    void setAction(void (*action_func)(MenuItem* self, void* args), void* args = nullptr) {
        action = action_func;
        actionargs = args;
    }

    void triggerAction() {
        if (action != nullptr) {
            action(this, actionargs);
        }
    }
};

class MenuClass {
    public:
    void init_menu(MenuItem* root);
    void render();
    void nextitem();
    void selectItem();
    int16_t menuyoffset = 0;
    int16_t menuxoffset = 0;

    static MenuClass* get_instance(){
        if(_instance == nullptr){
            _instance = new MenuClass();
        }
        return _instance;
    };

    MenuItem* getRoot() { return Root; }
    MenuItem* getCurrentMenu() { return CurrentMenu; }
    size_t getSelectedItem() { return selectedItem; }
    
    void setRoot(MenuItem* root) { Root = root; }
    void setCurrentMenu(MenuItem* menu) { CurrentMenu = menu; }
    void setSelectedItem(size_t index) { selectedItem = index; }

    private:
    MenuItem* Root;
    MenuItem* CurrentMenu;
    size_t selectedItem;
    static MenuClass* _instance;
};

namespace Menu {
    inline void init_menu(MenuItem* root) {MenuClass::get_instance()->init_menu(root);}
    inline void render() {MenuClass::get_instance()->render();}
    inline void setRoot(MenuItem* root) {MenuClass::get_instance()->setRoot(root);}
    inline void setCurrentMenu(MenuItem* menu) {MenuClass::get_instance()->setCurrentMenu(menu);}
    inline void setSelectedItem(size_t index) {MenuClass::get_instance()->setSelectedItem(index);}
    inline MenuItem* getRoot() {return MenuClass::get_instance()->getRoot();}
    inline MenuItem* getCurrentMenu() {return MenuClass::get_instance()->getCurrentMenu();}
    inline size_t getSelectedItem() {return MenuClass::get_instance()->getSelectedItem();}
    inline void nextitem() {MenuClass::get_instance()->nextitem();}
    inline void selectItem() {MenuClass::get_instance()->selectItem();}
}